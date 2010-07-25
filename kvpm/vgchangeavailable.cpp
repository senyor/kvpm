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


#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "logvol.h"
#include "processprogress.h"
#include "vgchangeavailable.h"
#include "volgroup.h"


/* This dialog changes the available status of
   the logical volumes in the volume group */


bool change_vg_available(VolGroup *volumeGroup)
{
    QString message;
    bool has_mounted = false;
    QList<LogVol *> lv_list = volumeGroup->getLogicalVolumes();
    
    for(int x = 0; x < lv_list.size(); x++){
	if( lv_list[x]->isMounted() )
	    has_mounted = true;
    }
    
    if( has_mounted ){
	message = i18n("Groups with mounted logical volumes "
		       "may not be made unavailable");

	KMessageBox::error( 0, message);
    }
    else{
        VGChangeAvailableDialog dialog(volumeGroup);
        dialog.exec();
    
        if(dialog.result() == QDialog::Accepted){
            ProcessProgress available(dialog.args(), i18n("Changing volume group availability..."));
            return true;
        }
    }
    return false;
}

VGChangeAvailableDialog::VGChangeAvailableDialog(VolGroup *volumeGroup, QWidget *parent) : 
    KDialog(parent), m_vg(volumeGroup)
{
    setWindowTitle( i18n("Volume group availability") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *name_label = new QLabel( i18n("Volume group: <b>%1</b>").arg(m_vg->getName() ) );
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    QGroupBox *group_box = new QGroupBox("Set volume group availabilty");
    layout->addWidget(group_box);
    QVBoxLayout *group_layout = new QVBoxLayout();
    group_box->setLayout(group_layout);

    m_available = new QRadioButton("Make all logical volumes available");
    m_available->setChecked(true);
    m_unavailable = new QRadioButton("Make all logical volumes unavailable");
    group_layout->addWidget(m_available);
    group_layout->addWidget(m_unavailable);
}

QStringList VGChangeAvailableDialog::args()
{
    QStringList args;

    if( m_available->isChecked() )
        args << "vgchange" << "--available" << "y" << m_vg->getName();
    else
        args << "vgchange" << "--available" << "n" << m_vg->getName();

    return args;
}
