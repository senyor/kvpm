/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "vgextend.h"

#include "masterlist.h"
#include "misc.h"
#include "progressbox.h"
#include "pvgroupbox.h"
#include "storagebase.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "topwindow.h"
#include "vgcreate.h"
#include "volgroup.h"

#include <lvm2app.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QApplication>
#include <QCheckBox>
#include <QEventLoop>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>


VGExtendDialog::VGExtendDialog(const VolGroup *const group, QWidget *parent) :
    KvpmDialog(parent),
    m_vg(group)
{
    const QList<const StorageBase *> devices(getUsablePvs());

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

VGExtendDialog::VGExtendDialog(const VolGroup *const group, const StorageBase *const device, QWidget *parent) :
    KvpmDialog(parent), 
    m_vg(group)
{
    QList<const StorageBase *> devices;
    devices << device;

    if (continueWarning())
        buildDialog(devices);
    else
        preventExec();
}

bool VGExtendDialog::continueWarning()
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

void VGExtendDialog::commit()
{
    const QByteArray vg_name   = m_vg->getName().toLocal8Bit();
    const QStringList pv_names = m_pv_checkbox->getNames();

    lvm_t lvm = MasterList::getLvm();
    vg_t  vg_dm;

    ProgressBox *const progress_box = TopWindow::getProgressBox();
    progress_box->setRange(0, pv_names.size());
    progress_box->setText("Extending VG");

    hide();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    if ((vg_dm = lvm_vg_open(lvm, vg_name.data(), "w", 0))) {

        for (int i = 0; i < pv_names.size(); ++i) {
            progress_box->setValue(i);
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            QByteArray name = pv_names[i].toLocal8Bit();

            pv_create_params_t params = lvm_pv_params_create(lvm, name.data());
            lvm_property_value_t value;
            value.is_settable = 1;
            value.is_string = 0;
            value.is_integer = 1;
            value.is_valid = 1;
            value.is_signed = 0;
            value.value.integer = m_copies_combo->currentIndex();
            lvm_pv_params_set_property(params, "pvmetadatacopies", &value);
            
            if (m_size_edit->hasAcceptableInput()) {
                value.value.integer = 2 * m_size_edit->text().toInt(); 
                lvm_pv_params_set_property(params, "pvmetadatasize", &value);
            }

            if (m_align_edit->hasAcceptableInput()) {
                value.value.integer = 2 * m_align_edit->text().toInt(); 
                lvm_pv_params_set_property(params, "data_alignment", &value);
            }

            if (m_offset_edit->hasAcceptableInput()) {
                value.value.integer = 2 * m_offset_edit->text().toInt(); 
                lvm_pv_params_set_property(params, "data_alignment_offset", &value);
            }

            lvm_pv_create_adv(params);

            if (lvm_vg_extend(vg_dm, name.data()))
                KMessageBox::error(nullptr, QString(lvm_errmsg(lvm)));
        }

        if (lvm_vg_write(vg_dm))
            KMessageBox::error(nullptr, QString(lvm_errmsg(lvm)));

        lvm_vg_close(vg_dm);
        progress_box->reset();
        return;
    }

    KMessageBox::error(nullptr, QString(lvm_errmsg(lvm)));
    progress_box->reset();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    return;
}

void VGExtendDialog::validateOK()
{
    if (m_pv_checkbox->getRemainingSpace())
        enableButtonOk(true);
    else
        enableButtonOk(false);
}

void VGExtendDialog::buildDialog(const QList<const StorageBase *> devices)
{
    setCaption(i18n("Extend Volume Group"));

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *const title = new QLabel(i18n("Extend volume group: <b>%1</b>", m_vg->getName()));
    title->setAlignment(Qt::AlignCenter);
    layout->addSpacing(5);
    layout->addWidget(title);
    layout->addSpacing(10);

    QTabWidget *const tab_widget = new QTabWidget(this);
    layout->addWidget(tab_widget);
    
    tab_widget->addTab(buildGeneralTab(devices), "General");
    tab_widget->addTab(buildAdvancedTab(), "Advanced");
}

QWidget *VGExtendDialog::buildGeneralTab(const QList<const StorageBase *> devices)
{
    QWidget *tab = new(QWidget);
    QVBoxLayout *const layout = new QVBoxLayout();
    tab->setLayout(layout);

    m_pv_checkbox = new PvGroupBox(devices, m_vg->getExtentSize());
    layout->addWidget(m_pv_checkbox);

    connect(m_pv_checkbox, SIGNAL(stateChanged()),
            this, SLOT(validateOK()));

    return tab;
}

QWidget *VGExtendDialog::buildAdvancedTab()
{
    QWidget *tab = new QWidget;
    QVBoxLayout *const layout = new QVBoxLayout();
    tab->setLayout(layout);

    QHBoxLayout *const copies_layout = new QHBoxLayout();
    QLabel *const copies_label = new QLabel("Metadata copies: ");
    copies_layout->addWidget(copies_label);
    m_copies_combo = new QComboBox();
    m_copies_combo->addItem(i18n("0"));
    m_copies_combo->addItem(i18n("1"));
    m_copies_combo->addItem(i18n("2"));
    m_copies_combo->setCurrentIndex(1);
    copies_layout->addWidget(m_copies_combo);
    copies_layout->addStretch();

    QLabel *const unit_label = new QLabel(i18n("All values are in KiloBytes"));
    unit_label->setAlignment(Qt::AlignCenter);
    
    QHBoxLayout *const size_layout = new QHBoxLayout();
    QLabel *const size_label = new QLabel(i18n("Metadata size:"));
    size_layout->addWidget(size_label);
    m_size_edit = new QLineEdit;
    QIntValidator *const size_validator = new QIntValidator();
    size_validator->setBottom(0);
    m_size_edit->setValidator(size_validator);
    m_size_edit->setPlaceholderText(i18n("default"));
    size_layout->addWidget(m_size_edit);

    QHBoxLayout *const align_layout = new QHBoxLayout();
    QLabel *align_label = new QLabel(i18n("Metadata align:"));
    align_layout->addWidget(align_label);
    m_align_edit = new QLineEdit;
    QIntValidator *const align_validator = new QIntValidator();
    align_validator->setBottom(0);
    m_align_edit->setValidator(align_validator);
    m_align_edit->setPlaceholderText(i18n("default"));
    align_layout->addWidget(m_align_edit);

    QHBoxLayout *const offset_layout = new QHBoxLayout();
    QLabel *offset_label = new QLabel(i18n("Metadata offset:"));
    offset_layout->addWidget(offset_label);
    m_offset_edit = new QLineEdit;
    QIntValidator *const offset_validator = new QIntValidator();
    offset_validator->setBottom(0);
    m_offset_edit->setValidator(offset_validator);
    m_offset_edit->setPlaceholderText(i18n("default"));
    offset_layout->addWidget(m_offset_edit);

    layout->addLayout(copies_layout);
    layout->addWidget(unit_label);
    layout->addLayout(size_layout);
    layout->addLayout(align_layout);
    layout->addLayout(offset_layout);
    
    return tab;
}
