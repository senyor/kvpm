/*
 *
 * 
 * Copyright (C) 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "logvol.h"
#include "masterlist.h"
#include "volgroup.h"
#include "processprogress.h"



bool merge_vg(VolGroup *volumeGroup)
{
    const QStringList vg_names = MasterList::getVgNames();
    const QStringList lv_names = volumeGroup->getLvNames();

    if( vg_names.size() < 2  ){
        KMessageBox::error(0, i18n("There is no other volume group to merge with") );
        return false;
    }

    for(int x = 0; x < lv_names.size(); x++){
        if( (volumeGroup->getLvByName(lv_names[x]))->isActive() ){
            KMessageBox::error(0, i18n("The volume group to merge must not have active logical volumes") );
            return false;
        }
    }

    VGMergeDialog dialog(volumeGroup);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress vgmerge( dialog.arguments() );
        return true;
    }
    else
        return false;
}

VGMergeDialog::VGMergeDialog(VolGroup *volumeGroup, QWidget *parent) : KDialog(parent), m_vg(volumeGroup)
{
    setWindowTitle( i18n("Merge Volume Group") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);
    QLabel *name_label = new QLabel( i18n("Volume Group: <b>%1</b>", m_vg->getName() ) );
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    QGroupBox *target_group = new QGroupBox( i18n("Merge Volume Group With:") );
    QVBoxLayout *target_group_layout = new QVBoxLayout;
    target_group->setLayout(target_group_layout);
    layout->addWidget(target_group);
    m_target_combo = new KComboBox();

    QStringList vg_names = MasterList::getVgNames();
    for(int x = 0; x < vg_names.size(); x++){  // remove this groups own name from list
        if( m_vg->getName() != vg_names[x] )
            m_target_combo->addItem( vg_names[x] );
    }
    target_group_layout->addWidget(m_target_combo);
    m_autobackup = new QCheckBox("autobackup");
    m_autobackup->setChecked(true);
    target_group_layout->addWidget(m_autobackup);

}

QStringList VGMergeDialog::arguments()
{
    QStringList args;

    args << "vgmerge";

    if(m_autobackup->isChecked())
        args << "--autobackup" << "y";
    else
        args << "--autobackup" << "n";

    args << m_target_combo->currentText() << m_vg->getName();

    return args;
}
