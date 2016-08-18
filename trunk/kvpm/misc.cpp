/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "misc.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include <libdevmapper.h>

#include <QFileInfo>
//#include <QPushButton>

#include "masterlist.h"
#include "storagebase.h"
#include "storagedevice.h"
#include "storagepartition.h"


NoMungeCheck::NoMungeCheck(const QString text, QWidget *parent) : QCheckBox(text, parent)
{
    m_unmunged_text = text;
}

QString NoMungeCheck::getAlternateText()
{
    return m_alternate_text;
}

QStringList NoMungeCheck::getAlternateTextList()
{
    return m_alternate_text_list;
}

QString NoMungeCheck::getUnmungedText()
{
    return m_unmunged_text;
}

void NoMungeCheck::setAlternateText(QString alternateText)
{
    m_alternate_text = alternateText;
}

void NoMungeCheck::setAlternateTextList(QStringList alternateTextList)
{
    m_alternate_text_list = alternateTextList;
}

void NoMungeCheck::setData(QVariant data)
{
    m_data = data;
}

QVariant NoMungeCheck::getData()
{
    return m_data;
}

NoMungeRadioButton::NoMungeRadioButton(const QString text, QWidget *parent) : QRadioButton(text, parent)
{
    m_unmunged_text = text;
}

QString NoMungeRadioButton::getAlternateText()
{
    return m_alternate_text;
}

QString NoMungeRadioButton::getUnmungedText()
{
    return m_unmunged_text;
}

void NoMungeRadioButton::setAlternateText(QString alternateText)
{
    m_alternate_text = alternateText;
}

void NoMungeRadioButton::setData(QVariant data)
{
    m_data = data;
}

QVariant NoMungeRadioButton::getData()
{
    return m_data;
}

QStringList splitUuid(QString const uuid) // Turns a one line uuid into two shorter lines for veiwing
{
    int split_index = 0;

    if (uuid.count('-') < 4) {
        return QStringList(uuid) << "";
    } else if (uuid.count('-') < 5) {
        for (int x = 0; x < 4; x++)
            split_index = uuid.indexOf('-', split_index + 1);
    } else {
        for (int x = 0; x < 5; x++)
            split_index = uuid.indexOf('-', split_index + 1);
    }

    if (split_index <= 0)
        split_index = uuid.size() - 1;

    QString const uuid_start = uuid.left(split_index + 1);
    QString const uuid_end   = uuid.right((uuid.size() - split_index) - 1);

    return QStringList(uuid_start)  << uuid_end;
}

// Checks to see if a device is busy on linux 2.6+
bool isBusy(const QString device)
{
    struct stat     st_buf;
    int             fd;

    QByteArray dev_qba = device.toLocal8Bit();

    if (stat(dev_qba.data(), &st_buf) != 0)
        return false;

    fd = open(dev_qba.data(), O_RDONLY | O_EXCL);
    if (fd < 0) {
        if (errno == EBUSY)
            return true;
    } else
        close(fd);

    return false;
}

QString findMapperPath(QString name)
{
    QString mapper_name;

    QFileInfo fi(name);
    if (fi.exists())
        mapper_name = fi.canonicalFilePath();
    else
        return name;

    QByteArray qba = mapper_name.toLocal8Bit();
    struct stat fs;
    if (stat(qba.data(), &fs))  // error
        return name;
    
    dm_lib_init();
    dm_log_with_errno_init(NULL);
    
    char buf[1000];
    if (!dm_device_get_name(major(fs.st_rdev), minor(fs.st_rdev), 0, buf, 1000))
        return name;
    
    dm_lib_release();
    
    mapper_name = QString("/dev/mapper/").append(QString(buf));
    
    fi.setFile(mapper_name);
    if(fi.exists())
        return mapper_name;
    else
        return name;
}

// returns a list of pvs suitable for creating or extending a vg
QList<const StorageBase *> getUsablePvs()
{
    QList<const StorageBase *> devices;

    for (auto dev : MasterList::getStorageDevices() ) {
        if (!dev->isDmBlock() && !dev->isMdBlock() && !dev->isPhysicalVolume()) {
            if ((dev->getRealPartitionCount() == 0) && !dev->isBusy()) {
                devices.append(dev);
            } else if (dev->getRealPartitionCount() > 0) {
                for (auto part : dev->getStoragePartitions()) {
                    if (!part->isBusy() && !part->isPhysicalVolume() && ((part->isNormal()) || (part->isLogical()))) {
                        if (!part->isDmBlock() && !part->isMdBlock())
                            devices.append(part);
                    }
                }
            }
        }
    }
    
    return devices;
}

