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


#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "processprogress.h"
#include "vgchangeresize.h"
#include "volgroup.h"


bool change_vg_resize(VolGroup *volumeGroup)
{
    VGChangeResizeDialog dialog(volumeGroup);
    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress resize(dialog.args(), i18n("Changing volume group resizability..."));
        return true;
    }
    return false;
}

VGChangeResizeDialog::VGChangeResizeDialog(VolGroup *volumeGroup, QWidget *parent) : 
    KDialog(parent), m_vg(volumeGroup)
{
    setWindowTitle( i18n("Volume group resizability") );

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *name_label = new QLabel( i18n("Volume group: <b>%1</b>").arg(m_vg->getName() ) );
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    QGroupBox *group_box = new QGroupBox("Set volume group resizability");
    layout->addWidget(group_box);
    QVBoxLayout *group_layout = new QVBoxLayout();
    group_box->setLayout(group_layout);

    m_resize = new QRadioButton("Allow physical volume addition and removal");
    m_no_resize = new QRadioButton("Prevent physical volume addition and removal");

    if( m_vg->isResizable() )
        m_resize->setChecked(true);
    else
        m_no_resize->setChecked(true);

    group_layout->addWidget(m_resize);
    group_layout->addWidget(m_no_resize);
}

QStringList VGChangeResizeDialog::args()
{
    QStringList args;

    if( m_resize->isChecked() )
        args << "vgchange" << "--resizeable" << "y" << m_vg->getName();
    else
        args << "vgchange" << "--resizeable" << "n" << m_vg->getName();

    return args;
}
