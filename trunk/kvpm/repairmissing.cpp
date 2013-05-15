/*
 *
 *
 * Copyright (C) 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "repairmissing.h"

#include "physvol.h"
#include "pvgroupbox.h"
#include "processprogress.h"
#include "volgroup.h"

#include <KApplication>
#include <KLocale>
#include <KMessageBox>

#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>



RepairMissingDialog::RepairMissingDialog(LogVol *const volume, QWidget *parent):
    KDialog(parent),
    m_lv(volume)
{
    if (m_lv->getParentMirror() != NULL)
        m_lv = m_lv->getParentMirror();
    LogVolList children;
    const QString lv_name = m_lv->getName();
    const bool is_raid = m_lv->isRaid();
    const bool is_lvm = m_lv->isLvmMirror();
    m_bailout = false;

    QList<PhysVol *> const pvs = getUsablePvs();

    if (!m_lv->isPartial()){
        m_bailout = true;
        KMessageBox::error(NULL, i18n("This volume has no missing physical volumes"));
    } else if (pvs.isEmpty() && is_raid && !m_lv->isMirror()){
        m_bailout = true;
        KMessageBox::error(NULL, i18n("No suitable physical volumes found"));
    } else {
        QVBoxLayout *const layout = new QVBoxLayout();
        QWidget *const main_widget = new QWidget();        
        QLabel  *const lv_name_label = new QLabel();
        
        if(is_lvm) {
            lv_name_label->setText(i18n("<b>Repair mirror: %1</b>", m_lv->getName()));
            setWindowTitle(i18n("Repair Mirror"));
        } else if(is_raid) {
            lv_name_label->setText(i18n("<b>Repair RAID device: %1</b>", m_lv->getName()));
            setWindowTitle(i18n("Repair RAID device"));
        }

        lv_name_label->setAlignment(Qt::AlignCenter);
 
        layout->addWidget(lv_name_label);
        layout->addSpacing(5);

        m_replace_box = new QCheckBox(i18n("Replace missing physical volumes"));
        QWidget *const physical = buildPhysicalWidget(pvs);
        if (pvs.isEmpty()) {
            KMessageBox::information(NULL, i18n("Replacement will be disabled: No suitable physical volumes found"));
            lv_name_label->setText(i18n("<b>Remove missing physcial volumes from mirror: %1?</b>", m_lv->getName()));
            m_replace_box->setChecked(false);
            m_replace_box->setEnabled(false);
            m_replace_box->hide();
            physical->hide();

            setButtons(KDialog::Yes | KDialog::No);
            setDefaultButton(KDialog::No);

            connect(this, SIGNAL(yesClicked()),
                    this, SLOT(commitChanges()));
        } else {
            m_replace_box->setChecked(true);

            if (is_raid && !m_lv->isMirror())
                m_replace_box->setEnabled(false);

            connect(m_replace_box, SIGNAL(toggled(bool)),
                    this, SLOT(setReplace(bool)));

            connect(this, SIGNAL(okClicked()),
                    this, SLOT(commitChanges()));

            resetOkButton();
        }
        layout->addWidget(m_replace_box);

        layout->addSpacing(5);
        layout->addWidget(physical);
        main_widget->setLayout(layout);
        setMainWidget(main_widget);
    }
}

/* The next function returns a list of physical volumes not already in
   use by unbroken parts of the volume since they should not be reused */

QList<PhysVol *> RepairMissingDialog::getUsablePvs()
{
    LogVolList  const images = m_lv->getAllChildrenFlat();
    QStringList pvs_in_use;

    
    // Currently, available pvs on the broken legs can't be used for repair!
    // If that gets fixed we will need to calculate the free space on the
    // pvs *including what the broken image is using* since that will go away
    // and be replaced the the new image. The following commented code won't be
    // enough.
    
    /* if (((images[x]->isRaidImage() || images[x]->isMetadata())) && !images[x]->isPartial())
       pvs_in_use << images[x]->getPvNamesAll();  */



    if (m_lv->isMirror()) {
        for (int x = images.size() - 1; x >= 0; x--) {
            if (((images[x]->isMirrorLeg() || images[x]->isLvmMirrorLog())))
                pvs_in_use << images[x]->getPvNamesAll();
        }
    } else if (m_lv->isRaid()) {
        for (int x = images.size() - 1; x >= 0; x--) {
            if (((images[x]->isRaidImage() || images[x]->isMetadata())))
                pvs_in_use << images[x]->getPvNamesAll();
        }
    } else {
        pvs_in_use << m_lv->getPvNamesAll();
    }

    QList<PhysVol *> unused_pvs = m_lv->getVg()->getPhysicalVolumes();

    for (int x = unused_pvs.size() - 1; x >= 0; x--) {
        if (!unused_pvs[x]->isAllocatable() || unused_pvs[x]->getRemaining() <= 0 || unused_pvs[x]->isMissing()) {
            unused_pvs.removeAt(x);
        } else {
            for (int y = pvs_in_use.size() - 1; y >= 0; y--) {
                if (unused_pvs[x]->getName() == pvs_in_use[y]) {
                    unused_pvs.removeAt(x);
                    break;
                }
            }
        }
    }

    return unused_pvs;
}

/* Here we create a string based on all
   the options that the user chose in the
   dialog and feed that to "lvconvert" */

QStringList RepairMissingDialog::arguments()
{
    QStringList args;

    args << "lvconvert" << "--repair" << "-y";  // -y: answer all prompts with "yes"

    if (m_pv_box->getPolicy() <= ANYWHERE) // don't pass INHERITED_*
        args << "--alloc" << policyToString(m_pv_box->getEffectivePolicy());

    if (!m_replace_box->isChecked())
        args << "-f" << m_lv->getFullName();  // no replace
    else      
        args << m_lv->getFullName() << m_pv_box->getNames();

    return args;
}

void RepairMissingDialog::commitChanges()
{
    hide();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    ProcessProgress repair_missing(arguments());
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

bool RepairMissingDialog::bailout()
{
    return m_bailout;
}

QWidget *RepairMissingDialog::buildPhysicalWidget(QList<PhysVol *> const pvs)
{
    QWidget *const physical = new QWidget;
    QVBoxLayout *const physical_layout = new QVBoxLayout();

    QList<QSharedPointer<PvSpace>> pv_space_list;

    for (auto pv : pvs) {
        pv_space_list << QSharedPointer<PvSpace>(new PvSpace(pv, pv->getRemaining(), pv->getContiguous()));
    }

    m_pv_box = new PvGroupBox(pv_space_list, m_lv->getPolicy(), m_lv->getVg()->getPolicy());
    physical_layout->addWidget(m_pv_box);
    physical_layout->addStretch();

    QHBoxLayout *const h_layout = new QHBoxLayout();
    QVBoxLayout *const lower_layout = new QVBoxLayout();
    physical_layout->addLayout(h_layout);
    h_layout->addStretch();
    h_layout->addLayout(lower_layout);
    h_layout->addStretch();

    physical->setLayout(physical_layout);

    connect(m_pv_box, SIGNAL(stateChanged()),
            this, SLOT(resetOkButton()));

    return physical;
}

int RepairMissingDialog::getImageNumber(QString name)
{
    int number = -1;
    bool ok = true;

    if (name.contains("_rmeta_") || (name.contains("_rimage_"))) {
        
        if (name.contains("_rmeta_"))
            name = name.remove(0, name.indexOf("_rmeta_") + 7);
        else
            name = name.remove(0, name.indexOf("_rimage_") + 8);
        
        number = name.toInt(&ok);

        if (!ok)
            number = -1;
    }
        
    return number;
}

void RepairMissingDialog::setReplace(const bool replace)
{
    m_pv_box->selectNone();
    m_pv_box->setEnabled(replace);
}

QList<PhysVol *> RepairMissingDialog::getSelectedPvs()
{
    QList<PhysVol *> pvs;

    QStringList const pvnames = m_pv_box->getNames();
    VolGroup *const vg = m_lv->getVg();

    for (int x = pvnames.size() - 1; x >= 0; x--) {
        PhysVol *const pv = vg->getPvByName(pvnames[x]);
        if (pv != NULL)
            pvs.append(pv);
    }

    return pvs;
}

LogVolList RepairMissingDialog::getPartialLvs()
{
    LogVolList partial = m_lv->getAllChildrenFlat();

    for (int x = partial.size() - 1; x >= 0; x--) {
        if (!partial[x]->isPartial())
            partial.removeAt(x);
    }

    return partial;
}

/* Enable or disable the OK button based on having
   enough physical volumes checked. */

void RepairMissingDialog::resetOkButton()
{
    if (!m_replace_box->isChecked()) {
        enableButtonOk(true);
        return;
    }

    LogVolList partial_lvs = getPartialLvs();


      /* RETEST THIS WHEN NEW LVM VERSION IS INSTALLED

      As of: lvs --version
                LVM version:     2.02.98(2) (2012-10-15)
                Library version: 1.02.77 (2012-10-15)
                Driver version:  4.23.0

                The allocation policy of lvconvert isn't using contiguous the same
                way as lvcreate and is probably being ignored.
                Anywhere policy not tested yet but should be impemented when contiguous is.  */


    /*  AllocationPolicy policy = m_pv_box->getPolicy();

      if (policy == INHERITED)
         policy = m_lv->getVg()->getPolicy();   */


    QList <long long> missing_bytes;
    QList <long long> missing_log_bytes;

    const bool is_lvm  = m_lv->isLvmMirror();

    while (partial_lvs.size()) {
        LogVol *lv = partial_lvs.takeAt(0);
        long long space = lv->getSize();

        if (is_lvm) {                  // We assume logs and mimages can be on the same pvs
            if (!lv->isTemporary()) {
                if (lv->isLvmMirrorLog() && !lv->isLvmMirror())
                    missing_log_bytes.append(space);
                else if (lv->isLvmMirrorLeg())
                    missing_bytes.append(space);
            }
        } else {  // we try to put the metadata and corresponding the image/leg segments together here

            const int num = getImageNumber(lv->getName());
            if (num >= 0) {
                for (int x = partial_lvs.size() - 1; x >= 0; x--) {
                    if (num == getImageNumber(partial_lvs[x]->getName()))
                        space += partial_lvs.takeAt(x)->getSize();
                }
            }

            missing_bytes.append(space);
        }
    }

    if (is_lvm) {
        for (int x = missing_log_bytes.size() - 1; x >= 0; x--) {
            if (missing_bytes.size() > x)
                missing_bytes[x] += missing_log_bytes[x];
            else
                missing_bytes.append(missing_log_bytes[x]);
        }
    }

    qSort(missing_bytes);
    for (int n = missing_bytes.size() - 1; n >= 0; n--) {
        if (missing_bytes[n] <= 0)
            missing_bytes.removeAt(n);
    }

    QList <long long> available_bytes;
    QList<PhysVol *> const pvs = getSelectedPvs();
    for (int x = pvs.size() - 1; x >= 0; x--) {
        available_bytes.append(pvs[x]->getRemaining());
    }
    qSort(available_bytes);

    if (missing_bytes.size()) {
        while (available_bytes.size()) {
            qSort(available_bytes);
            qSort(missing_bytes.begin(), missing_bytes.end(), qGreater<long long>());
            missing_bytes[0] -= available_bytes.takeLast();
        }
        qSort(missing_bytes.begin(), missing_bytes.end(), qGreater<long long>());

        if (missing_bytes[0] > 0)
            enableButtonOk(false);
        else
            enableButtonOk(true);
    } else {
        enableButtonOk(true);
    }

    return;
}

