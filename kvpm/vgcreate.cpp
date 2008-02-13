/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>
#include "processprogress.h"
#include "vgcreate.h"


bool create_vg(QString PhysicalVolumePath)
{
    VGCreateDialog dialog(PhysicalVolumePath);
    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress create_vg( dialog.arguments() );
        return TRUE;
    }
    else
        return FALSE;
}


VGCreateDialog::VGCreateDialog(QString PhysicalVolumePath, QWidget *parent) : 
    KDialog(parent),
    pv_path(PhysicalVolumePath)
{

    setWindowTitle(tr("Create Volume Group"));

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *name_label = new QLabel("Volume Group Name: ");
    vg_name = new KLineEdit();
    QHBoxLayout *name_layout = new QHBoxLayout();
    name_layout->addWidget(name_label);
    name_layout->addWidget(vg_name);

    QLabel *extent_label = new QLabel("Physical Extent Size: ");
    extent_size = new KComboBox();
    extent_size->insertItem(0,"1");
    extent_size->insertItem(1,"2");
    extent_size->insertItem(2,"4");
    extent_size->insertItem(3,"8");
    extent_size->insertItem(4,"16");
    extent_size->insertItem(5,"32");
    extent_size->insertItem(6,"64");
    extent_size->insertItem(7,"128");
    extent_size->insertItem(8,"256");
    extent_size->insertItem(9,"512");
    extent_size->setInsertPolicy(QComboBox::NoInsert);
    extent_size->setCurrentIndex(2);
    extent_suffix = new KComboBox();
    extent_suffix->insertItem(0,"KB");
    extent_suffix->insertItem(1,"MB");
    extent_suffix->insertItem(2,"GB");
    extent_suffix->setInsertPolicy(QComboBox::NoInsert);
    extent_suffix->setCurrentIndex(1);
    
    QHBoxLayout *extent_layout = new QHBoxLayout();
    extent_layout->addWidget(extent_label);
    extent_layout->addWidget(extent_size);
    extent_layout->addWidget(extent_suffix);

    QGroupBox *lv_box = new QGroupBox("Number of Logical Volumes");
    QVBoxLayout *lv_layout_v = new QVBoxLayout();
    QHBoxLayout *lv_layout_h = new QHBoxLayout();
    lv_box->setLayout(lv_layout_v);
    max_lvs_check = new QCheckBox("No Limit");
    max_lvs_check->setCheckState(Qt::Checked);
    lv_layout_v->addWidget(max_lvs_check);
    lv_layout_v->addLayout(lv_layout_h);
    QLabel *lv_label = new QLabel("Maximum: ");
    max_lvs = new KLineEdit();
    QIntValidator *lv_validator = new QIntValidator(1,255,this);
    max_lvs->setValidator(lv_validator);
    max_lvs->setEnabled(FALSE);
    lv_layout_h->addWidget(lv_label);
    lv_layout_h->addWidget(max_lvs);

    QGroupBox *pv_box = new QGroupBox("Number of Physical Volumes");
    QVBoxLayout *pv_layout_v = new QVBoxLayout();
    QHBoxLayout *pv_layout_h = new QHBoxLayout();
    pv_box->setLayout(pv_layout_v);
    max_pvs_check = new QCheckBox("No Limit");
    max_pvs_check->setCheckState(Qt::Checked);
    pv_layout_v->addWidget(max_pvs_check);
    pv_layout_v->addLayout(pv_layout_h);
    QLabel *pv_label = new QLabel("Maximum: ");
    max_pvs = new KLineEdit();
    QIntValidator *pv_validator = new QIntValidator(1,255,this);
    max_pvs->setValidator(pv_validator);
    max_pvs->setEnabled(FALSE);
    pv_layout_h->addWidget(pv_label);
    pv_layout_h->addWidget(max_pvs);

    clustered = new QCheckBox("Cluster Aware");
    clustered->setEnabled(FALSE);
    
    auto_backup = new QCheckBox("Automatic Backup");
    auto_backup->setCheckState(Qt::Checked);

    layout->addLayout(name_layout);
    layout->addLayout(extent_layout);
    layout->addWidget(lv_box);
    layout->addWidget(pv_box);
    layout->addWidget(clustered);
    layout->addWidget(auto_backup);

    connect(max_lvs_check, SIGNAL(stateChanged(int)), 
	    this, SLOT(limitLogicalVolumes(int)));

    connect(max_pvs_check, SIGNAL(stateChanged(int)), 
	    this, SLOT(limitPhysicalVolumes(int)));
}

QStringList VGCreateDialog::arguments()
{
    QStringList args;
    
    args << "/sbin/vgcreate"
	 << "--physicalextentsize"
	 << extent_size->currentText() + extent_suffix->currentText().at(0);
    
    if(clustered->isChecked())
	args << "--clustered" << "y";
    else
	args << "--clustered" << "n";

    if(auto_backup->isChecked())
	args << "--autobackup" << "y" ;
    else
	args << "--autobackup" << "n" ;

    if((!max_lvs_check->isChecked()) && (max_lvs->text() != ""))
	args << "--maxlogicalvolumes" << max_lvs->text();

    if((!max_pvs_check->isChecked()) && (max_pvs->text() != ""))
	args << "--maxphysicalvolumes" << max_pvs->text();
    
    args << vg_name->text() << pv_path;

    return args;
}

void VGCreateDialog::limitLogicalVolumes(int boxstate)
{
    if(boxstate == Qt::Unchecked)
	max_lvs->setEnabled(TRUE);
    else
	max_lvs->setEnabled(FALSE);
}

void VGCreateDialog::limitPhysicalVolumes(int boxstate)
{
    if(boxstate == Qt::Unchecked)
	max_pvs->setEnabled(TRUE);
    else
	max_pvs->setEnabled(FALSE);
}
