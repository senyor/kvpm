/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <lvm2app.h>
 
#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "processprogress.h"
#include "vgremove.h"
#include "volgroup.h"

bool remove_vg(VolGroup *volumeGroup)
{
    QString message;
    lvm_t  lvm = NULL;
    vg_t vg_dm = NULL;

    message = i18n("Are you certain you want to "
		   "delete volume group: <b>%1</b>").arg(volumeGroup->getName());

    if(KMessageBox::questionYesNo( 0, message) == 3){      // 3 = "yes" button
        if( (lvm = lvm_init(NULL)) ){
            if( (vg_dm = lvm_vg_open(lvm, volumeGroup->getName().toAscii().data(), "w", NULL)) ){
                if( lvm_vg_remove(vg_dm) ) 
                    KMessageBox::error(0, QString(lvm_errmsg(lvm)));
                else{
                    if( lvm_vg_write(vg_dm) )
                        KMessageBox::error(0, QString(lvm_errmsg(lvm)));
                }
                lvm_vg_close(vg_dm);
            }
            else
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));
            lvm_quit(lvm);
        }
        else
            KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        return true;
    }
    return false;
}
