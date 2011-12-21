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


#include "lvremove.h"

#include <KMessageBox>
#include <KLocale>

#include <QtGui>

#include "logvol.h"
#include "processprogress.h"


LVRemoveDialog::LVRemoveDialog(LogVol *const lv, QWidget *parent) : KDialog(parent), m_lv(lv)
{
    setButtons(KDialog::Yes | KDialog::No);
    setCaption( i18n("Delete Volume") );
    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);

    m_name = m_lv->getName();
    m_bailout = false;

    QHBoxLayout *const layout = new QHBoxLayout();
    QVBoxLayout *const right_layout = new QVBoxLayout();

    QLabel *const icon_label = new QLabel();
    icon_label->setPixmap( KIcon("dialog-warning").pixmap(64, 64) );
    layout->addWidget(icon_label);
    layout->addLayout(right_layout);

    QLabel *label;

    label = new QLabel("<b>Confirm Volume Deletion</b>");
    label->setAlignment(Qt::AlignCenter);
    right_layout->addWidget(label);
    right_layout->addSpacing(20);

    QHBoxLayout *const lower_layout = new QHBoxLayout();
    QVBoxLayout *const list_layout = new QVBoxLayout();

    const QString snap_msg1 = i18n("The volume: <b>%1</b> has snapshots.", m_name);
    const QString snap_msg2 = i18n("The following volumes will all be deleted:");
    const QString msg1 = i18n("Delete the volume named: %1?", "<b>" + m_name + "</b>");
    const QString msg2 = i18n("Any data on it will be lost.");

    if( m_lv->isOrigin() ){
        right_layout->addWidget( new QLabel(snap_msg1) );
        right_layout->addWidget( new QLabel(snap_msg2) );
        right_layout->addWidget( new QLabel("") );

        right_layout->addLayout(lower_layout);
        lower_layout->addSpacing(15);
        lower_layout->addLayout(list_layout);

        label = new QLabel("<b>" + m_name + "</b>");
        label->setAlignment(Qt::AlignLeft);
        list_layout->addWidget(label);

        QListIterator<LogVol *> snap_itr( m_lv->getSnapshots() );
        LogVol *snap;
        while( snap_itr.hasNext() ){
            snap = snap_itr.next();

            if( snap->isMounted() )
                m_bailout = true;

            label = new QLabel( "<b>" + snap->getName() + "</b>" );
            label->setAlignment(Qt::AlignLeft);
            list_layout->addWidget(label);
        }
        right_layout->addWidget( new QLabel("") );
        right_layout->addWidget( new QLabel( i18n("Are you certain you want to delete these volumes?") ) );
        right_layout->addWidget( new QLabel( i18n("Any data on them will be lost.") ) );
    }
    else{
        right_layout->addWidget( new QLabel(msg1) );
        right_layout->addWidget( new QLabel(msg2) );
    }

    dialog_body->setLayout(layout);

    connect(this, SIGNAL(yesClicked()), 
            this, SLOT(commitChanges()));

    if(m_bailout){
        hide();
        KMessageBox::error(this, i18n("A snapshot of this origin is busy or mounted. It can not be deleted.") );
    }
}

bool LVRemoveDialog::bailout()
{
    return m_bailout;
}

void LVRemoveDialog::commitChanges()
{
    const QString full_name = m_lv->getFullName().remove('[').remove(']');
    QStringList args;

    if( m_lv->isActive() && !m_lv->isSnap() ){
        args << "lvchange" 
             << "-an" 
             << full_name;
        
        ProcessProgress deactivate(args);
    }

    args.clear();
    args << "lvremove" 
         << "--force" 
         << full_name;
    
    ProcessProgress remove(args);
        
    return;
}
