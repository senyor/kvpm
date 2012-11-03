/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "lvproperties.h"

#include <KGlobal>
#include <KLocale>
#include <KConfigSkeleton>

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>

#include "logvol.h"
#include "misc.h"
#include "mountentry.h"
#include "volgroup.h"



/* if segment = -1 we have a multi segement logical volume but
   we are not focused on any one segment. Therefor stripes and
   stripe size have no meaning */

LVProperties::LVProperties(LogVol *const volume, const int segment, QWidget *parent)
    : QWidget(parent),
      m_lv(volume)
{
    QVBoxLayout *const layout = new QVBoxLayout();
    layout->setSpacing(2);
    layout->setMargin(2);

    KConfigSkeleton skeleton;

    bool show_mount,
         show_fsuuid,
         show_fslabel,
         show_uuid;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);

    skeleton.setCurrentGroup("LogicalVolumeProperties");
    skeleton.addItemBool("lp_mount",   show_mount,   true);
    skeleton.addItemBool("lp_fsuuid",  show_fsuuid,  false);
    skeleton.addItemBool("lp_fslabel", show_fslabel, false);
    skeleton.addItemBool("lp_uuid",    show_uuid,    false);

    layout->addWidget(generalFrame(segment));

    if (!m_lv->isLvmMirrorLeg()  && !m_lv->isVirtual()      && !m_lv->isRaidImage()    &&
        !m_lv->isPvmove()        && !m_lv->isLvmMirrorLog() && !m_lv->isMetadata()     &&
        !m_lv->isSnapContainer() && !m_lv->isThinPool()     && !m_lv->isThinPoolData() &&
        ((m_lv->getSegmentCount() == 1) || (segment == -1))) {

        if (show_mount)
            layout->addWidget(mountPointsFrame());

        layout->addWidget(physicalVolumesFrame(segment));

        if (show_fsuuid || show_fslabel)
            layout->addWidget(fsFrame(show_fsuuid, show_fslabel));
    } else
        layout->addWidget(physicalVolumesFrame(segment));

    if (show_uuid && !m_lv->isSnapContainer() && ((m_lv->getSegmentCount() == 1) || (segment == -1)))
        layout->addWidget(uuidFrame());

    layout->addStretch();

    setLayout(layout);
}

QFrame *LVProperties::mountPointsFrame()
{
    QLabel *label;
    QFrame *const frame = new QFrame();
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    frame->setLineWidth(2);

    const QList<MountEntry *> entries = m_lv->getMountEntries();

    if (entries.size() > 1) {
        label = new QLabel(i18n("<b>Mount Points</b>"));
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    } else {
        label = new QLabel(i18n("<b>Mount Point</b>")) ;
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }

    if (entries.size() == 0) {
        label = new QLabel(i18n("not mounted")) ;
        label->setAlignment(Qt::AlignLeft);
        layout->addWidget(label);
    } else {
        for (int x = 0; x < entries.size(); x++) {
            const int pos = entries[x]->getMountPosition();
            const QString mp = entries[x]->getMountPoint();

            if (pos > 0) {
                label = new QLabel(QString("%1 <%2>").arg(mp).arg(pos));
                label->setToolTip(QString("%1 <%2>").arg(mp).arg(pos));
                layout->addWidget(label);
            } else {
                label = new QLabel(mp);
                label->setToolTip(mp);
                layout->addWidget(label);
            }
        }
    }

    QListIterator<MountEntry *> entry_itr(entries);
    while (entry_itr.hasNext())
        delete entry_itr.next();

    return frame;
}

QFrame *LVProperties::uuidFrame()
{
    QStringList uuid;
    QFrame *const frame = new QFrame();
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    frame->setLineWidth(2);

    QLabel *label = new QLabel(i18n("<b>Logical Volume UUID</b>"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    uuid = splitUuid(m_lv->getUuid());
    label = new QLabel(uuid[0]);
    label->setToolTip(m_lv->getUuid());
    layout->addWidget(label);

    label = new QLabel(uuid[1]);
    label->setToolTip(m_lv->getUuid());
    layout->addWidget(label);

    return frame;
}

QFrame *LVProperties::fsFrame(const bool showFsUuid, const bool showFsLabel)
{
    QStringList uuid;
    QFrame *const frame = new QFrame();
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    frame->setLineWidth(2);
    QLabel *label;

    if (showFsLabel) {
        label = new QLabel(i18n("<b>Filesystem Label</b>"));
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
        label = new QLabel(m_lv->getFilesystemLabel());
        label->setToolTip(m_lv->getFilesystemLabel());
        label->setWordWrap(true);
        layout->addWidget(label);
    }

    if (showFsUuid) {
        label = new QLabel(i18n("<b>Filesystem UUID</b>"));
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);

        uuid = splitUuid(m_lv->getFilesystemUuid());
        label = new QLabel(uuid[0]);
        label->setToolTip(m_lv->getFilesystemUuid());
        layout->addWidget(label);
        label = new QLabel(uuid[1]);
        label->setToolTip(m_lv->getFilesystemUuid());
        layout->addWidget(label);
    }

    return frame;
}

QFrame *LVProperties::generalFrame(int segment)
{
    const long long extent_size = m_lv->getVg()->getExtentSize();
    const int segment_count = m_lv->getSegmentCount();
    long long extents, total_size, total_extents;
    int stripes, stripe_size;
    QStringList pv_list;

    QFrame *const frame = new QFrame();
    QVBoxLayout *const layout = new QVBoxLayout();
    frame->setLayout(layout);
    frame->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    frame->setLineWidth(2);

    KLocale::BinaryUnitDialect dialect;
    KLocale *const locale = KGlobal::locale();

    if (m_use_si_units)
        dialect = KLocale::MetricBinaryDialect;
    else
        dialect = KLocale::IECBinaryDialect;

    QString policy = policyToString(m_lv->getPolicy());
    if (m_lv->isLocked()) {
        policy.append(" (");
        policy.append(i18n("locked"));
        policy.append(")");
    }

    if (m_lv->isThinPool() || m_lv->isSnapContainer()) {
        total_extents = m_lv->getTotalSize() / extent_size;
        total_size = m_lv->getTotalSize();
        layout->addWidget(new QLabel(i18n("Total Extents: %1", total_extents)));
        layout->addWidget(new QLabel(i18n("Total Size: %1", locale->formatByteSize(total_size, 1, dialect))));

        if (m_lv->isWritable())
            layout->addWidget(new QLabel(i18n("Access: r/w")));
        else
            layout->addWidget(new QLabel(i18n("Access: r/o")));

        if (m_lv->isThinPool())
            layout->addWidget(new QLabel(i18n("Chunk Size: %1", locale->formatByteSize(m_lv->getChunkSize(), 1, dialect))));

        if (!m_lv->isThinVolume())
            layout->addWidget(new QLabel(i18n("Policy: %1", policy)));

    } else if ((segment >= 0) && (segment_count > 1)) {
        extents = m_lv->getSegmentExtents(segment);
        stripes = m_lv->getSegmentStripes(segment);
        stripe_size = m_lv->getSegmentStripeSize(segment);

        layout->addWidget(new QLabel(i18n("Extents: %1", extents)));

        if (!m_lv->isLvmMirror()) {

            QHBoxLayout *const stripe_layout = new QHBoxLayout();

            if (stripes != 1) {
                stripe_layout->addWidget(new QLabel(i18n("Stripes: %1", stripes)));
                stripe_layout->addWidget(new QLabel(i18n("Stripe size: %1", locale->formatByteSize(stripe_size, 1, dialect))));
            } else {
                stripe_layout->addWidget(new QLabel(i18n("Stripes: none")));
            }

            layout->addLayout(stripe_layout);
        }
    } else if ((segment >= 0) && (segment_count == 1)) {
        extents = m_lv->getSegmentExtents(segment);
        total_size = m_lv->getTotalSize();
        total_extents = total_size / extent_size;
        stripes = m_lv->getSegmentStripes(segment);
        stripe_size = m_lv->getSegmentStripeSize(segment);

        if (!(m_lv->isSnapContainer() || m_lv->isRaid() || m_lv->isLvmMirror()))
            layout->addWidget(new QLabel(i18n("Extents: %1", extents)));

        if ( !(m_lv->isRaidImage() || m_lv->isMetadata()) ) {
            QHBoxLayout *const stripe_layout = new QHBoxLayout();

            if (m_lv->isRaid() && m_lv->getRaidType() != 1) {
                layout->addWidget(new QLabel(i18n("Total extents: %1", total_extents)));
                layout->addWidget(new QLabel(i18n("Total size: %1", locale->formatByteSize(total_size, 1, dialect))));
                stripe_layout->addWidget(new QLabel(i18n("Stripes: %1", stripes)));
                stripe_layout->addWidget(new QLabel(i18n("Stripe size: %1", locale->formatByteSize(stripe_size, 1, dialect))));
            } else if (!m_lv->isThinVolume() && !m_lv->isLvmMirror() && !(m_lv->isRaid() && m_lv->getRaidType() == 1)) {
                
                if (stripes != 1) {
                    stripe_layout->addWidget(new QLabel(i18n("Stripes: %1", stripes)));
                    stripe_layout->addWidget(new QLabel(i18n("Stripe size: %1", locale->formatByteSize(stripe_size, 1, dialect))));
                } else
                    stripe_layout->addWidget(new QLabel(i18n("Stripes: none")));
                
            } else if (!m_lv->isThinVolume() && (!m_lv->isLvmMirrorLog() || (m_lv->isLvmMirrorLog() && m_lv->isLvmMirror()))) {
                layout->addWidget(new QLabel(i18n("Total extents: %1", total_extents)));
                layout->addWidget(new QLabel(i18n("Total size: %1", locale->formatByteSize(total_size, 1, dialect))));
            }

            layout->addLayout(stripe_layout);
 
            if (!(m_lv->isLvmMirrorLeg() || m_lv->isLvmMirrorLog() || m_lv->isThinPoolData()))
                layout->addWidget(new QLabel(i18n("Filesystem: %1", m_lv->getFilesystem())));
        }

        if (m_lv->isWritable())
            layout->addWidget(new QLabel(i18n("Access: r/w")));
        else
            layout->addWidget(new QLabel(i18n("Access: r/o")));

        if (!m_lv->isThinVolume())
            layout->addWidget(new QLabel(i18n("Policy: %1", policy)));

    } else {
        extents = m_lv->getExtents();
        layout->addWidget(new QLabel(i18n("Extents: %1", extents)));

        if (!(m_lv->isLvmMirrorLeg() || m_lv->isLvmMirrorLog())) {

            layout->addWidget(new QLabel(i18n("Filesystem: %1", m_lv->getFilesystem())));

            if (m_lv->isWritable())
                layout->addWidget(new QLabel(i18n("Access: r/w")));
            else
                layout->addWidget(new QLabel(i18n("Access: r/o")));

            layout->addWidget(new QLabel(i18n("Policy: %1", policy)));
        }
    }

    if (m_lv->isCowSnap())
        layout->addWidget(new QLabel(i18n("Origin: %1", m_lv->getOrigin())));

    return frame;
}

QFrame *LVProperties::physicalVolumesFrame(int segment)
{
    QStringList pv_list;
    QFrame *const frame = new QFrame();
    QVBoxLayout *const layout = new QVBoxLayout();

    frame->setLayout(layout);
    frame->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    frame->setLineWidth(2);

    QLabel *label = new QLabel(i18n("<b>Physical Volumes</b>"));
    if (m_lv->isThinVolume())
        label->setText(i18n("<b>Thin Pool</b>"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    if (m_lv->isThinVolume()) {
        label = new QLabel(m_lv->getPoolName());
        label->setToolTip(m_lv->getPoolName());
        layout->addWidget(label);
    } else if ((m_lv->isThinPool() || m_lv->isMirror() || m_lv->isSnapContainer() || m_lv->isRaid()) && !m_lv->isPvmove()) {
        pv_list = m_lv->getPvNamesAllFlat();
        for (int pv = 0; pv < pv_list.size(); pv++) {
            label = new QLabel(pv_list[pv]);
            label->setToolTip(pv_list[pv]);
            layout->addWidget(label);
        }
    } else if (segment > -1) {
        pv_list = m_lv->getPvNames(segment);
        for (int pv = 0; pv < pv_list.size(); pv++) {
            label = new QLabel(pv_list[pv]);
            label->setToolTip(pv_list[pv]);
            layout->addWidget(label);
        }
    } else {
        pv_list = m_lv->getPvNamesAll();
        for (int pv = 0; pv < pv_list.size(); pv++) {
            label = new QLabel(pv_list[pv]);
            label->setToolTip(pv_list[pv]);
            layout->addWidget(label);
        }
    }

    return frame;
}
