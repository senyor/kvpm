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

    QColor color;

    QColor  ext2_color,   ext3_color,    ext4_color,
            reiser_color, reiser4_color, msdos_color,
            jfs_color,    xfs_color,     none_color,
            free_color,   swap_color,    hfs_color,
            btrfs_color,  ntfs_color;

    skeleton.setCurrentGroup("FilesystemColors");
    skeleton.addItemColor("ext2",    ext2_color,  Qt::blue);
    skeleton.addItemColor("ext3",    ext3_color,  Qt::darkBlue);
    skeleton.addItemColor("ext4",    ext4_color,  Qt::cyan);
    skeleton.addItemColor("btrfs",   btrfs_color,   Qt::yellow);
    skeleton.addItemColor("reiser",  reiser_color,  Qt::red);
    skeleton.addItemColor("reiser4", reiser4_color, Qt::darkRed);
    skeleton.addItemColor("msdos",   msdos_color, Qt::darkYellow);
    skeleton.addItemColor("jfs",     jfs_color,   Qt::magenta);
    skeleton.addItemColor("xfs",     xfs_color,   Qt::darkCyan);
    skeleton.addItemColor("hfs",     hfs_color,   Qt::darkMagenta);
    skeleton.addItemColor("ntfs",    ntfs_color,  Qt::darkGray);
    skeleton.addItemColor("none",    none_color,  Qt::black);
    skeleton.addItemColor("free",    free_color,  Qt::green);
    skeleton.addItemColor("swap",    swap_color,  Qt::lightGray);

    if (use == "thin_pool") {
        color = free_color;
        m_brush = QBrush(color, Qt::Dense4Pattern);
    } else {

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
