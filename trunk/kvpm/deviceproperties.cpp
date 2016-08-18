/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
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

#include "misc.h"
#include "mountentry.h"
#include "physvol.h"
#include "storagedevice.h"
#include "storagepartition.h"

#include <KConfigSkeleton>
#include <KLocalizedString>

#include <QLabel>
#include <QVBoxLayout>



DeviceProperties::DeviceProperties(StorageDevice *const device, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *const layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);

    layout->addWidget(generalFrame(device));
    layout->addWidget(hardwareFrame(device));

    if (device->isPhysicalVolume())
        layout->addWidget(pvFrame(device->getPhysicalVolume()));

    setLayout(layout);

    layout->addStretch();
}

DeviceProperties::DeviceProperties(StoragePartition *const partition, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *const layout = new QVBoxLayout();
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->addWidget(generalFrame(partition));

    KConfigSkeleton skeleton;

    bool show_mount,
         show_fsuuid,
         show_fslabel;

    skeleton.setCurrentGroup("DeviceProperties");
    skeleton.addItemBool("dp_mount",   show_mount,   true);
    skeleton.addItemBool("dp_fsuuid",  show_fsuuid,  false);
    skeleton.addItemBool("dp_fslabel", show_fslabel, false);

    if (partition->isMountable()) {
        if (show_mount)
            layout->addWidget(mpFrame(partition));

        if (show_fsuuid || show_fslabel)
            layout->addWidget(fsFrame(partition, show_fsuuid, show_fslabel));
    } else if (partition->isPhysicalVolume()) {
        layout->addWidget(pvFrame(partition->getPhysicalVolume()));
    }

    setLayout(layout);

    layout->addStretch();
}

QFrame *DeviceProperties::generalFrame(StoragePartition *const partition)
{
    QFrame *const frame = new QFrame;
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    frame->setLineWidth(2);

    QLabel *const name_label =  new QLabel(QString("<b>%1</b>").arg(partition->getName()));
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    layout->addWidget(new QLabel(i18n("First sector: %1", partition->getTrueFirstSector())));

    if (partition->isFreespace()) {
        layout->addWidget(new QLabel(i18n("First aligned: %1 (to 1 MiB)", partition->getFirstSector())));
    }

    layout->addWidget(new QLabel(i18n("Last sector: %1", partition->getLastSector())));

    if (partition->isNormal() || partition->isLogical() || partition->isExtended()) {
        layout->addWidget(new QLabel());
        const QStringList flags = partition->getFlags();
        layout->addWidget(new QLabel(i18n("Flags: %1", flags.join(", "))));
    }

    frame->setLayout(layout);

    return frame;
}

QFrame *DeviceProperties::mpFrame(StoragePartition *const partition)
{
    QFrame *const frame = new QFrame();
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    frame->setLineWidth(2);

    auto entries = partition->getMountEntries();

    QLabel *label;

    if (entries.size() <= 1)
        label = new QLabel(i18n("<b>Mount Point</b>"));
    else
        label = new QLabel(i18n("<b>Mount Points</b>"));

    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    if (!entries.isEmpty()) {
        for (int x = 0; x < entries.size(); x++) {
            const QString mp = entries[x]->getMountPoint();
            const int pos = entries[x]->getMountPosition();

            if (entries[x]->getMountPosition() > 0) {
                label = new QLabel(mp + QString("<%1>").arg(pos));
                label->setToolTip(mp + QString("<%1>").arg(pos));
                layout->addWidget(label);
            } else {
                label = new QLabel(mp);
                label->setToolTip(mp);
                layout->addWidget(label);
            }
        }
    } else {
        layout->addWidget(new QLabel(i18n("Not mounted")));
    }

    frame->setLayout(layout);

    return frame;
}

QFrame *DeviceProperties::generalFrame(StorageDevice *const device)
{
    QFrame *frame = new QFrame;
    QVBoxLayout *layout = new QVBoxLayout();
    frame->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    frame->setLineWidth(2);

    QLabel *name_label = new QLabel(QString("<b>%1</b>").arg(device->getName()));
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    if (device->isPhysicalVolume())
        layout->addWidget(new QLabel(device->getDiskLabel()));
    else
        layout->addWidget(new QLabel(i18n("Partition table: %1", device->getDiskLabel())));

    layout->addWidget(new QLabel(i18n("Logical sector size: %1", device->getSectorSize())));
    layout->addWidget(new QLabel(i18n("Physical sector size: %1", device->getPhysicalSectorSize())));
    layout->addWidget(new QLabel(i18n("Sectors: %1", device->getSize() / device->getSectorSize())));

    if (!device->isWritable())
        layout->addWidget(new QLabel(i18nc("May be read and not written", "Read only")));
    else
        layout->addWidget(new QLabel(i18n("Read/write")));

    if (device->isBusy())
        layout->addWidget(new QLabel(i18n("Busy: Yes")));
    else
        layout->addWidget(new QLabel(i18n("Busy: No")));

    frame->setLayout(layout);

    return frame;
}

QFrame *DeviceProperties::fsFrame(StoragePartition *const partition, const bool showFsUuid, const bool showFsLabel)
{
    QLabel *label;
    QFrame *const frame = new QFrame;
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    frame->setLineWidth(2);

    if (showFsLabel) {
        label = new QLabel(i18n("<b>Filesystem Label</b>"));
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        label = new QLabel(partition->getFilesystemLabel());
        layout->addWidget(label);
    }

    if (showFsUuid) {
        label = new QLabel(i18n("<b>Filesystem UUID</b>"));
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        const QStringList uuid = splitUuid(partition->getFilesystemUuid());
        layout->addWidget(new QLabel(uuid[0]));
        layout->addWidget(new QLabel(uuid[1]));
    }

    frame->setLayout(layout);

    return frame;
}

QFrame *DeviceProperties::pvFrame(PhysVol *const pv)
{
    QFrame *const frame = new QFrame;
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    frame->setLineWidth(2);

    QLabel *label;
    label = new QLabel(i18n("<b>Physical Volume</b>"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    if (pv->isActive())
        label = new QLabel(i18n("State: active"));
    else
        label = new QLabel(i18n("State: inactive"));
    layout->addWidget(label);

    label = new QLabel(i18n("<b>UUID</b>"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    const QStringList uuid = splitUuid(pv->getUuid());
    layout->addWidget(new QLabel(uuid[0]));
    layout->addWidget(new QLabel(uuid[1]));

    frame->setLayout(layout);

    return frame;
}

QFrame *DeviceProperties::hardwareFrame(StorageDevice *const device)
{
    QFrame *const frame = new QFrame;
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    frame->setLineWidth(2);

    QLabel *label;
    label = new QLabel(i18n("<b>Hardware</b>"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    label = new QLabel(device->getHardware());
    label->setWordWrap(true);
    layout->addWidget(label);

    frame->setLayout(layout);

    return frame;
}
