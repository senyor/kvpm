/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "vgremovemissing.h"

#include <KIcon>
#include <KLocale>
#include <KMessageBox>

#include <QDebug>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>

#include "processprogress.h"
#include "volgroup.h"



VGRemoveMissingDialog::VGRemoveMissingDialog(VolGroup *const group, QWidget *parent) :
    KvpmDialog(parent),
    m_vg(group)
{
    setCaption(i18n("Remove missing physical volumes"));

    QHBoxLayout *const warning_layout = new QHBoxLayout();
    QVBoxLayout *const layout = new QVBoxLayout();

    QLabel *const icon_label = new QLabel();
    icon_label->setPixmap(KIcon("dialog-warning").pixmap(64, 64));
    warning_layout->addWidget(icon_label);
    warning_layout->addLayout(layout);

    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    dialog_body->setLayout(warning_layout);
    QLabel *message = new QLabel(i18n("<b>Removing missing physical volumes may result in data loss! Use with extreme care.</b>"));
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

void VGRemoveMissingDialog::commit()
{
    hide();

    QStringList args = QStringList() << "vgreduce"
                                     << "--removemissing";

    if (m_all_button->isChecked())
        args << "--force";

    args << m_vg->getName();

    ProcessProgress remove_missing(args);
}
