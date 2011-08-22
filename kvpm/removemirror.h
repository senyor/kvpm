/*
 *
 * 
 * Copyright (C) 2008, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef LVREMOVEMIRROR_H
#define LVREMOVEMIRROR_H

#include <KDialog>

#include <QLabel>
#include <QList>
#include <QStringList>


class LogVol;
class NoMungeCheck;
class VolGroup;

bool remove_mirror(LogVol *logicalVolume);

class RemoveMirrorDialog : public KDialog
{
Q_OBJECT

     LogVol *m_lv;
     VolGroup *m_vg;

     QList<NoMungeCheck *> m_mirror_leg_checks;
     
public:
     explicit RemoveMirrorDialog(LogVol *logicalVolume, QWidget *parent = 0);
     QStringList arguments();

private slots:     
     void validateCheckStates(int);
     
};

#endif
