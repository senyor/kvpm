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
#include "partaddgraphic.h"
#include "sizetostring.h"


bool add_partition(StoragePartition *partition)
{
    PedDisk *disk = partition->getPedPartition()->disk;
    bool logical_freespace = ( partition->getPedType() & 0x04 ) &&  ( partition->getPedType() & 0x01 );
    int count = ped_disk_get_primary_partition_count(disk);

    if( count >= ped_disk_get_max_primary_partition_count(disk)  && ( ! logical_freespace ) ){
        KMessageBox::error(0, i18n("Disk already has %1 primary partitions, the maximum", count));
        return false;
    }
    else{
        PartitionAddDialog dialog(partition);
        dialog.exec();
    
        if(dialog.result() == QDialog::Accepted)
            return true;
        else
            return false;
    }
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
    bool extended_allowed;       // true if we can create an extended partition here


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

    QFrame *display_graphic_frame = new QFrame();
    QVBoxLayout *display_graphic_layout = new QVBoxLayout();
    display_graphic_layout->setSpacing(0);
    display_graphic_layout->setMargin(0);
    display_graphic_frame->setFrameStyle(QFrame::Sunken | QFrame::Panel);
    display_graphic_frame->setLineWidth(2);
    display_graphic_frame->setLayout(display_graphic_layout);

    m_display_graphic = new PartAddGraphic();
    display_graphic_layout->addWidget(m_display_graphic);
    layout->addWidget(display_graphic_frame, 0, Qt::AlignCenter);
 
    QLabel *path = new QLabel("<b>" + m_partition->getName() + "</b>");
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
    m_align64_check = new QCheckBox("Align to 64 KiB");
    size_group_layout->addWidget(m_align64_check, 3, 0, 1, 2, Qt::AlignHCenter );

    m_preceding_label = new QLabel();
    size_group_layout->addWidget( m_preceding_label, 5, 0, 1, 2, Qt::AlignHCenter );

    m_remaining_label = new QLabel();
    size_group_layout->addWidget( m_remaining_label, 6, 0, 1, 2, Qt::AlignHCenter );

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

    excluded_group_layout->addWidget(m_excluded_edit,0,0);
    excluded_group_layout->addWidget(m_excluded_combo,0,1);
    excluded_group_layout->addWidget(m_excluded_spin,1,0);

    QString total_bytes = sizeToString( m_ped_sector_length * m_ped_sector_size );
    QLabel *excluded_label = new QLabel( i18n("Maximum size: %1",  total_bytes) );
    excluded_group_layout->addWidget( excluded_label, 4, 0, 1, 2, Qt::AlignHCenter );

    total_bytes = sizeToString(m_ped_sector_length * m_ped_sector_size);
    m_unexcluded_label = new QLabel( i18n("Remaining space: %1", total_bytes) );
    excluded_group_layout->addWidget( m_unexcluded_label, 5, 0, 1, 2, Qt::AlignHCenter );

    adjustExcludedEdit(0);
    adjustSizeEdit(100);

    connect(m_size_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(adjustSizeCombo(int)));

    connect(m_total_size_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(adjustSizeEdit(int)));

    connect(m_excluded_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(adjustExcludedCombo(int)));

    connect(m_excluded_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(adjustExcludedEdit(int)));

    connect(m_excluded_group, SIGNAL(toggled(bool)),
	    this, SLOT(clearExcludedGroup(bool)));

    connect(this, SIGNAL(okClicked()),
	    this, SLOT(commitPartition()));

    connect(m_size_edit, SIGNAL(textChanged(QString)),
	    this, SLOT(validate()));

    connect(m_excluded_edit, SIGNAL(textChanged(QString)),
	    this, SLOT(validate()));
}

void PartitionAddDialog::commitPartition()
{

    PedPartitionType type;
    PedSector first_sector = m_ped_start_sector;
    int sectors64kib;

    if( m_excluded_group->isChecked() )
        first_sector = m_ped_start_sector + m_excluded_sectors;

    PedSector last_sector  = first_sector + m_partition_sectors;
    if( last_sector > m_ped_end_sector )
        last_sector = m_ped_end_sector;
    PedSector length  = (last_sector - first_sector) + 1;

    if( ( (int)(0x10000 / m_ped_sector_size) ) > 0 )
        sectors64kib = 0x10000 / m_ped_sector_size;
    else
        sectors64kib = 1;

    PedAlignment *ped_start_alignment = ped_alignment_new(0, sectors64kib);
    PedAlignment *ped_end_alignment   = ped_alignment_new(0, 1);

    PedGeometry *start_geom = ped_geometry_new(m_ped_disk->dev, first_sector, (2 * sectors64kib) - 1);
    PedGeometry *end_geom   = ped_geometry_new(m_ped_disk->dev, first_sector, length);


    PedConstraint *ped_constraint64 = ped_constraint_new(ped_start_alignment,
							 ped_end_alignment, 
							 start_geom, 
							 end_geom, 
							 (2 * sectors64kib) - 1, 
							 length);
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


bool PartitionAddDialog::validatePartitionSize(QString size){

    long long free_sectors = m_ped_sector_length - m_excluded_sectors;
    int x = 0;

    const int size_combo_index = m_size_combo->currentIndex();

    if(m_size_validator->validate(size, x) == QValidator::Acceptable){

        if(!size_combo_index)
            m_partition_sectors = size.toLongLong();
        else{
            m_partition_sectors = convertSizeToSectors( size_combo_index, size.toDouble() );

            if( m_partition_sectors > free_sectors )
                m_partition_sectors = free_sectors;
        }
        return true;
    }
    else{
        m_partition_sectors = 0;
        return false;
    }

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


bool PartitionAddDialog::validateExcludedSize(QString size){

    int x = 0;

    const int excluded_combo_index = m_excluded_combo->currentIndex();

    if(m_excluded_validator->validate(size, x) == QValidator::Acceptable){

        if( ! excluded_combo_index)
            m_excluded_sectors = size.toLongLong();
        else
            m_excluded_sectors = convertSizeToSectors( excluded_combo_index, size.toDouble() );

        return true;
    }
    else{
        return false;
    }
}


void PartitionAddDialog::validate(){

    long long free_sectors = m_ped_sector_length - m_excluded_sectors;

    if( validateExcludedSize( m_excluded_edit->text() ) && 
        validatePartitionSize( m_size_edit->text() ) ){

        if( (m_partition_sectors <= free_sectors) && (m_partition_sectors > 0) )
            enableButtonOk(true);
        else
            enableButtonOk(false);
    }
    else
        enableButtonOk(false);


    updatePartition();
}

void PartitionAddDialog::adjustSizeEdit(int percentage){

    long long free_sectors = m_ped_sector_length - m_excluded_sectors;

    int top = qRound( ( (double)free_sectors / m_ped_sector_length ) * 100);

    m_total_size_spin->setMaximum( top );

    if( percentage == m_total_size_spin->maximum() )
        m_partition_sectors = free_sectors;
    else if(percentage == 0)
        m_partition_sectors = 0;
    else
        m_partition_sectors = (long long)(( (double) percentage / 100) * m_ped_sector_length);

    if( m_partition_sectors > free_sectors)
        m_partition_sectors = free_sectors;

    adjustSizeCombo( m_size_combo->currentIndex() );

    updatePartition();

}



/* if the excluded group is unchecked then zero the excluded sectors */

void PartitionAddDialog::clearExcludedGroup(bool on){

    if( ! on )
	m_excluded_spin->setValue(0);

}

void PartitionAddDialog::updatePartition(){

    long long free_sectors = m_ped_sector_length - m_excluded_sectors;

    QString total_bytes = sizeToString(m_ped_sector_size * free_sectors);
    m_unexcluded_label->setText( i18n("Available space: %1", total_bytes) );

    QString preceding_bytes_string = sizeToString(m_excluded_sectors * m_ped_sector_size);
    m_preceding_label->setText( i18n("Preceding space: %1", preceding_bytes_string) );

    PedSector following_sectors = m_ped_sector_length - (m_partition_sectors + m_excluded_sectors);
    PedSector following_space = following_sectors * m_ped_sector_size;

    if(following_space < 0)
        following_space = 0;

    QString following_bytes_string = sizeToString(following_space);
    m_remaining_label->setText( i18n("Following space: %1", following_bytes_string) );

    m_display_graphic->setPrecedingSectors(m_excluded_sectors);
    m_display_graphic->setPartitionSectors(m_partition_sectors);
    m_display_graphic->setFollowingSectors(following_sectors);
    m_display_graphic->repaint();

}


