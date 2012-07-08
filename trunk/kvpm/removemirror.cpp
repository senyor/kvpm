/*
 *
 *
 * Copyright (C) 2008, 2011, 2012  Benjamin Scott   <benscott@nwlink.com>
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



bool remove_mirror(LogVol *logicalVolume)
{
    RemoveMirrorDialog dialog(logicalVolume);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted) {
        ProcessProgress remove_mirror(dialog.arguments());
        return true;
    } else {
        return false;
    }
}

RemoveMirrorDialog::RemoveMirrorDialog(LogVol *logicalVolume, QWidget *parent):
    KDialog(parent),
    m_lv(logicalVolume)
{
    NoMungeCheck *temp_check;
    QStringList   pv_names;

    m_vg = m_lv->getVg();
    QList<LogVol *> lvs = m_lv->getChildren();

    setWindowTitle(i18n("Remove mirrors"));

    QWidget *const dialog_body = new QWidget(this);
    QVBoxLayout *const layout = new QVBoxLayout;
    dialog_body->setLayout(layout);
    setMainWidget(dialog_body);

    QLabel *const message = new QLabel(i18n("Select the mirror legs to remove:"));
    layout->addWidget(message);

    for (int x = lvs.size() - 1; x >= 0 ; x--) {
        if (lvs[x]->isMirrorLeg()) {

            temp_check = new NoMungeCheck(lvs[x]->getName());
            pv_names = lvs[x]->getPvNamesAll();
            temp_check->setAlternateTextList(pv_names);
            m_mirror_leg_checks.append(temp_check);
            layout->addWidget(temp_check);

            connect(temp_check, SIGNAL(stateChanged(int)),
                    this , SLOT(validateCheckStates(int)));
        }
    }
}

/*
   Here we create a string based on all
   the options that the user chose in the
   dialog and feed that to "lvconvert"
*/

QStringList RemoveMirrorDialog::arguments()
{
    int mirror_count = m_mirror_leg_checks.size();
    QStringList args;
    QStringList legs;       // mirror legs (actually pv names) being deleted

    for (int x = 0; x < m_mirror_leg_checks.size(); x++) {
        if (m_mirror_leg_checks[x]->isChecked()) {
            legs << m_mirror_leg_checks[x]->getAlternateTextList();
            mirror_count--;
        }
    }

    args << "lvconvert"
         << "--mirrors"
         << QString("%1").arg(mirror_count - 1)
         << m_lv->getFullName()
         << legs;

    return args;
}

/* One leg of the mirror must always be left intact,
   so we make certain at least one check box is left
   unchecked. The unchecked one is disabled. */

void RemoveMirrorDialog::validateCheckStates(int)
{
    const int check_box_count = m_mirror_leg_checks.size();
    int checked_count = 0;

    for (int x = 0; x < check_box_count; x++) {
        if (m_mirror_leg_checks[x]->isChecked()) {
            checked_count++;
        }
    }

    if (checked_count == (check_box_count - 1)) {

        for (int x = 0; x < check_box_count; x++) {
            if (!m_mirror_leg_checks[x]->isChecked()) {
                m_mirror_leg_checks[x]->setEnabled(false);
            }
        }
    } else {
        for (int x = 0; x < check_box_count; x++) {
            m_mirror_leg_checks[x]->setEnabled(true);

        }

    }
}
