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


#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KMessageBox>

#include <QtGui>

#include "executablefinder.h"
#include "topwindow.h"

class VolGroup;
class PhysVol;
class LogVol;
class MasterList;


TopWindow *MainWindow;
MasterList *master_list;
ExecutableFinder *g_executable_finder;


int main(int argc, char **argv)
{

    KAboutData aboutData( "kvpm", 0,
			  ki18n("kvpm"), "0.3.3",
			  ki18n("Linux volume and partition manager for KDE"),
			  KAboutData::License_GPL,
			  ki18n("Copyright (c) 2008 Benjamin Scott") );

    KCmdLineArgs::init( argc, argv, &aboutData);
    KApplication kvpm;

    if( geteuid() ){
	if( seteuid(0) ){
	    KMessageBox::error( 0, 
				"This program must be run as root (uid = 0)",
				"Insufficient Privilege");
	    return 0;
	}
    }

    g_executable_finder = new ExecutableFinder();
    
    
    MainWindow = new TopWindow(NULL);
    MainWindow->setGeometry(0,0,1000,750);
    MainWindow->show();
    return kvpm.exec();
}

