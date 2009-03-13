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

#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

#include <KLocale>
#include <KMessageBox>
#include <KProgressDialog>
#include <KPushButton>
#include <KButtonGroup>

#include <QtGui>
#include <QThread>

#include "partmoveresize.h"
#include "partaddgraphic.h"
#include "physvol.h"
#include "sizetostring.h"
#include "growfs.h"
#include "growpv.h"
#include "shrinkfs.h"
#include "shrinkpv.h"


bool moveresize_partition(StoragePartition *partition)
{
    PartitionMoveResizeDialog dialog(partition);
    QString fs = partition->getFileSystem();

    QString message = i18n("Currently only the ext2/3/4 file systems "
                           "are supported for file system shrinking. "
                           "Growing is supported for ext2/3/4, jfs and xfs "
                           "Moving a partition is supported for any filesystem. "
                           "Physical volumes may also be grown or shrunk.");

    if( ! ( fs == "ext2" || fs == "ext3" || fs == "ext4" ||
            fs == "xfs"  || fs == "jfs"  || partition->isPV() ) ){

        KMessageBox::information(0, message);
    }

    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted)
        return true;
    else
        return false;

}

PartitionMoveResizeDialog::PartitionMoveResizeDialog(StoragePartition *partition, 
				                     QWidget *parent) : 
    KDialog(parent),
    m_old_storage_part(partition)
  
{
    setup();

    const PedSector max_size = 1 + m_max_part_end - m_max_part_start;

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
 
    QGroupBox   *info_group = new QGroupBox( m_old_storage_part->getPartitionPath() );  
    QVBoxLayout *info_group_layout = new QVBoxLayout();
    info_group->setLayout(info_group_layout);
    layout->addWidget(info_group);

    m_preceding_label = new QLabel();
    m_size_label      = new QLabel();
    m_remaining_label = new QLabel();
    info_group_layout->addWidget( m_preceding_label );
    info_group_layout->addWidget( m_remaining_label );
    info_group_layout->addWidget( m_size_label );

    info_group_layout->addWidget( new QLabel( i18n("Minimum size: %1", sizeToString( m_min_shrink_size * m_ped_sector_size ))));

    info_group_layout->addWidget( new QLabel( i18n("Maximum size: %1", sizeToString( max_size * m_ped_sector_size ) ))); 

    QString filesystem = partition->getFileSystem();
    m_size_group = new QGroupBox( i18n("Modify partition size") );
    m_size_group->setCheckable(true);
    if ( ! (filesystem == "ext2" || filesystem == "ext3" || filesystem == "ext4" || 
            filesystem == "xfs"  || filesystem == "jfs"  || m_old_storage_part->isPV() ) ){
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

    KButtonGroup *size_arrow_group = new KButtonGroup;
    QHBoxLayout *size_arrow_layout = new QHBoxLayout;
    size_group_layout->addWidget( size_arrow_group, 7, 0, 1, 2, Qt::AlignCenter );

    size_arrow_group->setLayout( size_arrow_layout );
    KPushButton *size_min_arrow = new KPushButton(KIcon("go-first-view"), "Minimize");
    KPushButton *size_max_arrow = new KPushButton(KIcon("go-last-view"),  "Maximize");
    size_arrow_layout->addWidget( size_min_arrow );
    size_arrow_layout->addWidget( size_max_arrow );

    m_offset_group = new QGroupBox("Move partition start");
    QGridLayout *offset_group_layout = new QGridLayout();
    if( m_old_storage_part->isPV() ){
        if( m_old_storage_part->getPhysicalVolume()->isActive() ){
            m_offset_group->setEnabled(false);
        }
        else{
            m_offset_group->setCheckable(true);
            m_offset_group->setChecked(false);
        }
    }
    else{
        m_offset_group->setCheckable(true);
        m_offset_group->setChecked(false);
    }
    m_offset_group->setLayout(offset_group_layout);
    layout->addWidget(m_offset_group);

    m_offset_combo = new KComboBox();
    m_offset_combo->insertItem(0,"Sectors");
    m_offset_combo->insertItem(1,"MiB");
    m_offset_combo->insertItem(2,"GiB");
    m_offset_combo->insertItem(3,"TiB");
    m_offset_combo->setInsertPolicy(KComboBox::NoInsert);
    m_offset_combo->setCurrentIndex(2);

    m_offset_spin = new QSpinBox();
    m_offset_spin->setRange(0,100);
    m_offset_spin->setSuffix("%");

    m_offset_edit  = new KLineEdit();
    m_offset_validator = new QDoubleValidator(m_offset_edit);
    m_offset_edit->setValidator(m_offset_validator);
    m_offset_validator->setBottom(0);

    m_align64_check = new QCheckBox("Align to 64 KiB");
    m_align64_check->setChecked(true);
    offset_group_layout->addWidget(m_align64_check, 3, 0);//,  1, 2, Qt::AlignHCenter );
    offset_group_layout->addWidget(m_offset_edit,0,0);
    offset_group_layout->addWidget(m_offset_combo,0,1);
    offset_group_layout->addWidget(m_offset_spin,1,0);

    KButtonGroup *offset_arrow_group = new KButtonGroup;
    QHBoxLayout *offset_arrow_layout = new QHBoxLayout;
    offset_group_layout->addWidget( offset_arrow_group, 4, 0, 1, 2, Qt::AlignCenter );
    offset_arrow_group->setLayout( offset_arrow_layout );
    KPushButton *offset_min_arrow = new KPushButton(KIcon("go-first-view"), "Left");
    KPushButton *offset_max_arrow = new KPushButton(KIcon("go-last-view"),  "Right");
    offset_arrow_layout->addWidget( offset_min_arrow );
    offset_arrow_layout->addWidget( offset_max_arrow );

    m_size_spin->setValue( 100 * ( (long double)m_current_part->geom.length )/ max_size ); 
    m_offset_spin->setValue( 100 * (( (long double)m_current_part->geom.length - m_max_part_start ) / max_size) );

    connect(m_size_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(adjustSizeCombo(int)));

    connect(m_size_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validateVolumeSize(QString)));

    connect(m_size_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(adjustSizeEdit(int)));

    connect(m_offset_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(adjustOffsetCombo(int)));

    connect(m_offset_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validateOffsetSize(QString)));

    connect(m_offset_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(adjustOffsetEdit(int)));

    connect(m_size_group, SIGNAL(toggled(bool)),
	    this, SLOT(resetSizeGroup(bool)));

    connect(m_size_group, SIGNAL(toggled(bool)),
	    this, SLOT(setOffsetSpinMinMax()));

    connect(m_offset_group, SIGNAL(toggled(bool)),
	    this, SLOT(resetOffsetGroup(bool)));

    connect(this, SIGNAL(okClicked()),
	    this, SLOT(commitPartition()));

    connect(this, SIGNAL(resetClicked()),
	    this, SLOT(resetPartition()));

    connect(size_min_arrow, SIGNAL(clicked(bool)),
	    this, SLOT(minimizePartition(bool)));

    connect(size_max_arrow, SIGNAL(clicked(bool)),
	    this, SLOT(maximizePartition(bool)));

    connect(offset_min_arrow, SIGNAL(clicked(bool)),
	    this, SLOT(minimizeOffset(bool)));

    connect(offset_max_arrow, SIGNAL(clicked(bool)),
	    this, SLOT(maximizeOffset(bool)));

    adjustSizeCombo( m_size_combo->currentIndex() );
    adjustOffsetCombo( m_offset_combo->currentIndex() );
    resetDisplayGraphic();
    resetOffsetGroup(false);
    resetSizeGroup(false);
    setOffsetSpinMinMax();
    setSizeLabels();
}

void PartitionMoveResizeDialog::commitPartition()
{

    hide();

    bool grow   = false;
    bool shrink = false;
    bool move   = ( m_offset_group->isChecked() && 
                  ( m_new_part_start != m_current_part->geom.start ) );

    if( m_size_group->isChecked() ){
        if( m_new_part_size < m_current_part->geom.length ){
            grow   = false;
            shrink = true;
        }
        else if( m_new_part_size > m_current_part->geom.length ){
            grow   = true;
            shrink = false;
        }
        else{
            grow   = false;
            shrink = false;
        }
    }

    if( grow && move ){
        if( movePartition() )
            growPartition();
    }
    else if( grow ){
        growPartition();
    }
    else if( shrink && move ){
        if( shrinkPartition() )
            movePartition();
    }
    else if( shrink ){
        shrinkPartition();
    }
    else if( move ){
        movePartition();
    }
}

void PartitionMoveResizeDialog::adjustSizeEdit(int percentage){

    const PedSector max_sectors  = 1 + m_max_part_end - m_max_part_start;
    const PedSector current_size = m_current_part->geom.length;

    setSizeSpinMinMax();

    if( m_size_group->isChecked() ){

        if( percentage >= m_size_spin->maximum() )
            m_new_part_size = m_max_part_end - m_new_part_start;
        else if( ( percentage <= m_size_spin->minimum() ) || ( percentage <= 0 ) )
            m_new_part_size = m_min_shrink_size;
        else
            m_new_part_size = (long long)(( (double) percentage / 100) * ( max_sectors ));
    }
    else 
        m_new_part_size = current_size;

    adjustSizeCombo( m_size_combo->currentIndex() );

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

    setOffsetSpinMinMax();
    setSizeSpinMinMax();
    setSizeLabels();
    resetDisplayGraphic();
    resetOkButton();
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
    const long long max_start  = 1 + m_max_part_end - m_current_part->geom.length;

    if( m_size_group->isChecked() ){

        if(percentage >= m_offset_spin->maximum())
            m_new_part_start = 1 + m_max_part_end - m_new_part_size;
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

    setSizeSpinMinMax();
    setSizeLabels();
    adjustOffsetCombo( m_offset_combo->currentIndex() );
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

void PartitionMoveResizeDialog::validateOffsetSize(QString size){

    int x = 0;
    m_new_part_start = m_max_part_start;
        
    if(m_offset_validator->validate(size, x) == QValidator::Acceptable){

        if(! m_offset_combo->currentIndex() )
            m_new_part_start += size.toLongLong();
        else
            m_new_part_start += convertSizeToSectors( m_offset_combo->currentIndex(), 
                                                      size.toDouble() );
    }
    else
        m_new_part_start = m_current_part->geom.start;

    adjustSizeCombo( m_size_combo->currentIndex() );

}

/* if the offset group is unchecked then reset to original value */

void PartitionMoveResizeDialog::resetOffsetGroup(bool on){

    const long long max_size = 1 + m_max_part_end - m_max_part_start; 

    if( ! on ){
        m_new_part_start = m_current_part->geom.start;
        m_offset_spin->setValue( 100 * (( (long double)m_new_part_start - m_max_part_start ) / max_size) );
        adjustOffsetCombo( m_offset_combo->currentIndex() );
    }
}

/* if the size group is unchecked then reset to original value */

void PartitionMoveResizeDialog::resetSizeGroup(bool on){

    const PedSector max_size     = 1 + m_max_part_end - m_max_part_start; 
    const PedSector current_size = m_current_part->geom.length;

    if( ! on ){
        m_size_spin->setValue( 100 * (long double)current_size / max_size ); 
        m_new_part_size = current_size;
        adjustSizeCombo( m_size_combo->currentIndex() );
   }
}

void PartitionMoveResizeDialog::resetPartition(){

    resetOffsetGroup(false);
    resetSizeGroup(false);

}

void PartitionMoveResizeDialog::setup(){

    QString fs =  m_old_storage_part->getFileSystem();

    m_current_part = m_old_storage_part->getPedPartition();
    m_ped_disk = m_current_part->disk;   

    PedDevice   *ped_device = m_ped_disk->dev;
    PedGeometry *ped_max_geometry  = NULL;

    m_new_part_start  = (m_current_part->geom).start;
    m_new_part_size   = (m_current_part->geom).length;


    /* how big can it grow? */

    PedConstraint *constraint = ped_device_get_constraint(ped_device);

    ped_max_geometry = ped_disk_get_max_partition_geometry( m_ped_disk, 
                                                            m_current_part, 
                                                            constraint );

    m_max_part_start  = ped_max_geometry->start;
    m_max_part_end    = ped_max_geometry->end;
    m_ped_sector_size = ped_device->sector_size;

    if( m_current_part->type & PED_PARTITION_LOGICAL )
        m_logical = true;
    else
        m_logical = false;

    // +2 because counting starts at zero (+1) and one is used for metadata(+1)
    if( m_old_storage_part->isPV() ){
        m_min_shrink_size = 2 + m_old_storage_part->getPhysicalVolume()->getLastUsedExtent();
        m_min_shrink_size *= m_old_storage_part->getPhysicalVolume()->getExtentSize();
        m_min_shrink_size /= m_ped_sector_size;
    }
    else{
        m_min_shrink_size = get_min_fs_size(ped_partition_get_path(m_current_part), fs) / m_ped_sector_size;
    }

    if( m_min_shrink_size == 0 )                 // 0 means we can't shrink it 
        m_min_shrink_size = m_new_part_size;

}

bool PartitionMoveResizeDialog::movefs(long long from_start, 
                                       long long to_start, 
                                       long long length){

    const long long blocksize = 8000;    // sectors moved in each block

    PedDevice *device = m_ped_disk->dev;

    char *buff = static_cast<char *>( malloc( blocksize * m_ped_sector_size ) ) ;

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
            if( ! ped_device_read( device, buff, from_start + (x * blocksize), blocksize) ){
                KMessageBox::error( 0, "Move failed: could not read from device");
                return false;
            }
            if( ! ped_device_write(device, buff, to_start   + (x * blocksize), blocksize) ){
                KMessageBox::error( 0, "Move failed: could not write to device");
                return false;
            }
            progress_bar->setValue( x );
        }
        if( ! ped_device_read(device,  buff, from_start + (blockcount * blocksize), extra) ){
            KMessageBox::error( 0, "Move failed: could not read from device");
            return false;
        }
        if( ! ped_device_write(device, buff, to_start   + (blockcount * blocksize), extra) ){
            KMessageBox::error( 0, "Move failed: could not write to device");
            return false;
        }
    }
    else{                                              // moving right
        if( ! ped_device_read(device,  buff, from_start + (blockcount * blocksize), extra) ){
            KMessageBox::error( 0, "Move failed: could not read from device");
            return false;
        }
        if( ! ped_device_write(device, buff, to_start   + (blockcount * blocksize), extra) ){
            KMessageBox::error( 0, "Move failed: could not write to device");
            return false;
        }
        for( long long x = blockcount - 1; x >= 0 ; x--){
            if( ! ped_device_read( device, buff, from_start + (x * blocksize), blocksize) ){
                KMessageBox::error( 0, "Move failed: could not read from device");
                return false;
            }
            if( ! ped_device_write(device, buff, to_start   + (x * blocksize), blocksize) ){
                KMessageBox::error( 0, "Move failed: could not write to device");
                return false;
            }
            progress_bar->setValue( blockcount - x );
        }
    }

    ped_device_sync( device );
    ped_device_close( device );

    return true;
}

bool PartitionMoveResizeDialog::shrinkPartition(){

    PedDevice    *device = m_ped_disk->dev;
    PedAlignment *start_alignment = ped_alignment_new(0, 1);
    PedAlignment *end_alignment   = ped_alignment_new(0, 1);

    PedSector new_size;

    const QString fs = m_old_storage_part->getFileSystem();
    const bool is_pv = m_old_storage_part->isPV();

    if( m_new_part_size >= m_current_part->geom.length ){
        qDebug() << "shrinkPartition called with a *bigger* part size!";
        return false;
    }

    if( m_new_part_size < m_min_shrink_size)
        new_size = m_min_shrink_size;
    else if( m_new_part_size > ( 1 + m_max_part_end - m_current_part->geom.start ) )
        new_size = 1 + m_max_part_end - m_current_part->geom.start;
    else
        new_size = m_new_part_size;

    if( is_pv ){
        m_new_part_size = shrink_pv( ped_partition_get_path(m_current_part) , 
                                     new_size * m_ped_sector_size ) / m_ped_sector_size;
    }
    else{
        m_new_part_size = shrink_fs( ped_partition_get_path(m_current_part), 
                                     new_size * m_ped_sector_size, 
                                     fs ) / m_ped_sector_size;
    }

    if( m_new_part_size == 0 )  // The shrink failed
        return false;


    // This constraint assures we have a new partition at least as long as the fs can shrink it
    // We give an extra 128 sectors for the end of the partition

    PedGeometry *start_range = ped_geometry_new( device, m_current_part->geom.start, 1);
    PedGeometry *end_range   = ped_geometry_new( device, m_current_part->geom.start + m_new_part_size, 128 );

    PedSector minimum_size = m_new_part_size;
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

    int success = ped_disk_set_partition_geom( m_ped_disk, 
                                               m_current_part, 
                                               constraint, 
                                               m_max_part_start, 
                                               m_max_part_end );

    if( ! success ){  
        KMessageBox::error( 0, "Could not shrink partition");
        return false;
    }
    else {
        waitPartitionTableReload( ped_partition_get_path(m_current_part), m_ped_disk);

        // This test only matters if we are also going to move the partition after shrinking

        if( (m_new_part_start + m_current_part->geom.length - 1) > m_max_part_end )
            m_new_part_start = 1 + m_max_part_end - m_current_part->geom.length;

        return true;
    }
}

bool PartitionMoveResizeDialog::growPartition(){

    PedDevice  *device = m_ped_disk->dev;

    const QString fs = m_old_storage_part->getFileSystem();
    const bool is_pv = m_old_storage_part->isPV();

    int success;

    PedSector min_new_size, 
              max_new_size;  // max desired size

    const PedSector current_start = m_current_part->geom.start;
    PedSector current_size  = m_current_part->geom.length;
    PedSector max_part_size = 1 + m_max_part_end - current_start; // max possible size

    if( m_new_part_size <= ( current_size + 128 ) )
        return false;
    else if( m_new_part_size >= max_part_size )
        min_new_size = max_part_size - 128;
    else
        min_new_size = m_new_part_size - 128;

    max_new_size = min_new_size + 128;

    PedGeometry *start_range = ped_geometry_new(device, current_start, 1);
    PedGeometry *end_range   = ped_geometry_new(device, current_start + min_new_size - 1, 127);

    PedConstraint *constraint = ped_constraint_new( ped_alignment_new(0, 1),
                                                    ped_alignment_new(0, 1),      
                                                    start_range,
                                                    end_range,
                                                    min_new_size,
                                                    max_new_size);

    /* if constraint solves to NULL then the new part will fail, so just bail out */
    if( ped_constraint_solve_max( constraint ) == NULL )
        return false;

    success = ped_disk_set_partition_geom( m_ped_disk, 
                                           m_current_part, 
                                           constraint, 
                                           m_max_part_start, 
                                           m_max_part_end );

    if( ! success ){  
        KMessageBox::error( 0, "Could not grow partition");
        return false;
    }
    else {
        // Here we wait for linux to re-read the partition table before doing anything else.
        // Otherwise the resize program will fail.
            
        waitPartitionTableReload( ped_partition_get_path(m_current_part), m_ped_disk);

        if( is_pv ){            
           if( grow_pv( ped_partition_get_path(m_current_part) ) )
                return true;
            else
                return false;
        }
        else{
            if(grow_fs( ped_partition_get_path(m_current_part), fs ) )
                return true;
            else
                return false;
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


void PartitionMoveResizeDialog::setOffsetSpinMinMax(){

    const PedSector max_sectors = 1 + m_max_part_end - m_max_part_start;
    int max;

    if( ! m_size_group->isChecked() )
        max = qRound( (1.0 - ( (double)m_current_part->geom.length / max_sectors ) ) * 100);
    else
        max = qRound( (1.0 - ( (double)m_new_part_size / max_sectors ) ) * 100);
    //        max = qRound( (1.0 - ( (double)m_min_shrink_size / max_sectors ) ) * 100);

    m_offset_spin->setMaximum( max );
    m_offset_spin->setMinimum(0);
}

void PartitionMoveResizeDialog::setSizeSpinMinMax(){

    const PedSector preceding_sectors = m_new_part_start - m_max_part_start;
    const PedSector max_sectors       = 1 + m_max_part_end - m_max_part_start;

    const int max = qRound( (1.0 - ( (double)preceding_sectors / max_sectors ) )  * 100);
    const int min = qRound( ( (double)m_min_shrink_size / max_sectors ) * 100);

    m_size_spin->setMaximum( max );
    m_size_spin->setMinimum( min );
}

void PartitionMoveResizeDialog::setSizeLabels(){

    const PedSector preceding_sectors = m_new_part_start - m_max_part_start;

    QString part_size = sizeToString(m_ped_sector_size * m_new_part_size);
    m_size_label->setText( i18n("Partition size: %1", part_size) );

    QString preceding_bytes_string = sizeToString(preceding_sectors * m_ped_sector_size);
    m_preceding_label->setText( i18n("Preceding space: %1", preceding_bytes_string) );

    PedSector following_sectors = m_max_part_end - ( m_new_part_start + m_new_part_size );
    PedSector following_space   = following_sectors * m_ped_sector_size;

    if(following_space < 0)
        following_space = 0;

    QString following_bytes_string = sizeToString(following_space);
    m_remaining_label->setText( i18n("Following space: %1", following_bytes_string) );
}

bool PartitionMoveResizeDialog::movePartition(){

    PedDevice *device = m_ped_disk->dev;

    const PedSector  max_part_size = m_max_part_end - m_max_part_start + 1;
    const PedSector  current_size  = m_current_part->geom.length;
    PedSector current_start = m_current_part->geom.start;

    PedAlignment *start_alignment = NULL;
    PedAlignment *end_alignment   = ped_alignment_new(0, 1);
    int success = 0;

    if( (current_size > ( max_part_size - 256 ))||( fabs(current_start - m_new_part_start) < 256 ) )
        return false;
    else if( (m_new_part_start + current_size - 1) > ( m_max_part_end - 128 ) )
        m_new_part_start -= 128;

    if( m_align64_check->isChecked() )
        start_alignment = ped_alignment_new(0, 128);
    else
        start_alignment = ped_alignment_new(0, 1);

    PedGeometry *start_range = ped_geometry_new(device, m_new_part_start, 128);
    PedGeometry *end_range   = ped_geometry_new(device, m_new_part_start + current_size, 128);

    PedConstraint *min_size_constraint = ped_constraint_new( start_alignment,
                                                             end_alignment,
                                                             start_range,
                                                             end_range,
                                                             current_size,
                                                             current_size + 256);

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

    PedSector old_start = current_start;
    PedSector old_size  = current_size;

    success = ped_disk_set_partition_geom( m_ped_disk, 
                                           m_current_part, 
                                           constraint, 
                                           m_max_part_start, 
                                           m_max_part_end );

    current_start = m_current_part->geom.start;

    if( ! success ){  
        KMessageBox::error( 0, "Could not move partition");
        return false;
    }
    else {
        if( ! movefs( old_start, current_start, old_size) ){
            return false;
        }
        else{
            waitPartitionTableReload( ped_partition_get_path(m_current_part), m_ped_disk);
            return true;
        }
    }
}

/* This function polls the /dev directory to see if the partition table has been 
   updated and waits until it has been. It quits after 10 seconds if nothing is
   happening. It won't work without devfs. */

bool PartitionMoveResizeDialog::waitPartitionTableReload(char *partitionPath, PedDisk *disk){

    struct stat status;
    struct timespec request = {0, 100000000};  // 1 tenth of a second

    if( stat(partitionPath, &status) )
        return false;

    time_t old_ctime = status.st_ctime;

    if( ! ped_disk_commit(disk) ){
        KMessageBox::error( 0, "Could not commit partition");
        return false;
    }

    for( int x = 1 ; x < 100 ; x++ ){  // ten seconds max waiting

        nanosleep(&request, NULL);

        if( ! stat(partitionPath, &status) ){
            if( old_ctime < status.st_ctime )
                return true;
        }
    } 

    return false;
}


void PartitionMoveResizeDialog::minimizePartition(bool){

    m_size_spin->setValue( m_size_spin->maximum() );
    m_size_spin->setValue( m_size_spin->minimum() );

}

void PartitionMoveResizeDialog::maximizePartition(bool){

    m_size_spin->setValue( m_size_spin->minimum() );
    m_size_spin->setValue( m_size_spin->maximum() );

}

void PartitionMoveResizeDialog::minimizeOffset(bool){

    m_offset_spin->setValue( m_offset_spin->maximum() );
    m_offset_spin->setValue( m_offset_spin->minimum() );

}

void PartitionMoveResizeDialog::maximizeOffset(bool){

    m_offset_spin->setValue( m_offset_spin->minimum() );
    m_offset_spin->setValue( m_offset_spin->maximum() );

}
