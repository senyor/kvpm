/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "lvsizechartseg.h"

#include <KConfigSkeleton>
#include <KLocale>
#include <KMenu>

#include <QDebug>
#include <QHBoxLayout>
#include <QPainter>
#include <QString>

#include "logvol.h"
#include "lvactionsmenu.h"



/* This should be passed *lv = 0 if it is really free space on a volume
   group that is being displayed */

LVChartSeg::LVChartSeg(LogVol *const volume, const QString use, QWidget *parent) :
    QWidget(parent),
    m_lv(volume)
{
    KConfigSkeleton skeleton;

    QColor  ext2_color,   ext3_color,    ext4_color,
            reiser_color, reiser4_color, msdos_color,
            jfs_color,    xfs_color,     none_color,
            ntfs_color,   swap_color,    hfs_color,
            btrfs_color;

    skeleton.setCurrentGroup("FilesystemColors");
    skeleton.addItemColor("ext2",    ext2_color,  Qt::blue);
    skeleton.addItemColor("ext3",    ext3_color,  Qt::darkBlue);
    skeleton.addItemColor("ext4",    ext4_color,  Qt::cyan);
    skeleton.addItemColor("btrfs",   btrfs_color,   Qt::yellow);
    skeleton.addItemColor("reiser",  reiser_color,  Qt::red);
    skeleton.addItemColor("reiser4", reiser4_color, Qt::darkRed);
    skeleton.addItemColor("msdos",   msdos_color, Qt::darkYellow);
    skeleton.addItemColor("jfs",     jfs_color,   Qt::magenta);
    skeleton.addItemColor("xfs",     xfs_color,   Qt::darkGreen);
    skeleton.addItemColor("hfs",     hfs_color,   Qt::darkMagenta);
    skeleton.addItemColor("ntfs",    ntfs_color,  Qt::darkGray);
    skeleton.addItemColor("none",    none_color,  Qt::black);
    skeleton.addItemColor("swap",    swap_color,  Qt::lightGray);

    QColor mirror_color,  raid1_color,    raid456_color,
           thinvol_color, thinsnap_color, cowsnap_color,
           linear_color,  other_color,    pvmove_color,
           invalid_color, free_color; 

    skeleton.setCurrentGroup("VolumeTypeColors");
    skeleton.addItemColor("mirror",   mirror_color,   Qt::darkBlue);   // lvm type mirror
    skeleton.addItemColor("raid1",    raid1_color,    Qt::blue);
    skeleton.addItemColor("raid456",  raid456_color,  Qt::cyan);
    skeleton.addItemColor("thinvol",  thinvol_color,  Qt::lightGray);
    skeleton.addItemColor("thinsnap", thinsnap_color, Qt::darkRed);
    skeleton.addItemColor("cowsnap",  cowsnap_color,  Qt::darkYellow);
    skeleton.addItemColor("linear",   linear_color,   Qt::darkCyan);
    skeleton.addItemColor("pvmove",   pvmove_color,   Qt::magenta);
    skeleton.addItemColor("other",    other_color,    Qt::yellow);
    skeleton.addItemColor("invalid",  invalid_color,  Qt::red);
    skeleton.addItemColor("free",     free_color,     Qt::green);

    int type_combo_index;
    skeleton.setCurrentGroup("General");
    skeleton.addItemInt("type_combo", type_combo_index, 0);

    QColor color;

    if (m_lv && !m_lv->isActive() && m_lv->isValid()) {
        color = none_color;
        m_brush = QBrush(color, Qt::SolidPattern);
    } else if (use == "thin_pool") {
        color = free_color;
        m_brush = QBrush(color, Qt::Dense4Pattern);
    } else if (use == "freespace") {
        color = free_color;
        m_brush = QBrush(color, Qt::SolidPattern);
    } else if (m_lv && m_lv->isPvmove()) {   // must come before "mirror" because it's a type of mirror
        color = pvmove_color;
        m_brush = QBrush(color, Qt::SolidPattern);
    } else if (m_lv && type_combo_index == 1) {

        if (use == "ext2")
            color = ext2_color;
        else if (use == "ext3")
            color = ext3_color;
        else if (use == "ext4")
            color = ext4_color;
        else if (use == "btrfs")
            color = btrfs_color;
        else if (use == "reiserfs")
            color = reiser_color;
        else if (use == "reiser4")
            color = reiser4_color;
        else if (use == "hfs")
            color = hfs_color;
        else if (use == "vfat")
            color = msdos_color;
        else if (use == "jfs")
            color = jfs_color;
        else if (use == "xfs")
            color = xfs_color;
        else if (use == "ntfs")
            color = ntfs_color;
        else if (use == "swap")
            color = swap_color;
        else if (use == "freespace")
            color = free_color;
        else
            color = none_color;

        m_brush = QBrush(color, Qt::SolidPattern);
    } else if (m_lv){
        if (m_lv->isLvmMirror())
            color = mirror_color;
        else if (m_lv->isRaid() && m_lv->isMirror())
            color = raid1_color;
        else if (m_lv->isRaid() && !m_lv->isMirror())
            color = raid456_color;
        else if(m_lv->isCowSnap() && m_lv->isValid())
            color = cowsnap_color;
        else if(!m_lv->isValid())
            color = invalid_color;
        else if (m_lv->isPvmove())
            color = pvmove_color;
        else if (m_lv->isOrphan() || m_lv->isVirtual())
            color = other_color;
        else
            color = linear_color;

        m_brush = QBrush(color, Qt::SolidPattern);
    }

    setContextMenuPolicy(Qt::CustomContextMenu);
    
    if (m_lv)
        setToolTip(m_lv->getName());
    else
        setToolTip(i18n("free space"));
    
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(popupContextMenu(QPoint)));
}

void LVChartSeg::popupContextMenu(QPoint)
{
    emit lvMenuRequested(m_lv);
}

void LVChartSeg::paintEvent(QPaintEvent *)
{
    QPainter painter(this);

    painter.setBrush(m_brush);
    QRect rect = QRect(0, 0,  this->width(), this->height());
    painter.fillRect(rect, m_brush);
}
