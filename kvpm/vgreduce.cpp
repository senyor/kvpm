/*
 *
 *
 * Copyright (C) 2008, 2010, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
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

#include "masterlist.h"
#include "misc.h"
#include "physvol.h"
#include "pvgroupbox.h"
#include "volgroup.h"

#include <lvm2app.h>

#include <KLocale>
#include <KMessageBox>

#include <QDebug>
#include <QLabel>
#include <QSharedPointer>
#include <QVBoxLayout>




// Remove one selected pv from group

VGReduceDialog::VGReduceDialog(PhysVol *const pv, QWidget *parent) : 
    KvpmDialog(parent), 
    m_pv(pv),
    m_vg(pv->getVg())
{
    setCaption(i18n("Remove Physical Volume"));
    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    QLabel *const label = new QLabel(i18n("<b>Remove physical volume: %1?</b>", m_pv->getName()));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    layout->addSpacing(5);

    setButtons(KDialog::Yes | KDialog::No);
}

// Remove one or more pvs from selected group

VGReduceDialog::VGReduceDialog(VolGroup *const group, QWidget *parent) : 
    KvpmDialog(parent), 
    m_vg(group)
{
    setCaption(i18n("Reduce Volume Group"));
    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    const QString vg_name = m_vg->getName();

    QList<QSharedPointer<PvSpace>> pv_space_list;
    m_unremovable_pvs_present = false;
    for (auto pv : m_vg->getPhysicalVolumes()) {
        if (pv->getSize() - pv->getRemaining()) { // only unused pvs can be removed
            m_unremovable_pvs_present = true;
        } else {
            pv_space_list << QSharedPointer<PvSpace>(new PvSpace(pv, pv->getSize(), pv->getSize()));
        }
    }

    if (pv_space_list.isEmpty()){
        preventExec();
        KMessageBox::sorry(0, i18n("There are no physical volumes that can be removed"));
    } else if (pv_space_list.size() == 1 && !m_unremovable_pvs_present) {
        preventExec();
        KMessageBox::sorry(0, i18n("There is only one physical volume in this group"));
    } else {
        QLabel *label;

        label = new QLabel(i18n("<b>Reduce volume group: %1</b>", m_vg->getName()));
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
        layout->addSpacing(5);

        if (m_unremovable_pvs_present) {
            label = new QLabel(i18n("Select physical volumes to remove them from the volume group"));
        } else {
            label = new QLabel(i18n("Select physical volumes, <b>excluding one</b>, to "
                                    "remove them from the volume group"));
        }
        
        label->setWordWrap(true);
        QHBoxLayout *const label_layout = new QHBoxLayout;
        QWidget *const label_widget = new QWidget;
        label_layout->addWidget(label);
        label_widget->setLayout(label_layout);

        layout->addWidget(label_widget);
        m_pv_checkbox = new PvGroupBox(pv_space_list, NO_POLICY, NO_POLICY);

        layout->addWidget(m_pv_checkbox);
        m_pv_checkbox->setTitle(i18n("Unused physical volumes"));
        
        connect(m_pv_checkbox, SIGNAL(stateChanged()), this, SLOT(excludeOneVolume()));
        m_pv_checkbox->selectNone();
    }
}

void VGReduceDialog::commit()
{
    const QByteArray vg_name = m_vg->getName().toLocal8Bit();
    vg_t vg_dm = nullptr;
    lvm_t lvm = MasterList::getLvm();

    QStringList pv_list;
    if (m_pv)
        pv_list << m_pv->getName();
    else if (m_pv_checkbox)
        pv_list << m_pv_checkbox->getNames(); // pvs to remove by name
 
    if ((vg_dm = lvm_vg_open(lvm, vg_name.data(), "w", 0))) {
        
        for (auto name : pv_list) {
            const QByteArray name_ba = name.toLocal8Bit();
            
            if (lvm_vg_reduce(vg_dm, name_ba.data()))
                KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        }
        
        if (lvm_vg_write(vg_dm))
            KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        
        lvm_vg_close(vg_dm);
    } else {
        KMessageBox::error(0, QString(lvm_errmsg(lvm)));
    }
    
    return;
}

void VGReduceDialog::excludeOneVolume()
{
    const int boxes_checked = m_pv_checkbox->getNames().size();
    const int boxes_count   = m_pv_checkbox->getAllNames().size();

    if (boxes_checked > 0) {
        if (m_unremovable_pvs_present || (boxes_checked < boxes_count))
            enableButtonOk(true);
        else
            enableButtonOk(false);
    } else {
        enableButtonOk(false);
    }
}


