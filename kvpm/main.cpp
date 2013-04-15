/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
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

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KMessageBox>
#include <KSplashScreen>
#include <KStandardDirs>

class VolGroup;
class PhysVol;
class LogVol;


TopWindow *MainWindow;

int main(int argc, char **argv)
{

    KAboutData aboutData("kvpm", 0,
                         ki18n("kvpm"), "0.9.5",
                         ki18n("The Linux volume and partition manager for KDE.\n"
                               "Licensed under the GPL v3.0\n \n"
                               "Additional icons taken from the Silk icon set by Mark James.\n"
                               "http://www.famfamfam.com/lab/icons/silk/\n"
                               "Licensed under the under the Creative Commons Attribution 3.0 License."),
                         KAboutData::License_GPL_V3,
                         ki18n("Copyright (c) 2008, 2009, 2010, 2011, 2012 Benjamin Scott"));

    KCmdLineArgs::init(argc, argv, &aboutData);

    KApplication kvpm;

    if (geteuid() != 0) {

        KMessageBox::error(0,
                           i18n("This program must be run as root (uid = 0) "),
                           i18n("Insufficient Privilege"));
        return 0;
    }

    QPixmap splashImage(KGlobal::dirs()->findResource("data", "kvpm/images/splash.png"));
    KSplashScreen splash(splashImage);
    splash.show();

    ExecutableFinder *executable_finder = new ExecutableFinder();
    LvmConfig config;
    config.initialize();

    MasterList *master_list = new MasterList();
    TopWindow  *top_window  = new TopWindow(master_list, executable_finder, NULL);
    MainWindow = top_window;

    top_window->setAutoSaveSettings();
    top_window->show();
    splash.finish(top_window);

    return kvpm.exec();
}


