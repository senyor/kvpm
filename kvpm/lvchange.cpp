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


bool change_lv(LogVol *LogicalVolume)
{
    LVChangeDialog dialog(LogicalVolume);
    dialog.exec();
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress change_lv(dialog.arguments(),"Changing lv attributes");
	return TRUE;
    }
    else
	return FALSE;
}

LVChangeDialog::LVChangeDialog(LogVol *LogicalVolume, QWidget *parent) : KDialog(parent)
{
    LogVol *lv;
    lv = LogicalVolume;
    lv_full_name = lv->getFullName();
    
    setWindowTitle(tr("Change logical volume attributes"));
 
    tab_widget = new KTabWidget();
    setMainWidget(tab_widget);

    tab_widget->addTab(general_tab  = new LVChangeGeneralTab(lv), "General");
    tab_widget->addTab(advanced_tab = new LVChangeAdvancedTab(lv), "Advanced");
}

QStringList LVChangeDialog::arguments()
{
    QStringList args, temp;

    args << "/sbin/lvchange";
    
    if(general_tab->available_check->isChecked())
	args << "--available" << "y";
    else
	args << "--available" << "n";

    if(general_tab->contig_check->isChecked())
	args << "--contiguous" << "y";
    else
	args << "--contiguous" << "n";
    if(general_tab->ro_check->isChecked())
	args << "--permission" << "r";
    else
	args << "--permission" << "rw";
    
    if(advanced_tab->mirror_box->isEnabled())
    {
	if(advanced_tab->resync_check->isChecked())
	    args << "--resync";
	if(advanced_tab->monitor_check->isChecked())
	    args << "--monitor" << "y";
	else
	    args << "--monitor" << "n";
    }
       
    if(advanced_tab->persistant_box->isChecked()){
	args << "--force" << "-My";
	args << "--major" << advanced_tab->major_edit->text();
	args << "--minor" << advanced_tab->minor_edit->text();
    }
    else
	args << "--force" << "-Mn";
 
    args << lv_full_name;
   
    return args;
}

LVChangeGeneralTab::LVChangeGeneralTab(LogVol *lv, QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);
    
    available_check  = new QCheckBox("Make volume available for use");
    contig_check     = new QCheckBox("Allocate contiguous extents");
    ro_check         = new QCheckBox("Make volume read only");
    refresh_check    = new QCheckBox("Refresh volume metadata");
    layout->addWidget(available_check);
    layout->addWidget(contig_check);
    layout->addWidget(ro_check);
    layout->addWidget(refresh_check);

    if(lv->getState() != "Unavailable")
	available_check->setChecked(TRUE);
    
    if(lv->getPolicy() == "Contiguous")
	contig_check->setChecked(TRUE);

    if( !(lv->isWritable()) )
	ro_check->setChecked(TRUE);
}

LVChangeAdvancedTab::LVChangeAdvancedTab(LogVol *lv, QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);

    mirror_box = new QGroupBox("Mirrored volume operations");
    QVBoxLayout *mirror_layout = new QVBoxLayout();
    mirror_box->setLayout(mirror_layout);
    resync_check     = new QCheckBox("Re-synchronize mirrors");
    monitor_check    = new QCheckBox("Monitor mirrors with dmeventd");
    mirror_layout->addWidget(resync_check);
    mirror_layout->addWidget(monitor_check);
    layout->addWidget(mirror_box);
    
    persistant_box = new QGroupBox("Persistant device numbers");
    persistant_box->setCheckable(TRUE);
    QVBoxLayout *persistant_layout = new QVBoxLayout();
    persistant_box->setLayout(persistant_layout);
    major_edit = new QLineEdit(QString("%1").arg(lv->getMajorDevice()));
    persistant_layout->addWidget(major_edit);
    minor_edit = new QLineEdit(QString("%1").arg(lv->getMinorDevice()));
    persistant_layout->addWidget(minor_edit);
    layout->addWidget(persistant_box);

    if(lv->getType() != "Mirror")
	mirror_box->setEnabled(FALSE);

    if(lv->isPersistant())
	persistant_box->setChecked(TRUE);
    else
	persistant_box->setChecked(FALSE);
}
