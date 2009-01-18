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
#include <KMessageBox>

#include <QtGui>

#include "partadd.h"
#include "sizetostring.h"


bool add_partition(StoragePartition *partition)
{

    PartitionAddDialog dialog(partition);
    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted)
        return true;
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

    m_excluded_sectors = 0;

    bool logical_freespace;      // true if we are inside an extended partition
    bool extended_allowed;        // true if we can create an extended partition here


    /* check to see if partition table supports extended
       partitions and if it already has one */

    if( (PED_DISK_TYPE_EXTENDED & m_ped_disk->type->features) && 
	(! ped_disk_extended_partition(m_ped_disk) )  ){
        extended_allowed = true;
    }
    else
        extended_allowed = false;


    if( ped_free_partition->type & PED_PARTITION_LOGICAL )
        logical_freespace = true;
    else
        logical_freespace = false;


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
    QLabel *path = new QLabel("<b>" + m_partition->getPartitionPath() + "</b>");
    path->setAlignment( Qt::AlignHCenter );
    layout->addWidget(path);

    m_type_combo = new KComboBox;
    layout->addWidget(m_type_combo);

    m_type_combo->insertItem(0,"Primary");
    m_type_combo->insertItem(1,"Extended");

    if( logical_freespace){
        m_type_combo->insertItem(2,"Logical");
        m_type_combo->setEnabled(false);
	m_type_combo->setCurrentIndex(2);
    }
    else if(! extended_allowed){
	    m_type_combo->setEnabled(false);
	    m_type_combo->setCurrentIndex(0);
    }

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
    m_size_combo->insertItem(1,"MiB");
    m_size_combo->insertItem(2,"GiB");
    m_size_combo->insertItem(3,"TiB");
    m_size_combo->setInsertPolicy(KComboBox::NoInsert);
    m_size_combo->setCurrentIndex(2);

    size_group_layout->addWidget(m_size_edit,0,0);
    size_group_layout->addWidget(m_size_combo,0,1);
    size_group_layout->addWidget(m_total_size_spin,1,0);
    m_align64_check = new QCheckBox("Align to 64 sectors");
    size_group_layout->addWidget(m_align64_check, 3, 0, 1, 2, Qt::AlignHCenter );

    QString total_bytes = sizeToString(m_ped_sector_length * m_ped_sector_size);
    m_unexcluded_label = new QLabel( i18n("Available space: %1", total_bytes) );
    size_group_layout->addWidget( m_unexcluded_label, 4, 0, 1, 2, Qt::AlignHCenter );

    m_remaining_label = new QLabel();
    size_group_layout->addWidget( m_remaining_label, 5, 0, 1, 2, Qt::AlignHCenter );

    m_excluded_group = new QGroupBox("Exclude preceding space");
    QGridLayout *excluded_group_layout = new QGridLayout();
    m_excluded_group->setCheckable(true);
    m_excluded_group->setChecked(false);
    m_excluded_group->setLayout(excluded_group_layout);
    layout->addWidget(m_excluded_group);

    m_excluded_combo = new KComboBox();
    m_excluded_combo->insertItem(0,"Sectors");
    m_excluded_combo->insertItem(1,"MiB");
    m_excluded_combo->insertItem(2,"GiB");
    m_excluded_combo->insertItem(3,"TiB");
    m_excluded_combo->setInsertPolicy(KComboBox::NoInsert);
    m_excluded_combo->setCurrentIndex(2);

    m_excluded_spin = new QSpinBox();
    m_excluded_spin->setRange(0,100);
    m_excluded_spin->setValue(0);
    m_excluded_spin->setSuffix("%");

    m_excluded_edit  = new KLineEdit();
    m_excluded_validator = new QDoubleValidator(m_excluded_edit);
    m_excluded_edit->setValidator(m_excluded_validator);
    m_excluded_validator->setBottom(0);

    adjustExcludedEdit(0);
    adjustSizeEdit(100);

    excluded_group_layout->addWidget(m_excluded_edit,0,0);
    excluded_group_layout->addWidget(m_excluded_combo,0,1);
    excluded_group_layout->addWidget(m_excluded_spin,1,0);

    total_bytes = sizeToString( m_ped_sector_length * m_ped_sector_size );
    QLabel *excluded_label = new QLabel( i18n("Maximum size: %1",  total_bytes) );
    excluded_group_layout->addWidget( excluded_label, 4, 0, 1, 2, Qt::AlignHCenter );


    connect(m_size_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(adjustSizeCombo(int)));

    connect(m_size_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validateVolumeSize(QString)));

    connect(m_total_size_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(adjustSizeEdit(int)));

    connect(m_excluded_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(adjustExcludedCombo(int)));

    connect(m_excluded_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validateExcludedSize(QString)));

    connect(m_excluded_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(adjustExcludedEdit(int)));

    connect(m_excluded_group, SIGNAL(toggled(bool)),
	    this, SLOT(clearExcludedGroup(bool)));

    connect(this, SIGNAL(okClicked()),
	    this, SLOT(commit_partition()));

}

void PartitionAddDialog::commit_partition()
{

    PedPartitionType type;
    PedSector first_sector = m_ped_start_sector;

    if( m_excluded_group->isChecked() )
        first_sector = m_ped_start_sector + m_excluded_sectors;

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


    if(m_type_combo->currentIndex() == 0)
        type = PED_PARTITION_NORMAL ;
    else if(m_type_combo->currentIndex() == 1)
        type = PED_PARTITION_EXTENDED ;
    else
        type = PED_PARTITION_LOGICAL ;


    PedPartition *ped_new_partition = ped_partition_new(m_ped_disk, 
							type, 
							0, 
							first_sector, 
							(first_sector + length) -1);
    int error;  // error = 0 on failure

    error = ped_disk_add_partition(m_ped_disk, ped_new_partition, m_ped_constraints);

    if( error )
        error = ped_disk_commit(m_ped_disk);

    if( ! error )  
        KMessageBox::error( 0, "Creation of partition failed");

}


void PartitionAddDialog::adjustSizeEdit(int percentage){

    long long free_sectors = m_ped_sector_length - m_excluded_sectors;

    if(percentage == 100)
        m_partition_sectors = free_sectors;
    else if(percentage == 0)
        m_partition_sectors = 0;
    else
        m_partition_sectors = (long long)(( (double) percentage / 100) * free_sectors);

    adjustSizeCombo( m_size_combo->currentIndex() );

    QString total_bytes = sizeToString(m_ped_sector_size * free_sectors);
    m_unexcluded_label->setText( i18n("Available space: %1", total_bytes) );

    PedSector following_space = (m_ped_sector_length - (m_partition_sectors + m_excluded_sectors)) * m_ped_sector_size;

    QString following_bytes_string = sizeToString(following_space);
    m_remaining_label->setText( i18n("Remaining space: %1", following_bytes_string) );

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

    long long max_sectors = m_ped_sector_length - m_excluded_sectors;

    if( (m_partition_sectors <= max_sectors) && (m_partition_sectors > 0) )
        enableButtonOk(true);
    else
        enableButtonOk(false);

}

void PartitionAddDialog::adjustExcludedEdit(int percentage){

    long long free_sectors = m_ped_sector_length;

    if(percentage == 100)
        m_excluded_sectors = free_sectors;
    else if(percentage == 0)
        m_excluded_sectors = 0;
    else
        m_excluded_sectors = (long long)(( (double) percentage / 100) * free_sectors);

    adjustExcludedCombo( m_excluded_combo->currentIndex() );
    adjustSizeEdit(m_total_size_spin->value());
    resetOkButton();
}


void PartitionAddDialog::adjustExcludedCombo(int index){

    long double sized;
    long double valid_topd = ((long double)m_ped_sector_length * m_ped_sector_size);

    if(index){
 
        sized = ((long double)m_excluded_sectors * m_ped_sector_size);

        if(index == 1){
            sized /= (long double)0x100000;
	    valid_topd /= (long double)0x100000;
	}
        if(index == 2){
            sized /= (long double)0x40000000;
	    valid_topd /= (long double)0x40000000;
	}
        if(index == 3){
            sized /= (long double)(0x100000);
            sized /= (long double)(0x100000);
	    valid_topd /= (long double)0x100000;
	    valid_topd /= (long double)0x100000;
        }
        m_excluded_edit->setText(QString("%1").arg((double)sized));
    }
    else{
        m_excluded_edit->setText(QString("%1").arg(m_excluded_sectors));
    }

    m_excluded_validator->setTop((double)valid_topd);
}


void PartitionAddDialog::validateExcludedSize(QString size){

    int x = 0;

    const int excluded_combo_index = m_excluded_combo->currentIndex();

    if(m_excluded_validator->validate(size, x) == QValidator::Acceptable){

        if(!excluded_combo_index)
            m_excluded_sectors = size.toLongLong();
        else
            m_excluded_sectors = convertSizeToSectors( excluded_combo_index, size.toDouble() );

    }
    else{
        m_excluded_sectors = m_ped_sector_length;
    }

    adjustSizeEdit( m_total_size_spin->value() );

}

/* if the excluded group is unchecked then zero the excluded sectors */

void PartitionAddDialog::clearExcludedGroup(bool on){

    if( ! on )
	m_excluded_spin->setValue(0);

}
