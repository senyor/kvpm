/*
 *
 * 
 * Copyright (C) 2008, 2009, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KMessageBox>
#include <KSplashScreen>
#include <KStandardDirs>

#include <QtGui>
#include <QString>

#include "executablefinder.h"
#include "topwindow.h"
#include "kvpmsetup.h"

class VolGroup;
class PhysVol;
class LogVol;
class MasterList;


TopWindow *MainWindow;
MasterList *g_master_list;
ExecutableFinder *g_executable_finder;


int main(int argc, char **argv)
{
    
    KAboutData aboutData( "kvpm", 0,
			  ki18n("kvpm"), "0.8.3",
			  ki18n( "Linux volume and partition manager for KDE"),
			  KAboutData::License_GPL_V3,
			  ki18n("Copyright (c) 2008, 2009, 2010, 2011 Benjamin Scott") );

    KCmdLineArgs::init( argc, argv, &aboutData);

    KApplication kvpm;

    if( geteuid() != 0 ){
	
	KMessageBox::error( 0, 
			    i18n("This program must be run as root (uid = 0) "),
			    i18n("Insufficient Privilege") );
	return 0;
    }

    QPixmap splashImage(KGlobal::dirs()->findResource("data", "kvpm/images/splash.png"));
    KSplashScreen splash(splashImage);
    splash.show();

    if( ! isconfigured_kvpm() )
        setup_kvpm();

    g_executable_finder = new ExecutableFinder();
    
    MainWindow = new TopWindow(NULL);
    MainWindow->setAutoSaveSettings();
    MainWindow->show();
    splash.finish(MainWindow);

    return kvpm.exec();
}


