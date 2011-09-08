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

#include "vgsplit.h"

#include <KLocale>
#include <KMessageBox>
#include <QtGui>

#include "logvol.h"
#include "masterlist.h"
#include "physvol.h"
#include "processprogress.h"
#include "volgroup.h"

extern MasterList *master_list;

bool split_vg(VolGroup *volumeGroup)
{
    if( volumeGroup->getPhysicalVolumes().size() < 2  ){
        KMessageBox::error(0, i18n("A volume group must have at least two physical volumes to split group") );
        return false;
    }

    VGSplitDialog dialog(volumeGroup);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress vgsplit(dialog.arguments(), i18n("Splitting volume group"), true );
        return true;
    }
    else
        return false;
}

VGSplitDialog::VGSplitDialog(VolGroup *volumeGroup, QWidget *parent) : KDialog(parent), m_vg(volumeGroup)
{
    QList<PhysVol *> pv_list = m_vg->getPhysicalVolumes();
    m_lvs = m_vg->getLogicalVolumes();
    NoMungeCheck *temp_check;

    for(int x = 0; x < pv_list.size(); x++){
        temp_check = new NoMungeCheck( pv_list[x]->getName() );
        m_pv_checks.append(temp_check);

        connect(temp_check, SIGNAL(toggled(bool)), 
                this, SLOT(adjustTable(bool)));
    }    

    setWindowTitle( i18n("Split Volume Group") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);
    QLabel *name_label = new QLabel( i18n("Volume Group To Split: <b>%1</b>", m_vg->getName() ) );
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    m_new_vg_name = new KLineEdit();
    QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
    m_validator = new QRegExpValidator( rx, m_new_vg_name );
    m_new_vg_name->setValidator(m_validator);
    QLabel *new_name_label = new QLabel( i18n("New Volume Group Name") );
    QHBoxLayout *new_name_layout = new QHBoxLayout();
    new_name_layout->addWidget(new_name_label);
    new_name_layout->addWidget(m_new_vg_name);
    layout->addLayout(new_name_layout);

    connect(m_new_vg_name, SIGNAL(textEdited(QString)), 
            this, SLOT(validateName(QString)));

    connect(this, SIGNAL(okClicked()), 
            this, SLOT(deactivate()));

    for(int x = 0; x < m_lvs.size(); x++){
        if(m_lvs[x]->isOpen() && !(m_lvs[x]->isMirrorLog() || m_lvs[x]->isMirrorLeg()) ) // logs and legs are always open
            m_busy_pvs << getUnderlyingDevices(m_lvs[x]);
    }
    m_busy_pvs.removeDuplicates();

    QGroupBox *pv_box = new QGroupBox( i18n("Physcial volumes to split off") );
    layout->addWidget(pv_box);
    QGridLayout *pv_box_layout = new QGridLayout();
    pv_box->setLayout(pv_box_layout);
    int pv_check_count = m_pv_checks.size();
    for(int x = 0; x < pv_check_count; x++){
        for(int y = 0; y < m_busy_pvs.size(); y++){
            if(m_busy_pvs[y] == m_pv_checks[x]->getUnmungedText())
                m_pv_checks[x]->setEnabled(false);
        }
        if(pv_check_count < 11 )
            pv_box_layout->addWidget(m_pv_checks[x], x % 5, x / 5);
        else if (pv_check_count % 3 == 0)
            pv_box_layout->addWidget(m_pv_checks[x], x % (pv_check_count / 3), x / (pv_check_count / 3));
        else
            pv_box_layout->addWidget(m_pv_checks[x], x % ( (pv_check_count + 2) / 3), x / ( (pv_check_count + 2) / 3));
    }

    QLabel *unmovable_label = new QLabel( i18n("<b>Volumes that are in use can't be moved</b>") );
    unmovable_label->setAlignment(Qt::AlignCenter);
    if(m_busy_pvs.size()){
        layout->addWidget(unmovable_label);
    }

    m_pv_table = new QTableWidget();
    m_pv_table->setColumnCount(3);
    layout->addWidget(m_pv_table);
    adjustTable(true);
    setMinimumWidth(400);
}

void VGSplitDialog::adjustTable(bool)
{
    QTableWidgetItem *newItem;
    QStringList original_vg_pv_names, new_vg_pv_names, lv_pv_names;
    bool original_vg = false;
    bool new_vg = false;

    m_lvs_moving.clear();

    for(int x = 0; x < m_pv_checks.size(); x++){
        if( m_pv_checks[x]->isChecked() )
            new_vg_pv_names.append( m_pv_checks[x]->getUnmungedText() );
        else
            original_vg_pv_names.append( m_pv_checks[x]->getUnmungedText() );
    }

    m_pv_table->clear();
    m_pv_table->setDragDropMode(QAbstractItemView::InternalMove);
    QStringList headers;
    headers << "Original Group" << "Split Between Groups"<< "New Group";
    m_pv_table->setHorizontalHeaderLabels(headers);
    m_pv_table->setRowCount( m_lvs.size() );

    for(int x = 0; x < m_lvs.size(); x++){
        original_vg = false;
        new_vg = false;

        lv_pv_names = getUnderlyingDevices(m_lvs[x]);
        if(m_lvs[x]->isSnap())
            lv_pv_names << getUnderlyingDevices(m_vg->getLogVolByName( m_lvs[x]->getOrigin() ));
        for(int y = 0; y < lv_pv_names.size(); y++){

            if( original_vg_pv_names.contains(lv_pv_names[y]) )
                original_vg = true;

            if( new_vg_pv_names.contains(lv_pv_names[y]) )
                new_vg = true;
        }

        if(original_vg && !new_vg){
            newItem = new QTableWidgetItem( m_lvs[x]->getName() );
            newItem->setFlags(Qt::ItemIsEnabled);
            m_pv_table->setItem(x, 0, newItem);
        }
        else if(new_vg && !original_vg){
            newItem = new QTableWidgetItem( m_lvs[x]->getName() );
            m_lvs_moving.append( m_lvs[x]->getName() );
            newItem->setFlags(Qt::ItemIsEnabled);
            m_pv_table->setItem(x, 2, newItem);
        }
        else{
            newItem = new QTableWidgetItem( m_lvs[x]->getName() );
            newItem->setFlags(Qt::ItemIsEnabled);
            m_pv_table->setItem(x, 1, newItem);
        }
    }

    m_pv_table->resizeColumnsToContents();
    validateOK();
}

void VGSplitDialog::validateName(QString)
{
    validateOK();
}
void VGSplitDialog::validateOK()
{
    QString name = m_new_vg_name->text();
    int pos = 0;
    bool original_vg = false;
    bool new_vg = false;
    bool split_vg = false;

    if(m_validator->validate(name, pos) == QValidator::Acceptable && name != "." && name != ".."){

        // Must have at least one pv in old group and one in new group
        for(int x = 0; x < m_pv_checks.size(); x++){
            if(m_pv_checks[x]->isChecked())
                new_vg = true;
            else
                original_vg = true;
        }
        // No lv may be split between groups
        for(int x = 0; x < m_pv_table->columnCount(); x++){
            if( m_pv_table->item(x, 1) != NULL )
                split_vg = true;
        }

        if(new_vg && original_vg && !split_vg)
            enableButtonOk(true);
        else
            enableButtonOk(false);
    }
    else
        enableButtonOk(false);
}

QStringList VGSplitDialog::arguments()
{
    QStringList args;
    args << "vgsplit" << m_vg->getName() << m_new_vg_name->text();

    for(int x = 0; x < m_pv_checks.size(); x++){
        if(m_pv_checks[x]->isChecked())
            args << m_pv_checks[x]->getUnmungedText();
    }

    return args;
}

void VGSplitDialog::deactivate()
{
    lvm_t lvm = master_list->getLVM();
    vg_t vg_dm;
    dm_list *lv_dm_list;
    lvm_lv_list *lv_list;
    QList<lv_t> lvs_to_deactivate;

    if( (vg_dm = lvm_vg_open(lvm, m_vg->getName().toAscii().data(), "w", NULL)) ){

        for(int x = 0; x < m_lvs_moving.size(); x++){
            lv_dm_list = lvm_vg_list_lvs(vg_dm);
            dm_list_iterate_items(lv_list, lv_dm_list){ 
                if( QString( lvm_lv_get_name( lv_list->lv ) ).trimmed() == m_lvs_moving[x])
                    lvs_to_deactivate.append( lv_list->lv );
            }
        }
        
        for(int x = 0; x < lvs_to_deactivate.size(); x++){
            if( lvm_lv_is_active(lvs_to_deactivate[x]) ){
                if( lvm_lv_deactivate(lvs_to_deactivate[x]) )
                    KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
            }
        }
        lvm_vg_close(vg_dm);
        return;
    }
    else{
        KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
        return;
    }
    
    return;
}

QStringList VGSplitDialog::getUnderlyingDevices(LogVol *lv)
{
    QStringList devices;
    QList<LogVol *> legsAndLogs;

    if( lv->isMirror() ){
        for(int x = 0; x < m_lvs.size(); x++){
            if( (lv->getName() == m_lvs[x]->getOrigin()) && !m_lvs[x]->isSnap() )
                legsAndLogs.append(m_lvs[x]);
        }
        for(int x = 0; x < legsAndLogs.size(); x++){
            if(legsAndLogs[x]->isMirror())
                devices << getUnderlyingDevices(legsAndLogs[x]);
            else
                devices << legsAndLogs[x]->getDevicePathAll();
        }
    }
    else
        devices = lv->getDevicePathAll();

    return devices;
}
