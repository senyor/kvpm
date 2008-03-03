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

#include <KProcess>

#include <QtGui>

#include "executablefinder.h"
#include "processprogress.h"

extern ExecutableFinder *g_executable_finder;


ProcessProgress::ProcessProgress(QStringList arguments, 
				 QString operation, 
				 bool showProgress, 
				 QWidget *parent) : 
    QWidget(parent), 
    m_show_progress(showProgress)
{
    QProgressBar *progress_bar;
    QString executable_path;
 
    if(arguments.size() == 0){
	qDebug() << "ProcessProgress given an empty arguments list";
	close();
    }
    else{
	executable_path = arguments.takeFirst();
	executable_path = g_executable_finder->getExecutablePath(executable_path);

	qDebug() << executable_path;
	

	m_process = new KProcess(this);
	QStringList environment = QProcess::systemEnvironment();
	environment << "LVM_SUPPRESS_FD_WARNINGS=1";
	m_process->setEnvironment(environment);
	m_loop = new QEventLoop(this);
	
	if(m_show_progress){
	    m_progress_dialog = new KProgressDialog(this, "progress", operation);
	    m_progress_dialog->setAllowCancel(false);
	    progress_bar = m_progress_dialog->progressBar();
	    progress_bar->setRange(0,0);
	}
	
	connect(m_process,  SIGNAL(finished(int, QProcess::ExitStatus)), 
		this, SLOT(stopProgressLoop(int, QProcess::ExitStatus)));

	connect(m_process, SIGNAL(readyReadStandardOutput()),
		this, SLOT(readStandardOut()));

	connect(m_process, SIGNAL(readyReadStandardError()),
		this, SLOT(readStandardError()));
	
	m_process->setOutputChannelMode(KProcess::SeparateChannels);
	m_process->setReadChannel(QProcess::StandardOutput);
	m_process->setProgram(executable_path, arguments);
	
	m_process->start();
	m_process->closeWriteChannel();
	m_loop->exec(QEventLoop::ExcludeUserInputEvents);

	m_process->waitForFinished();
	
	if (m_process->exitCode()){
	    QMessageBox::critical(this, 
				  "Execution of " + executable_path + " produced errors", 
				  m_output_all.join(""));
	}
    }
}

void ProcessProgress::stopProgressLoop(int, QProcess::ExitStatus)
{
    m_loop->exit();
    if(m_show_progress)
	m_progress_dialog->close();
}

QStringList ProcessProgress::programOutput()
{
    return m_output_no_error;
}

int ProcessProgress::exitCode()
{
    return m_process->exitCode();
}

void ProcessProgress::readStandardOut()
{
    QString output_line;
    
    m_process->setReadChannel(QProcess::StandardOutput);
    while(m_process->canReadLine()){
	output_line = m_process->readLine();
	if(!output_line.contains("partial mode.", Qt::CaseInsensitive)){
	    m_output_no_error << output_line;
	}
	m_output_all << output_line;
    }
    
}

void ProcessProgress::readStandardError()
{
    QString output_line;
    
    m_process->setReadChannel(QProcess::StandardError);

    while(m_process->canReadLine()){
	output_line = m_process->readLine();
	m_output_all << output_line;
    }
    
}
