/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef PROCESSPROGRESS_H
#define PROCESSPROGRESS_H

#include <QObject>
#include <QProcess>
#include <QStringList>

class KProgressDialog;

class QEventLoop;


class ProcessProgress : public QObject
{
    Q_OBJECT

    QEventLoop *m_loop;
    QStringList m_output_all, m_output_no_error;
    QProcess *m_process;
    KProgressDialog *m_progress_dialog;
    int m_exit_code;

public:
    ProcessProgress(QStringList arguments, const bool allowCancel = false, QObject *parent = NULL);

    QStringList programOutput();
    QStringList programOutputAll();
    int exitCode();

private slots:
    void cleanup(const int code, const QProcess::ExitStatus status);
    void cancelProcess();
    void readStandardOut();
    void readStandardError();

};

#endif
