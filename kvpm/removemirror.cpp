/*
 *
 *
 * Copyright (C) 2008, 2011, 2012, 2013  Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "removemirror.h"

#include <KLocale>

#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>

#include "logvol.h"
#include "misc.h"
#include "processprogress.h"
#include "volgroup.h"



RemoveMirrorDialog::RemoveMirrorDialog(LogVol *mirror, QWidget *parent) :
    KvpmDialog(parent)
{
    while (mirror->isLvmMirrorLog() || mirror->isLvmMirrorLeg())
        mirror = mirror->getParentMirror();

    m_lv = mirror;
    m_vg = m_lv->getVg();

    setCaption(i18n("Remove Mirrors"));

    QWidget *const dialog_body = new QWidget(this);
    QVBoxLayout *const layout = new QVBoxLayout;
    dialog_body->setLayout(layout);
    setMainWidget(dialog_body);

    QLabel *const message = new QLabel(i18n("Select mirror legs to remove from <b>%1</b>", m_lv->getName()));
    layout->addSpacing(5);
    layout->addWidget(message);
    layout->addSpacing(10);

    for (auto lv : m_lv->getChildren()) {
        if (lv->isMirrorLeg()) {
            NoMungeCheck *const check = new NoMungeCheck(lv->getName());
            check->setAlternateTextList(lv->getPvNamesAll());
            m_leg_checks.append(check);
            layout->addWidget(check);

            connect(check, SIGNAL(stateChanged(int)),
                    this, SLOT(validateCheckStates()));
        }
    }

    layout->addSpacing(10);
    validateCheckStates();
}

void RemoveMirrorDialog::commit()
{
    hide();

    int mirror_count = m_leg_checks.size();
    QStringList legs;       // mirror legs (actually pv names) being deleted

    for (auto check : m_leg_checks) {
        if (check->isChecked()) {
            legs << check->getAlternateTextList();
            mirror_count--;
        }
    }

    QStringList args = QStringList() << "lvconvert"
                                     << "--mirrors"
                                     << QString("%1").arg(mirror_count - 1)
                                     << m_lv->getFullName()
                                     << legs;

    ProcessProgress remove(args);
}

/* One leg of the mirror must always be left intact,
   so we make certain at least one check box is left
   unchecked. The unchecked one is disabled. */

void RemoveMirrorDialog::validateCheckStates()
{
    const int check_box_count = m_leg_checks.size();
    int checked_count = 0;
    
    for (auto checkbox : m_leg_checks) {
        if (checkbox->isChecked()) {
            ++checked_count;
        }
    }

    enableButtonOk(static_cast<bool>(checked_count));
    
    if (checked_count == (check_box_count - 1)) {
        for (auto checkbox : m_leg_checks) {
            if (!checkbox->isChecked())
                checkbox->setEnabled(false);
        }
    } else {
        for (auto checkbox : m_leg_checks)
            checkbox->setEnabled(true);
    }
}
