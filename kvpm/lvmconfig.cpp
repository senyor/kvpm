/*
 *
 *
 * Copyright (C) 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "lvmconfig.h"

#include "executablefinder.h"
#include "processprogress.h"

#include <QDebug>

#include <KLocale>
#include <KMessageBox>
#include <KProcess>


// These are static being variables initialized here

QString LvmConfig::m_mirror_segtype_default = QString("mirror");
bool LvmConfig::m_mirror_logs_require_separate_pvs = false;
bool LvmConfig::m_thin_pool_metadata_require_separate_pvs = false;
bool LvmConfig::m_maximise_cling = false;


// This class holds the lvm2 system configuration settings. It
// is meant to be initialized once, at startup, and then 
// globally readable with static members.
LvmConfig::LvmConfig()
{
}

LvmConfig::~LvmConfig()
{
}

// runs 'lvm dumpconfig' and sets the static variables
void LvmConfig::initialize()
{
    QStringList output = getConfig();
    QStringList global;
    QStringList allocation;
    
    auto line = output.constBegin();
    
    while (line != output.constEnd()) {
        if (line->contains("allocation {")) {
            ++line;
            
            while ((line != output.constEnd()) && (!line->startsWith("}"))) {
                allocation << line->trimmed();
                ++line;
            }
            
            setAllocation(allocation);
        } else if (line->contains("global {")) {
            ++line;
            
            while ((line != output.constEnd()) && (!line->startsWith("}"))) {
                global << line->trimmed();
                ++line;
            }
            
            setGlobal(global);
        } else {
            ++line;
        }
    }
}

QStringList LvmConfig::getConfig()
{
    QStringList output;

    const QString executable_path = ExecutableFinder::getPath("lvm");
    const QStringList args = QStringList() << "dumpconfig";

    if (!executable_path.isEmpty()) {
        QStringList environment = QProcess::systemEnvironment();
        environment << "LVM_SUPPRESS_FD_WARNINGS=1";

        KProcess process;
        process.setEnvironment(environment);
        process.setOutputChannelMode(KProcess::SeparateChannels);
        process.setReadChannel(QProcess::StandardOutput);
        process.setProgram(executable_path, args);
        process.start();
        process.closeWriteChannel();
        process.waitForFinished();
        process.setReadChannel(QProcess::StandardOutput);
        while (process.canReadLine())
            output << process.readLine();

        if (process.exitCode() || (process.exitStatus() == QProcess::CrashExit)) {
            process.setReadChannel(QProcess::StandardError);

            QString errors;
            while (process.canReadLine())
                errors.append(process.readLine());

            if (process.exitStatus() != QProcess::CrashExit)
                KMessageBox::error(NULL, i18n("%1 produced this output: %2", process.program().takeFirst(), errors));
            else
                KMessageBox::error(NULL, i18n("%1 <b>crashed</b> with this output: %2", process.program().takeFirst(), errors));
        }
    } else {
        KMessageBox::error(NULL, i18n("Executable: '%1' not found", executable_path));
    }       

    return output;
}

void LvmConfig::setGlobal(QStringList &variables)
{
    for (auto line : variables) {
        if (line.contains("mirror_segtype_default"))  {
            if (line.contains("\"raid1\"") )
                m_mirror_segtype_default = "raid1";
            else
                m_mirror_segtype_default = "mirror";
        }
    }
}

void LvmConfig::setAllocation(QStringList &variables)
{
    for (auto line : variables) {
        if (line.contains("mirror_logs_require_separate_pvs"))
            m_mirror_logs_require_separate_pvs = line.contains("=1");
        if (line.contains("thin_pool_metadata_require_separate_pvs"))
            m_thin_pool_metadata_require_separate_pvs = line.contains("=1");
        if (line.contains("maximise_cling"))
            m_maximise_cling = line.contains("=1");
    }
}

QString LvmConfig::getMirrorSegtypeDefault()
{
    return m_mirror_segtype_default;
}

bool LvmConfig::getMirrorLogsRequireSeparatePvs()
{
    return m_mirror_logs_require_separate_pvs;
}

bool LvmConfig::getThinPoolMetadataRequireSeparatePvs()
{
    return m_thin_pool_metadata_require_separate_pvs;
}

bool LvmConfig::getMaximiseCling()
{
    return m_maximise_cling;
}

    

    

    
