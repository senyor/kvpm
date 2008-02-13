/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>
#include "processprogress.h"


ProcessProgress::ProcessProgress(QStringList Arguments, QString Operation, bool ShowProgress, QWidget *parent):QWidget(parent)
{
    QString program;
    QProgressBar *progress_bar;
 
    if(Arguments.size() == 0){
	qDebug() << "ProcessProgress given an empty arguments list";
	close();
    }
    else{
	program = Arguments.takeFirst();
	show_progress = ShowProgress;
	process = new QProcess(this);
	QStringList environment = QProcess::systemEnvironment();
	environment << "LVM_SUPPRESS_FD_WARNINGS=1";
	process->setEnvironment(environment);
	loop = new QEventLoop(this);
	
	if(show_progress){
	    progress_dialog = new KProgressDialog(this, "progress", Operation);
	    progress_dialog->setAllowCancel(FALSE);
	    progress_bar = progress_dialog->progressBar();
	    progress_bar->setRange(0,0);
	    
	}
	
	connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), 
		this, SLOT(stopProgressLoop(int, QProcess::ExitStatus)));
	connect(process, SIGNAL(readyReadStandardOutput()),
		this, SLOT(readStandardOut()));
	connect(process, SIGNAL(readyReadStandardError()),
		this, SLOT(readStandardError()));
	
	
	process->setReadChannel(QProcess::StandardOutput);
	
	process->start(program, Arguments);
	process->closeWriteChannel();
	
	loop->exec(QEventLoop::ExcludeUserInputEvents);
	process->waitForFinished();
	
	if (process->exitCode())
	    QMessageBox::critical(this, "Execution of " + program + " produced errors", output_all.join(""));
    }
}

ProcessProgress::ProcessProgress(QStringList Arguments, QWidget *parent):QWidget(parent)
{
    QProgressBar *progress_bar;
 
    if(Arguments.size() == 0)
	qDebug() << "ProcessProgress given an empty arguments list";

    show_progress = FALSE;
    QString Operation = "";
    QString program = Arguments.takeFirst();

    process = new QProcess(this);
    QStringList environment = QProcess::systemEnvironment();
    environment << "LVM_SUPPRESS_FD_WARNINGS=1";
    process->setEnvironment(environment);
    loop = new QEventLoop(this);

    if(show_progress){
	progress_dialog = new KProgressDialog(this, "progress", Operation);
	progress_dialog->setAllowCancel(FALSE);
	progress_bar = progress_dialog->progressBar();
	progress_bar->setRange(0,0);
    
    }

    connect(process, SIGNAL(finished(int, QProcess::ExitStatus)), 
	    this, SLOT(stopProgressLoop(int, QProcess::ExitStatus)));
    connect(process, SIGNAL(readyReadStandardOutput()),
	    this, SLOT(readStandardOut()));
    connect(process, SIGNAL(readyReadStandardError()),
	    this, SLOT(readStandardError()));
    

    process->setReadChannel(QProcess::StandardOutput);

    process->start(program, Arguments);
    process->closeWriteChannel();

    loop->exec(QEventLoop::ExcludeUserInputEvents);
    process->waitForFinished();

    if (process->exitCode())
        QMessageBox::critical(this, "Execution of " + program + " produced errors", output_all.join(""));
}

void ProcessProgress::stopProgressLoop(int, QProcess::ExitStatus)
{
    loop->exit();
    if(show_progress)
	progress_dialog->close();
    
}

QStringList ProcessProgress::programOutput()
{
    return output_no_error;
}

int ProcessProgress::exitCode()
{
    return process->exitCode();
}

void ProcessProgress::readStandardOut()
{
    QString output_line;
    
    process->setReadChannel(QProcess::StandardOutput);
    while(process->canReadLine()){
	output_line = process->readLine();
	if(!output_line.contains("partial mode.", Qt::CaseInsensitive)){
	    output_no_error << output_line;
	}
	output_all << output_line;
    }
    
}

void ProcessProgress::readStandardError()
{
    QString output_line;
    
    process->setReadChannel(QProcess::StandardError);

    while(process->canReadLine()){
	output_line = process->readLine();
	output_all << output_line;
    }
    
}

