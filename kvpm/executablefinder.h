/*
 *
 *
 * Copyright (C) 2008, 2009, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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


#include <QMap>
#include <QObject>
#include <QStringList>


class ExecutableFinder : public QObject
{
    Q_OBJECT

    QStringList m_default_search_paths;
    QStringList m_keys;                       // Names of the executables we are looking for
    QStringList m_not_found;                  // The ones we didn't find
    static QMap<QString, QString> m_path_map;

public:
    ExecutableFinder(QObject *parent = 0);
    static QString getPath(QString name);
    void reload();                            // rescan the system for needed executables
    void reload(QStringList search);          // As above using the stringlist for the search paths
    QStringList getAllPaths();
    QStringList getAllNames();
    QStringList getNotFound();
};

#endif
