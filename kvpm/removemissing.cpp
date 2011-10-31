/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "removemissing.h"

#include <KMessageBox>
#include <KLocale>

#include <QtGui>

#include "processprogress.h"
#include "volgroup.h"

bool remove_missing_pv(VolGroup *volumeGroup)
{
    RemoveMissingDialog dialog(volumeGroup);
    dialog.exec();
    
    if(dialog.result() == QDialog::Accepted){
        ProcessProgress remove_missing( dialog.arguments() );
        return true;
    }
    return false;
}

RemoveMissingDialog::RemoveMissingDialog(VolGroup *volumeGroup, QWidget *parent) : 
    KDialog(parent),
    m_vg(volumeGroup)
{

    setWindowTitle( i18n("Remove missing physical volumes") );
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);
    QLabel *message = new QLabel( i18n("<b>Removing missing physical volumes may result in data loss! Use with extreme care.</b>") );
    message->setWordWrap(true);
    layout->addWidget(message);

    QGroupBox *radio_box = new QGroupBox();
    QVBoxLayout *radio_box_layout = new QVBoxLayout();
    radio_box->setLayout(radio_box_layout);
    layout->addWidget(radio_box);

    m_empty_button = new QRadioButton("Remove only empty physical volumes");
    m_all_button   = new QRadioButton("Remove all missing physical volumes");
    m_empty_button->setChecked(true);
    radio_box_layout->addWidget(m_empty_button);
    radio_box_layout->addWidget(m_all_button);
}

QStringList RemoveMissingDialog::arguments()
{
    QStringList args;

    args << "vgreduce"
         << "--removemissing";
    
    if( m_all_button->isChecked() )
        args << "--force";

    args << m_vg->getName();
    return args;
}
