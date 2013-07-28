/*
 *
 *
 * Copyright (C) 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "pvactions.h"

#include "kvpmdialog.h"
#include "physvol.h"
#include "processprogress.h"
#include "pvchange.h"
#include "pvtree.h"
#include "pvmove.h"
#include "topwindow.h"
#include "vgreduce.h"
#include "volgroup.h"

#include <KAction>
#include <KLocale>

#include <QDebug>
#include <QTreeWidgetItem>


PVActions::PVActions(VolGroup *const group, QWidget *parent) :
    KActionCollection(parent),
    m_vg(group)
{
    KAction *const pv_move = addAction("pvmove");
    pv_move->setText(i18n("Move all physical extents"));
    pv_move->setIconText(i18n("Move all"));
    pv_move->setToolTip(i18n("Move all extents to another physical volume"));
    pv_move->setIcon(KIcon("lorry_go"));

    KAction *const vg_reduce = addAction("pvremove");
    vg_reduce->setText(i18n("Remove from volume group"));
    vg_reduce->setIconText(i18n("Remove"));
    vg_reduce->setToolTip(i18n("Remove physical volume from volume group"));
    vg_reduce->setIcon(KIcon("list-remove"));

    KAction *const pv_change = addAction("pvchange");
    pv_change->setText(i18n("Change physical volume attributes"));
    pv_change->setIconText(i18n("attributes"));
    pv_change->setToolTip(i18n("Change physical volume attributes"));
    pv_change->setIcon(KIcon("wrench_orange"));

    setActions(nullptr, false); 

    connect(this, SIGNAL(actionTriggered(QAction *)),
            this, SLOT(callDialog(QAction *)));
}

void PVActions::setActions(PhysVol *const pv, bool const isMoving)
{
    m_pv = pv;

    QAction *const pv_move   = action("pvmove");
    QAction *const vg_reduce = action("pvremove");
    QAction *const pv_change = action("pvchange");

    if (!pv || m_vg->isExported()) {
        pv_move->setEnabled(false);
        vg_reduce->setEnabled(false);
        pv_change->setEnabled(false);
    } else {
        pv_move->setEnabled(true);
        vg_reduce->setEnabled(true);
        pv_change->setEnabled(true);
        
        if (pv->getSize() == pv->getRemaining()) {  // pv has no extents in use
            pv_move->setEnabled(false);

            if (m_vg->getPvCount() > 1)
                vg_reduce->setEnabled(true);
            else
                vg_reduce->setEnabled(false);  // can't remove last pv from group
        } else {
            vg_reduce->setEnabled(false);

            if (m_vg->getPvCount() > 1) {  // can't move extents if there isn't another volume to put them on
                if (isMoving)                   // can't have more than one pvmove
                    pv_move->setEnabled(false); // See physvol.cpp about removing this
                else
                    pv_move->setEnabled(true);
            } else {
                pv_move->setEnabled(false);
            }
        }
    }
}

void PVActions::changePv(QTreeWidgetItem *item)
{
    PhysVol *pv = nullptr;
    bool ismoving = false;

    if (item) {
        pv = m_vg->getPvByName(item->data(0, 0).toString());
        ismoving = QVariant(item->data(7, 0)).toString().contains("pvmove");
    }

    setActions(pv, ismoving);
}

void PVActions::callDialog(QAction *action)
{
    if (m_pv) {
        KvpmDialog *dialog = nullptr;

        if (action->objectName() == "pvmove")
            dialog = new PVMoveDialog(m_pv);
        else if (action->objectName() == "pvchange")
            dialog = new PVChangeDialog(m_pv);
        else if (action->objectName() == "pvremove")
            dialog = new VGReduceDialog(m_pv);

        if (dialog) {
            int result = dialog->run();
            
            if (result == QDialog::Accepted || result == KDialog::Yes)
                MainWindow->reRun();
            
            dialog->deleteLater();
        }
    }
}         
