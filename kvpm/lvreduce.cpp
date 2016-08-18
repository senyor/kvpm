/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "lvreduce.h"

#include <KConfigSkeleton>
#include <KFormat>
#include <KLocalizedString>
#include <KMessageBox>

#include <QLabel>
#include <QVBoxLayout>

#include "fsreduce.h"
#include "logvol.h"
#include "processprogress.h"
#include "misc.h"
#include "sizeselectorbox.h"
#include "volgroup.h"


LVReduceDialog::LVReduceDialog(LogVol *const volume, QWidget *parent) : 
    KvpmDialog(parent),
    m_lv(volume)
{
    if (volume->isThinPool())
        setCaption(i18n("Reduce Thin Pool"));
    else
        setCaption(i18n("Reduce Logical Volume"));

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    const long long extent_size = m_lv->getVg()->getExtentSize();
    const long long current_extents = m_lv->getExtents();
    const QString fs = m_lv->getFilesystem();
    long long min_extents;
    bool force = false;

    const QString warning_message1 = i18n("If this <b>Inactive</b> logical volume is reduced "
                                          "any <b>data</b> it contains <b>will be lost!</b>");

    const QString warning_message2 = i18n("Only the ext2, ext3 and ext4 file systems "
                                          "are supported for file system reduction. If this "
                                          "logical volume is reduced any <b>data</b> it contains "
                                          "<b>will be lost!</b>");

    if (!m_lv->isActive() && !m_lv->isCowSnap()) {
        if (KMessageBox::warningContinueCancel(nullptr,
                                               warning_message1,
                                               QString(),
                                               KStandardGuiItem::cont(),
                                               KStandardGuiItem::cancel(),
                                               QString(),
                                               KMessageBox::Dangerous) == KMessageBox::Continue) {
            force = true;
        } else {
            preventExec();
        }
    } else if (m_lv->isMounted() && !m_lv->isCowSnap()) {
        KMessageBox::error(nullptr, i18n("The filesystem must be unmounted first"));
        preventExec();
    } else if (m_lv->isThinPool()) {
        KMessageBox::error(nullptr, i18n("Reducing thin pools isn't supported yet"));
        preventExec();
    } else if (!fs_can_reduce(fs) && !m_lv->isCowSnap()) {
        if (KMessageBox::warningContinueCancel(nullptr,
                                               warning_message2,
                                               QString(),
                                               KStandardGuiItem::cont(),
                                               KStandardGuiItem::cancel(),
                                               QString(),
                                               KMessageBox::Dangerous) == KMessageBox::Continue) {
            force = true;
        } else {
            preventExec();
        }
    }

    if (willExec() && !force && !m_lv->isCowSnap()) {
        const long long min_fs = get_min_fs_size(m_lv->getMapperPath(), m_lv->getFilesystem());

        if (min_fs % extent_size)
            min_extents = 1 + (min_fs / extent_size);
        else
            min_extents = min_fs / extent_size;

        if (min_fs == 0 || min_extents >= m_lv->getExtents()) {
            KMessageBox::error(nullptr, i18n("The filesystem is already as small as it can be"));
            preventExec();
        }
    } else {
        min_extents = 1;
    }

    if (willExec()) {
        bool use_si_units;
        KConfigSkeleton skeleton;
        skeleton.setCurrentGroup("General");
        skeleton.addItemBool("use_si_units", use_si_units, false);

        KFormat::BinaryUnitDialect dialect;

        if (use_si_units)
            dialect = KFormat::MetricBinaryDialect;
        else
            dialect = KFormat::IECBinaryDialect;

        QVBoxLayout *const label_layout = new QVBoxLayout();
        QWidget *const label_widget = new QWidget();
        label_widget->setLayout(label_layout);

        QLabel *const lv_name_label = new QLabel();
        if (m_lv->isThinPool())
            lv_name_label->setText(i18n("Reduce thin pool: <b>%1</b>", m_lv->getName()));
        else
            lv_name_label->setText(i18n("Reduce logical volume: <b>%1</b>", m_lv->getName()));

        lv_name_label->setAlignment(Qt::AlignCenter);
        QLabel *const lv_min_label  = new QLabel(i18n("Estimated minimum size: %1", KFormat().formatByteSize(min_extents * extent_size, 1, dialect)));

        m_size_selector = new SizeSelectorBox(extent_size, min_extents, current_extents, current_extents, true, false, true);

        label_layout->addWidget(lv_name_label);
        label_layout->addSpacing(10);
        label_layout->addWidget(lv_min_label);
        label_layout->addSpacing(5);
        layout->addWidget(label_widget);
        layout->addWidget(m_size_selector);

        connect(m_size_selector, SIGNAL(stateChanged()),
                this , SLOT(resetOkButton()));

        resetOkButton();
    }
}

void LVReduceDialog::commit()
{
    const QString fs = m_lv->getFilesystem();
    const long long target_size = m_size_selector->getNewSize() * m_lv->getVg()->getExtentSize();
    long long new_size;

    hide();

    if (m_lv->isCowSnap() || !m_lv->isActive())    // never reduce the fs of a snap!
        new_size = target_size;
    else if (fs_can_reduce(fs))
        new_size = fs_reduce(m_lv->getMapperPath(), target_size, fs);
    else
        new_size = target_size;

    if (new_size) {
        QStringList args = QStringList() << "lvreduce"
                                         << "--force"
                                         << "--size"
                                         << QString("%1K").arg(new_size / 1024)
                                         << m_lv->getMapperPath();

        ProcessProgress reduce_lv(args);
    }
}

void LVReduceDialog::resetOkButton()
{
    const long long diff = m_size_selector->getMaximumSize() - m_size_selector->getNewSize(); 
    enableButtonOk(m_size_selector->isValid() && diff);
}
