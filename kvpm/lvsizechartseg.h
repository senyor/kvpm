/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */
#ifndef LVSIZECHARTSEG_H
#define LVSIZECHARTSEG_H

#include <KMenu>

#include <QHBoxLayout>
#include <QString>

class VolGroup;
class LogVol;


class LVChartSeg : public QWidget
{
Q_OBJECT

     VolGroup *vg;
     LogVol *lv;
     KMenu *context_menu;
     
 public:
     LVChartSeg(VolGroup *VolumeGroup, LogVol *LogicalVolume, QString use, QWidget *parent);

 private slots:
     void popupContextMenu(QPoint);
     void extendLogicalVolume();
     void createLogicalVolume();
     void changeLogicalVolume();
     void reduceLogicalVolume();
     void removeLogicalVolume();
     void addMirror();
     void removeMirror();
     void createSnapshot();
     void mkfsLogicalVolume();
     void movePhysicalExtents();
     void mountFilesystem();
     void unmountFilesystem();

};

#endif
