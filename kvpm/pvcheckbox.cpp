/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <KPushButton>
#include <KLocale>
#include <QtGui>

#include "pvcheckbox.h"
#include "physvol.h"
#include "misc.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "volgroup.h"


PVCheckBox::PVCheckBox(QList<PhysVol *> physicalVolumes, QWidget *parent):
    QGroupBox(parent), 
    m_pvs(physicalVolumes)
{

    setTitle( i18n("Available physical volumes") );
    QGridLayout *layout = new QGridLayout();
    setLayout(layout);
    QHBoxLayout *button_layout = new QHBoxLayout();
    KPushButton *all_button = new KPushButton( i18n("Select all") );
    KPushButton *none_button = new KPushButton( i18n("Clear all") );
    NoMungeCheck *temp_check;
    int pv_check_count = m_pvs.size();
    m_space_label   = new QLabel;
    m_extents_label = new QLabel;

    if(pv_check_count < 1){
        m_extent_size = 1;
        QLabel *pv_label = new QLabel( i18n("<b>No suitable volumes found!</b>") );
        layout->addWidget(pv_label);
    }
    else if(pv_check_count < 2){
        m_extent_size = m_pvs[0]->getVolGroup()->getExtentSize();
        QLabel *pv_label = new QLabel( m_pvs[0]->getName() + "  " + sizeToString( m_pvs[0]->getUnused() ) );
        layout->addWidget(pv_label, 0, 0, 1, -1);
        layout->addWidget(m_space_label,   layout->rowCount(), 0, 1, -1);
        layout->addWidget(m_extents_label, layout->rowCount(), 0, 1, -1);
        calculateSpace();
    }
    else{
        m_extent_size = m_pvs[0]->getVolGroup()->getExtentSize();
        for(int x = 0; x < pv_check_count; x++){
	    temp_check = new NoMungeCheck( m_pvs[x]->getName() + "  " + sizeToString( m_pvs[x]->getUnused() ) );
	    temp_check->setAlternateText( m_pvs[x]->getName() );
	    temp_check->setData( QVariant( m_pvs[x]->getUnused() ) );
	    m_pv_checks.append(temp_check);

            if(pv_check_count < 11 )
                layout->addWidget(m_pv_checks[x], x % 5, x / 5);
            else if (pv_check_count % 3 == 0)
                layout->addWidget(m_pv_checks[x], x % (pv_check_count / 3), x / (pv_check_count / 3));
            else
                layout->addWidget(m_pv_checks[x], x % ( (pv_check_count + 2) / 3), x / ( (pv_check_count + 2) / 3));

	    connect(temp_check, SIGNAL(toggled(bool)), 
		    this, SLOT(calculateSpace()));
	}

        selectAll();
        layout->addWidget(m_space_label,   layout->rowCount(), 0, 1, -1);
        layout->addWidget(m_extents_label, layout->rowCount(), 0, 1, -1);
        layout->addLayout(button_layout,   layout->rowCount(), 0, 1, -1);
        button_layout->addStretch();
        button_layout->addWidget(all_button);
        button_layout->addStretch();
        button_layout->addWidget(none_button);
        button_layout->addStretch();
        connect(all_button,  SIGNAL(clicked(bool)), this, SLOT(selectAll()));
        connect(none_button, SIGNAL(clicked(bool)), this, SLOT(selectNone()));

    }
}

PVCheckBox::PVCheckBox(QList <StorageDevice *> devices, QList<StoragePartition *> partitions, 
                       long long extentSize, QWidget *parent):
    QGroupBox(parent), 
    m_devices(devices), 
    m_partitions(partitions), 
    m_extent_size(extentSize)
{
    setTitle( i18n("Available physical volumes") );
    QGridLayout *layout = new QGridLayout();
    setLayout(layout);
    QString name;
    long long size;
    int dev_count = 0;
    QHBoxLayout *button_layout = new QHBoxLayout();
    KPushButton *all_button = new KPushButton( i18n("Select all") );
    KPushButton *none_button = new KPushButton( i18n("Clear all") );
    NoMungeCheck *temp_check;
    int pv_check_count = m_devices.size() + m_partitions.size();
    m_space_label   = new QLabel;

    m_extents_label = new QLabel;

    if(pv_check_count < 1){
        QLabel *pv_label = new QLabel( i18n("none found") );
        layout->addWidget(pv_label);
    }
    else if(pv_check_count < 2){
        if( m_devices.size() ){
            name = m_devices[0]->getName();
            size = m_devices[0]->getSize();
        }
        else{
            name = m_partitions[0]->getName();
            size = m_partitions[0]->getSize();
        }
        QLabel *pv_label = new QLabel( name + "  " + sizeToString(size) );
        layout->addWidget(pv_label, 0, 0, 1, -1);
        layout->addWidget(m_space_label,   layout->rowCount(), 0, 1, -1);
        layout->addWidget(m_extents_label, layout->rowCount(), 0, 1, -1);

        calculateSpace();
    }
    else{
        for(int x = 0; x < m_devices.size(); x++){
            dev_count++;
            name = m_devices[x]->getName();
            size = m_devices[x]->getSize();
	    temp_check = new NoMungeCheck( name + "  " + sizeToString(size) );
	    temp_check->setAlternateText(name);
	    temp_check->setData( QVariant(size) );
	    m_pv_checks.append(temp_check);

            if(pv_check_count < 11 )
                layout->addWidget(m_pv_checks[x], x % 5, x / 5);
            else if (pv_check_count % 3 == 0)
                layout->addWidget(m_pv_checks[x], x % (pv_check_count / 3), x / (pv_check_count / 3));
            else
                layout->addWidget(m_pv_checks[x], x % ( (pv_check_count + 2) / 3), x / ( (pv_check_count + 2) / 3));

	    connect(temp_check, SIGNAL(toggled(bool)), 
		    this, SLOT(calculateSpace()));
	}

        for(int x = 0; x < m_partitions.size(); x++){
            name = m_partitions[x]->getName();
            size = m_partitions[x]->getSize();
	    temp_check = new NoMungeCheck( name + "  " + sizeToString(size) );
	    temp_check->setAlternateText(name);
	    temp_check->setData( QVariant(size) );
	    m_pv_checks.append(temp_check);

            if(pv_check_count < 11 )
                layout->addWidget(temp_check, (dev_count + x) % 5, (dev_count + x) / 5);
            else if (pv_check_count % 3 == 0)
                layout->addWidget(temp_check, (dev_count + x) % (pv_check_count / 3), (dev_count + x) / (pv_check_count / 3));
            else
                layout->addWidget(temp_check, (dev_count + x) % ( (pv_check_count + 2) / 3), (dev_count + x) / ( (pv_check_count + 2) / 3));

	    connect(temp_check, SIGNAL(toggled(bool)), 
		    this, SLOT(calculateSpace()));
	}

        selectAll();
        layout->addWidget(m_space_label,   layout->rowCount(), 0, 1, -1);
        layout->addWidget(m_extents_label, layout->rowCount(), 0, 1, -1);
        layout->addLayout(button_layout,   layout->rowCount(), 0, 1, -1);
        button_layout->addStretch();
        button_layout->addWidget(all_button);
        button_layout->addStretch();
        button_layout->addWidget(none_button);
        button_layout->addStretch();
        connect(all_button,  SIGNAL(clicked(bool)), this, SLOT(selectAll()));
        connect(none_button, SIGNAL(clicked(bool)), this, SLOT(selectNone()));

        setExtentSize(extentSize);
    }
}

QStringList PVCheckBox::getNames(){

    QStringList names;

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++){
            if(m_pv_checks[x]->isChecked())
                names << m_pv_checks[x]->getAlternateText();
        }
    }
    else if(m_pvs.size())
        names << m_pvs[0]->getName();
    else if(m_devices.size())
        names << m_devices[0]->getName();
    else if(m_partitions.size())
        names << m_partitions[0]->getName();

    return names;
}

QStringList PVCheckBox::getAllNames(){

    QStringList names;

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++){
            names << m_pv_checks[x]->getAlternateText();
        }
    }
    else if(m_pvs.size())
        names << m_pvs[0]->getName();
    else if(m_devices.size())
        names << m_devices[0]->getName();
    else if(m_partitions.size())
        names << m_partitions[0]->getName();

    return names;
}

long long PVCheckBox::getUnusedSpace(){

    long long space = 0;

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++){
            if(m_pv_checks[x]->isChecked())
                space += (m_pv_checks[x]->getData()).toLongLong();
        }
    }
    else if(m_pvs.size())
        space = m_pvs[0]->getUnused();
    else if(m_devices.size())
        space = m_devices[0]->getSize();
    else if(m_partitions.size())
        space = m_partitions[0]->getSize();
    else
        space = 0;

    return space;
}

QList<long long> PVCheckBox::getUnusedSpaceList(){

    QList<long long> space;

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++){
            if(m_pv_checks[x]->isChecked())
                space.append( (m_pv_checks[x]->getData()).toLongLong() );
        }
    }
    else if(m_pvs.size())
        space.append( m_pvs[0]->getUnused() );
    else if(m_devices.size())
        space.append( m_devices[0]->getSize() );
    else if(m_partitions.size())
        space.append( m_partitions[0]->getSize() );

    return space;
}

void PVCheckBox::selectAll(){

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++){
            if(m_pv_checks[x]->isEnabled())
                m_pv_checks[x]->setChecked(true);
        }
    }
    emit stateChanged();

    return;
}

void PVCheckBox::selectNone(){

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++)
            m_pv_checks[x]->setChecked(false);
    }

    emit stateChanged();

    return;
}

void PVCheckBox::calculateSpace(){

    m_space_label->setText( i18n("Selected space: %1", sizeToString(getUnusedSpace()) ) );
    m_extents_label->setText( i18n("Selected extents: %1", getUnusedSpace() / m_extent_size ) );

    emit stateChanged();

    return;
}

void PVCheckBox::setExtentSize(long long extentSize){
    m_extent_size = extentSize;

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++){
            if( (m_pv_checks[x]->getData()).toULongLong() > ( m_extent_size + 0xfffff ) ) // 1 MiB for MDA, fix this
                m_pv_checks[x]->setEnabled(true);                                         // when MDA size is put in
            else{                                                                         // liblvm2app
                m_pv_checks[x]->setChecked(false);
                m_pv_checks[x]->setEnabled(false);
            }
        }
    }

    calculateSpace();
}

void PVCheckBox::disableOrigin(PhysVol *originVolume){

    QString name;

    if(originVolume){ 

        name = originVolume->getName();

        for(int x = 0; x < m_pvs.size(); x++){
            if( m_pvs[x]->getName() == name ){
                m_pv_checks[x]->setChecked(false);
                m_pv_checks[x]->setEnabled(false);
            }
            else
                m_pv_checks[x]->setEnabled(true);
        }
    }
}
