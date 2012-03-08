/*
 *
 * 
 * Copyright (C) 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef KVPMCONFIGDIALOG_H
#define KVPMCONFIGDIALOG_H

#include <KConfigDialog>
#include <KConfigSkeleton>
#include <KColorButton>
#include <KEditListBox>
#include <KPageWidgetItem>
#include <KTabWidget>

#include <QColor>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QStringList>
#include <QSpinBox>
#include <QTableWidget>
#include <QWidget>

class ExecutableFinder;


class KvpmConfigDialog: public KConfigDialog
{
Q_OBJECT

    QTableWidget     *m_executables_table;
    KEditListBox     *m_edit_list;
    KConfigSkeleton  *m_skeleton;
    ExecutableFinder *m_executable_finder;

    QTabWidget *generalPage();
    QWidget    *colorsPage();
    QTabWidget *programsPage();
    QGroupBox *allGroup();
    QGroupBox *deviceGroup();
    QGroupBox *logicalGroup();
    QGroupBox *physicalGroup();
    QGroupBox *pvPropertiesGroup();
    QGroupBox *lvPropertiesGroup();
    QGroupBox *devicePropertiesGroup();
    QWidget *treesTab();
    QWidget *propertiesTab();
    void fillExecutablesTable();

public:
    KvpmConfigDialog( QWidget *parent, const QString name, KConfigSkeleton *const skeleton, ExecutableFinder *const executableFinder );
    ~KvpmConfigDialog();

public slots:
    void updateSettings();

};

#endif
