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

#ifndef PROCESSPROGRESS_H
#define PROCESSPROGRESS_H

#include <QWidget>
#include <QStringList>
#include <QEventLoop>
#include <QProcess>

class KProcess;
class KProgressDialog;

class ProcessProgress : public QObject
{
Q_OBJECT

    QEventLoop *m_loop;
    QStringList m_output_all, m_output_no_error;
    KProcess *m_process;
    KProgressDialog *m_progress_dialog;
    int m_exit_code;

 public:
    ProcessProgress(QStringList arguments, const bool canCancel = false, QObject *parent = NULL);

    QStringList programOutput();
    QStringList programOutputAll();
    int exitCode();
    
 public slots:   
    void stopProgressLoop(int exitCode, QProcess::ExitStatus);
    void cancelProcess();
    void readStandardOut();
    void readStandardError();
 
};

#endif
