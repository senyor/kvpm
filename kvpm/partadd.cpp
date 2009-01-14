/*
 *
 * 
 * Copyright (C) 2009 Benjamin Scott   <benscott@nwlink.com>
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

#include "partadd.h"



bool add_partition(StoragePartition *partition)
{

    PartitionAddDialog dialog(partition);
    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted){
        dialog.commit_partition();
        return true;
    }
    else
        return false;
}


PartitionAddDialog::PartitionAddDialog(StoragePartition *partition, 
				       QWidget *parent) : 
  KDialog(parent),
  m_partition(partition)
  
{

    PedPartition *ped_free_partition = partition->getPedPartition(); // This is really freespace
    PedGeometry   ped_geometry       = ped_free_partition->geom;

    m_ped_disk = ped_free_partition->disk;
    PedDevice  *ped_device    = m_ped_disk->dev;
    m_ped_sector_size = ped_device->sector_size;
 
// The hardware's own constraints, if any
    m_ped_constraints = ped_device_get_constraint(ped_device); 

    m_ped_start_sector  = ped_geometry.start;
    m_ped_end_sector    = ped_geometry.end;
    m_ped_sector_length = ped_geometry.length;

    setWindowTitle( i18n("Add partition") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);
    QLabel *path = new QLabel( m_partition->getPartitionPath() );
    layout->addWidget(path);

    m_size_group   = new QGroupBox("Specify partition size");
    QGridLayout *size_group_layout = new QGridLayout();
    m_size_group->setLayout(size_group_layout);
    layout->addWidget(m_size_group);

    m_total_size_spin = new QSpinBox();
    m_total_size_spin->setRange(0,100);
    m_total_size_spin->setValue(100);
    m_total_size_spin->setSuffix("%");

    m_size_edit  = new KLineEdit();
    m_size_validator = new QDoubleValidator(m_size_edit);
    m_size_edit->setValidator(m_size_validator);
    m_size_validator->setBottom(0);

    m_size_combo = new KComboBox();
    m_size_combo->insertItem(0,"Sectors");
    m_size_combo->insertItem(1,"MB");
    m_size_combo->insertItem(2,"GB");
    m_size_combo->insertItem(3,"TB");
    m_size_combo->setInsertPolicy(KComboBox::NoInsert);
    m_size_combo->setCurrentIndex(2);

    adjustSizeEdit(100);

    size_group_layout->addWidget(m_size_edit,0,0);
    size_group_layout->addWidget(m_size_combo,0,1);
    size_group_layout->addWidget(m_total_size_spin,1,0);
    size_group_layout->addWidget( new QLabel("Total length"),1,1);
    m_align64_check = new QCheckBox("Align to 64 sectors");
    size_group_layout->addWidget(m_align64_check, 3, 0, 1, 2, Qt::AlignHCenter );

    double total_mib = (m_ped_sector_length * m_ped_sector_size) / (1024 *1024);
    QLabel *length_label = new QLabel( i18n("Maximum size: %1 MiB",  total_mib) );
    size_group_layout->addWidget( length_label, 4, 0, 1, 2, Qt::AlignHCenter );

    m_sector_group = new QGroupBox("Exclude preceding space");
    QVBoxLayout *sector_group_layout = new QVBoxLayout;
    m_sector_group->setCheckable(true);
    m_sector_group->setChecked(false);
    m_sector_group->setLayout(sector_group_layout);
    layout->addWidget(m_sector_group);


    connect(m_size_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(adjustSizeCombo(int)));

    connect(m_size_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validateVolumeSize(QString)));

    connect(m_total_size_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(adjustSizeEdit(int)));

}

void PartitionAddDialog::commit_partition()
{

    m_excluded_sectors = 0;


    PedSector first_sector = m_ped_start_sector + m_excluded_sectors;
    PedSector last_sector  = first_sector + m_partition_sectors;
    if( last_sector > m_ped_end_sector )
        last_sector = m_ped_end_sector;
    PedSector length  = (last_sector - first_sector) + 1;

    PedAlignment *ped_start_alignment = new PedAlignment();
    ped_start_alignment->offset = 0;
    ped_start_alignment->grain_size = 64;
    
    PedAlignment *ped_end_alignment = new PedAlignment();
    ped_end_alignment->offset = 0;
    ped_end_alignment->grain_size = 1;
    
    PedGeometry *start_geom = new PedGeometry();
    PedGeometry *end_geom   = new PedGeometry();

    ped_geometry_init(start_geom, m_ped_disk->dev, first_sector, 127);
    ped_geometry_init(end_geom,   m_ped_disk->dev, first_sector, length);

    PedConstraint *ped_constraint64 = new PedConstraint;
    
    ped_constraint64->start_align = ped_start_alignment;
    ped_constraint64->end_align   = ped_end_alignment;
    ped_constraint64->start_range = start_geom;
    ped_constraint64->end_range   = end_geom;
    ped_constraint64->min_size    = 128;
    ped_constraint64->max_size    = length;
	
    if( m_align64_check->isChecked() )
	m_ped_constraints = ped_constraint_intersect(ped_constraint64, m_ped_constraints);


    PedPartitionType type = PED_PARTITION_NORMAL ;

    PedPartition *ped_new_partition = ped_partition_new(m_ped_disk, 
							type, 
							0, 
							first_sector, 
							(first_sector + length) -1);
    int error;

    error = ped_disk_add_partition(m_ped_disk, ped_new_partition, m_ped_constraints);
    qDebug("Add part error: %d", error);

    error = ped_disk_commit(m_ped_disk);
    qDebug("Commit error: %d", error);

}

void PartitionAddDialog::adjustSizeEdit(int percentage){

    long long free_sectors = m_ped_sector_length;

    if(percentage == 100)
        m_partition_sectors = free_sectors;
    else if(percentage == 0)
        m_partition_sectors = 0;
    else
        m_partition_sectors = (long long)(( (double) percentage / 100) * free_sectors);

    adjustSizeCombo( m_size_combo->currentIndex() );

    resetOkButton();
}


void PartitionAddDialog::adjustSizeCombo(int index){

    long double sized;

    if(index){
        sized = ((long double)m_partition_sectors * m_ped_sector_size);
        if(index == 1)
            sized /= (long double)0x100000;
        if(index == 2)
            sized /= (long double)0x40000000;
        if(index == 3){
            sized /= (long double)(0x100000);
            sized /= (long double)(0x100000);
        }
        m_size_edit->setText(QString("%1").arg((double)sized));
    }
    else
        m_size_edit->setText(QString("%1").arg(m_partition_sectors));

}


void PartitionAddDialog::validateVolumeSize(QString size){

    int x = 0;

    const int size_combo_index = m_size_combo->currentIndex();

    if(m_size_validator->validate(size, x) == QValidator::Acceptable){

        if(!size_combo_index)
            m_partition_sectors = size.toLongLong();
        else
            m_partition_sectors = convertSizeToSectors( size_combo_index, size.toDouble() );

    }
    else{
        m_partition_sectors = 0;
    }

    resetOkButton();
}


long long PartitionAddDialog::convertSizeToSectors(int index, double size)
{
    long double partition_size = size;

    if(index == 1)
        partition_size *= (long double)0x100000;
    if(index == 2)
        partition_size *= (long double)0x40000000;
    if(index == 3){
        partition_size *= (long double)0x100000;
        partition_size *= (long double)0x100000;
    }

    partition_size /= m_ped_sector_size;

    if (partition_size < 0)
        partition_size = 0;

    return qRound64(partition_size);
}

void PartitionAddDialog::resetOkButton(){

    long long max_sectors = m_ped_sector_length;

    if( (m_partition_sectors <= max_sectors) && (m_partition_sectors > 0) )
        enableButtonOk(true);
    else
        enableButtonOk(false);

}
