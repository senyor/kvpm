/*
 *
 *
 * Copyright (C) 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */



#include "kvpmdialog.h"



KvpmDialog::KvpmDialog(QWidget *parent) :
    KDialog(parent)
{
    connect(this, SIGNAL(okClicked()),  this, SLOT(commit()));
    connect(this, SIGNAL(yesClicked()), this, SLOT(commit()));
}

KvpmDialog::~KvpmDialog()
{

}

int KvpmDialog::run()
{
    int result = QDialog::Rejected;

    if (m_allow_run)
        result = exec();

    return result;
}

void KvpmDialog::preventExec()
{
    m_allow_run = false;
    setResult(QDialog::Rejected);
}

bool KvpmDialog::willExec()
{
    return m_allow_run;
}
