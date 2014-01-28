/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef UNMOUNT_H
#define UNMOUNT_H


#include <QStringList>

#include "mounttables.h"
#include "kvpmdialog.h"


class LogVol;
class MountEntry;
class NoMungeCheck;
class StoragePartition;


class UnmountDialog : public KvpmDialog
{
    Q_OBJECT

    bool m_single;
    QString m_mp;
    QList<NoMungeCheck *> m_check_list; // one check box for each mount point

public:
    UnmountDialog(StoragePartition *const partition, QWidget *parent = NULL);
    UnmountDialog(LogVol *const volume, QWidget *parent = NULL);
    void buildDialog(QString const device, const MountList entries);
    bool bailout();

private slots:
    void resetOkButton();
    void commit();

};


#endif
