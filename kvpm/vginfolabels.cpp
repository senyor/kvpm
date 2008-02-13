/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Klvm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <QtGui>

#include "sizetostring.h"
#include "vginfolabels.h"


VGInfoLabels::VGInfoLabels(VolGroup *VolumeGroup, QWidget *parent) : QFrame(parent)
{
    VolGroup *vg = VolumeGroup;
    QGridLayout *layout = new QGridLayout();
    QString clustered, resizable, write_mode;

    setFrameShape(QFrame::StyledPanel);
    setFrameShadow(QFrame::Sunken);
    
    QLabel *vg_extent_size_label, *vg_size_label;
    QLabel *vg_used_space_label, *vg_free_space_label, *lvm_fmt_label;
    QLabel *vg_resizable_label, *vg_clustered_label, *vg_allocateable_space_label;
    QLabel *lv_max_label, *pv_max_label, *policy_label;
    QLabel *vg_partial, *vg_write_mode;

    if(vg->isResizable())
	resizable = "Yes";
    else
	resizable = "No";

    if(vg->isClustered())
	clustered = "Yes";
    else    
	clustered = "No";

    if(vg->isWritable())
	write_mode = "read / write";
    else
	write_mode = "read only";
    
    if(vg->isPartial())
    {
	vg_partial = new QLabel("<b>*** Warning: Partial Volume Group ***</b>");
	layout->addWidget(vg_partial, 3, 1);
    }

    vg_used_space_label   = new QLabel("Used: " + sizeToString(vg->getUsedSpace()));
    vg_free_space_label   = new QLabel("Free: " +sizeToString(vg->getFreeSpace()));
    vg_allocateable_space_label   = new QLabel("Allocateable: " +sizeToString(vg->getAllocateableSpace()));
    vg_size_label         = new QLabel("Total: " + sizeToString(vg->getSize()));
    vg_extent_size_label  = new QLabel("Extent size: " + sizeToString(vg->getExtentSize()));
    lvm_fmt_label = new QLabel("Format: " + vg->getFormat());
    policy_label = new QLabel("Policy: " + vg->getPolicy());
    vg_resizable_label = new QLabel("Resizable: " + resizable);
    vg_clustered_label = new QLabel("Clustered: " + clustered);
    if(vg->getPhysVolMax())
	pv_max_label = new QLabel("Max physical volumes: " + QString("%1").arg(vg->getPhysVolMax()));
    else
	pv_max_label = new QLabel("Max physical volumes: Unlimited");
    if(vg->getLogVolMax())
	lv_max_label = new QLabel("Max logical volumes: " + QString("%1").arg(vg->getLogVolMax()));
    else
	lv_max_label = new QLabel("Max logical volumes: Unlimited");
    vg_write_mode = new QLabel("Access: " + write_mode);
    
    layout->addWidget(vg_size_label,        0, 0);
    layout->addWidget(vg_used_space_label,  0, 1);
    layout->addWidget(vg_clustered_label,   0, 2);
    layout->addWidget(vg_extent_size_label, 0, 3);
    layout->addWidget(vg_resizable_label,   0, 4);
    layout->addWidget(pv_max_label,         0, 5);

    layout->addWidget(vg_allocateable_space_label, 1, 0);
    layout->addWidget(vg_free_space_label,         1, 1);
    layout->addWidget(lvm_fmt_label,               1, 2);
    layout->addWidget(policy_label,                1, 3);
    layout->addWidget(vg_write_mode,               1, 4);
    layout->addWidget(lv_max_label,                1, 5);


    setLayout(layout);
}

