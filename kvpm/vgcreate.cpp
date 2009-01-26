/*
 *
 * 
 * Copyright (C) 2008, 2009 Benjamin Scott   <benscott@nwlink.com>
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
#include <KMessageBox>
#include <QtGui>

#include "physvol.h"
#include "processprogress.h"
#include "vgcreate.h"
#include "masterlist.h"

extern MasterList *master_list;

bool create_vg()
{
    QList<PhysVol *> physical_volumes;
    QStringList unused_pv_paths;

    physical_volumes = master_list->getPhysVols();
    for(int x = 0; x < physical_volumes.size(); x++){

      if( physical_volumes[x]->getVolumeGroupName() == "" )
	unused_pv_paths.append( physical_volumes[x]->getDeviceName() );
    }

    if( unused_pv_paths.size() > 0 ){
      VGCreateDialog dialog( unused_pv_paths );
      dialog.exec();
    
      if(dialog.result() == QDialog::Accepted){
          ProcessProgress create_vg( dialog.arguments(), i18n("Creating vg..."), true );
	  return true;
      }
      else
          return false;
    }
    else
        KMessageBox::error(0, i18n("No unused physical volumes found") );

    return false;
}

bool create_vg(QString physicalVolumePath)
{
    QStringList physicalVolumePathList(physicalVolumePath);

    VGCreateDialog dialog(physicalVolumePathList);
    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress create_vg( dialog.arguments(), i18n("Creating volume group..."), true );
        return true;
    }
    else
        return false;
}


VGCreateDialog::VGCreateDialog(QStringList physicalVolumePathList, QWidget *parent) : 
  KDialog(parent), m_pv_paths(physicalVolumePathList)
{

    setWindowTitle( i18n("Create Volume Group") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *name_label = new QLabel( i18n("Volume Group Name: ") );
    m_vg_name = new KLineEdit();

    QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
    m_validator = new QRegExpValidator( rx, m_vg_name );
    m_vg_name->setValidator(m_validator);
    QHBoxLayout *name_layout = new QHBoxLayout();
    name_layout->addWidget(name_label);
    name_layout->addWidget(m_vg_name);

    QLabel *extent_label = new QLabel( i18n("Physical Extent Size: ") );
    m_extent_size = new KComboBox();
    m_extent_size->insertItem(0,"1");
    m_extent_size->insertItem(1,"2");
    m_extent_size->insertItem(2,"4");
    m_extent_size->insertItem(3,"8");
    m_extent_size->insertItem(4,"16");
    m_extent_size->insertItem(5,"32");
    m_extent_size->insertItem(6,"64");
    m_extent_size->insertItem(7,"128");
    m_extent_size->insertItem(8,"256");
    m_extent_size->insertItem(9,"512");
    m_extent_size->setInsertPolicy(QComboBox::NoInsert);
    m_extent_size->setCurrentIndex(2);
    m_extent_suffix = new KComboBox();
    m_extent_suffix->insertItem(0,"KiB");
    m_extent_suffix->insertItem(1,"MiB");
    m_extent_suffix->insertItem(2,"GiB");
    m_extent_suffix->setInsertPolicy(QComboBox::NoInsert);
    m_extent_suffix->setCurrentIndex(1);

    QGroupBox *available_pv_box = new QGroupBox( i18n("Available physical volumes") ); 
    QVBoxLayout *available_pv_box_layout = new QVBoxLayout();
    available_pv_box->setLayout(available_pv_box_layout);
    NoMungeCheck *temp_check;
    if(m_pv_paths.size() < 2)
        available_pv_box_layout->addWidget( new QLabel(m_pv_paths[0] ) );
    else{
        for(int x = 0; x < m_pv_paths.size(); x++){
	    temp_check = new NoMungeCheck(m_pv_paths[x]);
	    available_pv_box_layout->addWidget(temp_check);
	    m_pv_checks.append(temp_check);

	    connect(temp_check, SIGNAL(toggled(bool)), 
		    this, SLOT(validateOK()));
	}
    }
    
    QHBoxLayout *extent_layout = new QHBoxLayout();
    extent_layout->addWidget(extent_label);
    extent_layout->addWidget(m_extent_size);
    extent_layout->addWidget(m_extent_suffix);

    QGroupBox *lv_box = new QGroupBox( i18n("Number of Logical Volumes") );
    QVBoxLayout *lv_layout_v = new QVBoxLayout();
    QHBoxLayout *lv_layout_h = new QHBoxLayout();
    lv_box->setLayout(lv_layout_v);
    m_max_lvs_check = new QCheckBox( i18n("No Limit") );
    m_max_lvs_check->setCheckState(Qt::Checked);
    lv_layout_v->addWidget(m_max_lvs_check);
    lv_layout_v->addLayout(lv_layout_h);
    QLabel *lv_label = new QLabel( i18n("Maximum: ") );
    m_max_lvs = new KLineEdit();
    QIntValidator *lv_validator = new QIntValidator(1,255,this);
    m_max_lvs->setValidator(lv_validator);
    m_max_lvs->setEnabled(false);
    lv_layout_h->addWidget(lv_label);
    lv_layout_h->addWidget(m_max_lvs);

    QGroupBox *pv_box = new QGroupBox( i18n("Number of Physical Volumes") );
    QVBoxLayout *pv_layout_v = new QVBoxLayout();
    QHBoxLayout *pv_layout_h = new QHBoxLayout();
    pv_box->setLayout(pv_layout_v);
    m_max_pvs_check = new QCheckBox( i18n("No Limit") );
    m_max_pvs_check->setCheckState(Qt::Checked);
    pv_layout_v->addWidget(m_max_pvs_check);
    pv_layout_v->addLayout(pv_layout_h);
    QLabel *pv_label = new QLabel( i18n("Maximum: ") );
    m_max_pvs = new KLineEdit();
    QIntValidator *pv_validator = new QIntValidator(1,255,this);
    m_max_pvs->setValidator(pv_validator);
    m_max_pvs->setEnabled(false);
    pv_layout_h->addWidget(pv_label);
    pv_layout_h->addWidget(m_max_pvs);

    m_clustered = new QCheckBox( i18n("Cluster Aware") );
    m_clustered->setEnabled(false);
    
    m_auto_backup = new QCheckBox( i18n("Automatic Backup") );
    m_auto_backup->setCheckState(Qt::Checked);

    layout->addLayout(name_layout);
    layout->addLayout(extent_layout);
    layout->addWidget(available_pv_box);
    layout->addWidget(lv_box);
    layout->addWidget(pv_box);
    layout->addWidget(m_clustered);
    layout->addWidget(m_auto_backup);

    enableButtonOk(false);

    connect(m_vg_name, SIGNAL(textChanged(QString)), 
	    this, SLOT(validateOK()));

    connect(m_max_lvs_check, SIGNAL(stateChanged(int)), 
	    this, SLOT(limitLogicalVolumes(int)));

    connect(m_max_pvs_check, SIGNAL(stateChanged(int)), 
	    this, SLOT(limitPhysicalVolumes(int)));
}

QStringList VGCreateDialog::arguments()
{
    QStringList args;
    
    args << "vgcreate"
	 << "--physicalextentsize"
	 << m_extent_size->currentText() + m_extent_suffix->currentText().at(0);
    
    if(m_clustered->isChecked())
	args << "--clustered" << "y";
    else
	args << "--clustered" << "n";

    if(m_auto_backup->isChecked())
	args << "--autobackup" << "y" ;
    else
	args << "--autobackup" << "n" ;

    if((!m_max_lvs_check->isChecked()) && (m_max_lvs->text() != ""))
	args << "--maxlogicalvolumes" << m_max_lvs->text();

    if((!m_max_pvs_check->isChecked()) && (m_max_pvs->text() != ""))
	args << "--maxphysicalvolumes" << m_max_pvs->text();

    args << m_vg_name->text(); 

    if( m_pv_checks.size() > 1 ){
        m_pv_paths.clear();	for(int x = 0; x < m_pv_checks.size(); x++){
	    if( m_pv_checks[x]->isChecked() )
	        m_pv_paths.append( m_pv_checks[x]->getUnmungedText() );
	}
    }
    args << m_pv_paths;

    return args;
}


/* The allowed characters in the name are letters, numbers, periods
   hyphens and underscores. Also, the names ".", ".." and names starting
   with a hyphen are disallowed. Finally disable the OK button if no 
   pvs are checked  */

void VGCreateDialog::validateOK()
{
    QString name = m_vg_name->text();
    int pos = 0;

    enableButtonOk(false);

    if(m_validator->validate(name, pos) == QValidator::Acceptable && name != "." && name != ".."){

        if( ! m_pv_checks.size() )        // if there is only one pv possible, there is no list
	        enableButtonOk(true);
	else{
	    for(int x = 0; x < m_pv_checks.size(); x++){
	        if( m_pv_checks[x]->isChecked() ){
		    enableButtonOk(true);
		    break;
		}
	    }
	}
    }
}

void VGCreateDialog::limitLogicalVolumes(int boxstate)
{
    if(boxstate == Qt::Unchecked)
	m_max_lvs->setEnabled(true);
    else
	m_max_lvs->setEnabled(false);
}

void VGCreateDialog::limitPhysicalVolumes(int boxstate)
{
    if(boxstate == Qt::Unchecked)
	m_max_pvs->setEnabled(true);
    else
	m_max_pvs->setEnabled(false);
}
