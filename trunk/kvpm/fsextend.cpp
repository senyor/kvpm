/*
 *
 * 
 * Copyright (C) 2009, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "fsextend.h"

#include <sys/mount.h>
#include <errno.h>
#include <string.h>

#include <KLocale>
#include <KMessageBox>

#include <QtGui>

#include "executablefinder.h"
#include "fsck.h"
#include "mountentry.h"
#include "mounttables.h"
#include "processprogress.h"


bool do_temp_mount(QString path, QString fs);
void do_temp_unmount();


bool fs_can_extend(const QString fs){

    QString executable;

    if(fs == "ext2" || fs == "ext3" || fs == "ext4" || fs == "reiserfs" || fs == "ntfs" || fs == "xfs"){

        if(fs == "ext2" || fs == "ext3" || fs == "ext4")
            executable = "resize2fs";
        else if(fs == "reiserfs")
            executable = "resize_reiserfs";
        else if(fs == "ntfs")
            executable = "ntfsresize";
        else if(fs == "xfs")
            executable = "xfs_growfs";

        if( !ExecutableFinder::getPath(executable).isEmpty() )
            return true;
        else{
            KMessageBox::error(NULL, i18n("Executable: '%1' not found, this filesystem cannot be extended", executable));
            return false;
        }
    }
    else if(fs == "jfs")
        return true;
    else
        return false;
}

// path is the device path, not mount point

bool fs_extend(QString path, QString fs, bool isLV){

    MountTables mount_info_list;
    QList<MountEntry *> mounts = mount_info_list.getMtabEntries(path);
    QString mp;               // mount point
    bool isMounted = false;
    const QByteArray path_array = path.toLocal8Bit();
    const QByteArray fs_array = fs.toLocal8Bit();

    if( mounts.size() ){
        mp = mounts[0]->getMountPoint();
        isMounted = true;
    }

    const QByteArray mp_array = mp.toLocal8Bit();
    QStringList arguments, output;
    unsigned long options = 0;

    if( ( ! isLV) && ( isMounted ) ){               
        KMessageBox::error(0, "only logical volumes may be mounted during resize, not partitions");
        return false;
    }

    QString error_message = i18n("It appears that the filesystem was not extended. "
                                 "It will need to be extended before the additional space can be used.");

    if( fs == "ext2" || fs == "ext3" || fs == "ext4" ){

        if( ! isMounted ){
            if( ! fsck( path ) )
                return false;
        }

        arguments << "resize2fs" 
                  << path; 

        ProcessProgress fs_grow(arguments);
        output = fs_grow.programOutput();

        if ( fs_grow.exitCode() ){
            KMessageBox::error(0, error_message);
            return false;
        }
        else
            return true;
    }
    else if( fs == "xfs" ){
        if( mp.isEmpty() ){
            if( do_temp_mount(path, fs) ){

                arguments << "xfs_growfs" 
                          << "/tmp/kvpm_tmp_mount"; 
            
                ProcessProgress fs_grow(arguments);

                do_temp_unmount();
            
                if ( fs_grow.exitCode() ){
                    KMessageBox::error(0, error_message);
                    return false;
                }
                else
                    return true;
            }
            do_temp_unmount();
            KMessageBox::error(0, error_message);
            return false;
        }
        else{

            arguments << "xfs_growfs" 
                      << mp; 
            
            ProcessProgress fs_grow(arguments);
            
            if ( fs_grow.exitCode() ){
                KMessageBox::error(0, error_message);
                return false;
            }
            else
                return true;
        }
    }
    else if( fs == "jfs" ){

        options = MS_REMOUNT;

        if( ! isMounted ){
            if( do_temp_mount(path, fs) ){
                if( ! mount( path_array.data(), "/tmp/kvpm_tmp_mount", NULL, options, "resize" )){
                    do_temp_unmount();
                    return true;
                }
                else{
                    KMessageBox::error(0, i18n("Error number: %1 %2", errno, strerror(errno)));
                    do_temp_unmount();                
                }                
            }
            return false;
        }
        else{
            if(mount( path_array.data(), mp_array.data(), NULL, options, "resize" )){
                KMessageBox::error(0, i18n("Error number: %1 %2", errno, strerror(errno)));
                return false;
            }
            else
                return true;
        }
    }
    else if( fs == "reiserfs" ){

        arguments << "resize_reiserfs" 
                  << "-fq"
                  << path; 
        
        ProcessProgress fs_grow(arguments);
        output = fs_grow.programOutput();
        
        if ( fs_grow.exitCode() ){
            KMessageBox::error(0, error_message);
            return false;
        }
        else
            return true;
    }
    else if( fs == "ntfs" ){

        arguments << "ntfsresize" 
                  << "--no-progress-bar"
                  << "--force"
                  << path; 

        ProcessProgress fs_grow(arguments);
        output = fs_grow.programOutput();
        
        if ( fs_grow.exitCode() ){
            KMessageBox::error(0, error_message);
            return false;
        }
        else
            return true;
    }

    return false;
}

bool do_temp_mount(QString path, QString fs){

    QDir temp_dir( "/tmp/kvpm_tmp_mount" );
    unsigned long options = 0;
    const QByteArray path_array = path.toLocal8Bit();
    const QByteArray fs_array = fs.toLocal8Bit();

    if( ! temp_dir.exists() ){
        temp_dir.cdUp();
        temp_dir.mkdir("kvpm_tmp_mount");
    }

    int error = mount( path_array.data(), "/tmp/kvpm_tmp_mount", fs_array.data(), options, NULL );

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
