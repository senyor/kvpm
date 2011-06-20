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

#include "dualselectorbox.h"
#include "partadd.h"
#include "partaddgraphic.h"
#include "pedexceptions.h"
#include "misc.h"
#include "sizeselectorbox.h"

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

PartitionAddDialog::PartitionAddDialog(StoragePartition *partition, QWidget *parent) : 
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
    m_sector_size = ped_device->sector_size;


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
    m_max_part_start  = ped_geometry.start;
    m_max_part_end    = ped_geometry.end;
    m_max_part_size   = ped_geometry.length;

    getMaximumPartition();

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

    m_preceding_label = new QLabel();
    layout->addWidget( m_preceding_label );

    m_remaining_label = new QLabel();
    layout->addWidget( m_remaining_label );

    QString total_bytes = sizeToString( m_max_part_size * m_sector_size );
    QLabel *excluded_label = new QLabel( i18n("Maximum size: %1",  total_bytes) );
    layout->addWidget( excluded_label );

    total_bytes = sizeToString(m_max_part_size * m_sector_size);
    m_unexcluded_label = new QLabel( i18n("Remaining space: %1", total_bytes) );
    layout->addWidget( m_unexcluded_label );

    m_dual_selector = new DualSelectorBox(m_sector_size, 0, m_max_part_size, m_max_part_size, 0, m_max_part_size, 0);

    validateChange();

    layout->addWidget(m_dual_selector);

    connect(m_dual_selector, SIGNAL(changed()),
            this, SLOT(validateChange()));

    connect(this, SIGNAL(okClicked()),
	    this, SLOT(commitPartition()));
}

void PartitionAddDialog::commitPartition()
{

    PedDevice *device = m_ped_disk->dev;
    PedPartitionType type;
    PedSector first_sector = m_max_part_start + m_dual_selector->getCurrentOffset();
    PedSector last_sector  = first_sector + m_dual_selector->getCurrentSize() - 1;
    const PedSector length  = (last_sector - first_sector) + 1;

    const PedSector sectors_1MiB  = 0x100000 / m_sector_size;   // sectors per megabyte

    if( first_sector < m_max_part_start )
        first_sector = m_max_part_start;

    if( last_sector > m_max_part_end )
        last_sector = m_max_part_end;

    if( length < ( 2 * sectors_1MiB ) )
        return;

    PedAlignment *start_align  = ped_alignment_new( 0, sectors_1MiB); 
    PedAlignment *end_align    = ped_alignment_new(-1, sectors_1MiB);

    PedGeometry *geom_1MiB = ped_geometry_new(device, m_max_part_start, m_max_part_size);
    first_sector = ped_alignment_align_down(start_align, geom_1MiB, first_sector);
    last_sector  = first_sector + length - 1;
    if( last_sector > m_max_part_end )
        last_sector = m_max_part_end;

    PedGeometry *start_range   = ped_geometry_new(device, first_sector, 1 + m_max_part_end - first_sector);
    PedGeometry *end_range   = ped_geometry_new(device, first_sector, 1 + m_max_part_end - first_sector);
 
    PedConstraint *constraint = ped_constraint_new( start_align, end_align,
                                                    start_range, end_range,
                                                    length - sectors_1MiB, length );

    if(m_type_combo->currentIndex() == 0)
        type = PED_PARTITION_NORMAL ;
    else if(m_type_combo->currentIndex() == 1)
        type = PED_PARTITION_EXTENDED ;
    else
        type = PED_PARTITION_LOGICAL ;

    PedPartition *ped_new_partition = ped_partition_new(m_ped_disk, type, 0, first_sector, last_sector);

    if( ped_disk_add_partition(m_ped_disk, ped_new_partition, constraint) )
        ped_disk_commit(m_ped_disk);
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

    partition_size /= m_sector_size;

    if (partition_size < 0)
        partition_size = 0;

    return qRound64(partition_size);
}

void PartitionAddDialog::validateChange(){

    const PedSector sectors_1MiB  = 0x100000 / m_sector_size;   // sectors per megabyte
    const long long offset = m_dual_selector->getCurrentOffset();
    const long long size   = m_dual_selector->getCurrentSize();

  
        if(( offset <= m_max_part_size - size ) && ( size <= m_max_part_size - offset  ) && 
           ( size >= 2 * sectors_1MiB ) && ( m_dual_selector->isValid() ) ){
            enableButtonOk(true);
    }
    else
        enableButtonOk(false);

    updatePartition();
}

void PartitionAddDialog::updatePartition(){

    long long offset = m_dual_selector->getCurrentOffset();
    long long size = m_dual_selector->getCurrentSize();
    long long free = m_max_part_size - offset;
    if(free < 0)
        free = 0;

    QString total_bytes = sizeToString(m_sector_size * free);
    m_unexcluded_label->setText( i18n("Available space: %1", total_bytes) );

    QString preceding_bytes_string = sizeToString(offset * m_sector_size);
    m_preceding_label->setText( i18n("Preceding space: %1", preceding_bytes_string) );

    PedSector following = m_max_part_size - (size + offset);
    if(following < 0)
        following = 0;

    PedSector following_space = following * m_sector_size;

    if( following_space < 32 * m_sector_size )
        following_space = 0;

    QString following_bytes_string = sizeToString(following_space);
    m_remaining_label->setText( i18n("Following space: %1", following_bytes_string) );

    m_display_graphic->setPrecedingSectors(offset);
    m_display_graphic->setPartitionSectors(size);
    m_display_graphic->setFollowingSectors(following);
    m_display_graphic->repaint();
}

void PartitionAddDialog::getMaximumPartition()
{
    PedPartition *ped_free_part = m_partition->getPedPartition();

    QList<PedPartition *> parts;
    PedPartition *next_part = NULL;
    bool is_logical;
    int index, prev, next;
    long long max_end = m_max_part_start + m_max_part_size - 1;
    const PedSector sectors_1MiB  = 0x100000 / m_sector_size;   // sectors per megabyte

    while( (next_part = ped_disk_next_partition(m_ped_disk, next_part) ) )
        parts.append(next_part);

    is_logical = ped_free_part->type & PED_PARTITION_LOGICAL;

    // look for the index of the current part
    for(index = 0; index < parts.size(); index++){
        if( parts[index]->geom.start == ped_free_part->geom.start )
            break;
    }

    if( index >= parts.size() ) // We couldn't find the current part?
        return;

    prev = index - 1;
    next = index + 1; 

    while( prev >= 0 ){
        if( is_logical == (parts[prev]->type & PED_PARTITION_LOGICAL) ){
            if( (parts[prev]->type & PED_PARTITION_METADATA) || (parts[prev]->type & PED_PARTITION_FREESPACE) ){
                if( m_max_part_start > parts[prev]->geom.start ){
                    m_max_part_start = parts[prev]->geom.start;
                }
            }
            else
                break;
        }
        else if( is_logical && (parts[prev]->type & PED_PARTITION_EXTENDED) ){  // first logical partiton needs 
            m_max_part_start += 64;                                             // start to be offset for 
            break;                                                              // alignment to account for EBR
        }
        else{
            break;
        }
        prev--;
    }

    while( next < parts.size() ){
        if( is_logical == (parts[next]->type & PED_PARTITION_LOGICAL) ){
            if( (parts[next]->type & PED_PARTITION_METADATA) || (parts[next]->type & PED_PARTITION_FREESPACE) ){
                if( max_end < parts[next]->geom.end ){
                    max_end = parts[next]->geom.end;
                }
            }
            else
                break;
        }
        else{
            break;
        }
        next++;
    }

    if(m_max_part_start < sectors_1MiB)
        m_max_part_start = sectors_1MiB;

    if(is_logical)
        m_max_part_start += 64;

    m_max_part_end = max_end;
    m_max_part_size = 1 + m_max_part_end - m_max_part_start;

    PedAlignment *align_1MiB = ped_alignment_new(0, sectors_1MiB);
    PedGeometry *geom_1MiB = ped_geometry_new(ped_free_part->disk->dev, m_max_part_start, m_max_part_size);
    m_max_part_start = ped_alignment_align_nearest( align_1MiB, geom_1MiB, m_max_part_start);

    PedAlignment *align_end = ped_alignment_new(-1, sectors_1MiB);
    m_max_part_end = ped_alignment_align_nearest( align_end, geom_1MiB, m_max_part_end);

    m_max_part_size = 1 + m_max_part_end - m_max_part_start;
}

