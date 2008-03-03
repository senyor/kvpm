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

class KProcess;


class ProcessProgress : public QWidget
{
Q_OBJECT

    bool m_show_progress;
    QEventLoop *m_loop;
    QStringList m_output_all, m_output_no_error;
    KProgressDialog *m_progress_dialog;
    KProcess *m_process;

 public:
    ProcessProgress(QStringList arguments,
		    QString operation, 
		    bool showProgress = false, 
		    QWidget *parent = 0);

    QStringList programOutput();
    int exitCode();
    
 public slots:   
     void stopProgressLoop(int exitCode, QProcess::ExitStatus);
     void readStandardOut();
     void readStandardError();
 
};

#endif
