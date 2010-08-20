/*
 *
 * 
 * Copyright (C) 2008, 2010 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include <lvm2app.h>

#include <KLocale>
#include <QtGui>
#include <KMessageBox>

#include "misc.h"
#include "vgchangeextent.h"
#include "volgroup.h"


// This function is for changing the extent size in a volume group

bool change_vg_extent(VolGroup *volumeGroup)
{
    VGChangeExtentDialog dialog(volumeGroup);
    dialog.exec();

    if(dialog.result() == QDialog::Accepted)
        return true;
    else
        return false;
}

VGChangeExtentDialog::VGChangeExtentDialog(VolGroup *volumeGroup, QWidget *parent) : 
    KDialog(parent)
{
    m_vg_name = volumeGroup->getName();
    setWindowTitle( i18n("Extent size") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *name_label = new QLabel( i18n("Volume group: <b>%1</b>").arg(m_vg_name) );
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    QLabel *message = new QLabel( i18n("The extent size is currently: %1").arg(sizeToString( volumeGroup->getExtentSize() )) );
    
    message->setWordWrap(true);
    layout->addWidget(message);

    m_extent_size = new KComboBox();
    m_extent_size->insertItem(0,"1");
    m_extent_size->insertItem(1,"2");
    m_extent_size->insertItem(2,"4");
    m_extent_size->insertItem(3,"8");
    m_extent_size->insertItem(4,"16");
    m_extent_size->insertItem(5,"32");
    m_extent_size->insertItem(6,"64");
    m_extent_size->insertItem(7,"128");
    m_extent_size->insertItem(8,"256");
    m_extent_size->insertItem(9,"512");
    m_extent_size->setInsertPolicy(QComboBox::NoInsert);
    m_extent_size->setCurrentIndex(2);

    m_extent_suffix = new KComboBox();
    m_extent_suffix->insertItem(0,"KiB");
    m_extent_suffix->insertItem(1,"MiB");
    m_extent_suffix->insertItem(2,"GiB");
    m_extent_suffix->setInsertPolicy(QComboBox::NoInsert);
    m_extent_suffix->setCurrentIndex(1);

    QHBoxLayout *combo_layout = new QHBoxLayout();
    layout->addLayout(combo_layout);
    combo_layout->addWidget(m_extent_size);
    combo_layout->addWidget(m_extent_suffix);

    connect(this, SIGNAL(okClicked()), 
            this, SLOT(commitChanges()));

    connect(m_extent_suffix, SIGNAL(activated(int)), 
            this, SLOT(limitExtentSize(int)));

}

void VGChangeExtentDialog::limitExtentSize(int index){

    int extent_index;

    if( index > 1 ){  // Gigabytes selected as suffix, more than 2Gib forbidden
        if( m_extent_size->currentIndex() > 2 )
            m_extent_size->setCurrentIndex(0);
        m_extent_size->setMaxCount(2);
    }
    else{
        extent_index = m_extent_size->currentIndex();
        m_extent_size->setMaxCount(10);
        m_extent_size->setInsertPolicy(QComboBox::InsertAtBottom);
        m_extent_size->insertItem(3,"4");
        m_extent_size->insertItem(3,"8");
        m_extent_size->insertItem(4,"16");
        m_extent_size->insertItem(5,"32");
        m_extent_size->insertItem(6,"64");
        m_extent_size->insertItem(7,"128");
        m_extent_size->insertItem(8,"256");
        m_extent_size->insertItem(9,"512");
        m_extent_size->setInsertPolicy(QComboBox::NoInsert);
        m_extent_size->setCurrentIndex(extent_index);
    }
}

void VGChangeExtentDialog::commitChanges()
{  
    lvm_t  lvm;
    vg_t vg_dm;
    uint32_t new_extent_size = m_extent_size->currentText().toULong();

    new_extent_size *= 1024;
    if( m_extent_suffix->currentIndex() > 0 )
        new_extent_size *= 1024;
    if( m_extent_suffix->currentIndex() > 1 )
        new_extent_size *= 1024;

    if( (lvm = lvm_init(NULL)) ){
        if( (vg_dm = lvm_vg_open( lvm, m_vg_name.toAscii().data(), "w", NULL )) ){

            if( (lvm_vg_set_extent_size(vg_dm, new_extent_size)) )
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));

            if( lvm_vg_write(vg_dm) )
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));

            lvm_vg_close(vg_dm);
            lvm_quit(lvm);
            return;
        }
        KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
        lvm_quit(lvm);
        return;
    }
    KMessageBox::error(0, QString(lvm_errmsg(lvm))); 
    return;
}
