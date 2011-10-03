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

    QWidget *buildLVLists(QStringList mobileLVNames, QStringList immobileLVNames);
    QWidget *buildPVLists(QStringList mobilePVNames, QStringList immobilePVNames);
    void volumeMobility(QStringList &mobileLVNames, QStringList &immobileLVNames, 
                        QStringList &mobilePVNames, QStringList &immobilePVNames);

    void pvState(QStringList &open, QStringList &closed );
    void movesWithVolume(bool isLV, QString name, QStringList &movingPVNames, QStringList &movingLVNames);
    void moveNames(bool isLVMove, 
                   QListWidget *lvSource, QListWidget *lvTarget, 
                   QListWidget *pvSource, QListWidget *pvTarget);

 public:
    explicit VGSplitDialog(VolGroup *volumeGroup, QWidget *parent = 0);
    QStringList arguments();    

 private slots:
    void enableLVArrows();
    void enablePVArrows();
    void addPVList();
    void removePVList();
    void addLVList();
    void removeLVList();
    void validateOK();
    void validateName(QString);
    void adjustTables();
    void deactivate();     // active lvs must be deactivated before moving

};

#endif
