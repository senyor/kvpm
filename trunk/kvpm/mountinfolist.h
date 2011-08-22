/*
 *
 * 
 * Copyright (C) 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef MOUNTINFOLIST_H
#define MOUNTINFOLIST_H

#include <QString>
#include <QList>

#ifndef MOUNTINFO_H
class MountInformation;
#endif

class MountInformationList
{

    QList<MountInformation *> m_list;
    
 public:
    MountInformationList();
    ~MountInformationList();
    QList<MountInformation *> getMountInformation(QString deviceName);
};

#endif
