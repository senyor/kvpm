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

#include "vgremove.h"

#include <lvm2app.h>
 
#include <KMessageBox>
#include <KLocale>

#include <QtGui>

#include "masterlist.h"
#include "topwindow.h"
#include "progressbox.h"
#include "volgroup.h"


bool remove_vg(VolGroup *volumeGroup)
{
    const QByteArray vg_name = volumeGroup->getName().toLocal8Bit();
    lvm_t  lvm = MasterList::getLvm();
    vg_t vg_dm = NULL;
    ProgressBox *const progress_box = TopWindow::getProgressBox();
    bool success = true;
    const QString message = i18n("Are you certain you want to delete volume group: <b>%1</b>?", volumeGroup->getName());

    if(KMessageBox::questionYesNo( 0, message) == KMessageBox::Yes){

        progress_box->setRange(0,3);
        progress_box->setValue(1);
        qApp->processEvents();

        if( (vg_dm = lvm_vg_open(lvm, vg_name.data(), "w", 0)) ){
            progress_box->setValue(2);
            qApp->processEvents();
            if( lvm_vg_remove(vg_dm) ){ 
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));
                success = false;
            }
            else{
                if( lvm_vg_write(vg_dm) ){
                    KMessageBox::error(0, QString(lvm_errmsg(lvm)));
                    success = false;
                }
            }
            lvm_vg_close(vg_dm);
            progress_box->setValue(3);
            qApp->processEvents();
        }
        else{
            KMessageBox::error(0, QString(lvm_errmsg(lvm)));
            success = false;
        }
    }
    else{
        KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        success = false;
    }

    progress_box->setValue(3);
    qApp->processEvents();
    return success;
}
