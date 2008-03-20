/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>

#include "lvchange.h"
#include "logvol.h"
#include "processprogress.h"


bool change_lv(LogVol *logicalVolume)
{
    LVChangeDialog dialog(logicalVolume);
    dialog.exec();

    if(dialog.result() == QDialog::Accepted){
        ProcessProgress change_lv(dialog.arguments(),"Changing lv attributes");
	return true;
    }
    else
	return false;
}

LVChangeDialog::LVChangeDialog(LogVol *logicalVolume, QWidget *parent) : 
    KDialog(parent),
    m_lv(logicalVolume)
{

    setWindowTitle(tr("Change logical volume attributes"));
 
    KTabWidget *tab_widget = new KTabWidget();
    setMainWidget(tab_widget);

    buildGeneralTab();
    buildAdvancedTab();
    
    tab_widget->addTab(m_general_tab, "General");
    tab_widget->addTab(m_advanced_tab, "Advanced");
}

QStringList LVChangeDialog::arguments()
{
    QStringList args, temp;

    args << "lvchange";
    
    if(available_check->isChecked())
	args << "--available" << "y";
    else
	args << "--available" << "n";

    if(contig_check->isChecked())
	args << "--contiguous" << "y";
    else
	args << "--contiguous" << "n";

    if(ro_check->isChecked())
	args << "--permission" << "r";
    else
	args << "--permission" << "rw";
    
    if(m_mirror_box->isEnabled()){
	if(resync_check->isChecked())
	    args << "--resync";

	if(monitor_check->isChecked())
	    args << "--monitor" << "y";
	else
	    args << "--monitor" << "n";
    }
       
    if(m_persistant_box->isChecked()){
	args << "--force" << "-My";
	args << "--major" << major_edit->text();
	args << "--minor" << minor_edit->text();
    }
    else{
	args << "--force" << "-Mn";
    }
    
    args << m_lv->getFullName();
   
    return args;
}

void LVChangeDialog::buildGeneralTab()
{
    m_general_tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    m_general_tab->setLayout(layout);
    
    available_check  = new QCheckBox("Make volume available for use");
    contig_check     = new QCheckBox("Allocate contiguous extents");
    ro_check         = new QCheckBox("Make volume read only");
    refresh_check    = new QCheckBox("Refresh volume metadata");
    layout->addWidget(available_check);
    layout->addWidget(contig_check);
    layout->addWidget(ro_check);
    layout->addWidget(refresh_check);

    if(m_lv->getState() != "Unavailable")
	available_check->setChecked(true);
    
    if(m_lv->getPolicy() == "Contiguous")
	contig_check->setChecked(true);

    if( !(m_lv->isWritable()) )
	ro_check->setChecked(true);
}

void LVChangeDialog::buildAdvancedTab()
{
    m_advanced_tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    m_advanced_tab->setLayout(layout);

    m_mirror_box = new QGroupBox("Mirrored volume operations");
    QVBoxLayout *mirror_layout = new QVBoxLayout();
    m_mirror_box->setLayout(mirror_layout);
    resync_check     = new QCheckBox("Re-synchronize mirrors");
    monitor_check    = new QCheckBox("Monitor mirrors with dmeventd");
    mirror_layout->addWidget(resync_check);
    mirror_layout->addWidget(monitor_check);
    layout->addWidget(m_mirror_box);
    
    m_persistant_box = new QGroupBox("Persistant device numbers");
    m_persistant_box->setCheckable(true);
    QVBoxLayout *persistant_layout = new QVBoxLayout();
    m_persistant_box->setLayout(persistant_layout);
    major_edit = new KLineEdit(QString("%1").arg(m_lv->getMajorDevice()));
    persistant_layout->addWidget(major_edit);
    minor_edit = new KLineEdit(QString("%1").arg(m_lv->getMinorDevice()));
    persistant_layout->addWidget(minor_edit);
    layout->addWidget(m_persistant_box);

    if( !m_lv->isMirror() )
	m_mirror_box->setEnabled(false);

    if( m_lv->isPersistant() )
	m_persistant_box->setChecked(true);
    else
	m_persistant_box->setChecked(false);
}
