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
   between partitions. It can also be an *empty* extened partition however.
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
    m_excluded_sectors = 0;

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
    m_total_size_spin = new QSpinBox();
    m_total_size_spin->setRange(0,100);
    m_total_size_spin->setValue(100);
    m_total_size_spin->setSuffix("%");

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
    size_group_layout->addWidget(m_total_size_spin,2,0);

    m_preceding_label = new QLabel();
    size_group_layout->addWidget( m_preceding_label, 5, 0, 1, 2, Qt::AlignHCenter );

    m_remaining_label = new QLabel();
    size_group_layout->addWidget( m_remaining_label, 6, 0, 1, 2, Qt::AlignHCenter );

    m_excluded_group = new QGroupBox("Offset partition start");
    QGridLayout *excluded_group_layout = new QGridLayout();
    m_excluded_group->setCheckable(true);
    m_excluded_group->setChecked(false);
    m_excluded_group->setLayout(excluded_group_layout);
    layout->addWidget(m_excluded_group);

    m_excluded_combo = new KComboBox();
    m_excluded_combo->insertItem(0,"MiB");
    m_excluded_combo->insertItem(1,"GiB");
    m_excluded_combo->insertItem(2,"TiB");
    m_excluded_combo->setInsertPolicy(KComboBox::NoInsert);
    m_excluded_combo->setCurrentIndex(1);

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

    setExcludedEditToSpin(0);
    setSizeEditToSpin(100);

    connect(m_size_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(setSizeEditToCombo(int)));

    connect(m_total_size_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(setSizeEditToSpin(int)));

    connect(m_size_edit, SIGNAL(textChanged(QString)),
	    this, SLOT(validate()));

    connect(m_excluded_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(setExcludedEditToCombo(int)));

    connect(m_excluded_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(setExcludedEditToSpin(int)));

    connect(m_excluded_group, SIGNAL(toggled(bool)),
	    this, SLOT(clearExcludedGroup(bool)));

    connect(m_lock_size_check, SIGNAL(toggled(bool)),
	    this, SLOT(lockSize(bool)));

    connect(m_excluded_edit, SIGNAL(textChanged(QString)),
	    this, SLOT(setSizeEditMinusExcludedEdit(QString)));

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

    if( m_excluded_group->isChecked() )
        first_sector = m_ped_start_sector + m_excluded_sectors;

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

void PartitionAddDialog::setSizeEditMinusExcludedEdit(QString excluded){

    long long free;
    long long partition;
    long double sized;
    int index = m_size_combo->currentIndex();

    if( ! m_lock_size_check->isChecked() ){
        m_excluded_sectors = convertSizeToSectors(m_excluded_combo->currentIndex(), excluded.toDouble());
        free = m_ped_sector_length - m_excluded_sectors;

        if( free < 1 )
            free = 0;

        partition = convertSizeToSectors(m_size_combo->currentIndex(), m_size_edit->text().toDouble());
        if( partition > free )
            partition = free;

        sized = ((long double)partition * m_ped_sector_size);

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

    validate();
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
}


bool PartitionAddDialog::validatePartitionSize(QString size){

    long long free_sectors = m_ped_sector_length - m_excluded_sectors;
    int x = 0;

    if(m_size_validator->validate(size, x) == QValidator::Acceptable){

        m_partition_sectors = convertSizeToSectors( m_size_combo->currentIndex(), size.toDouble() );

        if( m_partition_sectors > free_sectors )
            m_partition_sectors = free_sectors;
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

void PartitionAddDialog::setExcludedEditToSpin(int percentage){

    long long free = m_ped_sector_length;
    long long excluded;

    if(percentage == 100)
        excluded = free;
    else if(percentage == 0)
        excluded = 0;
    else
        excluded = (long long)(( (double) percentage / 100) * free);

    long double sized;
    int index = m_excluded_combo->currentIndex();

    sized = ((long double)excluded * m_ped_sector_size);

    if(index == 0)
        sized /= (long double)0x100000;
    else if(index == 1)
        sized /= (long double)0x40000000;
    else{
        sized /= (long double)(0x100000);
        sized /= (long double)(0x100000);
    }

    m_excluded_edit->setText(QString("%1").arg((double)sized));
}

bool PartitionAddDialog::excludedIsValid(){

    QString excluded_edit_text = m_excluded_edit->text();
    int x = 0;
    int index = m_excluded_combo->currentIndex();

    long long free_sectors = m_ped_sector_length;
    long long partition_sectors;

    if(index == 0)
        partition_sectors = (long long) (excluded_edit_text.toDouble() * 0x100000);
    else if(index == 1)
        partition_sectors = (long long) (excluded_edit_text.toDouble() * 0x40000000);
    else{
        partition_sectors = (long long) (excluded_edit_text.toDouble() * 0x100000);
        partition_sectors *= 0x100000;
    }

    partition_sectors /= m_ped_sector_size;

    if(( m_excluded_validator->validate(excluded_edit_text, x) == QValidator::Acceptable ) &&
       ( partition_sectors <= free_sectors )){
        return true;
    }

    return false;
}

bool PartitionAddDialog::sizeIsValid(){

    QString size_edit_text = m_size_edit->text();
    int x = 0;
    int index = m_size_combo->currentIndex();

    long long free_sectors = m_ped_sector_length - m_excluded_sectors;
    long long partition_sectors;

    if(index == 0)
        partition_sectors = (long long) (size_edit_text.toDouble() * 0x100000);
    else if(index == 1)
        partition_sectors = (long long) (size_edit_text.toDouble() * 0x40000000);
    else{
        partition_sectors = (long long) (size_edit_text.toDouble() * 0x100000);
        partition_sectors *= 0x100000;
    }

    partition_sectors /= m_ped_sector_size;

    if(( m_size_validator->validate(size_edit_text, x) == QValidator::Acceptable ) &&
       ( partition_sectors <= free_sectors ) &&
       ( partition_sectors > 1 )){
        return true;
    }

    return false;
}

void PartitionAddDialog::setExcludedEditToCombo(int index){

    long double sized;

    sized = ((long double)m_excluded_sectors * m_ped_sector_size);

    if(index == 0)
        sized /= (long double)0x100000;
    else if(index == 1)
        sized /= (long double)0x40000000;
    else{
        sized /= (long double)(0x100000);
        sized /= (long double)(0x100000);
    }

    m_excluded_edit->setText(QString("%1").arg((double)sized));
}

void PartitionAddDialog::validate(){

    QString excluded_edit_text = m_excluded_edit->text();
    QString size_edit_text = m_size_edit->text();
    enableButtonOk(false);

    if( excludedIsValid() ){
        m_excluded_sectors = convertSizeToSectors( m_excluded_combo->currentIndex(), excluded_edit_text.toDouble() );
        if( sizeIsValid() ){
            m_partition_sectors = convertSizeToSectors( m_size_combo->currentIndex(), size_edit_text.toDouble() );
            enableButtonOk(true);
        }
    }

    updatePartition();
}

void PartitionAddDialog::setSizeEditToSpin(int percentage){

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

    setSizeEditToCombo( m_size_combo->currentIndex() );

    updatePartition();

}

/* if the excluded group is unchecked then zero the excluded sectors */

void PartitionAddDialog::clearExcludedGroup(bool on){

    if( ! on ){
	m_excluded_spin->setValue(0);
        m_excluded_edit->setText("0");
    }
}

void PartitionAddDialog::lockSize(bool checked){
    if(checked){
        m_size_edit->setEnabled(false);
        m_total_size_spin->setEnabled(false);
        m_size_combo->setEnabled(false);
        m_excluded_spin->setMaximum( 100 - m_total_size_spin->value() );
    }
    else{
        m_size_edit->setEnabled(true);
        m_total_size_spin->setEnabled(true);
        m_size_combo->setEnabled(true);
        m_excluded_spin->setMaximum(100);
    }
}

void PartitionAddDialog::updatePartition(){

    long long free_sectors = m_ped_sector_length - m_excluded_sectors;
    if(free_sectors < 0)
        free_sectors = 0;

    QString total_bytes = sizeToString(m_ped_sector_size * free_sectors);
    m_unexcluded_label->setText( i18n("Available space: %1", total_bytes) );

    QString preceding_bytes_string = sizeToString(m_excluded_sectors * m_ped_sector_size);
    m_preceding_label->setText( i18n("Preceding space: %1", preceding_bytes_string) );

    PedSector following_sectors = m_ped_sector_length - (m_partition_sectors + m_excluded_sectors);
    if(following_sectors < 0)
        following_sectors = 0;

    PedSector following_space = following_sectors * m_ped_sector_size;

    if( following_space < 32 * m_ped_sector_size )
        following_space = 0;

    QString following_bytes_string = sizeToString(following_space);
    m_remaining_label->setText( i18n("Following space: %1", following_bytes_string) );

    m_display_graphic->setPrecedingSectors(m_excluded_sectors);
    m_display_graphic->setPartitionSectors(m_partition_sectors);
    m_display_graphic->setFollowingSectors(following_sectors);
    m_display_graphic->repaint();
}
