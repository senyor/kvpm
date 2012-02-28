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


#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KPushButton>

#include <QtGui>

#include "pvgroupbox.h"
#include "physvol.h"
#include "misc.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "volgroup.h"


PvGroupBox::PvGroupBox(QList<PhysVol *> volumes, QWidget *parent)
    : QGroupBox(parent), 
      m_pvs(volumes)
{
    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);

    setTitle( i18n("Available Physical Volumes") );
    QGridLayout *const layout = new QGridLayout();
    setLayout(layout);

    NoMungeCheck *check;
    const int pv_check_count = m_pvs.size();
    m_space_label   = new QLabel;
    m_extents_label = new QLabel;

    KLocale *const locale = KGlobal::locale();
    if(m_use_si_units)
        locale->setBinaryUnitDialect(KLocale::MetricBinaryDialect); 
    else
        locale->setBinaryUnitDialect(KLocale::IECBinaryDialect);

    if(pv_check_count < 1){
        m_extent_size = 1;
        QLabel *pv_label = new QLabel( i18n("<b>No suitable volumes found</b>") );
        layout->addWidget(pv_label);
    }
    else if(pv_check_count < 2){
        m_extent_size = m_pvs[0]->getVg()->getExtentSize();
        QLabel *pv_label = new QLabel( m_pvs[0]->getName() + "  " + locale->formatByteSize( m_pvs[0]->getRemaining() ) );
        layout->addWidget(pv_label, 0, 0, 1, -1);

        addLabelsAndButtons(layout, pv_check_count);
        calculateSpace();
    }
    else{
        m_extent_size = m_pvs[0]->getVg()->getExtentSize();
        for(int x = 0; x < pv_check_count; x++){
	    check = new NoMungeCheck( m_pvs[x]->getName() + "  " + locale->formatByteSize( m_pvs[x]->getRemaining() ) );
	    check->setAlternateText( m_pvs[x]->getName() );
	    check->setData( QVariant( m_pvs[x]->getRemaining() ) );
	    m_pv_checks.append(check);

            if(pv_check_count < 11 )
                layout->addWidget(m_pv_checks[x], x % 5, x / 5);
            else if (pv_check_count % 3 == 0)
                layout->addWidget(m_pv_checks[x], x % (pv_check_count / 3), x / (pv_check_count / 3));
            else
                layout->addWidget(m_pv_checks[x], x % ( (pv_check_count + 2) / 3), x / ( (pv_check_count + 2) / 3));

	    connect(check, SIGNAL(toggled(bool)), 
		    this, SLOT(calculateSpace()));
	}

        selectAll();
        addLabelsAndButtons(layout, pv_check_count);
    }
}

PvGroupBox::PvGroupBox(QList <StorageDevice *> devices, QList<StoragePartition *> partitions, 
                       long long extentSize, QWidget *parent)
    : QGroupBox(parent), 
      m_devices(devices), 
      m_partitions(partitions), 
      m_extent_size(extentSize)
{
    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);

    if(devices.size() + partitions.size() > 1)
        setTitle( i18n("Available Physical Volumes") );
    else
        setTitle( i18n("Physical Volume") );

    QGridLayout *const layout = new QGridLayout();
    setLayout(layout);
    QString name;
    long long size;
    int dev_count = 0;

    NoMungeCheck *check;
    const int pv_check_count = m_devices.size() + m_partitions.size();
    m_space_label   = new QLabel;
    m_extents_label = new QLabel;

    KLocale *const locale = KGlobal::locale();
    if(m_use_si_units)
        locale->setBinaryUnitDialect(KLocale::MetricBinaryDialect); 
    else
        locale->setBinaryUnitDialect(KLocale::IECBinaryDialect);

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
        QLabel *pv_label = new QLabel( name + "  " + locale->formatByteSize(size) );
        layout->addWidget(pv_label, 0, 0, 1, -1);
        addLabelsAndButtons(layout, pv_check_count);

        calculateSpace();
    }
    else{
        for(int x = 0; x < m_devices.size(); x++){
            dev_count++;
            name = m_devices[x]->getName();
            size = m_devices[x]->getSize();
	    check = new NoMungeCheck( name + "  " + locale->formatByteSize(size) );
	    check->setAlternateText(name);
	    check->setData( QVariant(size) );
	    m_pv_checks.append(check);

            if(pv_check_count < 11 )
                layout->addWidget(m_pv_checks[x], x % 5, x / 5);
            else if (pv_check_count % 3 == 0)
                layout->addWidget(m_pv_checks[x], x % (pv_check_count / 3), x / (pv_check_count / 3));
            else
                layout->addWidget(m_pv_checks[x], x % ( (pv_check_count + 2) / 3), x / ( (pv_check_count + 2) / 3));

	    connect(check, SIGNAL(toggled(bool)), 
		    this, SLOT(calculateSpace()));
	}

        for(int x = 0; x < m_partitions.size(); x++){
            name = m_partitions[x]->getName();
            size = m_partitions[x]->getSize();
	    check = new NoMungeCheck( name + "  " + locale->formatByteSize(size) );
	    check->setAlternateText(name);
	    check->setData( QVariant(size) );
	    m_pv_checks.append(check);

            if(pv_check_count < 11 )
                layout->addWidget(check, (dev_count + x) % 5, (dev_count + x) / 5);
            else if (pv_check_count % 3 == 0)
                layout->addWidget(check, (dev_count + x) % (pv_check_count / 3), (dev_count + x) / (pv_check_count / 3));
            else
                layout->addWidget(check, (dev_count + x) % ( (pv_check_count + 2) / 3), (dev_count + x) / ( (pv_check_count + 2) / 3));

	    connect(check, SIGNAL(toggled(bool)), 
		    this, SLOT(calculateSpace()));
	}

        selectAll();
        addLabelsAndButtons(layout, pv_check_count);

        setExtentSize(extentSize);
    }
}

QStringList PvGroupBox::getNames(){

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

QStringList PvGroupBox::getAllNames(){

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

long long PvGroupBox::getRemainingSpace(){

    long long space = 0;

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++){
            if(m_pv_checks[x]->isChecked())
                space += (m_pv_checks[x]->getData()).toLongLong();
        }
    }
    else if(m_pvs.size())
        space = m_pvs[0]->getRemaining();
    else if(m_devices.size())
        space = m_devices[0]->getSize();
    else if(m_partitions.size())
        space = m_partitions[0]->getSize();
    else
        space = 0;

    return space;
}

QList<long long> PvGroupBox::getRemainingSpaceList(){

    QList<long long> space;

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++){
            if(m_pv_checks[x]->isChecked())
                space.append( (m_pv_checks[x]->getData()).toLongLong() );
        }
    }
    else if(m_pvs.size())
        space.append( m_pvs[0]->getRemaining() );
    else if(m_devices.size())
        space.append( m_devices[0]->getSize() );
    else if(m_partitions.size())
        space.append( m_partitions[0]->getSize() );

    return space;
}

void PvGroupBox::selectAll(){

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++){
            if(m_pv_checks[x]->isEnabled())
                m_pv_checks[x]->setChecked(true);
        }
    }
    emit stateChanged();

    return;
}

void PvGroupBox::selectNone(){

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++)
            m_pv_checks[x]->setChecked(false);
    }

    emit stateChanged();

    return;
}

void PvGroupBox::calculateSpace(){

    KLocale *const locale = KGlobal::locale();
    if(m_use_si_units)
        locale->setBinaryUnitDialect(KLocale::MetricBinaryDialect); 
    else
        locale->setBinaryUnitDialect(KLocale::IECBinaryDialect);

    if(m_pv_checks.size() > 1){
        m_space_label->setText( i18n("Selected space: %1", locale->formatByteSize(getRemainingSpace()) ) );
        m_extents_label->setText( i18n("Selected extents: %1", getRemainingSpace() / m_extent_size ) );
    }
    else{
        m_space_label->setText( i18n("Space: %1", locale->formatByteSize(getRemainingSpace()) ) );
        m_extents_label->setText( i18n("Extents: %1", getRemainingSpace() / m_extent_size ) );
    }

    emit stateChanged();

    return;
}

void PvGroupBox::setExtentSize(long long extentSize){
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

void PvGroupBox::disableOrigin(PhysVol *originVolume){

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

QHBoxLayout *PvGroupBox::getButtons()
{
    QHBoxLayout *const layout = new QHBoxLayout();
    KPushButton *const all    = new KPushButton( i18n("Select all") );
    KPushButton *const none   = new KPushButton( i18n("Clear all") );

    layout->addStretch();
    layout->addWidget(all);
    layout->addStretch();
    layout->addWidget(none);
    layout->addStretch();

    connect(all,  SIGNAL(clicked(bool)), this, SLOT(selectAll()));
    connect(none, SIGNAL(clicked(bool)), this, SLOT(selectNone()));
    
    return layout;
}

void PvGroupBox::addLabelsAndButtons(QGridLayout *const layout, const int pvCount)
{
    QVBoxLayout *const spacer1 = new QVBoxLayout;
    QVBoxLayout *const spacer2 = new QVBoxLayout;
    spacer1->addSpacing(10);
    spacer2->addSpacing(10);
    const int count = layout->rowCount();

    layout->addLayout(spacer1,         count,     0, 1, -1);
    layout->addWidget(m_space_label,   count + 1, 0, 1, -1);
    layout->addWidget(m_extents_label, count + 2, 0, 1, -1);
    layout->addLayout(spacer2,         count + 3, 0, 1, -1);
    
    if(pvCount > 1)
        layout->addLayout(getButtons(), count + 4, 0, 1, -1);
}