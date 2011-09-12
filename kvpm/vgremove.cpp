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
#include <KProgressDialog>
#include <QtGui>

#include "masterlist.h"
#include "volgroup.h"

extern MasterList *g_master_list;

bool remove_vg(VolGroup *volumeGroup)
{
    QString message;
    lvm_t  lvm = g_master_list->getLVM();
    vg_t vg_dm = NULL;
    KProgressDialog *progress_dialog;
    bool success = true;

    message = i18n("Are you certain you want to "
		   "delete volume group: <b>%1</b>", volumeGroup->getName());

    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button

        progress_dialog = new KProgressDialog(NULL, i18n("progress"), i18n("deleting group"));
        progress_dialog->setAllowCancel(false);
        progress_dialog->setMinimumDuration(250); 
        QProgressBar *progress_bar = progress_dialog->progressBar();
        progress_bar->setRange(0,3);
        progress_bar->setValue(1);
        progress_dialog->show();
        qApp->processEvents();
                
        progress_bar->setValue(2);
        qApp->processEvents();
        if( (vg_dm = lvm_vg_open(lvm, volumeGroup->getName().toAscii().data(), "w", 0)) ){
            progress_bar->setValue(2);
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
            progress_bar->setValue(3);
            qApp->processEvents();
        }
        else{
            KMessageBox::error(0, QString(lvm_errmsg(lvm)));
            success = false;
        }
        progress_dialog->close();
        progress_dialog->delayedDestruct();
    }
    else{
        KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        success = false;
    }

    qApp->processEvents();
    return success;
}
