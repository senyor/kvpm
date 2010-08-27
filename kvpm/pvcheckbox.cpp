/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
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

PVCheckBox::PVCheckBox(QList<PhysVol *> physicalVolumes, long long extentSize, QWidget *parent):
    QGroupBox(parent), m_pvs(physicalVolumes), m_extent_size(extentSize)
{
    setTitle( i18n("Available physical volumes") );
    QGridLayout *layout = new QGridLayout();
    setLayout(layout);
    QHBoxLayout *button_layout = new QHBoxLayout();
    KPushButton *all_button = new KPushButton( i18n("Select all") );
    KPushButton *none_button = new KPushButton( i18n("Select none") );
    NoMungeCheck *temp_check;
    int pv_check_count = m_pvs.size();
    m_space_label   = new QLabel;
    m_extents_label = new QLabel;

    if(pv_check_count < 1){
        QLabel *pv_label = new QLabel( i18n("none found") );
        layout->addWidget(pv_label);
    }
    else if(pv_check_count < 2){
        QLabel *pv_label = new QLabel( m_pvs[0]->getDeviceName() + "  " + sizeToString( m_pvs[0]->getUnused() ) );
        layout->addWidget(pv_label, 0, 0, 1, -1);
        layout->addWidget(m_space_label,   layout->rowCount(), 0, 1, -1);
        layout->addWidget(m_extents_label, layout->rowCount(), 0, 1, -1);
        calculateSpace();
    }
    else{
        for(int x = 0; x < pv_check_count; x++){
	    temp_check = new NoMungeCheck( m_pvs[x]->getDeviceName() + "  " + sizeToString( m_pvs[x]->getUnused() ) );
	    temp_check->setAlternateText( m_pvs[x]->getDeviceName() );
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

QStringList PVCheckBox::getNames(){

    QStringList names;

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++){
            if(m_pv_checks[x]->isChecked())
                names << m_pv_checks[x]->getAlternateText();
        }
    }
    else if(m_pvs.size())
        names << m_pvs[0]->getDeviceName();

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
    else if(m_pvs.size()){
        space = m_pvs[0]->getUnused();
    }
    else{
        space = 0;
    }

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
    else if(m_pvs.size()){
        space.append( m_pvs[0]->getUnused() );
    }
    else{
        space.append(0);
    }

    return space;
}

void PVCheckBox::selectAll(){

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++)
            m_pv_checks[x]->setChecked(true);
    }

    return;
}

void PVCheckBox::selectNone(){

    if(m_pv_checks.size()){
        for(int x = 0; x < m_pv_checks.size(); x++)
            m_pv_checks[x]->setChecked(false);
    }

    return;
}

void PVCheckBox::calculateSpace(){

    m_space_label->setText( i18n("Selected space: %1").arg( sizeToString(getUnusedSpace()) ) );
    m_extents_label->setText( i18n("Selected extents: %1").arg( getUnusedSpace() / m_extent_size ) );
    emit stateChanged();

    return;
}

