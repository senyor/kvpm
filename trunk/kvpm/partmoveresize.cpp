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
#include <KProgressDialog>
#include <KPushButton>
#include <KButtonGroup>

#include <QtGui>
#include <QThread>

#include "fsextend.h"
#include "fsreduce.h"
#include "pedexceptions.h"
#include "partmoveresize.h"
#include "partaddgraphic.h"
#include "physvol.h"
#include "processprogress.h"
#include "pvextend.h"
#include "pvreduce.h"
#include "sizeselectorbox.h"
#include "misc.h"
#include "volgroup.h"


bool moveresize_partition(StoragePartition *partition)
{
    PartitionMoveResizeDialog dialog(partition);
    QString fs = partition->getFilesystem();

    QString message = i18n("Currently only the ext2/3/4 file systems "
                           "are supported for file system shrinking. "
                           "Growing is supported for ext2/3/4, jfs, xfs and Reiserfs. "
                           "Moving a partition is supported for any filesystem. "
                           "Physical volumes may also be grown, shrunk or moved");

    if( ! ( fs == "ext2" || fs == "ext3" || fs == "ext4" || fs == "reiserfs" ||
            fs == "xfs"  || fs == "jfs"  || partition->isPV() ) ){

        KMessageBox::information(0, message);
    }

    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted)
        return true;
    else
        return false;

}

PartitionMoveResizeDialog::PartitionMoveResizeDialog(StoragePartition *partition, QWidget *parent) : 
    KDialog(parent),
    m_old_storage_part(partition)
  
{
    setup();

    const QString fs = m_old_storage_part->getFilesystem();
    const long long existing_offset = m_existing_part->geom.start - m_max_part_start; 
    const long long max_offset = m_max_part_size - m_min_shrink_size;
    long long max_size;

    /*
      !!!!!!! ADD TEST FOR SHRINK PROGS !!!!!!

      Global function  canShrink() ?
      Global function  canMove() ?

    */


    if( ! ( fs == "ext2" || fs == "ext3" || fs == "ext4" || fs == "reiserfs" || 
            fs == "xfs"  || fs == "jfs"  || partition->isPV() ) )
        {
            max_size = m_existing_part->geom.length;
        }
    else
        max_size = m_max_part_size;


    setButtons( KDialog::Ok | KDialog::Cancel | KDialog::Reset );
    setWindowTitle( i18n("Move or resize a partition") );

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
 
    QGroupBox   *info_group = new QGroupBox( m_old_storage_part->getName() );  
    QVBoxLayout *info_group_layout = new QVBoxLayout();
    info_group->setLayout(info_group_layout);
    layout->addWidget(info_group);

    m_change_by_label = new QLabel();
    m_preceding_label = new QLabel();
    m_following_label = new QLabel();
    info_group_layout->addWidget( m_preceding_label );
    info_group_layout->addWidget( m_following_label );
    info_group_layout->addStretch();
    info_group_layout->addWidget( new QLabel( i18n("Minimum size: %1", sizeToString( m_min_shrink_size * m_sector_size ))));
    info_group_layout->addWidget( new QLabel( i18n("Maximum size: %1", sizeToString( max_size * m_sector_size ) ))); 
    info_group_layout->addStretch();
    info_group_layout->addWidget( m_change_by_label );

    if( m_old_storage_part->isPV() ){
        if( m_old_storage_part->getPhysicalVolume()->isActive() ){
            max_size -= existing_offset;
            m_size_selector = new SizeSelectorBox(m_sector_size, m_min_shrink_size, max_size, 
                                                  m_existing_part->geom.length, false, false);
            m_offset_selector = new SizeSelectorBox(m_sector_size, existing_offset, existing_offset, 
                                                    existing_offset, false, true );
        }
        else{
            m_size_selector = new SizeSelectorBox(m_sector_size, m_min_shrink_size, max_size, 
                                                  m_existing_part->geom.length, false, false);
            m_offset_selector = new SizeSelectorBox(m_sector_size, 0, max_offset, existing_offset, false, true); 
        }
    }
    else{
        m_size_selector = new SizeSelectorBox(m_sector_size, m_min_shrink_size, max_size, 
                                              m_existing_part->geom.length, false, false);
        m_offset_selector = new SizeSelectorBox(m_sector_size, 0, max_offset, existing_offset, false, true ); 
    }

    layout->addWidget(m_size_selector);
    layout->addWidget(m_offset_selector);

    offsetChanged();
    sizeChanged();
    updateAndValidatePartition();

    connect(m_size_selector, SIGNAL(stateChanged()),
	    this , SLOT(sizeChanged()));

    connect(m_offset_selector, SIGNAL(stateChanged()),
	    this , SLOT(offsetChanged()));

    connect(this, SIGNAL(resetClicked()),
	    this, SLOT(resetSelectors()));

    connect(this, SIGNAL(okClicked()),
	    this, SLOT(commitPartition()));

}

void PartitionMoveResizeDialog::commitPartition()
{
    long long new_size   = m_size_selector->getCurrentSize();
    long long new_offset = m_offset_selector->getCurrentSize();
    bool grow   = false;
    bool shrink = false;
    bool move   = ( (m_max_part_start + new_offset) != m_existing_part->geom.start );

    if( new_size < m_existing_part->geom.length ) {
        grow   = false;
        shrink = true;
    }
    else if( new_size > m_existing_part->geom.length ){
        grow   = true;
        shrink = false;
    }
    else{
        grow   = false;
        shrink = false;
    }

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    if( grow && move ){
        if( movePartition() ){
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            growPartition();
        }
    }
    else if( grow ){
        growPartition();
    }
    else if( shrink && move ){
        if( shrinkPartition() ){
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            movePartition();
        }
    }
    else if( shrink ){
        shrinkPartition();
    }
    else if( move ){
        movePartition();
    }

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void PartitionMoveResizeDialog::sizeChanged()
{
    const long long max = m_max_part_size;
    const long long current_size   = m_size_selector->getCurrentSize();
    const long long current_offset = m_offset_selector->getCurrentSize();

    if( m_size_selector->isValid() ){
        if( ! m_offset_selector->isValid() )
            m_offset_selector->setCurrentSize( m_offset_selector->getCurrentSize() ); // reset to last valid value 

        if( ! m_offset_selector->isLocked() ){
                
            if( m_size_selector->isLocked() )
                m_offset_selector->setConstrainedMax( max - current_size );
            else
                m_offset_selector->setConstrainedMax( max - m_size_selector->getMinimumSize() );
            
            if( m_offset_selector->getCurrentSize() > ( max - current_size ) )
                if( ! m_offset_selector->setCurrentSize( max - current_size ) )
                    m_size_selector->setCurrentSize( max - current_offset );
        }
        else{
            m_size_selector->setConstrainedMax( max - current_offset);
        }        
    }

    updateAndValidatePartition();
} 

void PartitionMoveResizeDialog::offsetChanged()
{
    const long long max = m_max_part_size;
    const long long current_size   = m_size_selector->getCurrentSize();
    const long long current_offset = m_offset_selector->getCurrentSize();

    if( m_offset_selector->isValid() ){
        if( ! m_size_selector->isValid() )
            m_size_selector->setCurrentSize( m_size_selector->getCurrentSize() ); // poke it to make it valid 

        if( ! m_size_selector->isLocked() ){
            m_size_selector->setConstrainedMax( max );

            if( m_offset_selector->isLocked() )
                m_size_selector->setConstrainedMax( max - current_offset );
            else
                m_size_selector->setConstrainedMax( max - m_offset_selector->getMinimumSize() );

            if( m_size_selector->getCurrentSize() > ( max - current_offset ) )
                if( ! m_size_selector->setCurrentSize( max - current_offset ) )
                    m_offset_selector->setCurrentSize( max - current_size );
        }
        else{
            m_offset_selector->setConstrainedMax( max - current_size );
        }
    }

    updateAndValidatePartition();
}

void PartitionMoveResizeDialog::updateAndValidatePartition()
{
    if( !m_offset_selector->isValid() || !m_size_selector->isValid() ){
        (button(KDialog::Ok))->setEnabled(false);
        return;
    }

    const long long preceding_sectors = m_offset_selector->getCurrentSize();
    const long long following_sectors = m_max_part_size - ( m_offset_selector->getCurrentSize() + m_size_selector->getCurrentSize() );

    if( preceding_sectors < 0 || following_sectors < 0 ){
        (button(KDialog::Ok))->setEnabled(false);
        return;
    }

    updateGraphicAndLabels();

    (button(KDialog::Ok))->setEnabled(true);
}

void PartitionMoveResizeDialog::resetSelectors()
{
    m_size_selector->resetToInitial();
    m_offset_selector->resetToInitial();
}

void PartitionMoveResizeDialog::setup()
{
    QString fs = m_old_storage_part->getFilesystem();

    m_existing_part = m_old_storage_part->getPedPartition();
    m_ped_disk = m_existing_part->disk;   

    PedDevice   *ped_device = m_ped_disk->dev;
    PedGeometry *ped_max_geometry  = NULL;

    /* how big can it grow? */
    PedConstraint *constraint = ped_device_get_constraint(ped_device);
    constraint = ped_constraint_any(ped_device);
    ped_max_geometry  = ped_disk_get_max_partition_geometry( m_ped_disk, m_existing_part, constraint );
    m_sector_size     = ped_device->sector_size;

    m_max_part_start  = ped_max_geometry->start;
    m_max_part_size   = ped_max_geometry->length;

    // remove this when possible!!!!
    getMaximumPartition();

    if( m_existing_part->type & PED_PARTITION_LOGICAL )
        m_logical = true;
    else
        m_logical = false;

    if( m_old_storage_part->isPV() ){

        PhysVol* const pv = m_old_storage_part->getPhysicalVolume();
        const long mda_count = pv->getMDACount();
        const long long extent_size = pv->getVolGroup()->getExtentSize();
        const long long mda_extents = (pv->getMDASize() / extent_size) + 1;

        m_min_shrink_size = 1 + (mda_extents * mda_count) + pv->getLastUsedExtent();
        m_min_shrink_size *= extent_size;
        m_min_shrink_size /= m_sector_size;
    }
    else{
        m_min_shrink_size = get_min_fs_size(ped_partition_get_path(m_existing_part), fs) / m_sector_size;
    }

    if( m_min_shrink_size == 0 )                 // 0 means we can't shrink it 
        m_min_shrink_size = m_existing_part->geom.length;
}

bool PartitionMoveResizeDialog::movefs(long long from_start, long long to_start, long long length)
{
    const long long blocksize = 8000;    // sectors moved in each block

    PedDevice *device = m_ped_disk->dev;

    char *buff = static_cast<char *>( malloc( blocksize * m_sector_size ) ) ;

    long long blockcount = length / blocksize;
    long long extra = length % blocksize;

    ped_device_open(device);

    KProgressDialog *progress_dialog = new KProgressDialog(this, i18n("progress"), i18n("moving data...") );
    progress_dialog->setAllowCancel(false);
    progress_dialog->show();
    QProgressBar *progress_bar = progress_dialog->progressBar();
    progress_bar->setRange(0, blockcount);
    int event_timer = 0;
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    if( to_start < from_start ){                       // moving left
        for( long long x = 0; x < blockcount  ; x++){
            event_timer++;
            if(event_timer > 5){
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                event_timer = 0;
            }
            if( ! ped_device_read( device, buff, from_start + (x * blocksize), blocksize) ){
                progress_dialog->close();
                KMessageBox::error( 0, i18n("Move failed: could not read from device") );
                return false;
            }
            if( ! ped_device_write(device, buff, to_start   + (x * blocksize), blocksize) ){
                progress_dialog->close();
                KMessageBox::error( 0, i18n("Move failed: could not write to device") );
                return false;
            }
            progress_bar->setValue( x );
        }
        if( ! ped_device_read(device,  buff, from_start + (blockcount * blocksize), extra) ){
            progress_dialog->close();
            KMessageBox::error( 0, i18n("Move failed: could not read from device") );
            return false;
        }
        if( ! ped_device_write(device, buff, to_start   + (blockcount * blocksize), extra) ){
            progress_dialog->close();
            KMessageBox::error( 0, i18n("Move failed: could not write to device") );
            return false;
        }
    }
    else{                                              // moving right
        if( ! ped_device_read(device,  buff, from_start + (blockcount * blocksize), extra) ){
            progress_dialog->close();
            KMessageBox::error( 0, i18n("Move failed: could not read from device") );
            return false;
        }
        if( ! ped_device_write(device, buff, to_start   + (blockcount * blocksize), extra) ){
            progress_dialog->close();
            KMessageBox::error( 0, i18n("Move failed: could not write to device") );
            return false;
        }
        for( long long x = blockcount - 1; x >= 0 ; x--){
            event_timer++;
            if(event_timer > 5){
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                event_timer = 0;
            }
            if( ! ped_device_read( device, buff, from_start + (x * blocksize), blocksize) ){
                progress_dialog->close();
                KMessageBox::error( 0, i18n("Move failed: could not read from device") );
                return false;
            }
            if( ! ped_device_write(device, buff, to_start   + (x * blocksize), blocksize) ){
                progress_dialog->close();
                KMessageBox::error( 0, i18n("Move failed: could not write to device") );
                return false;
            }
            progress_bar->setValue( blockcount - x );
        }
    }
    progress_dialog->close();

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    ped_device_sync( device );
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    ped_device_close( device );
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    return true;
}

bool PartitionMoveResizeDialog::shrinkPartition()
{
    hide();

    PedDevice* const device = m_ped_disk->dev;
    const PedSector sectors_64KiB = 0x10000 / m_sector_size;    // sectors per 64 kilobytes
    const PedSector current_start = m_existing_part->geom.start;
    const PedSector current_size  = m_existing_part->geom.length;
    const QString fs = m_old_storage_part->getFilesystem();
    const bool is_pv = m_old_storage_part->isPV();

    PedSector new_size = m_size_selector->getCurrentSize();

    if( new_size >= current_size ){
        qDebug() << "shrinkPartition called with a *bigger* part size!";
        return false;
    }

    if( new_size < m_min_shrink_size)
        new_size = m_min_shrink_size;
    else if( new_size > m_max_part_size )
        new_size = m_max_part_size;

    PedSector reduced_size;

    if( is_pv ){
        reduced_size = 1 + pv_reduce( ped_partition_get_path(m_existing_part) ,
                                      new_size * m_sector_size ) / m_sector_size;
    }
    else{
        reduced_size = 1 + fs_reduce( ped_partition_get_path(m_existing_part), 
                                      new_size * m_sector_size, fs ) / m_sector_size;
    }

    if( reduced_size == 0 )  // The shrink failed
        return false;

    // This constraint assures we have a new partition at least as long as the fs can shrink it
    // We allow an extra 64KiB sectors for the end of the partition

    PedAlignment * const start_alignment = ped_alignment_new(0, 1);
    PedAlignment * const end_alignment   = ped_alignment_new(0, 1);
    PedGeometry *  const start_range = ped_geometry_new( device, current_start, 1);
    PedGeometry *  const end_range   = ped_geometry_new( device, current_start + reduced_size - 1, sectors_64KiB );

    PedSector minimum_size = reduced_size;
    PedSector maximum_size = reduced_size + sectors_64KiB;

    PedConstraint *constraint = ped_constraint_new( start_alignment, end_alignment,
                                                    start_range, end_range,
                                                    minimum_size, maximum_size);

    int success = ped_disk_set_partition_geom( m_ped_disk, m_existing_part, constraint, 
                                               current_start, current_start + maximum_size );

    if( ! success ){  
        KMessageBox::error( 0, i18n("Partition shrink failed") );
        return false;
    }
    else{
        pedCommitAndWait(m_ped_disk);
        return true;
    }
}

bool PartitionMoveResizeDialog::growPartition()
{
    hide();

    const PedSector sectors_64KiB = 0x10000 / m_sector_size;    // sectors per 64 kilobytes
    PedDevice* const device = m_ped_disk->dev;
    const QString fs = m_old_storage_part->getFilesystem();
    const bool is_pv = m_old_storage_part->isPV();
    const PedSector current_start = m_existing_part->geom.start;
    const PedSector current_size  = m_existing_part->geom.length;
    const PedSector max_end   = m_max_part_start + m_max_part_size - 1;
    const PedSector max_start = m_max_part_start;

    int success;
    PedSector min_new_size, 
              max_new_size;  // max desired size

    // Don't grow less than 128 KiB
    if( m_size_selector->getCurrentSize() <= ( current_size + ( 2 * sectors_64KiB ) ) )
        return false;
    else if( m_size_selector->getCurrentSize() + current_start > max_end )
        max_new_size = max_end - current_start;
    else
        max_new_size = m_size_selector->getCurrentSize();

    min_new_size = max_new_size - sectors_64KiB;

    PedGeometry *start_range = ped_geometry_new(device, current_start, 1);
    PedGeometry *end_range   = ped_geometry_new(device, current_start + min_new_size - 1, sectors_64KiB);

    PedConstraint *constraint = ped_constraint_new( ped_alignment_new(0, 1), ped_alignment_new(0, 1),      
                                                    start_range, end_range,
                                                    min_new_size, max_new_size);

    /* if constraint solves to NULL then the new part will fail, so just bail out */
    if( ped_constraint_solve_max( constraint ) == NULL ){
        KMessageBox::error( 0, i18n("Partition extention failed") );
        return false;
    }

    success = ped_disk_set_partition_geom(m_ped_disk, m_existing_part, constraint, max_start, max_end);

    if( ! success ){  
        KMessageBox::error( 0, i18n("Partition extention failed") );
        return false;
    }
    else {
        // Here we wait for linux and udev to re-read the partition table before doing anything else.
        // Otherwise the resize program will fail.
         
        pedCommitAndWait(m_ped_disk);

        if( is_pv ){            
           if( pv_extend( ped_partition_get_path(m_existing_part) ) )
                return true;
            else
                return false;
        }
        else{
            if( fs_extend( ped_partition_get_path(m_existing_part), fs ) )
                return true;
            else
                return false;
        }
    }
}

void PartitionMoveResizeDialog::updateGraphicAndLabels()
{
    PedSector preceding_sectors = m_offset_selector->getCurrentSize();
    PedSector following_sectors = m_max_part_size - ( preceding_sectors + m_size_selector->getCurrentSize() );
    long long change_size = m_sector_size * (m_size_selector->getCurrentSize() - m_existing_part->geom.length );

    m_display_graphic->setPrecedingSectors(preceding_sectors);
    m_display_graphic->setPartitionSectors(m_size_selector->getCurrentSize());
    m_display_graphic->setFollowingSectors(following_sectors);
    m_display_graphic->repaint();

    QString change = sizeToString(change_size);
    if(change_size < 0)
        m_change_by_label->setText( i18n("<b>Shrink by : %1</b>", change) );
    else
        m_change_by_label->setText( i18n("<b>Grow by : %1</b>", change) );

    QString preceding_bytes_string = sizeToString(preceding_sectors * m_sector_size);
    m_preceding_label->setText( i18n("Preceding space: %1", preceding_bytes_string) );

    long long following_space = following_sectors * m_sector_size;

    if(following_space < 0)
        following_space = 0;

    QString following_bytes_string = sizeToString(following_space);
    m_following_label->setText( i18n("Following space: %1", following_bytes_string) );
}

bool PartitionMoveResizeDialog::movePartition()
{
    hide();

    PedDevice *device = m_ped_disk->dev;

    const PedSector sectors_1MiB  = 0x100000 / m_sector_size;   // sectors per megabyte
    const PedSector sectors_64KiB = 0x10000 / m_sector_size;    // sectors per 64 kilobytes

    const PedSector max_size  = m_max_part_size;
    const PedSector max_start = m_max_part_start;
    const PedSector max_end   = m_max_part_start + m_max_part_size - 1;
    const PedSector current_size  = m_existing_part->geom.length;
    PedSector current_start = m_existing_part->geom.start;
    PedSector new_start     = m_max_part_start + m_offset_selector->getCurrentSize();

    // don't move if the move is less than 1 megabyte 
    // and check that we have at least 1 meg to spare

    if( fabs(current_start - new_start) < sectors_1MiB )
        return false;
    else if( new_start < current_start ){  // moving left
        if( ( current_start - max_start ) < sectors_1MiB )  
            return false;
        else if( ( new_start - max_start ) < ( sectors_1MiB / 2 ) )  
            new_start = max_start + ( sectors_1MiB / 2);
    }
    else {                                 // moving right
        if( ( max_end - (current_start + current_size - 1) ) < sectors_1MiB )  
            return false;
        else if( ( new_start + current_size - 1 )  > ( max_end - sectors_1MiB / 2 ) )  
            new_start = 1 + max_end - ( ( sectors_1MiB / 2) + current_size );
    }

    PedAlignment *start_alignment_none  = ped_alignment_new(0, 1); 
    PedAlignment *start_alignment_64KiB = ped_alignment_new(0, sectors_64KiB); 
    PedAlignment *start_alignment_1MiB  = ped_alignment_new(0, sectors_1MiB); 
    PedAlignment *end_alignment = ped_alignment_new(0, 1);

    PedGeometry *start_range = ped_geometry_new(device, new_start - ( sectors_1MiB / 2 ), sectors_1MiB );

    PedGeometry *end_range   = ped_geometry_new(device, new_start + ( current_size - 1 ) - ( sectors_1MiB / 2 ), 
                                                sectors_1MiB  );

    // First try 1 Meg alignment, then 64k if that fails and then none as last resort

    //    ped_exception_set_handler(my_constraint_handler);

    PedConstraint *constraint_none  = ped_constraint_new( start_alignment_none, end_alignment,
                                                          start_range, end_range,
                                                          current_size, ( current_size - 2 ) + sectors_1MiB);

    PedConstraint *constraint_64KiB = ped_constraint_new( start_alignment_64KiB, end_alignment,
                                                          start_range, end_range,
                                                          current_size, ( current_size - 1 )+ sectors_1MiB);

    PedConstraint *constraint_1MiB = ped_constraint_new( start_alignment_1MiB, end_alignment,
                                                         start_range, end_range,
                                                         current_size, current_size + sectors_1MiB - 1);

    // This constraint assures we don't go past the edges of any
    // adjoining partitions or the partition table itself

    PedGeometry *max_start_range = ped_geometry_new( device, max_start, max_size ); 
    PedGeometry *max_end_range   = ped_geometry_new( device, max_start, max_size );

    PedConstraint *constraint_max = ped_constraint_new( start_alignment_none, end_alignment,
                                                        max_start_range, max_end_range,
                                                        1, max_size);

    if(constraint_none){
        constraint_none = ped_constraint_intersect(constraint_max, constraint_none);
    }
    if(constraint_64KiB){
        constraint_64KiB = ped_constraint_intersect(constraint_max, constraint_64KiB);
    }
    if(constraint_1MiB){
        constraint_1MiB = ped_constraint_intersect(constraint_max, constraint_1MiB);
    }

    PedSector old_start = current_start;
    PedSector old_size  = current_size;

    int success = 0;

    if(constraint_1MiB){
        success = ped_disk_set_partition_geom( m_ped_disk, m_existing_part, constraint_1MiB, 
                                               max_start, max_end );
    }

    if(constraint_64KiB && (!success) ){
        success = ped_disk_set_partition_geom( m_ped_disk, m_existing_part, constraint_64KiB, 
                                               max_start, max_end );
    }

    if(constraint_none && (!success)){
        success = ped_disk_set_partition_geom( m_ped_disk, m_existing_part, constraint_none, 
                                               max_start, max_end );
    }

    ped_exception_set_handler(my_handler);
    current_start = m_existing_part->geom.start;

    if( !success ){  
        KMessageBox::error( 0, i18n("Repartitioning failed: data not moved") );
        return false;
    }
    else {
        if( !movefs(old_start, current_start, old_size) ){
            return false;
        }
        else{
            pedCommitAndWait(m_ped_disk);
            return true;
        }
    }
}

/* The following function waits for udev to acknowledge the partion changes before exiting */

bool PartitionMoveResizeDialog::pedCommitAndWait(PedDisk *disk)
{
    QStringList args;

    if( !ped_disk_commit(disk) )
        return false;

    args << "udevadm" << "settle";
    ProcessProgress wait_settle( args, i18n("Wating for udev..."), true);

    return true;
}

void PartitionMoveResizeDialog::getMaximumPartition()
{
    QList<PedPartition *> parts;
    PedPartition *next_part = NULL;
    bool is_logical;
    int index, prev, next;
    long long max_end = m_max_part_start + m_max_part_size - 1;
    const PedSector sectors_1MiB  = 0x100000 / m_sector_size;   // sectors per megabyte

    while( (next_part = ped_disk_next_partition(m_ped_disk, next_part) ) )
        parts.append(next_part);

    is_logical = m_existing_part->type & PED_PARTITION_LOGICAL;

    // look for the index of the current part
    for(index = 0; index < parts.size(); index++){
        if( parts[index]->geom.start == m_existing_part->geom.start )
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

    m_max_part_size = 1 + max_end - m_max_part_start;
}

