/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "vgcreate.h"

#include "masterlist.h"
#include "progressbox.h"
#include "pvgroupbox.h"
#include "storagebase.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "topwindow.h"

#include <lvm2app.h>

#include <KApplication>
#include <KComboBox>
#include <KLineEdit>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>

#include <QCheckBox>
#include <QEventLoop>
#include <QLabel>
#include <QRegExpValidator>
#include <QVBoxLayout>


VGCreateDialog::VGCreateDialog(QWidget *parent) : 
    KvpmDialog(parent)
{
    QList<const StorageBase *> devices;
    devices = getUsablePvs();

    if (devices.size() > 0) {
        if (continueWarning())
            buildDialog(devices);
        else
            preventExec();
    } else {
        preventExec();
        KMessageBox::sorry(nullptr, i18n("No unused potential physical volumes found"));
    }
}

VGCreateDialog::VGCreateDialog(StorageBase *const device, QWidget *parent) : 
    KvpmDialog(parent)
{
    QList<const StorageBase *> devices;
    devices.append(device);
    
    if (continueWarning())
        buildDialog(devices);
    else
        preventExec();
}

bool VGCreateDialog::continueWarning()
{
    const QString warning = i18n("If a device or partition is added to a volume group, "
                                 "any data currently on that device or partition will be lost.");

    return (KMessageBox::warningContinueCancel(nullptr,
                                               warning,
                                               QString(),
                                               KStandardGuiItem::cont(),
                                               KStandardGuiItem::cancel(),
                                               QString(),
                                               KMessageBox::Dangerous) == KMessageBox::Continue);
}

void VGCreateDialog::extentSizeChanged()
{
    limitExtentSize(m_extent_suffix->currentIndex());

    long long new_extent_size = m_extent_size->currentText().toLongLong();

    new_extent_size *= 1024;
    if (m_extent_suffix->currentIndex() > 0)
        new_extent_size *= 1024;
    if (m_extent_suffix->currentIndex() > 1)
        new_extent_size *= 1024;

    m_pv_checkbox->setExtentSize(new_extent_size);
}

void VGCreateDialog::limitExtentSize(int index)
{
    int extent_index;

    if (index > 1) {  // Gigabytes selected as suffix, more than 2Gib forbidden
        if (m_extent_size->currentIndex() > 2)
            m_extent_size->setCurrentIndex(0);
        m_extent_size->setMaxCount(2);
    } else {
        extent_index = m_extent_size->currentIndex();
        m_extent_size->setMaxCount(10);
        m_extent_size->setInsertPolicy(QComboBox::InsertAtBottom);
        m_extent_size->insertItem(2, "4");
        m_extent_size->insertItem(3, "8");
        m_extent_size->insertItem(4, "16");
        m_extent_size->insertItem(5, "32");
        m_extent_size->insertItem(6, "64");
        m_extent_size->insertItem(7, "128");
        m_extent_size->insertItem(8, "256");
        m_extent_size->insertItem(9, "512");
        m_extent_size->setInsertPolicy(QComboBox::NoInsert);
        m_extent_size->setCurrentIndex(extent_index);
    }
}

void VGCreateDialog::commit()
{
    lvm_t lvm = MasterList::getLvm();
    vg_t vg_dm;
    uint32_t new_extent_size = m_extent_size->currentText().toULong();
    const QStringList pv_names = m_pv_checkbox->getNames();
    const QByteArray vg_name_array = m_vg_name->text().toLocal8Bit();
    ProgressBox *const progress_box = TopWindow::getProgressBox();
    QByteArray pv_name_qba;

    hide();

    new_extent_size *= 1024;
    if (m_extent_suffix->currentIndex() > 0)
        new_extent_size *= 1024;
    if (m_extent_suffix->currentIndex() > 1)
        new_extent_size *= 1024;

    progress_box->setRange(0, pv_names.size());
    progress_box->setText(i18n("Creating Group"));
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    if ((vg_dm = lvm_vg_create(lvm, vg_name_array.data()))) {

        if ((lvm_vg_set_extent_size(vg_dm, new_extent_size)))
            KMessageBox::error(nullptr, QString(lvm_errmsg(lvm)));

        for (int x = 0; x < pv_names.size(); x++) {
            progress_box->setValue(x);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            pv_name_qba = pv_names[x].toLocal8Bit();
            if (lvm_vg_extend(vg_dm, pv_name_qba.data()))
                KMessageBox::error(nullptr, QString(lvm_errmsg(lvm)));
        }

        // ****To Do... None of the following are supported by liblvm2app yet****
        //   if(m_clustered->isChecked())
        //   if(m_auto_backup->isChecked())
        //   if((m_max_lvs_check->isChecked()) && (m_max_lvs->text() != ""))
        //   if((m_max_pvs_check->isChecked()) && (m_max_pvs->text() != ""))


        if (lvm_vg_write(vg_dm))
            KMessageBox::error(nullptr, QString(lvm_errmsg(lvm)));

        lvm_vg_close(vg_dm);
        progress_box->reset();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        return;
    }

    KMessageBox::error(nullptr, QString(lvm_errmsg(lvm)));
    progress_box->reset();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    return;
}

/* The allowed characters in the name are letters, numbers, periods
   hyphens and underscores. Also, the names ".", ".." and names starting
   with a hyphen are disallowed. Finally disable the OK button if no
   pvs are checked  */

void VGCreateDialog::validateOK()
{
    QString name = m_vg_name->text();
    int pos = 0;
    long long space = m_pv_checkbox->getRemainingSpace();

    enableButtonOk(false);

    if (m_validator->validate(name, pos) == QValidator::Acceptable && name != "." && name != "..") {
        if (space)
            enableButtonOk(true);
    }
}

void VGCreateDialog::buildDialog(QList<const StorageBase *> devices)
{
    setCaption(i18n("Create New Volume Group"));

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *const title = new QLabel(i18n("Create a new volume group"));
    title->setAlignment(Qt::AlignCenter);
    layout->addSpacing(5);
    layout->addWidget(title);
    layout->addSpacing(10);

    QLabel *const name_label = new QLabel(i18n("Group name: "));
    m_vg_name = new KLineEdit();
    name_label->setBuddy(m_vg_name);

    QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
    m_validator = new QRegExpValidator(rx, m_vg_name);
    m_vg_name->setValidator(m_validator);
    QHBoxLayout *const name_layout = new QHBoxLayout();
    name_layout->addWidget(name_label);
    name_layout->addWidget(m_vg_name);

    QLabel *const extent_label = new QLabel(i18n("Physical extent size: "));
    m_extent_size = new KComboBox();
    extent_label->setBuddy(m_extent_size);
    m_extent_size->insertItem(0, "1");
    m_extent_size->insertItem(1, "2");
    m_extent_size->insertItem(2, "4");
    m_extent_size->insertItem(3, "8");
    m_extent_size->insertItem(4, "16");
    m_extent_size->insertItem(5, "32");
    m_extent_size->insertItem(6, "64");
    m_extent_size->insertItem(7, "128");
    m_extent_size->insertItem(8, "256");
    m_extent_size->insertItem(9, "512");
    m_extent_size->setInsertPolicy(QComboBox::NoInsert);
    m_extent_size->setCurrentIndex(2);
    m_extent_suffix = new KComboBox();
    m_extent_suffix->insertItem(0, "KiB");
    m_extent_suffix->insertItem(1, "MiB");
    m_extent_suffix->insertItem(2, "GiB");
    m_extent_suffix->setInsertPolicy(QComboBox::NoInsert);
    m_extent_suffix->setCurrentIndex(1);

    m_pv_checkbox = new PvGroupBox(devices, 0x400000);  // 4 MiB default extent size
    layout->addWidget(m_pv_checkbox);

    connect(m_pv_checkbox, SIGNAL(stateChanged()),
            this, SLOT(validateOK()));

    QHBoxLayout *extent_layout = new QHBoxLayout();
    extent_layout->addWidget(extent_label);
    extent_layout->addWidget(m_extent_size);
    extent_layout->addWidget(m_extent_suffix);
    extent_layout->addStretch();

    /*  liblvm does not support setting limits on lvs and pvs yet (will it ever?)

    QGroupBox *lv_box = new QGroupBox( i18n("Number of Logical Volumes") );
    QVBoxLayout *lv_layout_v = new QVBoxLayout();
    QHBoxLayout *lv_layout_h = new QHBoxLayout();
    lv_box->setLayout(lv_layout_v);
    m_max_lvs_check = new QCheckBox( i18n("No Limit") );
    m_max_lvs_check->setCheckState(Qt::Checked);
    lv_layout_v->addWidget(m_max_lvs_check);
    lv_layout_v->addLayout(lv_layout_h);
    QLabel *lv_label = new QLabel( i18n("Maximum: ") );
    m_max_lvs = new KLineEdit();
    QIntValidator *lv_validator = new QIntValidator(1,255,this);
    m_max_lvs->setValidator(lv_validator);
    m_max_lvs->setEnabled(false);
    lv_layout_h->addWidget(lv_label);
    lv_layout_h->addWidget(m_max_lvs);

    QGroupBox *pv_box = new QGroupBox( i18n("Number of Physical Volumes") );
    QVBoxLayout *pv_layout_v = new QVBoxLayout();
    QHBoxLayout *pv_layout_h = new QHBoxLayout();
    pv_box->setLayout(pv_layout_v);
    m_max_pvs_check = new QCheckBox( i18n("No Limit") );
    m_max_pvs_check->setCheckState(Qt::Checked);
    pv_layout_v->addWidget(m_max_pvs_check);
    pv_layout_v->addLayout(pv_layout_h);
    QLabel *pv_label = new QLabel( i18n("Maximum: ") );
    m_max_pvs = new KLineEdit();
    QIntValidator *pv_validator = new QIntValidator(1,255,this);
    m_max_pvs->setValidator(pv_validator);
    m_max_pvs->setEnabled(false);
    pv_layout_h->addWidget(pv_label);
    pv_layout_h->addWidget(m_max_pvs);

    */

    m_clustered = new QCheckBox(i18n("Cluster Aware"));
    m_clustered->setEnabled(false);

    m_auto_backup = new QCheckBox(i18n("Automatic Backup"));
    m_auto_backup->setCheckState(Qt::Checked);
    m_auto_backup->setEnabled(false);

    layout->addLayout(name_layout);
    layout->addLayout(extent_layout);
    //    layout->addWidget(lv_box);
    //    layout->addWidget(pv_box);
    layout->addWidget(m_clustered);
    layout->addWidget(m_auto_backup);

    enableButtonOk(false);

    connect(m_vg_name, SIGNAL(textChanged(QString)),
            this, SLOT(validateOK()));
    /*
    connect(m_max_lvs_check, SIGNAL(stateChanged(int)),
        this, SLOT(limitLogicalVolumes(int)));

    connect(m_max_pvs_check, SIGNAL(stateChanged(int)),
        this, SLOT(limitPhysicalVolumes(int)));
    */

    connect(m_extent_size, SIGNAL(activated(int)),
            this, SLOT(extentSizeChanged()));

    connect(m_extent_suffix, SIGNAL(activated(int)),
            this, SLOT(extentSizeChanged()));
}

/*
void VGCreateDialog::limitLogicalVolumes(int boxstate)
{
    if(boxstate == Qt::Unchecked)
    m_max_lvs->setEnabled(true);
    else
    m_max_lvs->setEnabled(false);
}

void VGCreateDialog::limitPhysicalVolumes(int boxstate)
{
    if(boxstate == Qt::Unchecked)
    m_max_pvs->setEnabled(true);
    else
    m_max_pvs->setEnabled(false);
}
*/
