/*
 *
 *
 * Copyright (C) 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
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

#include <KDialog>
#include <KLineEdit>
#include <KListWidget>

#include <QStringList>
#include <QCheckBox>
#include <QRegExpValidator>
#include <QTableWidget>

class LogVol;
class VolGroup;

bool split_vg(VolGroup *volumeGroup);

class VGSplitDialog : public KDialog
{
    Q_OBJECT

    KListWidget *m_left_lv_list;
    KListWidget *m_right_lv_list;
    KListWidget *m_left_pv_list;
    KListWidget *m_right_pv_list;
    KPushButton *m_lv_add;
    KPushButton *m_lv_remove;
    KPushButton *m_pv_add;
    KPushButton *m_pv_remove;

    KLineEdit   *m_new_vg_name;
    QRegExpValidator *m_validator;
    VolGroup *m_vg;

    QWidget *buildLvLists(const QStringList mobileLvNames, const QStringList immobileLvNames);
    QWidget *buildPvLists(const QStringList mobilePvNames, const QStringList immobilePvNames);

    void volumeMobility(QStringList &mobileLvNames, QStringList &immobileLvNames,
                        QStringList &mobilePvNames, QStringList &immobilePvNames);

    void pvState(QStringList &open, QStringList &closed);

    void movesWithVolume(const bool isLV, const QString name,
                         QStringList &movingPvNames, QStringList &movingLvNames);

    void moveNames(const bool isLvMove,
                   KListWidget *const lvSource, KListWidget *const lvTarget,
                   KListWidget *const pvSource, KListWidget *const pvTarget);

public:
    explicit VGSplitDialog(VolGroup *volumeGroup, QWidget *parent = 0);
    QStringList arguments();

private slots:
    void enableLvArrows();
    void enablePvArrows();
    void addPvList();
    void removePvList();
    void addLvList();
    void removeLvList();
    void validateOK();
    void deactivate();     // active lvs must be deactivated before moving

};

#endif
