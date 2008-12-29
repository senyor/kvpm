/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>
#include <KLocale>

#include "deviceproperties.h"
#include "storagedevice.h"

DeviceProperties::DeviceProperties( StorageDevice *Device, QWidget *parent) : QWidget(parent) 
{

    QVBoxLayout *layout = new QVBoxLayout();
    setLayout(layout);
    
    layout->addWidget( new QLabel( i18n("Device: %1").arg( Device->getDevicePath() ) ) );
    layout->addWidget( new QLabel( i18n("Disk label: %1").arg( Device->getDiskLabel() ) ) );
}

