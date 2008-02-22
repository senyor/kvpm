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
#ifndef PROCESSPROGRESS_H
#define PROCESSPROGRESS_H

#include <QWidget>
#include <QStringList>
#include <QEventLoop>
#include <KProgressDialog>

class ProcessProgress : public QWidget
{
Q_OBJECT

    bool show_progress;
    QEventLoop *loop;
    QStringList output_all, output_no_error;
    KProgressDialog *progress_dialog;
    QProcess *process;

 public:
    ProcessProgress(QStringList Arguments, QString Operation, bool ShowProgress = FALSE, QWidget *parent = 0);
   ProcessProgress(QStringList Arguments, QWidget *parent = 0);
    QStringList programOutput();
    int exitCode();
    
 public slots:   
     void stopProgressLoop(int ExitCode, QProcess::ExitStatus ExitStatus);
     void readStandardOut();
     void readStandardError();
 
};

#endif
