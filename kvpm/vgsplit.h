/*
 *
 *
 * Copyright (C) 2010, 2011, 2012, 2013, 2014 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef VGSPLIT_H
#define VGSPLIT_H


#include <QList>
#include <QStringList>

#include "kvpmdialog.h"
#include "logvol.h"

class QLineEdit;
class QListWidget;
class QRegExpValidator;

class LogVol;
class VolGroup;


class VGSplitDialog : public KvpmDialog
{
    Q_OBJECT

    QListWidget *m_left_lv_list;
    QListWidget *m_right_lv_list;
    QListWidget *m_left_pv_list;
    QListWidget *m_right_pv_list;
    QPushButton *m_lv_add;
    QPushButton *m_lv_remove;
    QPushButton *m_pv_add;
    QPushButton *m_pv_remove;

    QLineEdit   *m_new_vg_name;
    QRegExpValidator *m_validator;
    VolGroup *m_vg;

    LvList getFullLvList();
    void deactivate();     // active lvs must be deactivated before moving

    QWidget *buildLvLists(const QStringList mobileLvNames, const QStringList fixedLvNames);
    QWidget *buildPvLists(const QStringList mobilePvNames, const QStringList fixedPvNames);

    QStringList getPvs(LogVol *const lv);

    void volumeMobility(QStringList &mobileLvNames, QStringList &fixedLvNames,
                        QStringList &mobilePvNames, QStringList &fixedPvNames);

    void pvState(QStringList &open, QStringList &closed);

    void movesWithVolume(const bool isLV, const QString name,
                         QStringList &movingPvNames, QStringList &movingLvNames);

    void moveNames(const bool isLvMove,
                   QListWidget *const lvSource, QListWidget *const lvTarget,
                   QListWidget *const pvSource, QListWidget *const pvTarget);

public:
    explicit VGSplitDialog(VolGroup *volumeGroup, QWidget *parent = nullptr);

private slots:
    void addLvList();
    void addPvList();
    void commit();
    void enableLvArrows();
    void enablePvArrows();
    void removePvList();
    void removeLvList();
    void validateOK();

};

#endif
