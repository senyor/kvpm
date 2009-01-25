/*
 *
 * 
 * Copyright (C) 2008, 2009 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#ifndef EXECUTABLEFINDER_H
#define EXECUTABLEFINDER_H

#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>

class ExecutableFinder : public QObject
{

    QStringList m_default_search_paths;
    QStringList m_keys;                       // Names of the executables we are looking for
    QStringList m_not_found;                  // The ones we didn't find
    QMap<QString, QString> m_path_map;
    
 public:
    ExecutableFinder(QObject *parent = 0);
    QString getExecutablePath(QString name);
    QStringList getAllPaths();
    QStringList getAllNames();
    QStringList getNotFound();
    void reload();                            // rescan the system for needed executables
};

#endif
