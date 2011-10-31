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

#include <KPushButton>
#include <KLocale>
#include <KMessageBox>
#include <KTabWidget>

#include <QtGui>

#include "logvol.h"
#include "masterlist.h"
#include "misc.h"
#include "physvol.h"
#include "processprogress.h"
#include "volgroup.h"

extern MasterList *g_master_list;

bool split_vg(VolGroup *volumeGroup)
{
    if( volumeGroup->getPhysicalVolumes().size() < 2  ){
        KMessageBox::error(0, i18n("A volume group must have at least two physical volumes to split group") );
        return false;
    }

    VGSplitDialog dialog(volumeGroup);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress vgsplit( dialog.arguments() );
        return true;
    }
    else
        return false;
}

VGSplitDialog::VGSplitDialog(VolGroup *volumeGroup, QWidget *parent) : KDialog(parent), m_vg(volumeGroup)
{
    QList<PhysVol *> pv_list = m_vg->getPhysicalVolumes();

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
    new_name_layout->addStretch();
    new_name_layout->addWidget(new_name_label);
    new_name_layout->addWidget(m_new_vg_name);
    new_name_layout->addStretch();
    layout->addLayout(new_name_layout);

    connect(m_new_vg_name, SIGNAL(textEdited(QString)), 
            this, SLOT(validateOK()));

    connect(this, SIGNAL(okClicked()), 
            this, SLOT(deactivate()));

    KTabWidget *tw = new KTabWidget();
    layout->addWidget(tw);

    QStringList mobile_lv_names, immobile_lv_names, mobile_pv_names, immobile_pv_names;

    volumeMobility( mobile_lv_names, immobile_lv_names, mobile_pv_names, immobile_pv_names);

    tw->addTab( buildLVLists(mobile_lv_names, immobile_lv_names), i18n("Logical volume view") );
    tw->addTab( buildPVLists(mobile_pv_names, immobile_pv_names), i18n("Physical volume view") );

    enableLVArrows();
    enablePVArrows();

    validateOK();
    setMinimumWidth(400);
}

void VGSplitDialog::validateOK()
{
    QString name = m_new_vg_name->text();
    int pos = 0;
    bool original_vg = false;
    bool new_vg = false;

    if(m_validator->validate(name, pos) == QValidator::Acceptable && name != "." && name != ".."){
        
        if( m_left_pv_list->count() ) // Must have at least one pv in old group and one in new group
            original_vg = true;

        if( m_right_pv_list->count() )
            new_vg = true;

        if(new_vg && original_vg)
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

    for(int x = m_right_pv_list->count() - 1; x >= 0; x--)        
        args << m_right_pv_list->item(x)->data(Qt::DisplayRole).toString();

    return args;
}

void VGSplitDialog::deactivate()
{
    QStringList moving_lvs;
    const QByteArray vg_name = m_vg->getName().toLocal8Bit();
    lvm_t lvm = g_master_list->getLVM();
    vg_t vg_dm;
    dm_list *lv_dm_list;
    lvm_lv_list *lv_list;
    QList<lv_t> lvs_to_deactivate;

    for(int x = m_right_lv_list->count() - 1; x >= 0; x--)        
        moving_lvs << m_right_lv_list->item(x)->data(Qt::DisplayRole).toString();

    if( (vg_dm = lvm_vg_open(lvm, vg_name.data(), "w", NULL)) ){

        for(int x = 0; x < moving_lvs.size(); x++){
            lv_dm_list = lvm_vg_list_lvs(vg_dm);
            dm_list_iterate_items(lv_list, lv_dm_list){ 
                if( QString( lvm_lv_get_name( lv_list->lv ) ).trimmed() == moving_lvs[x])
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

QWidget *VGSplitDialog::buildLVLists(const QStringList mobileLVNames, const QStringList immobileLVNames)
{
    QWidget *lv_list = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout;
    QVBoxLayout *left_layout = new QVBoxLayout;
    QVBoxLayout *center_layout = new QVBoxLayout;
    QVBoxLayout *right_layout = new QVBoxLayout;
    m_lv_add = new KPushButton( KIcon("arrow-right"), i18n("Add") );
    m_lv_remove = new KPushButton( KIcon("arrow-left"), i18n("Remove") );

    m_left_lv_list = new KListWidget();
    m_right_lv_list = new KListWidget();
    m_left_lv_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_right_lv_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_left_lv_list->setSortingEnabled(true);
    m_right_lv_list->setSortingEnabled(true);

    QListWidgetItem *lv_item;

    for(int x = mobileLVNames.size() - 1; x >= 0; x--){
        lv_item = new QListWidgetItem(mobileLVNames[x]);
        lv_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_left_lv_list->addItem(lv_item);
    }
    for(int x = immobileLVNames.size() - 1; x >= 0; x--){
        lv_item = new QListWidgetItem(immobileLVNames[x]);
        lv_item->setFlags(Qt::NoItemFlags);
        m_left_lv_list->addItem(lv_item);
    }

    QLabel *temp_label = new QLabel( i18n("Original volume group") );
    temp_label->setAlignment(Qt::AlignCenter);
    left_layout->addWidget(temp_label);
    left_layout->addWidget(m_left_lv_list);
    layout->addLayout(left_layout);
    center_layout->addStretch();
    center_layout->addWidget(m_lv_add);
    center_layout->addWidget(m_lv_remove);
    center_layout->addStretch();
    layout->addLayout(center_layout);
    temp_label = new QLabel( i18n("New volume group") );
    temp_label->setAlignment(Qt::AlignCenter);
    right_layout->addWidget(temp_label);
    right_layout->addWidget(m_right_lv_list);
    layout->addLayout(right_layout);

    lv_list->setLayout(layout);

    connect(m_left_lv_list, SIGNAL(itemSelectionChanged()), 
            this, SLOT(enableLVArrows()));

    connect(m_right_lv_list, SIGNAL(itemSelectionChanged()), 
            this, SLOT(enableLVArrows()));

    connect(m_lv_add, SIGNAL(clicked()), 
            this, SLOT(addLVList()));

    connect(m_lv_remove, SIGNAL(clicked()), 
            this, SLOT(removeLVList()));

    return lv_list;
}

QWidget *VGSplitDialog::buildPVLists(const QStringList mobilePVNames, const QStringList immobilePVNames)
{
    QWidget *pv_list = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout;
    QVBoxLayout *left_layout = new QVBoxLayout;
    QVBoxLayout *center_layout = new QVBoxLayout;
    QVBoxLayout *right_layout = new QVBoxLayout;
    m_pv_add = new KPushButton( KIcon("arrow-right"), "Add" );
    m_pv_remove = new KPushButton( KIcon("arrow-left"), "Remove" );

    QList<PhysVol *> pvs = m_vg->getPhysicalVolumes();
    QList<LogVol *>  lvs = m_vg->getLogicalVolumes();
    QListWidgetItem *pv_item;
    QStringList open_pv_names;
    QStringList closed_pv_names;

    m_left_pv_list = new KListWidget();
    m_right_pv_list = new KListWidget();
    m_left_pv_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_right_pv_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_left_pv_list->setSortingEnabled(true);
    m_right_pv_list->setSortingEnabled(true);

    for(int x = mobilePVNames.size() - 1; x >= 0; x--){
        pv_item = new QListWidgetItem( mobilePVNames[x] );
        pv_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_left_pv_list->addItem(pv_item);
    }

    for(int x = immobilePVNames.size() - 1; x >= 0; x--){
        pv_item = new QListWidgetItem( immobilePVNames[x] );
        pv_item->setFlags(Qt::NoItemFlags);
        m_left_pv_list->addItem(pv_item);
    }

    QLabel *temp_label = new QLabel( i18n("Original volume group") );
    temp_label->setAlignment(Qt::AlignCenter);
    left_layout->addWidget(temp_label);
    left_layout->addWidget(m_left_pv_list);
    layout->addLayout(left_layout);

    center_layout->addStretch();
    center_layout->addWidget(m_pv_add);
    center_layout->addWidget(m_pv_remove);
    center_layout->addStretch();
    layout->addLayout(center_layout);

    temp_label = new QLabel( i18n("New volume group") );
    temp_label->setAlignment(Qt::AlignCenter);
    right_layout->addWidget(temp_label);
    right_layout->addWidget(m_right_pv_list);
    layout->addLayout(right_layout);

    pv_list->setLayout(layout);

    connect(m_left_pv_list, SIGNAL(itemSelectionChanged()), 
            this, SLOT(enablePVArrows()));

    connect(m_right_pv_list, SIGNAL(itemSelectionChanged()), 
            this, SLOT(enablePVArrows()));

    connect(m_pv_add, SIGNAL(clicked()), 
            this, SLOT(addPVList()));

    connect(m_pv_remove, SIGNAL(clicked()), 
            this, SLOT(removePVList()));

    return pv_list;
}

void VGSplitDialog::enableLVArrows()
{
    if( m_left_lv_list->selectedItems().size() )
        m_lv_add->setEnabled(true);
    else
        m_lv_add->setEnabled(false);

    if( m_right_lv_list->selectedItems().size() )
        m_lv_remove->setEnabled(true);
    else
        m_lv_remove->setEnabled(false);
}

void VGSplitDialog::enablePVArrows()
{
    if( m_left_pv_list->selectedItems().size() )
        m_pv_add->setEnabled(true);
    else
        m_pv_add->setEnabled(false);

    if( m_right_pv_list->selectedItems().size() )
        m_pv_remove->setEnabled(true);
    else
        m_pv_remove->setEnabled(false);
}

void VGSplitDialog::addLVList()
{
    moveNames(true, m_left_lv_list, m_right_lv_list, m_left_pv_list, m_right_pv_list);
}

void VGSplitDialog::removeLVList()
{
    moveNames(true, m_right_lv_list, m_left_lv_list, m_right_pv_list, m_left_pv_list);
}

void VGSplitDialog::addPVList()
{
    moveNames(false, m_left_lv_list, m_right_lv_list, m_left_pv_list, m_right_pv_list);
}

void VGSplitDialog::removePVList()
{
    moveNames(false, m_right_lv_list, m_left_lv_list, m_right_pv_list, m_left_pv_list);
}

void VGSplitDialog::moveNames(const bool isLVMove,
                              KListWidget *const lvSource, KListWidget *const lvTarget,
                              KListWidget *const pvSource, KListWidget *const pvTarget)
{
    QList<QListWidgetItem *> selected_items;
    QList<QListWidgetItem *> moving_items;
    QStringList moving_pv_names, moving_lv_names;
    QStringList pv_names, lv_names;
    QString name;

    if(isLVMove){
        selected_items = lvSource->selectedItems();

        for(int x = selected_items.size() - 1; x >= 0; x--){
            name = lvSource->item( lvSource->row(selected_items[x]))->data(Qt::DisplayRole).toString();
            movesWithVolume(true, name, pv_names, lv_names);
            moving_pv_names.append(pv_names);
            moving_lv_names.append(lv_names);
        }

        moving_pv_names.removeDuplicates();
        moving_lv_names.removeDuplicates();
    }
    else{
        selected_items = pvSource->selectedItems();

        for(int x = selected_items.size() - 1; x >= 0; x--){
            name = pvSource->item( pvSource->row(selected_items[x]))->data(Qt::DisplayRole).toString();
            movesWithVolume(false, name, pv_names, lv_names);
            moving_pv_names.append(pv_names);
            moving_lv_names.append(lv_names);
        }

        moving_pv_names.removeDuplicates();
        moving_lv_names.removeDuplicates();
    }

    for(int x = moving_lv_names.size() - 1; x >= 0; x--){

        moving_items = lvSource->findItems(moving_lv_names[x], Qt::MatchExactly);

        for(int y = moving_items.size() - 1; y >= 0; y--){
            lvSource->takeItem( lvSource->row( moving_items[y] ) );        
            lvTarget->addItem( moving_items[y] );        
        }
    }

    for(int x = moving_pv_names.size() - 1; x >= 0; x--){

        moving_items = pvSource->findItems(moving_pv_names[x], Qt::MatchExactly);

        for(int y = moving_items.size() - 1; y >= 0; y--){
            pvSource->takeItem( pvSource->row( moving_items[y] ) );        
            pvTarget->addItem( moving_items[y] );        
        }
    }

    validateOK();
}

void VGSplitDialog::volumeMobility(QStringList &mobileLVNames, QStringList &immobileLVNames, 
                                   QStringList &mobilePVNames, QStringList &immobilePVNames)
{
    QStringList all_pv_names;
    const QList<LogVol *>  lvs = m_vg->getLogicalVolumes();
    const QList<PhysVol *> pvs = m_vg->getPhysicalVolumes();
    LogVol *temp;
    bool movable = true;
    bool growing = true;
    int immobile_count = 0;

    mobileLVNames.clear(); 
    mobilePVNames.clear();
    immobileLVNames.clear();
    immobilePVNames.clear();

    pvState(immobilePVNames, mobilePVNames);

    while(growing){
        for(int x = lvs.size() - 1; x >= 0 ; x--){

            all_pv_names = lvs[x]->getPVNamesAllFlat();
            movable = true;

            for(int y = all_pv_names.size() - 1; y >= 0; y--){
                for(int z = immobilePVNames.size() - 1; z >= 0; z--){
                    if( immobilePVNames[z] == all_pv_names[y] ){
                        movable = false;
                        break;
                    }
                } 
            }

            if( !movable )
                immobileLVNames.append( lvs[x]->getName() );
        }
        
        for(int x = immobileLVNames.size() - 1; x >= 0; x--){
            temp = m_vg->getLVByName( immobileLVNames[x] );

            if( temp->isOrigin() && ( temp->getParent() != NULL ) )
                immobilePVNames.append( temp->getParent()->getPVNamesAllFlat() );
            else
                immobilePVNames.append( temp->getPVNamesAllFlat() );
        }

        immobilePVNames.removeDuplicates();
        immobileLVNames.removeDuplicates();

        if( immobileLVNames.size() > immobile_count )
            growing = true;
        else
            growing = false;

        immobile_count = immobileLVNames.size();
    }

    mobilePVNames.clear();
    mobileLVNames.clear();

    for(int x = lvs.size() - 1; x >= 0; x--)
        mobileLVNames.append( lvs[x]->getName() );

    for(int x = mobileLVNames.size() - 1; x >= 0; x--){
        for(int y = immobileLVNames.size() - 1; y >= 0; y--){
            if(mobileLVNames[x] == immobileLVNames[y]){
                mobileLVNames.removeAt(x);
                break;          
            }
        }
    }

    for(int x = pvs.size() - 1; x >= 0; x--)
        mobilePVNames.append( pvs[x]->getName() );

    for(int x = mobilePVNames.size() - 1; x >= 0; x--){
        for(int y = immobilePVNames.size() - 1; y >= 0; y--){
            if(mobilePVNames[x] == immobilePVNames[y]){
                mobilePVNames.removeAt(x);
                break;          
            }
        }
    }

    mobilePVNames.removeDuplicates();
    mobileLVNames.removeDuplicates();
}

void VGSplitDialog::pvState(QStringList &open, QStringList &closed )
{
    const QList<PhysVol *> pvs = m_vg->getPhysicalVolumes();
    const QList<LogVol *>  lvs = m_vg->getLogicalVolumes();
    QList<LogVol *> snaps;

    for(int x = lvs.size() - 1; x >=0; x--){
        if( lvs[x]->isOpen() ){
            open.append( lvs[x]->getPVNamesAllFlat() );
        }
        else if( lvs[x]->isSnapContainer() || lvs[x]->isOrigin() ){
            snaps = lvs[x]->getSnapshots();
            for(int y = snaps.size() - 1; y >= 0; y--){
                if( snaps[y]->isOpen() ){
                    open.append( lvs[x]->getPVNamesAllFlat() ); // if any snap is open the whole container is open
                    break;
                }
            }
        }
    }

    open.removeDuplicates();

    QStringList all_pv_names;

    for(int x = pvs.size() - 1; x >= 0; x--)
        all_pv_names.append( pvs[x]->getName() );

    for(int x = all_pv_names.size() - 1; x >= 0; x--){
        for(int y = open.size() - 1; y >= 0; y--){
            if( all_pv_names[x] == open[y] ){
                all_pv_names.removeAt(x);
                break;
            }
        }
    }

    closed = all_pv_names;
}

void VGSplitDialog::movesWithVolume(const bool isLV, const QString name, 
                                    QStringList &movingPVNames, QStringList &movingLVNames)
{
    QStringList all_pv_names;
    const QList<LogVol *>  lvs = m_vg->getLogicalVolumes();
    const QList<PhysVol *> pvs = m_vg->getPhysicalVolumes();
    LogVol *temp;
    bool growing = true;
    bool moving = true;
    int moving_lv_count;
    int moving_pv_count;
    movingPVNames.clear();
    movingLVNames.clear();

    if(isLV){
        moving_lv_count = 1;
        moving_pv_count = 0;
        temp = m_vg->getLVByName(name);

        if( temp->isOrigin() && ( temp->getParent() != NULL ) )
            movingPVNames = temp->getParent()->getPVNamesAllFlat();
        else
            movingPVNames = temp->getPVNamesAllFlat();
    }
    else{
        moving_lv_count = 0;
        moving_pv_count = 1;
        movingPVNames.append(name);
    }

    while(growing){
        for(int x = lvs.size() - 1; x >= 0 ; x--){

            if( lvs[x]->isOrigin() && ( lvs[x]->getParent() != NULL ) )
                all_pv_names = lvs[x]->getParent()->getPVNamesAllFlat();
            else
                all_pv_names = lvs[x]->getPVNamesAllFlat();

            moving = false;

            for(int y = all_pv_names.size() - 1; y >= 0; y--){
                for(int z = movingPVNames.size() - 1; z >= 0; z--){
                    if( movingPVNames[z] == all_pv_names[y] ){
                        moving = true;
                        break;
                    }
                } 
            }
            if( moving )
               movingLVNames.append( lvs[x]->getName() );
        }
        
        for(int x = movingLVNames.size() - 1; x >= 0; x--){
            temp = m_vg->getLVByName( movingLVNames[x] );

            if( temp->isOrigin() && ( temp->getParent() != NULL ) )
                movingPVNames.append( temp->getParent()->getPVNamesAllFlat() );
            else
                movingPVNames.append( temp->getPVNamesAllFlat() );
        }

        movingLVNames.removeDuplicates();
        movingPVNames.removeDuplicates();

        if( ( movingLVNames.size() > moving_lv_count ) || ( movingPVNames.size() > moving_pv_count ) )
            growing = true;
        else
            growing = false;

        moving_lv_count = movingLVNames.size();
        moving_pv_count = movingPVNames.size();
    }
}
