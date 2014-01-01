/*
 *
 *
 * Copyright (C) 2013, 2014 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "externalraid.h"

#include <libdevmapper.h>


#include <QDebug>
#include <QFile>
#include <QRegExp>
#include <QTextStream>


    
QStringList dmraid_get_raid(dm_tree_node *const parent, dm_tree_node *const root);
QStringList dmraid_get_block(dm_tree_node *const parent, dm_tree_node *const root);




void dmraid_get_devices(QStringList &block, QStringList &raid)
{
    dm_lib_init();
    dm_log_with_errno_init(nullptr);

    dm_task *const dmt_list = dm_task_create(DM_DEVICE_LIST);
    dm_task *const dmt_info = dm_task_create(DM_DEVICE_INFO);
    dm_tree *const tree = dm_tree_create();

    dm_task_run(dmt_list);
    dm_names *list_ptr = dm_task_get_names(dmt_list);

    if(list_ptr->dev) {    // first dev == 0 for empty list
        int next = 0;
        dm_info dmi;

        do {
            if (dm_task_set_name(dmt_info, list_ptr->name)) { 
                if (dm_task_run(dmt_info)) {
                    if (dm_task_get_info(dmt_info, &dmi)) {
                        dm_tree_add_dev(tree, dmi.major, dmi.minor);
                        next = list_ptr->next;
                        list_ptr = (dm_names *)(next + (char *)list_ptr); // add offset "next" bytes to pointer
                    }
                }
            } 
        } while (next);
    }

    dm_tree_node *const root = dm_tree_find_node(tree, 0, 0);

    raid << dmraid_get_raid(root, root);
    block << dmraid_get_block(root, root);

    raid.removeDuplicates();
    block.removeDuplicates();

    dm_tree_free(tree);
    dm_task_destroy(dmt_list);
    dm_task_destroy(dmt_info);
    dm_lib_release();
}

QStringList dmraid_get_raid(dm_tree_node *const parent, dm_tree_node *const root)
{
    QStringList raid;
    void *handle = nullptr;
    void *childhandle = nullptr;
    const QRegExp rxp("^-[0-9]+$");
    const QString parent_uuid(dm_tree_node_get_uuid(parent));

    while(dm_tree_node *node = dm_tree_next_child(&handle, parent, 0)) {
        if(node == root)
            break;

        QString uuid(dm_tree_node_get_uuid(node));

        if (uuid.startsWith("DMRAID-")) {
            if(!uuid.remove(parent_uuid).contains(rxp)) {
                raid << QString("/dev/mapper/").append(dm_tree_node_get_name(node));
                raid << dmraid_get_raid(node, root);          
            }  
        } else if (uuid.startsWith("LVM-")) {  
            raid << dmraid_get_raid(node, root);            
        } else if (uuid.isEmpty()) {    // might be a partition 
            raid << dmraid_get_raid(node, root);   // look for and underlying device
        }

    }
    
    return raid;
}

QStringList dmraid_get_block(dm_tree_node *const parent, dm_tree_node *const root)
{
    QStringList block;
    const int buff_size = 1000;
    char dev_name[buff_size];
    void *handle = nullptr;

    while(dm_tree_node *node = dm_tree_next_child(&handle, parent, 0)) {
        if(node == root)
            break;

        const QString uuid(dm_tree_node_get_uuid(node));

        if (uuid.startsWith("DMRAID-") || uuid.startsWith("LVM-")) {
            block << dmraid_get_block(node, root);
        } else if (QString(dm_tree_node_get_uuid(parent)).startsWith("DMRAID-") && uuid.isEmpty()) {
            const dm_info *const dmi = dm_tree_node_get_info(node);
            dm_device_get_name(dmi->major, dmi->minor, 1, dev_name, buff_size);
            block << QString("/dev/").append(dev_name);
        } else {
            block << dmraid_get_block(node, root);
        }
    }
    
    return block;
}

void mdraid_get_devices(QStringList &block, QStringList &raid)
{
    QFile mdstat("/proc/mdstat");

    if(mdstat.exists() && mdstat.open(QIODevice::ReadOnly)) {
        
        QTextStream stream(&mdstat);
        QString line;
        const QRegExp raid_rx("^md[0-9]+ :");
        const QRegExp bracket_rx("\\[[0-9]+\\]$");

        do {
            line = stream.readLine();
            if(line.contains(raid_rx)) {
                QStringList split = line.split(' ');
                raid << QString("/dev/").append(split[0]);
                for(int x = 4; x < split.size(); ++x)
                    block << QString("/dev/").append(split[x].remove(bracket_rx));
            }
        } while (!line.isNull());
    }

    raid.removeDuplicates();
    block.removeDuplicates();

    return;
}
