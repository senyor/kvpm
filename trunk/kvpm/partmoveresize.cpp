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

#include "partmoveresize.h"
#include "partaddgraphic.h"
#include "sizetostring.h"


bool moveresize_partition(StoragePartition *partition)
{

    PartitionMoveResizeDialog dialog(partition);
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
 
    QLabel *path = new QLabel("<b>" + m_old_storage_part->getPartitionPath() + "</b>");
    path->setAlignment( Qt::AlignHCenter );
    layout->addWidget(path);

    m_size_group   = new QGroupBox( i18n("Modify partition size") );
    m_size_group->setCheckable(true);
    m_size_group->setChecked(false);

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
    m_align64_check = new QCheckBox("Align to 64 sectors");
    size_group_layout->addWidget(m_align64_check, 3, 0, 1, 2, Qt::AlignHCenter );

    m_preceding_label = new QLabel();
    size_group_layout->addWidget( m_preceding_label, 5, 0, 1, 2, Qt::AlignHCenter );

    m_remaining_label = new QLabel();
    size_group_layout->addWidget( m_remaining_label, 6, 0, 1, 2, Qt::AlignHCenter );

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

    offset_group_layout->addWidget(m_offset_edit,0,0);
    offset_group_layout->addWidget(m_preceding_combo,0,1);
    offset_group_layout->addWidget(m_offset_spin,1,0);

    const long long max_size = 1 + m_max_part_end - m_max_part_start;

    QString total_bytes = sizeToString( max_size * m_ped_sector_size );
    QLabel *preceding_label = new QLabel( i18n("Maximum size: %1",  total_bytes) );
    offset_group_layout->addWidget( preceding_label, 4, 0, 1, 2, Qt::AlignHCenter );

    QString remaining_bytes = sizeToString( (max_size - m_new_part_size) * m_ped_sector_size );

    m_unexcluded_label = new QLabel( i18n("Remaining space: %1", remaining_bytes ));
    offset_group_layout->addWidget( m_unexcluded_label, 5, 0, 1, 2, Qt::AlignHCenter );

    m_size_spin->setValue( 100 * ( (long double)m_old_part_size )/ max_size ); 

    m_offset_spin->setValue( 100 * (( (long double)m_old_part_start - m_max_part_start ) / max_size) );

    connect(m_size_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(adjustSizeCombo(int)));

    connect(m_size_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validateVolumeSize(QString)));

    connect(m_size_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(adjustSizeEdit(int)));

    connect(m_preceding_combo, SIGNAL(currentIndexChanged(int)),
	    this , SLOT(adjustPrecedingCombo(int)));

    connect(m_offset_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(validatePrecedingSize(QString)));

    connect(m_offset_spin, SIGNAL(valueChanged(int)),
	    this, SLOT(adjustPrecedingEdit(int)));

    connect(m_size_group, SIGNAL(toggled(bool)),
	    this, SLOT(resetSizeGroup(bool)));

    connect(m_offset_group, SIGNAL(toggled(bool)),
	    this, SLOT(resetOffsetGroup(bool)));

    connect(this, SIGNAL(okClicked()),
	    this, SLOT(commitPartition()));


    adjustSizeCombo( m_size_combo->currentIndex() );
    adjustPrecedingCombo( m_preceding_combo->currentIndex() );
    resetDisplayGraphic();

}

void PartitionMoveResizeDialog::commitPartition()
{
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

    const long long preceding_sectors = m_new_part_start - m_max_part_start;
    const long long max_sectors       = 1 + m_max_part_end - m_max_part_start;

    const int top = qRound( (1.0 - ( (double)preceding_sectors / max_sectors ) )  * 100);

    m_size_spin->setMaximum( top );

    if( percentage >= m_size_spin->maximum() )
        m_new_part_size = m_max_part_end - m_new_part_start;
    else if(percentage == 0)
        m_new_part_size = 0;
    else
        m_new_part_size = (long long)(( (double) percentage / 100) * ( max_sectors ));

    QString total_bytes = sizeToString(m_ped_sector_size * ( max_sectors - m_new_part_size ));
    m_unexcluded_label->setText( i18n("Available space: %1", total_bytes) );

    QString preceding_bytes_string = sizeToString(preceding_sectors * m_ped_sector_size);
    m_preceding_label->setText( i18n("Preceding space: %1", preceding_bytes_string) );

    PedSector following_sectors = m_max_part_end - ( m_new_part_start + m_new_part_size );
    long long following_space   = following_sectors * m_ped_sector_size;

    if(following_space < 0)
        following_space = 0;

    QString following_bytes_string = sizeToString(following_space);
    m_remaining_label->setText( i18n("Following space: %1", following_bytes_string) );

    adjustSizeCombo( m_size_combo->currentIndex() );
    resetDisplayGraphic();
    resetOkButton();
}


void PartitionMoveResizeDialog::adjustSizeCombo(int index){

    long double sized;

    if(index){
        sized = ((long double)m_new_part_size * m_ped_sector_size);
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
        m_size_edit->setText(QString("%1").arg(m_new_part_size));

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
    long long max_sectors       = 1 + m_max_part_end - m_max_part_start;


    QString total_bytes = sizeToString(m_ped_sector_size * ( max_sectors - m_new_part_size ));
    m_unexcluded_label->setText( i18n("Available space: %1", total_bytes) );

    QString preceding_bytes_string = sizeToString(preceding_sectors * m_ped_sector_size);
    m_preceding_label->setText( i18n("Preceding space: %1", preceding_bytes_string) );

    PedSector following_sectors = m_max_part_end - ( m_new_part_start + m_new_part_size );
    long long following_space   = following_sectors * m_ped_sector_size;

    if(following_space < 0)
        following_space = 0;

    QString following_bytes_string = sizeToString(following_space);
    m_remaining_label->setText( i18n("Following space: %1", following_bytes_string) );

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

    long long max_sectors = 1 + m_max_part_end - m_max_part_start;

    if( (m_new_part_size <= max_sectors) && (m_new_part_size > 0) )
        enableButtonOk(true);
    else
        enableButtonOk(false);



        enableButtonOk(true);


}

void PartitionMoveResizeDialog::adjustPrecedingEdit(int percentage){

    const long long max_length = 1 + m_max_part_end - m_max_part_start;

    // This is how far right we can move partition and keep it the same size
    const long long max_start  = 1 + m_max_part_end - m_old_part_size;

    const int top = qRound( (1.0 - ( (double)m_old_part_size / max_length ) )  * 100);

    if( m_size_group->isChecked() ){
        m_offset_spin->setMaximum( 100 );

        if(percentage == 100)
            m_new_part_start = m_max_part_end;
        else if(percentage == 0)
            m_new_part_start = m_max_part_start;
        else
            m_new_part_start = m_max_part_start + ( ( (double)percentage / 100 ) * max_length ); 
    }
    else{
        m_offset_spin->setMaximum( top );

        if(percentage >= top)
            m_new_part_start = max_start;
        else if(percentage == 0)
            m_new_part_start = m_max_part_start;
        else
            m_new_part_start = m_max_part_start + ( ( (double)percentage / 100 ) * max_length ); 

        if( m_new_part_start > max_start )
            m_new_part_start = max_start;
    }

    adjustPrecedingCombo( m_preceding_combo->currentIndex() );

    adjustSizeEdit(m_size_spin->value());

    resetOkButton();
}


void PartitionMoveResizeDialog::adjustPrecedingCombo(int index){

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
        m_offset_spin->setValue( 100 * (( (long double)m_new_part_start - m_max_part_start ) / max_size) );
        m_new_part_start = m_old_part_start;
        adjustPrecedingCombo( m_preceding_combo->currentIndex() );
    }
}

/* if the size group is unchecked then reset to original value */

void PartitionMoveResizeDialog::resetSizeGroup(bool on){

    const long long max_size = 1 + m_max_part_end - m_max_part_start; 

    if( ! on ){
        m_size_spin->setValue( 100 * ( (long double)m_new_part_size )/ max_size ); 
        m_new_part_size = m_old_part_size;
        adjustSizeCombo( m_size_combo->currentIndex() );
   }
}

void PartitionMoveResizeDialog::setup(){

    PedPartition  *ped_old_partition = m_old_storage_part->getPedPartition();
    PedGeometry    ped_old_geometry  = ped_old_partition->geom;         
    PedGeometry   *ped_max_geometry  = NULL;

    m_ped_disk = ped_old_partition->disk;
    PedDevice  *ped_device = m_ped_disk->dev;

//The hardware's own constraints, if any
    m_ped_constraints = ped_device_get_constraint(ped_device); 

    /* how big can it grow? */

    ped_max_geometry = ped_disk_get_max_partition_geometry( m_ped_disk, 
                                                            ped_old_partition, 
                                                            m_ped_constraints );

    m_max_part_start  = ped_max_geometry->start;
    m_max_part_end    = ped_max_geometry->end;
    m_ped_sector_size = ped_device->sector_size;
    m_new_part_start  = ped_old_geometry.start;
    m_new_part_size   = ped_old_geometry.length;
    m_old_part_start  = ped_old_geometry.start;
    m_old_part_size   = ped_old_geometry.length;

    if( ped_old_partition->type & PED_PARTITION_LOGICAL )
        m_logical = true;
    else
        m_logical = false;

}

bool PartitionMoveResizeDialog::shrinkfs(){
    return true;
}

bool PartitionMoveResizeDialog::growfs(){
    return true;
}

bool PartitionMoveResizeDialog::movefs(long long from_start, 
                                       long long to_start, 
                                       long long length){

    qDebug("From: %lld", from_start);
    qDebug("To:   %lld", to_start);
    qDebug("Leng: %lld", length);
    qDebug();

    const long long blocksize = 1000;

    PedDevice *device = m_ped_disk->dev;

    char *buff = static_cast<char *>( malloc( blocksize * m_ped_sector_size ) ) ;

    int error;
    long long blockcount = length / blocksize;
    long long extra = length % blocksize;
    qDebug("Block count: %lld", blockcount);
    ped_device_open( device );

    if( to_start < from_start ){                       // moving left
        for( long long x = 0; x < blockcount  ; x++){
            
            error = ped_device_read( device, buff, from_start + (x * blocksize), blocksize);
            ped_device_write(device, buff, to_start   + (x * blocksize), blocksize);

            qDebug() << "Goin left......"; 
            qDebug("From: %lld  To: %lld", from_start + (x*blocksize), to_start + (x*blocksize));

        }
        ped_device_read(device,  buff, from_start + (blockcount * blocksize), extra);
        ped_device_write(device, buff, to_start   + (blockcount * blocksize), extra);

        qDebug();
        qDebug("Start: %lld   End: %lld", to_start + (blocksize*blockcount), 
               to_start + (blocksize*blockcount) + extra -1 );
        qDebug();
        qDebug();
    }
    else{

        ped_device_read(device,  buff, from_start + (blockcount * blocksize), extra);
        ped_device_write(device, buff, to_start   + (blockcount * blocksize), extra);

        qDebug();
        qDebug("Start: %lld   End: %lld", to_start + (blocksize*blockcount), 
               to_start + (blocksize*blockcount) + extra - 1 );
        qDebug();
        qDebug();

        for( long long x = blockcount - 1; x >= 0 ; x--){
            
            qDebug() << "Goin Right......"; 
            qDebug("From: %lld  To: %lld", from_start + (x*blocksize), to_start + (x*blocksize));

            ped_device_read( device, buff, from_start + (x * blocksize), blocksize);
            ped_device_write(device, buff, to_start   + (x * blocksize), blocksize);

        }
        qDebug();
        qDebug();
    }

    ped_device_sync( device );
    ped_device_close( device );

    return true;
}

bool PartitionMoveResizeDialog::shrinkPartition(){
    return true;
}

bool PartitionMoveResizeDialog::growPartition(){
    return true;
}

bool PartitionMoveResizeDialog::movePartition(){

    //    PedAlignment *ped_start_alignment = ped_alignment_new(0, 64);

    PedPartition     *old_partition = m_old_storage_part->getPedPartition();
    PedPartition     *new_partition = NULL;
    PedPartitionType  type   = old_partition->type;
    PedDevice        *device = m_ped_disk->dev;
    PedAlignment     *start_alignment = ped_alignment_new(0, 1);
    PedAlignment     *end_alignment   = ped_alignment_new(0, 1);
    int error;

    // This constraint assures we have a new partition at least as long as the old one
    // We give +/- 64 sectors for the start and end of the partition

    PedGeometry *start_range = ped_geometry_new( device, m_new_part_start - 64, 128 );
    PedGeometry *end_range = ped_geometry_new( device, m_new_part_start + m_old_part_size - 64, 128 );

    PedSector minimum_size = m_old_part_size;
    PedSector maximum_size = m_old_part_size + 128;

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


    ped_disk_delete_partition( m_ped_disk, old_partition );

    new_partition = ped_partition_new( m_ped_disk, 
                                       type, 
                                       0, 
                                       m_max_part_start, 
                                       m_max_part_end );


    error = ped_disk_add_partition( m_ped_disk, 
                                    new_partition, 
                                    constraint);

    PedSector new_start = new_partition->geom.start;

    if( error ){

        movefs( m_old_part_start, new_start, m_old_part_size   );
        error = ped_disk_commit(m_ped_disk);

        if( ! error )  
            KMessageBox::error( 0, "Commiting of partition failed");
    }
    if( ! error )  
        KMessageBox::error( 0, "Creation of partition failed");




    return true;
}


void PartitionMoveResizeDialog::resetDisplayGraphic(){
   
    PedSector preceding_sectors = m_new_part_start - m_max_part_start;
    PedSector following_sectors = m_max_part_end - ( m_new_part_start + m_new_part_size );

    m_display_graphic->setPrecedingSectors(preceding_sectors);
    m_display_graphic->setPartitionSectors(m_new_part_size);
    m_display_graphic->setFollowingSectors(following_sectors);
    m_display_graphic->repaint();
}
