/*
 *
 *
 * Copyright (C) 2008, 2010, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "vgreduce.h"

#include "masterlist.h"
#include "misc.h"
#include "physvol.h"
#include "pvgroupbox.h"
#include "volgroup.h"

#include <lvm2app.h>

#include <KIcon>
#include <KLocale>
#include <KMessageBox>

#include <QDebug>
#include <QLabel>
#include <QStackedWidget>
#include <QVBoxLayout>




// Remove one selected pv from group

VGReduceDialog::VGReduceDialog(PhysVol *const pv, QWidget *parent) : 
    KvpmDialog(parent), 
    m_pv(pv),
    m_vg(pv->getVg())
{
    setCaption(i18n("Remove Physical Volume"));
    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    QLabel *const label = new QLabel(i18n("Remove physical volume: <b>%1?</b>", m_pv->getName()));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    layout->addSpacing(5);

    QList<QSharedPointer<PvSpace> > pv_space_list = getPvSpaceList();

    if (pv->getSize() != pv->getRemaining()) {
        preventExec();
        KMessageBox::sorry(nullptr, i18n("This physical volume is still in use and cannot be removed"));
    } else if (pv_space_list.size() == 1 && !hasUnremovablePv()) {
        preventExec();
        KMessageBox::sorry(nullptr, i18n("There is only one physical volume in this group, try deleting the group instead"));
    } else if (!hasMda(QStringList( m_pv->getName() ))) {
        preventExec();
        KMessageBox::sorry(nullptr, i18n("Physical volume \'%1\' " 
                                         "contains the only usable metadata area for this volume group "
                                         "and cannot be removed.", 
                                         m_pv->getName()));
    }

    setButtons(KDialog::Yes | KDialog::No);
}

// Remove one or more pvs from selected group

VGReduceDialog::VGReduceDialog(VolGroup *const group, QWidget *parent) : 
    KvpmDialog(parent), 
    m_vg(group)
{
    setCaption(i18n("Reduce Volume Group"));
    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout;

    const QString vg_name = m_vg->getName();

    QList<QSharedPointer<PvSpace> > pv_space_list = getPvSpaceList();

    if (pv_space_list.isEmpty()){
        preventExec();
        KMessageBox::sorry(nullptr, i18n("There are no physical volumes that can be removed"));
    } else if (pv_space_list.size() == 1 && !hasUnremovablePv()) {
        preventExec();
        KMessageBox::sorry(nullptr, i18n("There is only one physical volume in this group, try deleting the group instead."));
    } else if (pv_space_list.size() == 1 && !hasMda(QStringList(pv_space_list[0]->pv->getName()))) {
        preventExec();
        KMessageBox::sorry(nullptr, i18n("The only physical volume with no logical volumes on it is \'%1.\' " 
                                         "It contains the only usable metadata area for this volume group "
                                         "and cannot be removed.", 
                                         pv_space_list[0]->pv->getName()));
    } else {
        QLabel *label;

        label = new QLabel(i18n("Reduce volume group: <b>%1</b>", m_vg->getName()));
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
        layout->addSpacing(10);

        if (pv_space_list.size() == 1) {
            label = new QLabel(i18n("This is the only physical volume that can be removed"));
        } else if (hasUnremovablePv()) {
            label = new QLabel(i18n("Select physical volumes to remove them from the volume group"));
        } else {
            label = new QLabel(i18n("Select physical volumes, excluding one, to "
                                    "remove them from the volume group"));
        }
        
        label->setWordWrap(true);
        QHBoxLayout *const label_layout = new QHBoxLayout;
        QWidget *const label_widget = new QWidget;
        label_layout->addWidget(label);
        label_widget->setLayout(label_layout);
        layout->addWidget(label_widget);

        m_pv_checkbox = new PvGroupBox(pv_space_list, NO_POLICY, NO_POLICY);
        m_pv_checkbox->setTitle(i18n("Unused physical volumes"));
        layout->addWidget(m_pv_checkbox);
        m_error_stack = createErrorWidget();
        layout->addWidget(m_error_stack);
        m_error_stack->setCurrentIndex(0);

        connect(m_pv_checkbox, SIGNAL(stateChanged()), this, SLOT(resetOkButton()));
        m_pv_checkbox->selectNone();
    }

    dialog_body->setLayout(layout);
}

void VGReduceDialog::commit()
{
    const QByteArray vg_name = m_vg->getName().toLocal8Bit();
    vg_t vg_dm = nullptr;
    lvm_t lvm = MasterList::getLvm();

    QStringList pv_list;
    if (m_pv)
        pv_list << m_pv->getName();
    else if (m_pv_checkbox)
        pv_list << m_pv_checkbox->getNames(); // pvs to remove by name
 
    if ((vg_dm = lvm_vg_open(lvm, vg_name.data(), "w", 0))) {
        for (auto name : pv_list) {
            const QByteArray name_ba = name.toLocal8Bit();
            
            if (lvm_vg_reduce(vg_dm, name_ba.data()))
                KMessageBox::error(nullptr, QString(lvm_errmsg(lvm)));
        }
        
        if (lvm_vg_write(vg_dm))
            KMessageBox::error(nullptr, QString(lvm_errmsg(lvm)));
        
        lvm_vg_close(vg_dm);
    } else {
        KMessageBox::error(nullptr, QString(lvm_errmsg(lvm)));
    }
    
    return;
}

void VGReduceDialog::resetOkButton()
{
    const QStringList names = m_pv_checkbox->getNames();
    const int boxes_checked = names.size();
    const int boxes_count   = m_pv_checkbox->getAllNames().size();

    if ((boxes_checked > 0) && (hasUnremovablePv() || (boxes_checked < boxes_count))) {
        if (hasMda(names)) { 
            enableButtonOk(true);
            m_error_stack->setCurrentIndex(0);
        } else {
            enableButtonOk(false);
            m_error_stack->setCurrentIndex(1);  // no remaining MDA
        } 
    } else {
        if (boxes_checked > 0) {
            enableButtonOk(false);
            m_error_stack->setCurrentIndex(2); // no remaining pv
        } else {
            enableButtonOk(false);
            m_error_stack->setCurrentIndex(0); // nothing selected
        }
    }
}

// Checks to see if at least one pv not being removed has
// a metadata area on it. A vg must always have at least one.
bool VGReduceDialog::hasMda(const QStringList remove)
{
    QStringList mda_names;

    for (auto pv : m_vg->getPhysicalVolumes()) {
        if (pv->getMdaUsed())
            mda_names << pv->getName();
    }

    for (int x = mda_names.size() - 1; x >= 0 ; --x) {
        for (auto name : remove) {
            if (name == mda_names[x]) {
                mda_names.removeAt(x);
                break;
            }
        }
    }

    return !mda_names.isEmpty();
}

QStackedWidget *VGReduceDialog::createErrorWidget()
{
    QStackedWidget *const stack = new QStackedWidget();
    stack->addWidget(new QWidget);

    QLabel *icon_label;
    QLabel *error_label;

    QWidget *const error_widget1 = new QWidget();
    stack->addWidget(error_widget1);
    QHBoxLayout *const layout1 = new QHBoxLayout();
    icon_label = new QLabel("");
    icon_label->setPixmap(KIcon("dialog-warning").pixmap(32, 32));
    error_label = new QLabel(i18n("Cannot remove all the physical volumes with usable metadata areas"));
    error_label->setWordWrap(true);
    layout1->addWidget(icon_label);
    layout1->addWidget(error_label);
    layout1->addStretch();
    error_widget1->setLayout(layout1);

    QWidget *const error_widget2 = new QWidget();
    stack->addWidget(error_widget2);
    QHBoxLayout *const layout2 = new QHBoxLayout();
    icon_label = new QLabel("");
    icon_label->setPixmap(KIcon("dialog-warning").pixmap(32, 32));
    error_label = new QLabel(i18n("A volume group must always have at least " 
                                  "one physical volume in it"));
    error_label->setWordWrap(true);
    layout2->addWidget(icon_label);
    layout2->addWidget(error_label);
    layout2->addStretch();
    error_widget2->setLayout(layout2);

    return stack;
}

QList<QSharedPointer<PvSpace> > VGReduceDialog::getPvSpaceList() 
{
    QList<QSharedPointer<PvSpace> > list;

    for (auto pv : m_vg->getPhysicalVolumes()) {
        if (pv->getSize() == pv->getRemaining())
            list << QSharedPointer<PvSpace>(new PvSpace(pv, pv->getSize(), pv->getSize()));
    }

    return list;
}

bool VGReduceDialog::hasUnremovablePv()
{
    bool unremovable = false;

    for (auto pv : m_vg->getPhysicalVolumes()) {
        if (pv->getSize() != pv->getRemaining()) {    // only unused pvs can be removed
            unremovable = true;
            break;
        }
    }

    return unremovable;
}
