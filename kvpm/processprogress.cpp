/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include "executablefinder.h"
#include "topwindow.h"
#include "progressbox.h"

#include <sys/types.h>
#include <signal.h>

#include <KApplication>
#include <KLocale>
#include <KMessageBox>
#include <KProcess>
#include <KProgressDialog>

#include <QDebug>
#include <QEventLoop>


ProcessProgress::ProcessProgress(QStringList arguments, const bool allowCancel, QObject *parent) : QObject(parent)
{
    m_exit_code = 127;  // command not found
    m_progress_dialog = NULL;

    if (arguments.size() == 0) {
        qDebug() << "ProcessProgress given an empty arguments list";
    } else {

        const QString executable = arguments.takeFirst();
        const QString executable_path = ExecutableFinder::getPath(executable);

        if (!executable_path.isEmpty()) {

            qApp->setOverrideCursor(Qt::WaitCursor);
            m_process = new KProcess(this);
            QStringList environment = QProcess::systemEnvironment();
            environment << "LVM_SUPPRESS_FD_WARNINGS=1";
            m_process->setEnvironment(environment);
            m_loop = new QEventLoop(this);

            if (allowCancel) {
                TopWindow::getProgressBox()->hide();
                m_progress_dialog = new KProgressDialog(NULL,
                                                        i18n("progress"),
                                                        i18n("Running program: %1", executable),
                                                        Qt::CustomizeWindowHint |
                                                        Qt::WindowTitleHint);
                m_progress_dialog->setAllowCancel(true);
                m_progress_dialog->setMinimumDuration(250);
                m_progress_dialog->progressBar()->setRange(0, 0);
                m_progress_dialog->setModal(true);
                connect(m_progress_dialog, SIGNAL(rejected()), this, SLOT(cancelProcess()));
            } else {
                m_progress_dialog = NULL;
                TopWindow::getProgressBox()->setRange(0, 0);
                TopWindow::getProgressBox()->setText(executable);
            }

            connect(m_process,  SIGNAL(finished(int, QProcess::ExitStatus)),
                    this, SLOT(cleanup(int, QProcess::ExitStatus)));

            connect(m_process, SIGNAL(readyReadStandardOutput()),
                    this,      SLOT(readStandardOut()));

            connect(m_process, SIGNAL(readyReadStandardError()),
                    this,      SLOT(readStandardError()));

            m_process->setOutputChannelMode(KProcess::SeparateChannels);
            m_process->setReadChannel(QProcess::StandardOutput);
            m_process->setProgram(executable_path, arguments);

            m_process->start();
            m_process->closeWriteChannel();

            if (allowCancel)
                m_loop->exec(QEventLoop::AllEvents);
            else
                m_loop->exec(QEventLoop::ExcludeUserInputEvents);
        } else {
            KMessageBox::error(NULL, i18n("Executable: '%1' not found", executable));
        }
    }
}

void ProcessProgress::cleanup(const int code, const QProcess::ExitStatus status)
{
    m_exit_code = code;
    m_loop->exit();

    bool cancelled = false;

    qApp->restoreOverrideCursor();
    if (m_progress_dialog != NULL) {
        m_progress_dialog->close();
        cancelled = m_progress_dialog->wasCancelled();
        delete m_progress_dialog;
    }

    if (m_exit_code || (status == QProcess::CrashExit)) {

        if ((m_exit_code == 0) && (status == QProcess::CrashExit))
            m_exit_code = 1;  // if it crashed without an exit code, set a non zero exit code

        const QString errors = m_output_all.join("");

        if (status != QProcess::CrashExit || cancelled)
            KMessageBox::error(NULL, i18n("%1 produced this output: %2", m_process->program().takeFirst(), errors));
        else
            KMessageBox::error(NULL, i18n("%1 <b>crashed</b> with this output: %2", m_process->program().takeFirst(), errors));
    }

    TopWindow::getProgressBox()->reset();
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

void ProcessProgress::cancelProcess()
{
    const QString warning = i18n("<b>Really kill process %1</b>", m_process->program().takeFirst());

    kill(m_process->pid(), SIGSTOP);

    if (KMessageBox::warningYesNo(NULL,
                                  warning,
                                  QString(),
                                  KStandardGuiItem::yes(),
                                  KStandardGuiItem::no(),
                                  QString(),
                                  KMessageBox::Dangerous) == KMessageBox::Yes) {
        m_process->kill();
        m_progress_dialog->show();
        m_progress_dialog->setLabelText(i18n("Waiting for process to finish"));
        m_progress_dialog->setAllowCancel(false);
    } else if (m_process->state() == QProcess::Running) {
        m_progress_dialog->show();
        kill(m_process->pid(), SIGCONT);
    }
}

void ProcessProgress::readStandardOut()
{
    QString output_line;

    m_process->setReadChannel(QProcess::StandardOutput);
    while (m_process->canReadLine()) {
        output_line = m_process->readLine();
        if (!output_line.contains("partial mode.", Qt::CaseInsensitive)) {
            m_output_no_error << output_line;
        }
        m_output_all << output_line;
    }
}

void ProcessProgress::readStandardError()
{
    QString output_line;

    m_process->setReadChannel(QProcess::StandardError);

    while (m_process->canReadLine()) {
        output_line = m_process->readLine();
        m_output_all << output_line;
    }
}
