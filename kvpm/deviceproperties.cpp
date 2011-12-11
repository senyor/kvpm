/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "deviceproperties.h"


#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KSeparator>

#include <QtGui>

#include "misc.h"
#include "physvol.h"
#include "storagedevice.h"
#include "storagepartition.h"


DeviceProperties::DeviceProperties(StorageDevice *const device, QWidget *parent) 
  : QWidget(parent) 
{
    QVBoxLayout *layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    setLayout(layout);

    layout->addWidget( generalFrame(device) );
    layout->addWidget( hardwareFrame(device) );

    if( device->isPhysicalVolume() )
	layout->addWidget( pvFrame( device->getPhysicalVolume() ) );

    layout->addStretch();
}

DeviceProperties::DeviceProperties( StoragePartition *const partition, QWidget *parent)
    : QWidget(parent) 
{
    QVBoxLayout *const layout = new QVBoxLayout();
    setLayout(layout);
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget( generalFrame(partition) );

    KConfigSkeleton skeleton;

    bool show_mount, 
         show_fsuuid, 
         show_fslabel;

    skeleton.setCurrentGroup("DeviceProperties");
    skeleton.addItemBool("mount",   show_mount,   true);
    skeleton.addItemBool("fsuuid",  show_fsuuid,  false);
    skeleton.addItemBool("fslabel", show_fslabel, false);
    
    if( partition->isMountable() ){
        if(show_mount)
            layout->addWidget( mpFrame(partition) );

        if(show_fsuuid || show_fslabel)
            layout->addWidget( fsFrame(partition, show_fsuuid, show_fslabel) );
    }
    else if( partition->isPhysicalVolume() )
	layout->addWidget( pvFrame( partition->getPhysicalVolume() ) );

    layout->addStretch();
}

QFrame *DeviceProperties::generalFrame(StoragePartition *const partition)
{
    QFrame *const frame = new QFrame;
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);   

    QLabel *const name_label =  new QLabel( QString("<b>%1</b>").arg( partition->getName() ) );
    name_label->setAlignment( Qt::AlignCenter );
    layout->addWidget(name_label);

    layout->addWidget( new QLabel( i18n("First sector: %1", partition->getFirstSector() ) ) );
    layout->addWidget( new QLabel( i18n("Last sector: %1", partition->getLastSector() ) ) );

    if( partition->getType() == "logical" || partition->getType() == "normal"){
        layout->addWidget( new QLabel() );
        const QStringList flags = partition->getFlags();
        layout->addWidget( new QLabel( i18n("Flags: %1", flags.join(", ") ) ) );
    }

    return frame;
}

QFrame *DeviceProperties::mpFrame(StoragePartition *const partition)
{
    QFrame *const frame = new QFrame();
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);   
    
    QStringList mount_points   = partition->getMountPoints();
    const QList<int>  mount_position = partition->getMountPosition();
    QLabel *label;    

    if( mount_points.size() <= 1 )
        label = new QLabel( i18n("<b>Mount point</b>") );
    else
        label = new QLabel( i18n("<b>Mount points</b>") );
    
    label->setAlignment( Qt::AlignCenter );
    layout->addWidget( label );
    
    if( mount_points.size() ){
        for(int x = 0; x < mount_points.size(); x++){
            if( mount_position[x] > 1 )
                mount_points[x] = mount_points[x] + QString("<%1>").arg(mount_position[x]);
            
            layout->addWidget( new QLabel( mount_points[x] ) );
        }
    }
    else{
        layout->addWidget( new QLabel( i18n("Not mounted") ) );
    }
    
    return frame;
}

QFrame *DeviceProperties::generalFrame(StorageDevice *const device)
{
    QFrame *frame = new QFrame;
    QVBoxLayout *layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);   

    QLabel *name_label = new QLabel( QString("<b>%1</b>").arg( device->getName() ) );
    name_label->setAlignment( Qt::AlignCenter );
    layout->addWidget(name_label);

    if( device->isPhysicalVolume() )
        layout->addWidget( new QLabel( device->getDiskLabel() ) );
    else
        layout->addWidget( new QLabel( i18n("Partition table: %1", device->getDiskLabel() ) ) );

    layout->addWidget( new QLabel( i18n("Logical sector size: %1", device->getSectorSize() ) ) );
    layout->addWidget( new QLabel( i18n("Physical sector size: %1", device->getPhysicalSectorSize() ) ) );
    layout->addWidget( new QLabel( i18n("Sectors: %1", device->getSize() / device->getSectorSize() ) ) );

    if( !device->isWritable() )
        layout->addWidget( new QLabel( i18nc("May be read and not written", "Read only") ) );
    else
        layout->addWidget( new QLabel( i18n("Read/write") ) );

    if( device->isBusy() )
        layout->addWidget( new QLabel( i18n("Busy: Yes") ) );
    else
        layout->addWidget( new QLabel( i18n("Busy: No") ) );

    return frame;
}

QFrame *DeviceProperties::fsFrame(StoragePartition *const partition, const bool showFsUuid, const bool showFsLabel)
{
    QLabel *label;
    QFrame *const frame = new QFrame;
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);   
    
    if(showFsLabel){
        label = new QLabel( i18n("<b>Filesystem LABEL</b>") );
        label->setAlignment( Qt::AlignCenter );
        layout->addWidget(label);
        
        label = new QLabel( partition->getFilesystemLabel() );
        layout->addWidget(label);
    }
    
    if(showFsUuid){
        label = new QLabel( i18n("<b>Filesystem UUID</b>") );
        label->setAlignment( Qt::AlignCenter );
        layout->addWidget(label);
        
        const QStringList uuid = splitUuid( partition->getFilesystemUuid() );
        layout->addWidget( new QLabel(uuid[0]) );
        layout->addWidget( new QLabel(uuid[1]) );
    }

    return frame;
}

QFrame *DeviceProperties::pvFrame(PhysVol *const pv)
{
    QFrame *const frame = new QFrame;
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);

    QLabel *label;    
    label = new QLabel( i18n("<b>Physical volume</b>") );
    label->setAlignment( Qt::AlignCenter );
    layout->addWidget(label);
    
    if( pv->isActive() )
        label = new QLabel( i18n("State: active") );
    else
        label = new QLabel( i18n("State: inactive") );
    layout->addWidget(label);
    
    label = new QLabel( i18n("<b>UUID</b>") );
    label->setAlignment( Qt::AlignCenter );
    layout->addWidget(label);
    
    const QStringList uuid = splitUuid( pv->getUuid() );
    layout->addWidget( new QLabel(uuid[0]) );
    layout->addWidget( new QLabel(uuid[1]) );
    
    return frame;
}

QFrame *DeviceProperties::hardwareFrame(StorageDevice *const device)
{
    QFrame *const frame = new QFrame;
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setLayout(layout);

    frame->setFrameStyle( QFrame::Sunken | QFrame::StyledPanel );
    frame->setLineWidth(2);   

    QLabel *label;
    label = new QLabel( i18n("<b>Hardware</b>") );
    label->setAlignment( Qt::AlignCenter );
    layout->addWidget(label);
    label = new QLabel( device->getHardware() );
    label->setWordWrap(true);
    layout->addWidget(label);

    return frame;
}
