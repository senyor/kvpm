/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#include "lvreduce.h"

#include <KMessageBox>
#include <KLocale>

#include <QtGui>

#include "logvol.h"
#include "processprogress.h"
#include "fsreduce.h"
#include "misc.h"
#include "sizeselectorbox.h"
#include "volgroup.h"


LVReduceDialog::LVReduceDialog(LogVol *const volume, QWidget *parent)
    : KDialog(parent),
      m_lv(volume)
{
    m_vg = m_lv->getVg();
    setWindowTitle( i18n("Reduce Logical Volume") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    const long long extent_size = m_vg->getExtentSize();
    const long long current_lv_extents = m_lv->getExtents();
    const QString fs = m_lv->getFilesystem();
    long long min_lv_extents;
    bool force = false;
    m_bailout = false;

    const QString warning_message1 = i18n("If this <b>Inactive</b> logical volume is reduced "
                                          "any <b>data</b> it contains <b>will be lost!</b>");

    const QString warning_message2 = i18n("Only the ext2, ext3 and ext4 file systems "
                                          "are supported for file system reduction. If this " 
                                          "logical volume is reduced any <b>data</b> it contains "
                                          "<b>will be lost!</b>");

    if( !m_lv->isActive() && !m_lv->isSnap() ){
	if(KMessageBox::warningContinueCancel(0, warning_message1) == KMessageBox::Continue)
            force = true;
	else
            m_bailout = true;
    }
    else if( m_lv->isMounted() && !m_lv->isSnap() ){
        KMessageBox::error(0, i18n("The filesystem must be unmounted first") );
        m_bailout = true;
    }
    else if( !fs_can_reduce(fs) && !m_lv->isSnap() ){
	if(KMessageBox::warningContinueCancel(0, warning_message2) == KMessageBox::Continue)
	    force = true;
	else
            m_bailout = true;
    }

    if( !m_bailout && !force && !m_lv->isSnap() ){

        const long long min_fs_size = get_min_fs_size( m_lv->getMapperPath(), m_lv->getFilesystem() );

        if( min_fs_size % extent_size )
            min_lv_extents = 1 + (min_fs_size / extent_size);
        else
            min_lv_extents = min_fs_size / extent_size;
        
        if( min_fs_size == 0 || min_lv_extents >= m_lv->getExtents() ){
            KMessageBox::error(0, i18n("The filesystem is already as small as it can be") );
            m_bailout = true;
        }
    }
    else
        min_lv_extents = 1;

    if(!m_bailout){                

        QLabel *lv_name_label  = new QLabel( i18n("<b>Volume: %1</b>", m_lv->getName() ));
        lv_name_label->setAlignment(Qt::AlignCenter);
        
        m_size_selector = new SizeSelectorBox(extent_size, min_lv_extents, current_lv_extents, current_lv_extents, true, false, true);
                
        layout->addWidget(lv_name_label);
        layout->addWidget(m_size_selector);
        
        connect(this, SIGNAL(okClicked()),
                this, SLOT(doShrink()));
    }
}

bool LVReduceDialog::bailout()
{
    return m_bailout;
}

void LVReduceDialog::doShrink()
{
    QStringList lv_arguments;
    const QString fs =  m_lv->getFilesystem();
    const long long target_size = m_size_selector->getCurrentSize() * m_vg->getExtentSize();
    long long new_size;

    hide();

    if( m_lv->isSnap() || !m_lv->isActive() )   // never reduce the fs of a snap!
        new_size = target_size;
    else if( fs_can_reduce(fs) )
        new_size = fs_reduce( m_lv->getMapperPath(), target_size, fs );
    else
        new_size = target_size;

    if( new_size ){
        lv_arguments << "lvreduce" 
                     << "--force" 
                     << "--size" 
                     << QString("%1K").arg( new_size / 1024 )
                     << m_lv->getMapperPath();

        ProcessProgress reduce_lv(lv_arguments);
    }
}
