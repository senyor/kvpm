/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "processprogress.h"

#include <KMessageBox>
#include <KProcess>
#include <KLocale>

#include <QtGui>

#include "executablefinder.h"
#include "topwindow.h"
#include "progressbox.h"

extern ExecutableFinder *g_executable_finder;


ProcessProgress::ProcessProgress(QStringList arguments, QObject *parent) : QObject(parent)
{
    QString executable, 
            executable_path,
            output_errors;

    m_exit_code = 127;  // command not found

    if(arguments.size() == 0){
	qDebug() << "ProcessProgress given an empty arguments list";
    }
    else{

        executable = arguments.takeFirst();
	executable_path = g_executable_finder->getExecutablePath( executable );

        if( !executable_path.isEmpty() ){

            qApp->setOverrideCursor(Qt::WaitCursor);
            m_process = new KProcess(this);
            QStringList environment = QProcess::systemEnvironment();
            environment << "LVM_SUPPRESS_FD_WARNINGS=1";
            m_process->setEnvironment(environment);
            m_loop = new QEventLoop(this);

            TopWindow::getProgressBox()->setRange(0,0);
            TopWindow::getProgressBox()->setText(executable);
	
            connect(m_process,  SIGNAL(finished(int, QProcess::ExitStatus)), 
                    this, SLOT(stopProgressLoop(int, QProcess::ExitStatus)));

            connect(m_process, SIGNAL(readyReadStandardOutput()),
                    this,      SLOT(readStandardOut()));

            connect(m_process, SIGNAL(readyReadStandardError()),
                    this,      SLOT(readStandardError()));
	
            m_process->setOutputChannelMode(KProcess::SeparateChannels);
            m_process->setReadChannel(QProcess::StandardOutput);
            m_process->setProgram(executable_path, arguments);
            
            m_process->start();
            m_process->closeWriteChannel();
            m_loop->exec(QEventLoop::ExcludeUserInputEvents);
            
            m_process->waitForFinished();
            TopWindow::getProgressBox()->reset();
            qApp->restoreOverrideCursor();
            m_exit_code = m_process->exitCode();

            if ( m_exit_code || ( m_process->exitStatus() == QProcess::CrashExit ) ){

                if ( ( m_exit_code == 0 ) && ( m_process->exitStatus() == QProcess::CrashExit ) )
                    m_exit_code = 1;  // if it crashed without an exit code, set a non zero exit code

                output_errors = m_output_all.join("");

                if( m_process->exitStatus() != QProcess::CrashExit ) 
                    KMessageBox::error(NULL, i18n("%1 produced this output: %2", executable_path, output_errors) );
                else
                    KMessageBox::error(NULL, i18n("%1 <b>crashed</b> with this output: %2", executable_path, output_errors) );
            }
        }
        else{
            KMessageBox::error(NULL, i18n("Executable: '%1' not found", executable));
        }
    }
}

void ProcessProgress::stopProgressLoop(int, QProcess::ExitStatus)
{
    m_loop->exit();
    TopWindow::getProgressBox()->setText("");
    TopWindow::getProgressBox()->setRange(0,3);
    TopWindow::getProgressBox()->setValue(3);
}

QStringList ProcessProgress::programOutput()
{
    return m_output_no_error;
}

QStringList ProcessProgress::programOutputAll()
{
    return m_output_all;
}

int ProcessProgress::exitCode()
{
    return m_exit_code;
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
