/*
 *
 *
 * Copyright (C) 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "dmraid.h"

#include <libdevmapper.h>


#include <QDebug>

    
QStringList dmraid_get_raid(dm_tree_node *parent);
QStringList dmraid_get_block(dm_tree_node *parent);


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

    dm_tree_node *const root_node = dm_tree_find_node(tree, 0, 0);

    raid << dmraid_get_raid(root_node);
    block << dmraid_get_block(root_node);

    raid.removeDuplicates();
    block.removeDuplicates();

    dm_tree_free(tree);
    dm_task_destroy(dmt_list);
    dm_task_destroy(dmt_info);
    dm_lib_release();
}

QStringList dmraid_get_raid(dm_tree_node *parent)
{
    QStringList raid;
    void *handle = nullptr;
    void *childhandle = nullptr;
    
    while(dm_tree_node *node = dm_tree_next_child(&handle, parent, 0)) {
        if (QString(dm_tree_node_get_uuid(node)).startsWith("DMRAID-"))
            raid << QString("/dev/mapper/").append(dm_tree_node_get_name(node));
        else if (QString(dm_tree_node_get_uuid(node)).startsWith("LVM-"))  
            raid << dmraid_get_raid(node);            
        else if (QString(dm_tree_node_get_uuid(node)).isEmpty()) // might be a partition 
            raid << dmraid_get_raid(node);   // look for and underlying device
    }
    
    return raid;
}

QStringList dmraid_get_block(dm_tree_node *parent)
{
    QStringList block;
    const int buff_size = 1000;
    char dev_name[buff_size];
    void *handle = nullptr;

    while(dm_tree_node *node = dm_tree_next_child(&handle, parent, 0)) {

        const QString uuid(dm_tree_node_get_uuid(node));

        if (uuid.startsWith("DMRAID-") || uuid.startsWith("LVM-")) {
            block << dmraid_get_block(node);
        } else if (QString(dm_tree_node_get_uuid(parent)).startsWith("DMRAID-") && uuid.isEmpty()) {
            const dm_info *const dmi = dm_tree_node_get_info(node);
            dm_device_get_name(dmi->major, dmi->minor, 1, dev_name, buff_size);
            block << QString("/dev/").append(dev_name);
        } else {
            block << dmraid_get_block(node);
        }
    }
    
    return block;
}
