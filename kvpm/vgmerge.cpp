/*
 *
 *
 * Copyright (C) 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "vgmerge.h"

#include <KComboBox>
#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "logvol.h"
#include "masterlist.h"
#include "processprogress.h"
#include "volgroup.h"



VGMergeDialog::VGMergeDialog(VolGroup *const volumeGroup, QWidget *parent)
    : KDialog(parent),
      m_vg(volumeGroup)
{
    setWindowTitle(i18n("Merge Volume Group"));

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);
    QLabel *name_label = new QLabel(i18n("Merge volume group: <b>%1</b>", m_vg->getName()));
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);
    layout->addSpacing(5);

    QGroupBox *const target_group = new QGroupBox();
    QVBoxLayout *target_layout = new QVBoxLayout();

    QHBoxLayout *const combo_layout = new QHBoxLayout();
    QLabel *const target_label = new QLabel(i18n("Merge with:"));
    combo_layout->addWidget(target_label);
    m_target_combo = new KComboBox();
    target_label->setBuddy(m_target_combo);

    QList<VolGroup *> groups = MasterList::getVolGroups();
    for (int x = 0; x < groups.size(); x++) { // remove this groups own name from list
        if (m_vg->getName() != groups[x]->getName()) {
            m_target_combo->addItem(groups[x]->getName());
            m_extent_size.append(groups[x]->getExtentSize());
        }
    }
    combo_layout->addWidget(m_target_combo);
    m_autobackup = new QCheckBox("autobackup");
    m_autobackup->setChecked(true);
    target_layout->addLayout(combo_layout);

    m_error_stack = new QStackedWidget();
    QWidget *const no_error_widget = new QWidget();
    QWidget *const error_widget = new QWidget();
    QHBoxLayout *const error_layout = new QHBoxLayout();
    QLabel *const icon_label = new QLabel();
    icon_label->setPixmap(KIcon("dialog-warning").pixmap(32, 32));
    error_layout->addWidget(icon_label);
    error_layout->addWidget(new QLabel(i18n("Error: Extent size must match")));
    error_widget->setLayout(error_layout);
    m_error_stack->addWidget(no_error_widget);
    m_error_stack->addWidget(error_widget);
    target_layout->addWidget(m_error_stack);

    target_group->setLayout(target_layout);
    layout->addWidget(target_group);

    layout->addWidget(m_autobackup);

    connect(this, SIGNAL(okClicked()),
            this, SLOT(commitChanges()));

    connect(m_target_combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(compareExtentSize()));

    compareExtentSize();
}

void VGMergeDialog::commitChanges()
{
    QStringList args = QStringList() << "vgmerge";

    if (m_autobackup->isChecked())
        args << "--autobackup" << "y";
    else
        args << "--autobackup" << "n";

    args << m_target_combo->currentText() << m_vg->getName();

    ProcessProgress vgmerge(args);
}

bool  VGMergeDialog::bailout()
{
    const QStringList vg_names = MasterList::getVgNames();
    const QStringList lv_names = m_vg->getLvNames();

    if (vg_names.size() < 2) {
        KMessageBox::error(0, i18n("There is no other volume group to merge with"));
        return true;
    }

    for (int x = 0; x < lv_names.size(); x++) {
        if ((m_vg->getLvByName(lv_names[x]))->isActive()) {
            KMessageBox::error(0, i18n("The volume group to merge must not have active logical volumes"));
            return true;
        }
    }

    return false;
}

void VGMergeDialog::compareExtentSize()
{
    if (m_extent_size[ m_target_combo->currentIndex() ] == m_vg->getExtentSize()) {
        m_error_stack->setCurrentIndex(0);
        enableButtonOk(true);
    } else {
        m_error_stack->setCurrentIndex(1);
        enableButtonOk(false);
    }
}
