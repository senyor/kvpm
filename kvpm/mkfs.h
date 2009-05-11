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

#ifndef MKFS_H
#define MKFS_H

#include <KDialog>
#include <KTabWidget>
#include <KLineEdit>
#include <KComboBox>

#include <QGroupBox>
#include <QRadioButton>
#include <QLabel>
#include <QStringList>

class VolGroup;
class LogVol;
class StoragePartition;

bool make_fs(LogVol *logicalVolume);
bool make_fs(StoragePartition *partition);

class MkfsDialog : public KDialog
{
Q_OBJECT

    KTabWidget  *m_tab_widget;
    QGroupBox   *radio_box, *m_stripe_box;

    QRadioButton *ext2, *ext3, *ext4, *reiser, *reiser4, *jfs, *xfs, *vfat, *swap;

    KComboBox *m_block_combo;      // blocksize
    KLineEdit *m_volume_edit;      // volume name
    KLineEdit *m_stride_edit;      // stride size
    KLineEdit *m_count_edit;       // strides per stripe
    QString m_path;

    int m_stride_size, m_stride_count;

    void buildDialog();

 public slots:
    void setAdvancedTab(bool);

 public:
    MkfsDialog(LogVol *logicalVolume, QWidget *parent = 0);
    MkfsDialog(StoragePartition *partition, QWidget *parent = 0);
    QStringList arguments();

};

#endif
