/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "executablefinder.h"
#include "lvmconfig.h"
#include "masterlist.h"
#include "topwindow.h"

#include <unistd.h>

#include <QApplication>
#include <QCommandLineParser>
#include <QSplashScreen>
#include <QStandardPaths>

#include <KAboutData>
#include <KLocalizedString>
#include <KMessageBox>


class VolGroup;
class PhysVol;
class LogVol;


TopWindow *g_top_window;

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("kvpm");

    KAboutData about_data(QStringLiteral("kvpm"),
                          xi18nc("@title", "<application>kvpm</application>"), QStringLiteral("0.9.10"),
                          xi18nc("@title", "The Linux volume and partition manager for KDE.\n"
                                "Licensed under the GPL v3.0\n \n"
                                "Additional icons taken from the Silk icon set by Mark James.\n"
                                "http://www.famfamfam.com/lab/icons/silk/\n"
                                "Licensed under the under the Creative Commons Attribution 3.0 License."),
                          KAboutLicense::GPL_V3,
                          xi18nc("@info:credit", "Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2016 Benjamin Scott"));

    QCommandLineParser parser;
    parser.setApplicationDescription(about_data.shortDescription());
    parser.addHelpOption();
    parser.addVersionOption();
    about_data.setupCommandLine(&parser);
    parser.process(app);
    about_data.processCommandLine(&parser);

    if (geteuid() != 0) {

        KMessageBox::sorry(0,
                           i18n("This program must be run as root (uid = 0) "),
                           i18n("Insufficient Privilege"));
        return 0;
    }

    QPixmap splashImage(QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kvpm/images/splash.png"));
    QSplashScreen splash(splashImage);
    splash.show();

    ExecutableFinder *executable_finder = new ExecutableFinder();
    LvmConfig config;
    config.initialize();

    MasterList *const master_list = new MasterList();
    g_top_window  = new TopWindow(master_list, executable_finder, nullptr);

    g_top_window->setAutoSaveSettings();
    g_top_window->show();
    splash.finish(g_top_window);

    return app.exec();
}


