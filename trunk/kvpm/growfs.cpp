/*
 *
 * 
 * Copyright (C) 2009 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <sys/mount.h>
#include <errno.h>
#include <string.h>

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "growfs.h"
#include "mountinfo.h"
#include "processprogress.h"
#include "fsck.h"

bool do_temp_mount(QString path, QString fs);
void do_temp_unmount();

bool grow_fs(QString path, QString fs){

    MountInformationList mount_info_list;
    QList<MountInformation *> mounts = mount_info_list.getMountInformation( path );
    QString mp;      // mount point

    QStringList arguments, output;
    unsigned long options = 0;

    if( fs == "ext2" || fs == "ext3" || fs == "ext4" ){

        fsck( path );

        arguments << "resize2fs" 
                  << path; 

        ProcessProgress fs_grow(arguments, i18n("Growing filesystem..."), true );
        output = fs_grow.programOutput();

        if ( fs_grow.exitCode() )
            return false;
        else
            return true;
    }
    else if( fs == "xfs" ){

        if( do_temp_mount(path, fs) ){

            arguments << "xfs_growfs" 
                      << "/tmp/kvpm_tmp_mount"; 
            
            ProcessProgress fs_grow(arguments, i18n("Growing filesystem..."), true );

            do_temp_unmount();
            
            if ( fs_grow.exitCode() )
                return false;
            else
                return true;
        }

        do_temp_unmount();
        return false;
    }
    else if( fs == "jfs" ){

        options = MS_REMOUNT;

        if( mounts.size() == 0 ){
            if( do_temp_mount(path, fs) ){
                if(mount( path.toAscii().data(), "/tmp/kvpm_tmp_mount", NULL, options,"resize" )){
                    KMessageBox::error(0, i18n("Error number: %1 %2", errno, strerror(errno)));
                    do_temp_unmount();                
                    return false;
                }
                else{
                    do_temp_unmount();
                    return true;
                }                
            }
            else{
                return false;
            }
        }
        else{
            KMessageBox::error(0, "the filesystems must be unmounted first");
            return false;
        }
    }

    return false;
}

bool do_temp_mount(QString path, QString fs){

    QDir temp_dir( "/tmp/kvpm_tmp_mount" );
    unsigned long options = 0;

    if( ! temp_dir.exists() ){
        temp_dir.cdUp();
        temp_dir.mkdir("kvpm_tmp_mount");
    }

    int error = mount( path.toAscii().data(), 
                       "/tmp/kvpm_tmp_mount",
                       fs.toAscii().data(),
                       options,
                       NULL );

    if( error ){
        KMessageBox::error(0, QString("Error number: %1 %2").arg(errno).arg(strerror(errno)));
        return false;
    }

    return true;
}

void do_temp_unmount(){

    umount2("/tmp/kvpm_tmp_mount", 0);

    QDir temp_dir( "/tmp" );

    temp_dir.rmdir("kvpm_tmp_mount");

}
