/*
 *
 * 
 * Copyright (C) 2008 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include <KLocale>
#include <QtGui>

#include "processprogress.h"
#include "sizetostring.h"
#include "vgchangeextent.h"
#include "volgroup.h"

bool change_vg_extent(VolGroup *volumeGroup)
{
    VGChangeExtentDialog dialog(volumeGroup);
    dialog.exec();

    if(dialog.result() == QDialog::Accepted){
        ProcessProgress extent( dialog.arguments(), i18n("Changing extents..."));

        return true;
    }
    else{
        return false;
    }
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

    m_extent_size = new QComboBox();
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

    m_extent_suffix = new QComboBox();
    m_extent_suffix->insertItem(0,"KiB");
    m_extent_suffix->insertItem(1,"MiB");
    m_extent_suffix->insertItem(2,"GiB");
    m_extent_suffix->setInsertPolicy(QComboBox::NoInsert);
    m_extent_suffix->setCurrentIndex(1);

    QHBoxLayout *combo_layout = new QHBoxLayout();
    layout->addLayout(combo_layout);
    combo_layout->addWidget(m_extent_size);
    combo_layout->addWidget(m_extent_suffix);
}

QStringList VGChangeExtentDialog::arguments()
{  
    QStringList args;
    
    args << "vgchange"
	 << "--physicalextentsize"
	 << m_extent_size->currentText() + m_extent_suffix->currentText().at(0)
	 << m_vg_name;

    return args;
}
