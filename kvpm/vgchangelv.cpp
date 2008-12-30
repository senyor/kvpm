/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <KLocale>
#include <QtGui>

#include "processprogress.h"
#include "vgchangelv.h"
#include "volgroup.h"

/* This dialog sets a limit on the number of
   logical volumes a group may contain. The
   group may also be unlimited with lvm2 format */


bool change_vg_lv(VolGroup *volumeGroup)
{
    VGChangeLVDialog dialog(volumeGroup);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress change( dialog.arguments(), i18n("Changing limit...") );
	return true;
    }
    else
	return false;
}


VGChangeLVDialog::VGChangeLVDialog(VolGroup *volumeGroup, QWidget *parent) :
    KDialog(parent),
    m_vg(volumeGroup)
{

    setWindowTitle( i18n("Logical volumes limit") );
    m_vg_name = m_vg->getName();

// We don't want the limit set to less than the number already in existence!

    int lv_count = m_vg->getLogVolCount();
    if(lv_count <= 0)
	lv_count = 1;

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *name_label = new QLabel( i18n("Volume group: <b>%1</b>").arg(m_vg_name) );
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);
    
    m_limit_lvs = new QGroupBox( i18n("Maximum logical volumes") );
    QVBoxLayout *groupbox_layout = new QVBoxLayout();
    m_limit_lvs->setLayout(groupbox_layout);

    QLabel *message_label  = new QLabel();
    message_label->setWordWrap(true);
    layout->addWidget(message_label);
    QLabel *current_limit_label = new QLabel;
    layout->addWidget(current_limit_label);

    if( m_vg->getLogVolMax() )
	current_limit_label->setText( i18n( "Current limit: %1" ).arg( m_vg->getLogVolMax() ) );
    else
	current_limit_label->setText( i18n("Current limit: unlimited") );
    
    m_max_lvs = new QSpinBox();
    groupbox_layout->addWidget(m_max_lvs);

    if(m_vg->getFormat() == "lvm1"){
        message_label->setText( i18n("This volume group is in lvm1 format. Unless you "
				     "have a reason to set the limit lower it is normally best "
				     "to leave the limit at the maximum allowed: 255.") );
	
	m_max_lvs->setEnabled(true);
	m_max_lvs->setRange(lv_count, 255);
	m_max_lvs->setValue(255);
    }
    else{
        message_label->setText( i18n("This volume group is in lvm2 format. Unless you "
				     "have a reason to limit the maximum logical volumes it is "
				     "normally best to leave them unlimited.") );
	
	m_limit_lvs->setCheckable(true);
	m_limit_lvs->setChecked(false);
	m_limit_lvs->setEnabled(true);
	m_max_lvs->setMinimum(lv_count);
	m_max_lvs->setRange(lv_count, 32767); // does anyone need more than 32 thousand?
    }

    layout->addWidget(m_limit_lvs);
}

QStringList VGChangeLVDialog::arguments()
{
    QStringList args;
    
    args << "vgchange"
	 << "--logicalvolume";

    if(m_max_lvs->isEnabled())
	args << QString( "%1" ).arg( m_max_lvs->value() );
    else
	args << QString( "%1" ).arg( 0 );           // unlimited

    args << m_vg_name;

    return args;
}
