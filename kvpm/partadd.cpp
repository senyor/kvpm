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

    m_size_group   = new QGroupBox("Specify relative size");
    QGridLayout *size_group_layout = new QGridLayout();
    m_size_group->setCheckable(true);
    m_size_group->setChecked(true);
    m_size_group->setLayout(size_group_layout);
    layout->addWidget(m_size_group);

    m_start_size_spin = new QSpinBox();
    m_end_size_spin   = new QSpinBox();
    m_total_size_spin = new QSpinBox();
    m_total_size_spin->setMaximum( (m_ped_sector_length * m_ped_sector_size)/ (1024 *1024) );
    m_total_size_spin->setValue( (m_ped_sector_length * m_ped_sector_size)/ (1024 *1024) );

    m_start_size_spin->setSuffix(" MiB");
    m_end_size_spin->setSuffix(" MiB");
    m_total_size_spin->setSuffix(" MiB");

    size_group_layout->addWidget(m_start_size_spin,0,0);
    size_group_layout->addWidget(m_end_size_spin,  1,0);
    size_group_layout->addWidget(m_total_size_spin,2,0);
    size_group_layout->addWidget( new QLabel("Preceding free space"), 0,1);
    size_group_layout->addWidget( new QLabel("Following free space"),   1,1);
    size_group_layout->addWidget( new QLabel("Total length"),2,1);
    m_align64_check = new QCheckBox("Align to 64 sectors");
    size_group_layout->addWidget(m_align64_check, 3, 0, 1, 2, Qt::AlignHCenter );

    double total_mib = (m_ped_sector_length * m_ped_sector_size) / (1024 *1024);
    QLabel *length_label = new QLabel( i18n("Maximum size: %1 MiB",  total_mib) );
    size_group_layout->addWidget( length_label, 4, 0, 1, 2, Qt::AlignHCenter );

    m_sector_group = new QGroupBox("Specify absolute sectors");
    QVBoxLayout *sector_group_layout = new QVBoxLayout;
    m_sector_group->setCheckable(true);
    m_sector_group->setChecked(false);
    m_sector_group->setLayout(sector_group_layout);
    layout->addWidget(m_sector_group);

    m_start_sector_spin = new QSpinBox();
    m_start_sector_spin->setMinimum(m_ped_start_sector);
    m_start_sector_spin->setMaximum(m_ped_end_sector);
    m_start_sector_spin->setValue(m_ped_start_sector);
    sector_group_layout->addWidget(m_start_sector_spin);

    m_end_sector_spin = new QSpinBox();
    m_end_sector_spin->setMinimum(m_ped_start_sector);
    m_end_sector_spin->setMaximum(m_ped_end_sector);
    m_end_sector_spin->setValue(m_ped_end_sector);
    sector_group_layout->addWidget(m_end_sector_spin);

    connect(m_start_sector_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(recalculate(int)));

    connect(m_end_sector_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(recalculate(int)));

    connect(m_size_group, SIGNAL(clicked(bool)),
	    this, SLOT(sectorGroupBoxAlternate(bool)));

    connect(m_sector_group, SIGNAL(clicked(bool)),
	    this, SLOT(sizeGroupBoxAlternate(bool)));
}

void PartitionAddDialog::commit_partition()
{

    if( m_sector_group->isChecked() ){

        m_ped_start_sector = m_start_sector_spin->value();
        m_ped_end_sector   = m_end_sector_spin->value();

    }
    else{

	m_ped_start_sector += ( m_start_size_spin->value() * 1024 * 1024) / m_ped_sector_size;
	m_ped_end_sector   -= ( m_end_size_spin->value()   * 1024 * 1024) / m_ped_sector_size;

	if( m_align64_check->isChecked() ){

	    PedAlignment *ped_start_alignment = new PedAlignment();
	    ped_start_alignment->offset = 0;
	    ped_start_alignment->grain_size = 64;

	    PedAlignment *ped_end_alignment = new PedAlignment();
	    ped_end_alignment->offset = 0;
	    ped_end_alignment->grain_size = 1;
	    
	    PedGeometry *start_geom = new PedGeometry();
	    PedGeometry *end_geom   = new PedGeometry();
	    
	    ped_geometry_init(start_geom, m_ped_disk->dev, m_ped_start_sector, 127);
	    ped_geometry_init(end_geom, m_ped_disk->dev, m_ped_start_sector, m_ped_sector_length);
	    
	    PedConstraint *ped_constraint64 = new PedConstraint;
	    
	    ped_constraint64->start_align = ped_start_alignment;
	    ped_constraint64->end_align   = ped_end_alignment;
	    ped_constraint64->start_range = start_geom;
	    ped_constraint64->end_range   = end_geom;
	    ped_constraint64->min_size    = 64;
	    ped_constraint64->max_size    = m_ped_sector_length;
	    
	    m_ped_constraints = ped_constraint_intersect(ped_constraint64, m_ped_constraints);
	}
    }

    PedPartitionType type = PED_PARTITION_NORMAL ;

    PedPartition *ped_new_partition = ped_partition_new(m_ped_disk, 
							type, 
							0, 
							m_ped_start_sector, 
							m_ped_end_sector);

    int error;

    error = ped_disk_add_partition(m_ped_disk, ped_new_partition, m_ped_constraints);
    qDebug("Add part error: %d", error);

    error = ped_disk_commit(m_ped_disk);
    qDebug("Commit error: %d", error);

}

void PartitionAddDialog::recalculate(int)
{

    m_start_sector_spin->setMaximum( m_end_sector_spin->value() );

    m_end_sector_spin->setMinimum( m_start_sector_spin->value() );

}

/* called when the sector group box is clicked  */
void PartitionAddDialog::sizeGroupBoxAlternate(bool checked)
{
    if(checked)
        m_size_group->setChecked(false);
    else
        m_size_group->setChecked(true);
}

/* called when the size group box is clicked  */
void PartitionAddDialog::sectorGroupBoxAlternate(bool checked)
{
    if(checked)
        m_sector_group->setChecked(false);
    else
        m_sector_group->setChecked(true);
}
