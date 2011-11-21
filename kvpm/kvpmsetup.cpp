/*
 *
 * 
 * Copyright (C) 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "kvpmsetup.h" 

#include <KConfigSkeleton>
#include <KColorButton>

#include <QtGui>


bool isconfigured_kvpm()
{
    bool configured;

    KConfigSkeleton skeleton;

    skeleton.addItemBool( "isconfigured", configured, false );

    if( configured )
        return true;
    else
        return false;
}


bool setup_kvpm()
{
    KConfigSkeleton *skeleton = new KConfigSkeleton();

    int fs_warn_percent,
        pv_warn_percent;

    bool configured, device, partition, capacity, devremaining, usage, group, flags, mount,
         volume, size, remaining, type, filesystem, stripes, stripesize, snapmove, 
         state, access, tags, mountpoints, pvname, pvsize, pvremaining, pvused, pvstate,
         pvallocate, pvtags, pvlvnames, total, percent;

    bool show_vg_info, show_lv_bar;

    QColor ext2, ext3, ext4, btrfs, reiser, reiser4, physvol, msdos,
           jfs, xfs, hfs, ntfs, none, free, swap;

    QStringList default_entries;

    skeleton->addItemBool( "isconfigured", configured );

    configured = true;

    skeleton->setCurrentGroup("General");
    skeleton->addItemBool( "show_vg_info", show_vg_info );
    skeleton->addItemBool( "show_lv_bar",  show_lv_bar );

    show_vg_info = true;
    show_lv_bar  = true;

    skeleton->setCurrentGroup("DeviceTreeColumns");
    skeleton->addItemBool( "device",    device );
    skeleton->addItemBool( "partition", partition );
    skeleton->addItemBool( "capacity",  capacity );
    skeleton->addItemBool( "remaining", devremaining );
    skeleton->addItemBool( "usage",     usage );
    skeleton->addItemBool( "group",     group );
    skeleton->addItemBool( "flags",     flags );
    skeleton->addItemBool( "mount",     mount );

    device = true;
    partition = true;
    capacity = true;
    devremaining = true;
    usage = true;
    group = true;
    flags = true;
    mount = true;

    skeleton->setCurrentGroup("VolumeTreeColumns");
    skeleton->addItemBool( "volume",     volume );
    skeleton->addItemBool( "size",       size );
    skeleton->addItemBool( "remaining",  remaining );
    skeleton->addItemBool( "type",       type );
    skeleton->addItemBool( "filesystem", filesystem );
    skeleton->addItemBool( "stripes",    stripes );
    skeleton->addItemBool( "stripesize", stripesize );
    skeleton->addItemBool( "snapmove",   snapmove );
    skeleton->addItemBool( "state",      state );
    skeleton->addItemBool( "access",     access );
    skeleton->addItemBool( "tags",       tags );
    skeleton->addItemBool( "mountpoints", mountpoints );

    volume = true;
    size = true;
    remaining = true;
    type = true;
    filesystem = false;
    stripes = false;
    stripesize = false;
    snapmove = true;
    state = true;
    access = false;
    tags = true;
    mountpoints = false;

    skeleton->setCurrentGroup("PhysicalTreeColumns");
    skeleton->addItemBool( "pvname",      pvname );
    skeleton->addItemBool( "pvsize",      pvsize );
    skeleton->addItemBool( "pvremaining", pvremaining );
    skeleton->addItemBool( "pvused",      pvused );
    skeleton->addItemBool( "pvstate",     pvstate );
    skeleton->addItemBool( "pvallocate",  pvallocate );
    skeleton->addItemBool( "pvtags",      pvtags );
    skeleton->addItemBool( "pvlvnames",   pvlvnames );

    pvname = true;
    pvsize = true;
    pvremaining = true;
    pvused = false;
    pvstate = false;
    pvallocate = true;
    pvtags = true;
    pvlvnames = true;

    skeleton->setCurrentGroup("AllTreeColumns");
    skeleton->addItemBool( "total",   total );
    skeleton->addItemBool( "percent", percent );
    skeleton->addItemInt( "fs_warn", fs_warn_percent );
    skeleton->addItemInt( "pv_warn", pv_warn_percent );

    total   = true;
    percent = true;
    fs_warn_percent = 10;
    pv_warn_percent = 0;

    skeleton->setCurrentGroup("FilesystemColors");
    skeleton->addItemColor("ext2",    ext2);
    skeleton->addItemColor("ext3",    ext3);
    skeleton->addItemColor("ext4",    ext4);
    skeleton->addItemColor("btrfs",   btrfs);
    skeleton->addItemColor("reiser",  reiser);
    skeleton->addItemColor("reiser4", reiser4);
    skeleton->addItemColor("physvol", physvol);
    skeleton->addItemColor("msdos", msdos);
    skeleton->addItemColor("jfs",   jfs);
    skeleton->addItemColor("xfs",   xfs);
    skeleton->addItemColor("hfs",   hfs);
    skeleton->addItemColor("ntfs",  ntfs);
    skeleton->addItemColor("none",  none);
    skeleton->addItemColor("free",  free);
    skeleton->addItemColor("swap",  swap);

    ext2 = Qt::blue;
    ext3 = Qt::darkBlue;
    ext4 = Qt::cyan;
    btrfs   = Qt::yellow;
    reiser  = Qt::red;
    reiser4 = Qt::darkRed;
    physvol = Qt::darkGreen;
    msdos   = Qt::darkYellow;
    jfs     = Qt::magenta;
    xfs     = Qt::darkCyan;
    hfs     = Qt::darkMagenta;
    ntfs    = Qt::darkGray;
    none    = Qt::black;
    free    = Qt::green;
    swap    = Qt::lightGray;

    skeleton->setCurrentGroup("SystemPaths");
    skeleton->addItemStringList("SearchPath", default_entries, QStringList() );

    default_entries.clear();
    default_entries << "/sbin/" 
                    << "/usr/sbin/" 
                    << "/bin/" 
                    << "/usr/bin/" 
                    << "/usr/local/bin/"
                    << "/usr/local/sbin/";

    skeleton->writeConfig();

    delete skeleton;

    return true;
}


