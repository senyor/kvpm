/*
 *
 *
 * Copyright (C) 2008, 2010, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#include "vgreduce.h"

#include <lvm2app.h>

#include <KMessageBox>
#include <KLocale>
#include <QtGui>

#include "masterlist.h"
#include "misc.h"
#include "physvol.h"
#include "pvgroupbox.h"
#include "volgroup.h"



bool reduce_vg(VolGroup *const volumeGroup)
{
    VGReduceDialog dialog(volumeGroup);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
        return true;
    else
        return false;
}

VGReduceDialog::VGReduceDialog(VolGroup *const volumeGroup, QWidget *parent) : KDialog(parent), m_vg(volumeGroup)
{
    setWindowTitle(i18n("Reduce Volume Group"));
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    m_unremovable_pvs_present = false;
    QString vg_name = m_vg->getName();

    QList<PhysVol *> member_pvs = m_vg->getPhysicalVolumes();
    int pv_count = m_vg->getPvCount();
    m_unremovable_pvs_present = false;

    for (int x = pv_count - 1; x >= 0; x--) {
        if (member_pvs[x]->getSize() - member_pvs[x]->getRemaining()) { // only unused pvs can be removed
            member_pvs.removeAt(x);
            m_unremovable_pvs_present = true;
        }
    }

    QLabel *label;
    if (m_unremovable_pvs_present) {
        label = new QLabel(i18n("Select physical volumes to "
                                "remove them from volume group <b>%1</b>", vg_name));
    } else {
        label = new QLabel(i18n("Select physical volumes <b>excluding one</b> to "
                                "remove them from volume group <b>%1</b>", vg_name));
    }

    label->setWordWrap(true);
    layout->addWidget(label);

    m_pv_checkbox = new PvGroupBox(member_pvs);
    layout->addWidget(m_pv_checkbox);
    m_pv_checkbox->setTitle(i18n("Unused physical volumes"));

    connect(m_pv_checkbox, SIGNAL(stateChanged()), this, SLOT(excludeOneVolume()));
    m_pv_checkbox->selectNone();

    connect(this, SIGNAL(okClicked()),
            this, SLOT(commitChanges()));
}

void VGReduceDialog::commitChanges()
{
    const QByteArray vg_name = m_vg->getName().toLocal8Bit();
    QByteArray pv_name;
    vg_t vg_dm = NULL;
    lvm_t lvm = MasterList::getLvm();

    QStringList pv_list; // pvs to remove by name
    pv_list << m_pv_checkbox->getNames();

    if ((vg_dm = lvm_vg_open(lvm, vg_name.data(), "w", 0))) {
        for (int x = 0; x < pv_list.size(); x++) {
            pv_name = pv_list[x].toLocal8Bit();
            if (lvm_vg_reduce(vg_dm, pv_name.data()))
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        }
        if (lvm_vg_write(vg_dm))
            KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        lvm_vg_close(vg_dm);
    } else
        KMessageBox::error(0, QString(lvm_errmsg(lvm)));

    return;
}

void VGReduceDialog::excludeOneVolume()
{
    QStringList names = m_pv_checkbox->getNames();
    QStringList all_names = m_pv_checkbox->getAllNames();
    int boxes_checked = names.size();
    int boxes_count   = all_names.size();

    if (boxes_checked > 0) {
        if (m_unremovable_pvs_present || (boxes_checked < boxes_count))
            enableButtonOk(true);
        else
            enableButtonOk(false);
    } else
        enableButtonOk(false);
}


