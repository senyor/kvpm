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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include <KLocale>
#include <KMessageBox>
#include <KProgressDialog>

#include <QtGui>
#include <QThread>

#include "partmoveresize.h"
#include "partaddgraphic.h"
#include "processprogress.h"
#include "sizetostring.h"


bool moveresize_partition(StoragePartition *partition)
{
    PartitionMoveResizeDialog dialog(partition);
    QString fs = partition->getFileSystem();

    QString message = i18n("Currently only the ext2 and ext3  file systems "
                           "are supported for file system resizing. "
                           "Moving a partition is supported for any filesystem");

    if( ! (fs == "ext2" || fs == "ext3") )
        KMessageBox::information(0, message);

    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted)
        return true;
    else
        return false;

}

/* This if passed "true" function checks the /dev directory to see if 
   the partition in question exists and waits for its creation if it does 
   not before returning. If passed "false" it waits for the partition
   to be deleted */ 


bool waitPartitionTableReload(QString partitionPath , bool exists){

    QString error_string;
    int fd;
    char buff[512];
    errno = 0;

    if(exists){

        // wait for the /dev entry to appear

        while( (fd = open( partitionPath.toAscii().data(), O_RDONLY)) < 1 ){
            sleep( 1 );
        }

        // make sure we can read from it

        while( read(fd, buff, 512) == 0 )
            sleep( 1 );

        close(fd);
    }
    else{

        // if the dev entry still exists, wait until we can't read from it

        if( (fd = open( partitionPath.toAscii().data(), O_RDONLY)) > 0 ){
            while( read(fd, buff, 512) != 0 ){
                lseek(fd, 0, SEEK_SET);
                sleep( 1 );
            }
        }
        close(fd);        
    }

    return true;

}

PartitionMoveResizeDialog::PartitionMoveResizeDialog(StoragePartition *partition, 
				                     QWidget *parent) : 
    KDialog(parent),
    m_old_storage_part(partition)
  
{
    setup();

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
 
    QGroupBox   *info_group = new QGroupBox( m_old_storage_part->getPartitionPath() );  
    QVBoxLayout *info_group_layout = new QVBoxLayout();
    info_group->setLayout(info_group_layout);
    layout->addWidget(info_group);

    m_preceding_label = new QLabel();
    m_size_label      = new QLabel();
    m_remaining_label = new QLabel();
    info_group_layout->addWidget( m_preceding_label );
    info_group_layout->addWidget( m_size_label );
    info_group_layout->addWidget( m_remaining_label );

    QString filesystem = partition->getFileSystem();
    m_size_group = new QGroupBox( i18n("Modify partition size") );
    m_size_group->setCheckable(true);
    if ( ! (filesystem == "ext2" || filesystem == "ext3" ) ){
        m_size_group->setChecked(false);
        m_size_group->setEnabled(false);
    }
    else
        m_size_group->setChecked(true);

    QGridLayout *size_group_layout = new QGridLayout();
    m_size_group->setLayout(size_group_layout);
    layout->addWidget(m_size_group);

    m_size_spin = new QSpinBox();
    m_size_spin->setRange(0,100);
    m_size_spin->setSuffix("%");

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
    size_group_layout->addWidget(m_size_spin,1,0);

    QLabel *min_size_label = new QLabel( QString("Minimum size: %1").arg( sizeToString( m_min_shrink_size * m_ped_sector_size )));
    size_group_layout->addWidget( min_size_label, 5, 0, 1, 2 );

    const long long max_size = 1 + m_max_part_end - m_max_part_start;

    QString total_bytes = sizeToString( max_size * m_ped_sector_size );
    QLabel *preceding_label = new QLabel( i18n("Maximum size: %1",  total_bytes) );
    size_group_layout->addWidget( preceding_label, 6, 0, 1, 2);

    m_offset_group = new QGroupBox("Move partition start");
    QGridLayout *offset_group_layout = new QGridLayout();
    m_offset_group->setCheckable(true);
    m_offset_group->setChecked(false);
    m_offset_group->setLayout(offset_group_layout);
    layout->addWidget(m_offset_group);

    m_preceding_combo = new KComboBox();
    m_preceding_combo->insertItem(0,"Sectors");
    m_preceding_combo->insertItem(1,"MiB");
    m_preceding_combo->insertItem(2,"GiB");
    m_preceding_combo->insertItem(3,"TiB");
    m_preceding_combo->setInsertPolicy(KComboBox::NoInsert);
    m_preceding_combo->setCurrentIndex(2);

    m_offset_spin = new QSpinBox();
    m_offset_spin->setRange(0,100);
    m_offset_spin->setSuffix("%");

    m_offset_edit  = new KLineEdit();
    m_offset_validator = new QDoubleValidator(m_offset_edit);
    m_offset_edit->setValidator(m_offset_validator);
    m_offset_validator->setBottom(0);

    m_align64_check = new QCheckBox("Align to 64 KiB");
    m_align64_check->setChecked(true);
    offset_group_layout->addWidget(m_align64_check, 3, 0, 1, 2, Qt::AlignHCenter );
    offset_group_layout->addWidget(m_offset_edit,0,0);
    offset_group_layout->addWidget(m_preceding_combo,0,1);
    offset_group_layout->addWidget(m_offset_spin,1,0);

    m_size_spin->setValue( 100 * ( (long double)m_old_part_size )/ max_size ); 

    m_offset_spin->setValue( 100 * (( (long double)m_old_part_start - m_max_part_start ) / max_size) );

    connect(m_size_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(adjustSizeCombo(int)));

    connect(m_size_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validateVolumeSize(QString)));

    connect(m_size_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(adjustSizeEdit(int)));

    connect(m_preceding_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(adjustOffsetCombo(int)));

    connect(m_offset_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validatePrecedingSize(QString)));

    connect(m_offset_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(adjustOffsetEdit(int)));

    connect(m_size_group, SIGNAL(toggled(bool)),
	    this, SLOT(resetSizeGroup(bool)));

    connect(m_size_group, SIGNAL(toggled(bool)),
	    this, SLOT(setOffsetSpinTop()));

    connect(m_offset_group, SIGNAL(toggled(bool)),
	    this, SLOT(resetOffsetGroup(bool)));

    connect(this, SIGNAL(okClicked()),
	    this, SLOT(commitPartition()));


    adjustSizeCombo( m_size_combo->currentIndex() );
    adjustOffsetCombo( m_preceding_combo->currentIndex() );
    resetDisplayGraphic();
    setOffsetSpinTop();
    setSizeLabels();
}

void PartitionMoveResizeDialog::commitPartition()
{

    hide();

    bool grow   = false;
    bool shrink = false;
    bool move   = ( m_offset_group->isChecked() && ( m_new_part_start != m_old_part_start ) );

    if( m_size_group->isChecked() ){
        if( m_new_part_size < m_old_part_size ){
            grow   = false;
            shrink = true;
        }
        else if( m_new_part_size > m_old_part_size ){
            grow   = true;
            shrink = false;
        }
        else{
            grow   = false;
            shrink = false;
        }
    }

    if(grow){
        if(move)
            movePartition();
        growPartition();
    }
    else if(shrink){
        shrinkPartition();
        if(move)
            movePartition();
    }
    else if(move)
            movePartition();
}


void PartitionMoveResizeDialog::adjustSizeEdit(int percentage){

    const long long max_sectors = 1 + m_max_part_end - m_max_part_start;

    setSizeSpinTop();

    if( percentage >= m_size_spin->maximum() )
        m_new_part_size = m_max_part_end - m_new_part_start;
    else if(percentage == 0)
        m_new_part_size = 0;
    else
        m_new_part_size = (long long)(( (double) percentage / 100) * ( max_sectors ));

    setSizeLabels();
    adjustSizeCombo( m_size_combo->currentIndex() );
    resetDisplayGraphic();
    resetOkButton();
}


void PartitionMoveResizeDialog::adjustSizeCombo(int index){

    long double sized;
    long double valid_topd = (((long double)m_max_part_end - m_new_part_start + 1) * m_ped_sector_size);

    if(index){
        sized = ((long double)m_new_part_size * m_ped_sector_size);
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
        m_size_edit->setText(QString("%1").arg((double)sized));
    }
    else
        m_size_edit->setText(QString("%1").arg(m_new_part_size));

    m_offset_validator->setTop( (double)valid_topd );
}


void PartitionMoveResizeDialog::validateVolumeSize(QString size){

    int x = 0;

    const int size_combo_index = m_size_combo->currentIndex();

    if(m_size_validator->validate(size, x) == QValidator::Acceptable){

        if(!size_combo_index)
            m_new_part_size = size.toLongLong();
        else
            m_new_part_size = convertSizeToSectors( size_combo_index, size.toDouble() );

    }
    else{
        m_new_part_size = 0;
    }

    long long preceding_sectors = m_new_part_start - m_max_part_start;
    PedSector following_sectors = m_max_part_end - ( m_new_part_start + m_new_part_size );

    setSizeLabels();

    m_display_graphic->setPrecedingSectors(preceding_sectors);
    m_display_graphic->setPartitionSectors(m_new_part_size);
    m_display_graphic->setFollowingSectors(following_sectors);
    m_display_graphic->repaint();

    resetOkButton();
}


long long PartitionMoveResizeDialog::convertSizeToSectors(int index, double size)
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

void PartitionMoveResizeDialog::resetOkButton(){

    long long max_sectors = 1 + m_max_part_end - m_new_part_start;

    if( (m_new_part_size <= max_sectors) && (m_new_part_size > 0) )
        enableButtonOk(true);
    else
        enableButtonOk(false);
}

void PartitionMoveResizeDialog::adjustOffsetEdit(int percentage){

    const long long max_length = 1 + m_max_part_end - m_max_part_start;

    // This is how far right we can move partition and keep it the same size
    const long long max_start  = 1 + m_max_part_end - m_old_part_size;

    if( m_size_group->isChecked() ){
        if(percentage == 100)
            m_new_part_start = m_max_part_end;
        else if(percentage == 0)
            m_new_part_start = m_max_part_start;
        else
            m_new_part_start = m_max_part_start + ( ( (double)percentage / 100 ) * max_length ); 

        adjustSizeCombo( m_size_combo->currentIndex() );
    }
    else{
        if( percentage >= m_offset_spin->maximum() )
            m_new_part_start = max_start;
        else if(percentage == 0)
            m_new_part_start = m_max_part_start;
        else
            m_new_part_start = m_max_part_start + ( ( (double)percentage / 100 ) * max_length ); 

        if( m_new_part_start > max_start )
            m_new_part_start = max_start;
    }

    setSizeSpinTop();
    setSizeLabels();
    adjustOffsetCombo( m_preceding_combo->currentIndex() );
    resetDisplayGraphic();
    resetOkButton();
}


void PartitionMoveResizeDialog::adjustOffsetCombo(int index){

    const long long max_length = 1 + m_max_part_end - m_max_part_start;

    long double sized;
    long double valid_topd = (((long double)max_length - m_new_part_size) * m_ped_sector_size);

    if(index){
 
        sized = ((long double)(1 + m_new_part_start - m_max_part_start) * m_ped_sector_size);

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
        m_offset_edit->setText(QString("%1").arg((double)sized));
    }
    else{
        m_offset_edit->setText(QString("%1").arg( m_new_part_start - m_max_part_start ));
    }

    m_offset_validator->setTop( (double)valid_topd );

}

void PartitionMoveResizeDialog::validatePrecedingSize(QString size){

    int x = 0;

    const int preceding_combo_index = m_preceding_combo->currentIndex();

    if(m_offset_validator->validate(size, x) == QValidator::Acceptable){

        if(!preceding_combo_index)
            m_new_part_start = m_max_part_start + size.toLongLong();
        else
            m_new_part_start = m_max_part_start + convertSizeToSectors( preceding_combo_index, size.toDouble() );

    }
    else{
        m_new_part_start = m_max_part_start;
    }

    adjustSizeEdit( m_size_spin->value() );

}

/* if the offset group is unchecked then reset to original value */

void PartitionMoveResizeDialog::resetOffsetGroup(bool on){

    const long long max_size = 1 + m_max_part_end - m_max_part_start; 

    if( ! on ){
        m_new_part_start = m_old_part_start;
        m_offset_spin->setValue( 100 * (( (long double)m_new_part_start - m_max_part_start ) / max_size) );
        adjustOffsetCombo( m_preceding_combo->currentIndex() );
        m_align64_check->setChecked(false);
    }
}

/* if the size group is unchecked then reset to original value */

void PartitionMoveResizeDialog::resetSizeGroup(bool on){

    const long long max_size = 1 + m_max_part_end - m_max_part_start; 

    if( ! on ){
        m_size_spin->setValue( 100 * ( (long double)m_old_part_size )/ max_size ); 
        m_new_part_size = m_old_part_size;
        adjustSizeCombo( m_size_combo->currentIndex() );
   }
}

void PartitionMoveResizeDialog::setup(){

    m_current_part = m_old_storage_part->getPedPartition();
    m_ped_disk = m_current_part->disk;   

    PedDevice   *ped_device = m_ped_disk->dev;
    PedGeometry *ped_max_geometry  = NULL;

    m_new_part_start  = (m_current_part->geom).start;
    m_new_part_size   = (m_current_part->geom).length;
    m_old_part_start  = (m_current_part->geom).start;
    m_old_part_size   = (m_current_part->geom).length;


    /* how big can it grow? */

    PedConstraint *constraint = ped_device_get_constraint(ped_device);

    ped_max_geometry = ped_disk_get_max_partition_geometry( m_ped_disk, 
                                                            m_current_part, 
                                                            constraint );

    m_max_part_start  = ped_max_geometry->start;
    m_max_part_end    = ped_max_geometry->end;
    m_ped_sector_size = ped_device->sector_size;

    qDebug( "MAX PART  start: %lld   end:  %lld", m_max_part_start, m_max_part_end );

    m_cylinder_sectors = (ped_device->hw_geom).heads * (ped_device->hw_geom).sectors;

    qDebug("cylinder size: %d", m_cylinder_sectors);
    qDebug("Check: %d", ped_disk_check(m_ped_disk)); 
    qDebug("HEADS: %d", (ped_device->hw_geom).heads );

    if( m_current_part->type & PED_PARTITION_LOGICAL )
        m_logical = true;
    else
        m_logical = false;

    m_min_shrink_size = getMinShrinkSize();
}

long long PartitionMoveResizeDialog::shrinkfs(PedSector length){

    QStringList arguments, 
                output,
                temp_stringlist;

    QString size_string;

    long long block_size = getFsBlockSize();

    arguments << "resize2fs" 
              << m_old_storage_part->getPartitionPath() 
              << QString("%1s").arg(length);

    ProcessProgress fs_shrink(arguments, i18n("Shrinking filesystem..."), true );
    output = fs_shrink.programOutput();
    temp_stringlist = output.filter("is now");

    if( temp_stringlist.size() > 0 ){

        size_string = temp_stringlist[0];
        size_string = size_string.remove( 0, size_string.indexOf("now") + 3 );
        size_string.truncate(  size_string.indexOf("blocks") );
        size_string = size_string.simplified();
        return (size_string.toLongLong() * block_size) / m_ped_sector_size;

    }

    return m_old_part_size;
}

bool PartitionMoveResizeDialog::growfs(){

    QStringList arguments, output;

    arguments << "resize2fs" 
              << m_old_storage_part->getPartitionPath(); 

    ProcessProgress fs_grow(arguments, i18n("Growing filesystem..."), true );
    output = fs_grow.programOutput();

    if ( fs_grow.exitCode() )
        return false;
    else
        return true;
}

bool PartitionMoveResizeDialog::movefs(long long from_start, 
                                       long long to_start, 
                                       long long length){

    const long long blocksize = 8000;    // sectors moved in each block

    PedDevice *device = m_ped_disk->dev;

    char *buff = static_cast<char *>( malloc( blocksize * m_ped_sector_size ) ) ;

    int error;
    long long blockcount = length / blocksize;
    long long extra = length % blocksize;

    ped_device_open( device );

    KProgressDialog *progress_dialog = new KProgressDialog(this, 
                                                           i18n("progress"),
                                                           i18n("moving filesystem...") );
    progress_dialog->setAllowCancel(false);
    progress_dialog->show();
    QProgressBar *progress_bar = progress_dialog->progressBar();
    progress_bar->setRange(0, blockcount);

    if( to_start < from_start ){                       // moving left
        for( long long x = 0; x < blockcount  ; x++){
            error = ped_device_read( device, buff, from_start + (x * blocksize), blocksize);
            ped_device_write(device, buff, to_start   + (x * blocksize), blocksize);
            progress_bar->setValue( x );
        }
        ped_device_read(device,  buff, from_start + (blockcount * blocksize), extra);
        ped_device_write(device, buff, to_start   + (blockcount * blocksize), extra);
    }
    else{                                              // moving right

        ped_device_read(device,  buff, from_start + (blockcount * blocksize), extra);
        ped_device_write(device, buff, to_start   + (blockcount * blocksize), extra);

        for( long long x = blockcount - 1; x >= 0 ; x--){
            ped_device_read( device, buff, from_start + (x * blocksize), blocksize);
            ped_device_write(device, buff, to_start   + (x * blocksize), blocksize);
            progress_bar->setValue( blockcount - x );
        }
    }

    ped_device_sync( device );
    ped_device_close( device );

    // ADD ERROR MESSAGE

    return true;
}

bool PartitionMoveResizeDialog::shrinkPartition(){

    PedPartitionType  type   = m_current_part->type;
    PedDevice        *device = m_ped_disk->dev;
    PedAlignment     *start_alignment = ped_alignment_new(0, 1);
    PedAlignment     *end_alignment   = ped_alignment_new(0, 1);
    int success;
    PedSector new_size;

    if( m_new_part_size < m_min_shrink_size)
        new_size = m_min_shrink_size;
    else if( m_new_part_size > ( 1 + m_max_part_end - m_old_part_start ) )
        new_size = 1 + m_max_part_end - m_old_part_start;
    else
        new_size = m_new_part_size;

    new_size = shrinkfs(new_size);

    // This constraint assures we have a new partition at least as long as the fs can shrink it
    // We give an extra 128 sectors for the end of the partition

    PedGeometry *start_range = ped_geometry_new( device, m_old_part_start, 1);
    PedGeometry *end_range   = ped_geometry_new( device, m_old_part_start + new_size, 128 );

    PedSector minimum_size = new_size;
    PedSector maximum_size = new_size + 128;

    PedConstraint *min_size_constraint = ped_constraint_new( start_alignment,
                                                             end_alignment,
                                                             start_range,
                                                             end_range,
                                                             minimum_size,
                                                             maximum_size);

    // This constraint assures we don't go past the edges of any
    // adjoining partitions or the partition table itself

    PedSector max_part_length = m_max_part_end - m_max_part_start + 1;

    PedGeometry *max_start_range = ped_geometry_new( device, m_max_part_start, max_part_length ); 
    PedGeometry *max_end_range   = ped_geometry_new( device, m_max_part_start, max_part_length );

    PedConstraint *max_size_constraint = ped_constraint_new( start_alignment,
                                                             end_alignment,
                                                             max_start_range,
                                                             max_end_range,
                                                             1,
                                                             max_part_length);

    PedConstraint *constraint = ped_constraint_intersect(max_size_constraint, 
                                                         min_size_constraint);

    ped_disk_delete_partition( m_ped_disk, m_current_part );

    m_current_part = ped_partition_new( m_ped_disk, 
                                        type, 
                                        0, 
                                        m_max_part_start, 
                                        m_max_part_end );

    success = ped_disk_add_partition( m_ped_disk, 
                                    m_current_part, 
                                    constraint);

    if( success ){

        success = ped_disk_commit(m_ped_disk);

        if( ! success ){  
            KMessageBox::error( 0, "Could not commit partition");
            return false;
        }
        else
            return true;
    }
    else {
        KMessageBox::error( 0, "Could not create partition");
        return false;
    }

}

bool PartitionMoveResizeDialog::growPartition(){

    PedPartitionType  type   = m_current_part->type;
    PedDevice        *device = m_ped_disk->dev;
    PedAlignment     *start_alignment = ped_alignment_new(0, 1);
    PedAlignment     *end_alignment   = ped_alignment_new(0, 1);
    int success;

    //        new_size = m_new_part_size;

    PedGeometry *start_range = ped_geometry_new( device, m_old_part_start, 1);
    PedGeometry *end_range   = ped_geometry_new( device, m_old_part_start + m_new_part_size, 128 );

    PedSector minimum_size = m_new_part_size - 128;
    PedSector maximum_size = m_new_part_size + 128;

    PedConstraint *min_size_constraint = ped_constraint_new( start_alignment,
                                                             end_alignment,
                                                             start_range,
                                                             end_range,
                                                             minimum_size,
                                                             maximum_size);

    // This constraint assures we don't go past the edges of any
    // adjoining partitions or the partition table itself

    PedSector max_part_length = m_max_part_end - m_max_part_start + 1;

    PedGeometry *max_start_range = ped_geometry_new( device, m_max_part_start, max_part_length ); 
    PedGeometry *max_end_range   = ped_geometry_new( device, m_max_part_start, max_part_length );

    PedConstraint *max_size_constraint = ped_constraint_new( start_alignment,
                                                             end_alignment,
                                                             max_start_range,
                                                             max_end_range,
                                                             1,
                                                             max_part_length);

    PedConstraint *constraint = ped_constraint_intersect(max_size_constraint, 
                                                         min_size_constraint);

    ped_disk_delete_partition( m_ped_disk, m_current_part );

    success = ped_disk_commit(m_ped_disk);
    waitPartitionTableReload( m_old_storage_part->getPartitionPath() , false  );

    m_current_part = ped_partition_new( m_ped_disk, 
                                        type, 
                                        0, 
                                        m_max_part_start, 
                                        m_max_part_end );

    success = ped_disk_add_partition( m_ped_disk, 
                                    m_current_part, 
                                    constraint);


    if( ! success ){  
        KMessageBox::error( 0, "Could not create partition");
        return false;
    }
    else {

        success = ped_disk_commit(m_ped_disk);

        if( ! success ){  
            KMessageBox::error( 0, "Could not commit partition");
            return false;
        }
        else{

            // Here we wait for linux to re-read the partition table before doing anything else.
            // Otherwise the resize program will fail.

            waitPartitionTableReload( m_old_storage_part->getPartitionPath() , true  );

            if( growfs() )
                return true;
            else
                return false;
        }
    }
}

bool PartitionMoveResizeDialog::movePartition(){

    PedPartitionType  type   = m_current_part->type;
    PedDevice        *device = m_ped_disk->dev;
    PedSector  max_part_size = m_max_part_end - m_max_part_start + 1;

    PedAlignment     *start_alignment = NULL;
    PedAlignment     *end_alignment   = ped_alignment_new(0, 1);
    int success = 0;


    // Bail out if the partition is within 256 sectors of max size
    if( m_old_part_size > ( max_part_size - 256 ) )
        return false;
    else if( (m_new_part_start + m_old_part_size - 1) > ( m_max_part_end - 128 ) )
        m_new_part_start -= 128;

    if( m_align64_check->isChecked() )
        start_alignment = ped_alignment_new(0, 128);
    else
        start_alignment = ped_alignment_new(0, 1);

    PedGeometry *start_range = ped_geometry_new(device, m_new_part_start, 256);
    PedGeometry *end_range   = ped_geometry_new(device, m_new_part_start + m_old_part_size, 256);

    PedSector min_size = m_old_part_size;
    PedSector max_size = m_old_part_size + 256;

    PedConstraint *min_size_constraint = ped_constraint_new( start_alignment,
                                                             end_alignment,
                                                             start_range,
                                                             end_range,
                                                             min_size,
                                                             max_size);


    // This constraint assures we don't go past the edges of any
    // adjoining partitions or the partition table itself

    PedGeometry *max_start_range = ped_geometry_new( device, m_max_part_start, max_part_size ); 
    PedGeometry *max_end_range   = ped_geometry_new( device, m_max_part_start, max_part_size );

    PedConstraint *max_size_constraint = ped_constraint_new( start_alignment,
                                                             end_alignment,
                                                             max_start_range,
                                                             max_end_range,
                                                             1,
                                                             max_part_size);

    PedConstraint *constraint = ped_constraint_intersect(max_size_constraint, 
                                                         min_size_constraint);

    ped_disk_delete_partition( m_ped_disk, m_current_part );

    m_current_part = ped_partition_new( m_ped_disk, 
                                        type, 
                                        0, 
                                        m_max_part_start, 
                                        m_max_part_end );

    success = ped_disk_add_partition( m_ped_disk, 
                                      m_current_part, 
                                      constraint);

    if( ! success ){  
        KMessageBox::error( 0, "Could not create partition");
        return false;
    }
    else {

        m_new_part_start = m_current_part->geom.start;
        m_new_part_size  = m_current_part->geom.length;

        if( ! movefs( m_old_part_start, m_new_part_start, m_old_part_size) ){
            KMessageBox::error( 0, "Filesystem move failed");
            return false;
        }
        else{
            success = ped_disk_commit(m_ped_disk);

            if( ! success ){  
                KMessageBox::error( 0, "Could not commit partition");
                return false;
            }
            else{
                return true;
            }
        }
    }
}

void PartitionMoveResizeDialog::resetDisplayGraphic(){
   
    PedSector preceding_sectors = m_new_part_start - m_max_part_start;
    PedSector following_sectors = m_max_part_end - ( m_new_part_start + m_new_part_size );

    m_display_graphic->setPrecedingSectors(preceding_sectors);
    m_display_graphic->setPartitionSectors(m_new_part_size);
    m_display_graphic->setFollowingSectors(following_sectors);
    m_display_graphic->repaint();
}


// Returns estimated minimum size of filesystem after shrinking, in sectors
// Returns 0 on failure

long long PartitionMoveResizeDialog::getMinShrinkSize(){

    QStringList arguments, 
                output;

    QString size_string,
            fs;

    fs = m_old_storage_part->getFileSystem();

    if( fs == "ext2" || fs == "ext3" ){
        arguments << "resize2fs" << "-P" << m_old_storage_part->getPartitionPath();

        long long block_size = getFsBlockSize();
        if( block_size ){                        // if blocksize failed skip this part
            
            ProcessProgress fs_scan(arguments, i18n("Checking minimum shrink size") );
            output = fs_scan.programOutput();
            
            if( output.size() > 0 ){

                size_string = output[0];
                if ( size_string.contains("Estimated", Qt::CaseInsensitive) ){
                    size_string = size_string.remove( 0, size_string.indexOf(":") + 1 );
                    size_string = size_string.simplified();
                return (size_string.toLongLong() * block_size) / m_ped_sector_size;
                }
                else
                    return m_old_part_size;
            }
        }
        
        return m_old_part_size;
    }

    return m_old_part_size;
}

//Return 0 on failure or return blocksize in bytes

long long PartitionMoveResizeDialog::getFsBlockSize(){

    QStringList arguments, 
                output,
                temp_stringlist;

    QString block_string;

    arguments << "dumpe2fs" << m_old_storage_part->getPartitionPath();

    ProcessProgress blocksize_scan(arguments, i18n("Checking blocksize") );
    output = blocksize_scan.programOutput();

    temp_stringlist << output.filter("Block size", Qt::CaseInsensitive);

    if( temp_stringlist.size() ){
        block_string = temp_stringlist[0];
        block_string = block_string.remove( 0, block_string.indexOf(":") + 1 );
        block_string = block_string.simplified();
    }
    else
        return 0;

    return block_string.toLongLong();
}

void PartitionMoveResizeDialog::setOffsetSpinTop(){

    const long long max_sectors = 1 + m_max_part_end - m_max_part_start;

    const int top = qRound( (1.0 - ( (double)m_old_part_size / max_sectors ) )  * 100);

    if( m_size_group->isChecked() )
        m_offset_spin->setMaximum(100);
    else
        m_offset_spin->setMaximum(top);
}

void PartitionMoveResizeDialog::setSizeSpinTop(){

    const long long preceding_sectors = m_new_part_start - m_max_part_start;
    const long long max_sectors       = 1 + m_max_part_end - m_max_part_start;

    const int top = qRound( (1.0 - ( (double)preceding_sectors / max_sectors ) )  * 100);

    m_size_spin->setMaximum( top );
}

void PartitionMoveResizeDialog::setSizeLabels(){

    const long long preceding_sectors = m_new_part_start - m_max_part_start;

    QString part_size = sizeToString(m_ped_sector_size * m_new_part_size);
    m_size_label->setText( i18n("Partition size: %1", part_size) );

    QString preceding_bytes_string = sizeToString(preceding_sectors * m_ped_sector_size);
    m_preceding_label->setText( i18n("Preceding space: %1", preceding_bytes_string) );

    PedSector following_sectors = m_max_part_end - ( m_new_part_start + m_new_part_size );
    long long following_space   = following_sectors * m_ped_sector_size;

    if(following_space < 0)
        following_space = 0;

    QString following_bytes_string = sizeToString(following_space);
    m_remaining_label->setText( i18n("Following space: %1", following_bytes_string) );
}

