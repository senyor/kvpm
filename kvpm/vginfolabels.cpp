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

VGInfoLabels::VGInfoLabels(VolGroup *VolumeGroup, QWidget *parent) : QWidget(parent) 
{
    QLabel *extent_size_label, *size_label, *used_label, 
	   *free_label, *lvm_fmt_label, *resizable_label, 
	   *clustered_label, *allocateable_label,
	   *max_lv_label, *max_pv_label, *policy_label,
	   *partial_label, *vg_write_mode;

    QString clustered, resizable, write_mode;
    QVBoxLayout *upper_layout = new QVBoxLayout();
    
    QHBoxLayout *hlayout1 = new QHBoxLayout();
    QHBoxLayout *hlayout2 = new QHBoxLayout();
    upper_layout->addLayout(hlayout1);
    upper_layout->addLayout(hlayout2);
    
    QVBoxLayout *vlayout1 = new QVBoxLayout();
    QVBoxLayout *vlayout2 = new QVBoxLayout();
    QVBoxLayout *vlayout3 = new QVBoxLayout();
    QVBoxLayout *vlayout4 = new QVBoxLayout();
    QVBoxLayout *vlayout5 = new QVBoxLayout();
    QVBoxLayout *vlayout6 = new QVBoxLayout();

    QFrame *label_frame1 = new QFrame();
    QFrame *label_frame2 = new QFrame();
    QFrame *label_frame3 = new QFrame();
    QFrame *label_frame4 = new QFrame();
    QFrame *label_frame5 = new QFrame();
    QFrame *label_frame6 = new QFrame();

    label_frame1->setFrameShape(QFrame::StyledPanel);
    label_frame2->setFrameShape(QFrame::StyledPanel);
    label_frame3->setFrameShape(QFrame::StyledPanel);
    label_frame4->setFrameShape(QFrame::StyledPanel);
    label_frame5->setFrameShape(QFrame::StyledPanel);
    label_frame6->setFrameShape(QFrame::StyledPanel);

    label_frame1->setFrameShadow(QFrame::Sunken);
    label_frame2->setFrameShadow(QFrame::Sunken);
    label_frame3->setFrameShadow(QFrame::Sunken);
    label_frame4->setFrameShadow(QFrame::Sunken);
    label_frame5->setFrameShadow(QFrame::Sunken);
    label_frame6->setFrameShadow(QFrame::Sunken);

    label_frame1->setLayout(vlayout1);
    label_frame2->setLayout(vlayout2);
    label_frame3->setLayout(vlayout3);
    label_frame4->setLayout(vlayout4);
    label_frame5->setLayout(vlayout5);
    label_frame6->setLayout(vlayout6);
    
    if(VolumeGroup->isResizable())
	resizable = "Yes";
    else
	resizable = "No";

    if(VolumeGroup->isClustered())
	clustered = "Yes";
    else    
	clustered = "No";

    if(VolumeGroup->isWritable())
	write_mode = "r/w";
    else
	write_mode = "r/o";
    
    if(VolumeGroup->isPartial())
    {
	partial_label = new QLabel("<b>*** Warning: Partial Volume Group ***</b>");
	hlayout2->addWidget(partial_label);
    }

    used_label   = new QLabel("Used: " + sizeToString( VolumeGroup->getUsedSpace() ));
    free_label   = new QLabel("Free: " +sizeToString( VolumeGroup->getFreeSpace() ));
    allocateable_label   = new QLabel("Allocateable: " + 
				      sizeToString( VolumeGroup->getAllocateableSpace() ));
    size_label         = new QLabel("Total: " + sizeToString(VolumeGroup->getSize()));
    extent_size_label  = new QLabel("Extent size: " + 
				    sizeToString( VolumeGroup->getExtentSize() ));
    lvm_fmt_label = new QLabel("Format: " + VolumeGroup->getFormat());
    policy_label = new QLabel("Policy: " + VolumeGroup->getPolicy());
    resizable_label = new QLabel("Resizable: " + resizable);
    clustered_label = new QLabel("Clustered: " + clustered);
    vg_write_mode = new QLabel("Access: " + write_mode);

    vlayout1->addWidget(size_label);
    vlayout1->addWidget(allocateable_label);
    vlayout2->addWidget(used_label);    
    vlayout2->addWidget(free_label);  
    vlayout3->addWidget(clustered_label);
    vlayout3->addWidget(lvm_fmt_label);
    vlayout4->addWidget(extent_size_label);
    vlayout4->addWidget(policy_label);
    vlayout5->addWidget(resizable_label);
    vlayout5->addWidget(vg_write_mode);
    
    hlayout1->addWidget(label_frame1);
    hlayout1->addWidget(label_frame2);
    hlayout1->addWidget(label_frame3);
    hlayout1->addWidget(label_frame4);
    hlayout1->addWidget(label_frame5);
    if( VolumeGroup->getLogVolMax() || VolumeGroup->getPhysVolMax() ){
	if(VolumeGroup->getPhysVolMax())
	    max_pv_label = new QLabel("Max pvs: " + QString("%1").arg(VolumeGroup->getPhysVolMax()));
	else
	    max_pv_label = new QLabel("Max pvs: Unlimited");
	if(VolumeGroup->getLogVolMax())
	    max_lv_label = new QLabel("Max lvs: " + QString("%1").arg(VolumeGroup->getLogVolMax()));
	else
	    max_lv_label = new QLabel("Max lvs: Unlimited");

	vlayout6->addWidget(max_pv_label);
	vlayout6->addWidget(max_lv_label);
	hlayout1->addWidget(label_frame6);
    }
    
    setLayout(upper_layout);
}

