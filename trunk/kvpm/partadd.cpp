/*
 *
 * 
 * Copyright (C) 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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
#include "pedexceptions.h"
#include "misc.h"

/* 
   The "partition" we get here is usually a ped pointer to a the freespace 
   between partitions. It can also be an *empty* extended partition however.
*/

bool add_partition(StoragePartition *partition)
{
    PedDisk *disk = partition->getPedPartition()->disk;
    unsigned ped_type = partition->getPedType();
    bool logical_freespace = ( ped_type & PED_PARTITION_FREESPACE ) && ( ped_type & PED_PARTITION_LOGICAL );
    int count = ped_disk_get_primary_partition_count(disk);
    int max_count = ped_disk_get_max_primary_partition_count(disk);

    if( count >= max_count  && ( ! ( logical_freespace || ( ped_type & PED_PARTITION_EXTENDED ) ) ) ){
        KMessageBox::error(0, i18n("Disk already has %1 primary partitions, the maximum", count));
        return false;
    }
    else if( ( ped_type & PED_PARTITION_EXTENDED ) && ( ! partition->isEmpty() ) ){
        KMessageBox::error(0, i18n("This should not happen. Try selecting the freespace and not the partiton itself"));
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

    PedPartition *ped_free_partition = partition->getPedPartition();
    PedGeometry   ped_geometry;
    PedDisk      *ped_disk  = ped_free_partition->disk;
    PedDisk      *temp_disk = ped_disk_new( ped_disk->dev );
    PedDevice    *ped_device;
    bool logical_freespace;      // true if we are inside an extended partition
    bool extended_allowed;       // true if we can create an extended partition here

    // If this is an empty extended partition and not freespace inside
    // one then look for the freespace.

    if( ped_free_partition->type & PED_PARTITION_EXTENDED ){
        do{
            ped_free_partition = ped_disk_next_partition (temp_disk, ped_free_partition);
            if( ! ped_free_partition )
                qDebug() << "Extended partition with no freespace found!";
        }
        while( !((ped_free_partition->type & PED_PARTITION_FREESPACE) && (ped_free_partition->type & PED_PARTITION_LOGICAL)) );
    }

    ped_geometry = ped_free_partition->geom;
    m_ped_disk   = ped_free_partition->disk;
    ped_device   = m_ped_disk->dev;
    m_ped_sector_size = ped_device->sector_size;
    m_offset_sectors = 0;

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
    m_ped_constraints   = ped_device_get_constraint(ped_device); 
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

    m_lock_size_check = new QCheckBox("Lock partition size");
    size_group_layout->addWidget(m_lock_size_check);
    m_size_spin = new QSpinBox();
    m_size_spin->setRange(0,100);
    m_size_spin->setValue(100);
    m_size_spin->setSuffix("%");

    m_size_edit  = new KLineEdit();
    m_size_validator = new QDoubleValidator(m_size_edit);
    m_size_edit->setValidator(m_size_validator);
    m_size_validator->setBottom(0);

    m_size_combo = new KComboBox();
    m_size_combo->insertItem(0,"MiB");
    m_size_combo->insertItem(1,"GiB");
    m_size_combo->insertItem(2,"TiB");
    m_size_combo->setInsertPolicy(KComboBox::NoInsert);
    m_size_combo->setCurrentIndex(1);

    size_group_layout->addWidget(m_size_edit,1,0);
    size_group_layout->addWidget(m_size_combo,1,1);
    size_group_layout->addWidget(m_size_spin,2,0);

    m_preceding_label = new QLabel();
    size_group_layout->addWidget( m_preceding_label, 5, 0, 1, 2, Qt::AlignHCenter );

    m_remaining_label = new QLabel();
    size_group_layout->addWidget( m_remaining_label, 6, 0, 1, 2, Qt::AlignHCenter );

    m_offset_group = new QGroupBox("Offset partition start");
    QGridLayout *excluded_group_layout = new QGridLayout();
    m_offset_group->setCheckable(true);
    m_offset_group->setChecked(false);
    m_offset_group->setLayout(excluded_group_layout);
    layout->addWidget(m_offset_group);

    m_offset_combo = new KComboBox();
    m_offset_combo->insertItem(0,"MiB");
    m_offset_combo->insertItem(1,"GiB");
    m_offset_combo->insertItem(2,"TiB");
    m_offset_combo->setInsertPolicy(KComboBox::NoInsert);
    m_offset_combo->setCurrentIndex(1);

    m_offset_spin = new QSpinBox();
    m_offset_spin->setRange(0,100);
    m_offset_spin->setValue(0);
    m_offset_spin->setSuffix("%");

    m_offset_edit  = new KLineEdit();
    m_offset_validator = new QDoubleValidator(m_offset_edit);
    m_offset_edit->setValidator(m_offset_validator);
    m_offset_validator->setBottom(0);

    excluded_group_layout->addWidget(m_offset_edit,0,0);
    excluded_group_layout->addWidget(m_offset_combo,0,1);
    excluded_group_layout->addWidget(m_offset_spin,1,0);

    QString total_bytes = sizeToString( m_ped_sector_length * m_ped_sector_size );
    QLabel *excluded_label = new QLabel( i18n("Maximum size: %1",  total_bytes) );
    excluded_group_layout->addWidget( excluded_label, 4, 0, 1, 2, Qt::AlignHCenter );

    total_bytes = sizeToString(m_ped_sector_length * m_ped_sector_size);
    m_unexcluded_label = new QLabel( i18n("Remaining space: %1", total_bytes) );
    excluded_group_layout->addWidget( m_unexcluded_label, 5, 0, 1, 2, Qt::AlignHCenter );

    setOffsetEditToSpin(0);
    setSizeEditToSpin(100);

    connect(m_size_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(setSizeEditToCombo(int)));

    connect(m_size_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(setSizeEditToSpin(int)));

    connect(m_size_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validateSize(QString)));

    connect(m_offset_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(setOffsetEditToCombo(int)));

    connect(m_offset_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(setOffsetEditToSpin(int)));

    connect(m_offset_group, SIGNAL(toggled(bool)),
	    this, SLOT(clearOffsetGroup(bool)));

    connect(m_lock_size_check, SIGNAL(toggled(bool)),
	    this, SLOT(lockSize(bool)));

    connect(m_offset_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validateOffset(QString)));

    connect(this, SIGNAL(okClicked()),
	    this, SLOT(commitPartition()));
}

void PartitionAddDialog::commitPartition()
{

    PedPartitionType type;
    PedSector first_sector = m_ped_start_sector;
    int sectors4KiB;
    int sectors64KiB;
    int sectors1MiB;

    if( m_offset_group->isChecked() )
        first_sector = m_ped_start_sector + m_offset_sectors;

    PedSector last_sector  = first_sector + m_partition_sectors;
    if( last_sector > m_ped_end_sector )
        last_sector = m_ped_end_sector;
    PedSector length  = (last_sector - first_sector) + 1;

    if( ( (int)(0x1000 / m_ped_sector_size) ) > 0 )
        sectors4KiB = 0x1000 / m_ped_sector_size;
    else
        sectors4KiB = 1;

    if( ( (int)(0x10000 / m_ped_sector_size) ) > 0 )
        sectors64KiB = 0x10000 / m_ped_sector_size;
    else
        sectors64KiB = 1;

    if( ( (int)(0x100000 / m_ped_sector_size) ) > 0 )
        sectors1MiB = 0x100000 / m_ped_sector_size;
    else
        sectors1MiB = 1;

    PedAlignment *ped_start_alignment4KiB  = ped_alignment_new(0, sectors4KiB);
    PedAlignment *ped_start_alignment64KiB = ped_alignment_new(0, sectors64KiB);
    PedAlignment *ped_start_alignment1MiB  = ped_alignment_new(0, sectors1MiB);
    PedAlignment *ped_end_alignment   = ped_alignment_new(0, 1);

    PedGeometry *start_geom4KiB  = ped_geometry_new(m_ped_disk->dev, first_sector, (2 * sectors4KiB) - 1);
    PedGeometry *start_geom64KiB = ped_geometry_new(m_ped_disk->dev, first_sector, (2 * sectors64KiB) - 1);
    PedGeometry *start_geom1MiB  = ped_geometry_new(m_ped_disk->dev, first_sector, (2 * sectors1MiB) - 1);
    PedGeometry *end_geom   = ped_geometry_new(m_ped_disk->dev, first_sector, length);

    PedConstraint *ped_constraint4KiB = ped_constraint_new(ped_start_alignment4KiB,
                                                           ped_end_alignment, 
                                                           start_geom4KiB, 
                                                           end_geom, 
                                                           (2 * sectors4KiB) - 1, 
                                                           length);

    PedConstraint *ped_constraint64KiB = ped_constraint_new(ped_start_alignment64KiB,
                                                            ped_end_alignment, 
                                                            start_geom64KiB, 
                                                            end_geom, 
                                                            (2 * sectors64KiB) - 1, 
                                                            length);

    PedConstraint *ped_constraint1MiB = ped_constraint_new(ped_start_alignment1MiB,
                                                           ped_end_alignment, 
                                                           start_geom1MiB, 
                                                           end_geom, 
                                                           (2 * sectors1MiB) - 1, 
                                                           length);

    ped_constraint4KiB  = ped_constraint_intersect(ped_constraint4KiB,  m_ped_constraints);
    ped_constraint64KiB = ped_constraint_intersect(ped_constraint64KiB, m_ped_constraints);
    ped_constraint1MiB  = ped_constraint_intersect(ped_constraint1MiB,  m_ped_constraints);

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

    ped_exception_set_handler(my_constraint_handler);

    if( ped_disk_add_partition(m_ped_disk, ped_new_partition, ped_constraint1MiB) ){
        ped_exception_set_handler(my_handler);
        ped_disk_commit(m_ped_disk);
    }
    else if( ped_disk_add_partition(m_ped_disk, ped_new_partition, ped_constraint64KiB) ){
        ped_exception_set_handler(my_handler);
        ped_disk_commit(m_ped_disk);
    }
    else{
        ped_exception_set_handler(my_handler);
        ped_disk_add_partition(m_ped_disk, ped_new_partition, ped_constraint4KiB);
        ped_disk_commit(m_ped_disk);
    }
}

void PartitionAddDialog::setSizeEditMinusOffsetEdit(){

    long long free;
    long double sized;
    int index = m_size_combo->currentIndex();
    QString excluded = m_offset_edit->text();

    if( ! m_lock_size_check->isChecked() ){
        m_offset_sectors = convertSizeToSectors(m_offset_combo->currentIndex(), excluded.toDouble());
        free = m_ped_sector_length - m_offset_sectors;

        if( free < 1 ){
            free = 0;
            m_offset_sectors = m_ped_sector_length; 
        }

        if( m_partition_sectors > free )
            m_partition_sectors = free;

        m_size_spin->setMaximum( qRound( ( (double)free / m_ped_sector_length ) * 100) );

        sized = ((long double)m_partition_sectors * m_ped_sector_size);

        if(index == 0)
            sized /= (long double)0x100000;
        else if(index == 1)
            sized /= (long double)0x40000000;
        else{
            sized /= (long double)(0x100000);
            sized /= (long double)(0x100000);
        }
        
        m_size_edit->setText( QString("%1").arg( (double)sized ) );
    }
    else{
        m_offset_sectors = convertSizeToSectors(m_offset_combo->currentIndex(), excluded.toDouble());
    
        if(m_offset_sectors > (m_ped_sector_length - m_partition_sectors))
            m_offset_sectors = m_ped_sector_length - m_partition_sectors;
    }

    validateChange();
}

void PartitionAddDialog::setSizeEditToCombo(int index){

    long double sized;

    sized = ((long double)m_partition_sectors * m_ped_sector_size);

    if(index == 0)
        sized /= (long double)0x100000;
    else if(index == 1)
        sized /= (long double)0x40000000;
    else{
        sized /= (long double)(0x100000);
        sized /= (long double)(0x100000);
    }

    m_size_edit->setText( QString("%1").arg( (double)sized ) );
    validateChange();
}

long long PartitionAddDialog::convertSizeToSectors(int index, double size)
{
    long double partition_size = size;

    if(index == 0)
        partition_size *= (long double)0x100000;
    else if(index == 1)
        partition_size *= (long double)0x40000000;
    else{
        partition_size *= (long double)0x100000;
        partition_size *= (long double)0x100000;
    }

    partition_size /= m_ped_sector_size;

    if (partition_size < 0)
        partition_size = 0;

    return qRound64(partition_size);
}

void PartitionAddDialog::setOffsetEditToSpin(int percentage){

    long long free = m_ped_sector_length;
    long long excluded;

    if(percentage == 100)
        excluded = free;
    else if(percentage == 0)
        excluded = 0;
    else if(percentage == m_offset_spin->maximum())
        excluded = m_ped_sector_length - m_partition_sectors;
    else
        excluded = (long long)(( (double) percentage / 100) * free);

    long double sized;
    int index = m_offset_combo->currentIndex();

    sized = ((long double)excluded * m_ped_sector_size);

    if(index == 0)
        sized /= (long double)0x100000;
    else if(index == 1)
        sized /= (long double)0x40000000;
    else{
        sized /= (long double)(0x100000);
        sized /= (long double)(0x100000);
    }

    m_offset_edit->setText(QString("%1").arg((double)sized));
    setSizeEditMinusOffsetEdit();
    validateChange();

}

void PartitionAddDialog::validateSize(QString text){

    int x = 0;
    long long free = m_ped_sector_length - m_offset_sectors;
    long long partition = convertSizeToSectors( m_size_combo->currentIndex(), text.toDouble() );

    if(( m_size_validator->validate(text, x) == QValidator::Acceptable )){
            
        if( partition <= free ){
            m_partition_sectors = partition;
            enableButtonOk(true);
        }
        else if( partition <= m_ped_sector_length ){
            m_partition_sectors = partition;
            enableButtonOk(false);
        }
        else
            enableButtonOk(false);
    }
    else
        enableButtonOk(false);

    if(( m_offset_sectors > m_ped_sector_length - m_partition_sectors ) || 
       ( m_partition_sectors > m_ped_sector_length - m_offset_sectors   ) || 
       ( m_partition_sectors <= 0 )){
        enableButtonOk(false);
    }

    updatePartition();
}


void PartitionAddDialog::validateOffset(QString text){

    int x = 0;
    long long free = m_ped_sector_length - m_partition_sectors;
    long long offset = convertSizeToSectors( m_offset_combo->currentIndex(), text.toDouble() );

    if( m_offset_validator->validate(text, x) == QValidator::Acceptable ){

       if( offset <= free ){
           m_offset_sectors = offset;
           enableButtonOk(true);
       }
       else if( offset <= m_ped_sector_length ){
           m_offset_sectors = offset;
           setSizeEditMinusOffsetEdit();
           enableButtonOk(true);
       }
       else
           enableButtonOk(false);
    }
    else
        enableButtonOk(false);

    if(( m_offset_sectors > m_ped_sector_length - m_partition_sectors ) || 
       ( m_partition_sectors > m_ped_sector_length - m_offset_sectors   ) || 
       ( m_partition_sectors <= 0 )){
        enableButtonOk(false);
    }
    
    updatePartition();
}

void PartitionAddDialog::validateChange(){

    if(( m_offset_sectors <= m_ped_sector_length - m_partition_sectors ) && 
       ( m_partition_sectors <= m_ped_sector_length - m_offset_sectors   ) &&
       ( m_partition_sectors > 0 )){
        enableButtonOk(true);
    }
    else
        enableButtonOk(false);

    updatePartition();
}

void PartitionAddDialog::setOffsetEditToCombo(int index){

    long double sized = ((long double)m_offset_sectors * m_ped_sector_size);

    if(index == 0)
        sized /= (long double)0x100000;
    else if(index == 1)
        sized /= (long double)0x40000000;
    else{
        sized /= (long double)(0x100000);
        sized /= (long double)(0x100000);
    }

    m_offset_edit->setText(QString("%1").arg((double)sized));
    setSizeEditMinusOffsetEdit();
    validateChange();
}

void PartitionAddDialog::setSizeEditToSpin(int percentage){

    long long free_sectors = m_ped_sector_length - m_offset_sectors;

    if( percentage == m_size_spin->maximum() )
        m_partition_sectors = free_sectors;
    else if(percentage == 0)
        m_partition_sectors = 0;
    else
        m_partition_sectors = (long long)(( (double) percentage / 100) * m_ped_sector_length);

    if( m_partition_sectors > free_sectors)
        m_partition_sectors = free_sectors;

    setSizeEditToCombo( m_size_combo->currentIndex() );
}

/* if the excluded group is unchecked then zero the excluded sectors */

void PartitionAddDialog::clearOffsetGroup(bool on){

    if( ! on ){
	m_offset_spin->setValue(0);
        m_offset_edit->setText("0");
        setSizeEditMinusOffsetEdit();
    }

    validateChange();
}

void PartitionAddDialog::lockSize(bool checked){

    int percent_free = qRound((100 * ((double)(m_ped_sector_length - m_partition_sectors))) / m_ped_sector_length);

    if(checked){
        m_size_edit->setEnabled(false);
        m_size_spin->setEnabled(false);
        m_size_combo->setEnabled(false);
        m_offset_spin->setMaximum(percent_free);
    }
    else{
        m_size_edit->setEnabled(true);
        m_size_spin->setEnabled(true);
        m_size_combo->setEnabled(true);
        m_offset_spin->setMaximum(100);
    }

    validateChange();
}

void PartitionAddDialog::updatePartition(){

    long long free_sectors = m_ped_sector_length - m_offset_sectors;
    if(free_sectors < 0)
        free_sectors = 0;

    QString total_bytes = sizeToString(m_ped_sector_size * free_sectors);
    m_unexcluded_label->setText( i18n("Available space: %1", total_bytes) );

    QString preceding_bytes_string = sizeToString(m_offset_sectors * m_ped_sector_size);
    m_preceding_label->setText( i18n("Preceding space: %1", preceding_bytes_string) );

    PedSector following_sectors = m_ped_sector_length - (m_partition_sectors + m_offset_sectors);
    if(following_sectors < 0)
        following_sectors = 0;

    PedSector following_space = following_sectors * m_ped_sector_size;

    if( following_space < 32 * m_ped_sector_size )
        following_space = 0;

    QString following_bytes_string = sizeToString(following_space);
    m_remaining_label->setText( i18n("Following space: %1", following_bytes_string) );

    m_display_graphic->setPrecedingSectors(m_offset_sectors);
    m_display_graphic->setPartitionSectors(m_partition_sectors);
    m_display_graphic->setFollowingSectors(following_sectors);
    m_display_graphic->repaint();
}
