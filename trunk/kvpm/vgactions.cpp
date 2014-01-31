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


#include "vgactions.h"

#include "kvpmdialog.h"
#include "physvol.h"
#include "topwindow.h"
#include "vgreduce.h"
#include "volgroup.h"

#include "vgchange.h"
#include "vgcreate.h"
#include "vgextend.h"
#include "vgremove.h"
#include "vgremovemissing.h"
#include "vgrename.h"
#include "vgexport.h"
#include "vgimport.h"
#include "vgsplit.h"
#include "vgmerge.h"


#include <KAction>
#include <KLocale>

#include <QDebug>



VGActions::VGActions(QWidget *parent) :
    KActionCollection(parent)
{
    KAction *const remove = addAction("vgremove");
    KAction *const reduce = addAction("vgreduce");
    KAction *const extend = addAction("vgextend");
    KAction *const rename = addAction("vgrename");
    KAction *const merge  = addAction("vgmerge");
    KAction *const split  = addAction("vgsplit");
    KAction *const change = addAction("vgchange");
    KAction *const create = addAction("vgcreate");
    KAction *const vgimport = addAction("vgimport");
    KAction *const vgexport = addAction("vgexport");
    KAction *const remove_missing = addAction("vgremovemissing");
    
    remove->setText(i18n("Delete Volume Group..."));
    reduce->setText(i18n("Reduce Volume Group..."));
    extend->setText(i18n("Extend Volume Group..."));
    rename->setText(i18n("Rename Volume Group..."));
    merge->setText(i18n("Merge Volume Group..."));
    split->setText(i18n("Split Volume Group..."));
    change->setText(i18n("Change Volume Group Attributes..."));
    create->setText(i18n("Create Volume Group..."));
    vgimport->setText(i18n("Import Volume Group..."));
    vgexport->setText(i18n("Export Volume Group..."));
    remove_missing->setText(i18n("Remove Missing Physcial Volumes..."));
    
    remove->setToolTip(i18n("Delete Volume Group"));
    reduce->setToolTip(i18n("Reduce Volume Group"));
    extend->setToolTip(i18n("Extend Volume Group"));
    rename->setToolTip(i18n("Rename Volume Group"));
    merge->setToolTip(i18n("Merge Volume Group"));
    split->setToolTip(i18n("Split Volume Group"));
    change->setToolTip(i18n("Change Volume Group Attributes"));
    create->setToolTip(i18n("Create Volume Group"));
    vgimport->setToolTip(i18n("Import Volume Group"));
    vgexport->setToolTip(i18n("Export Volume Group"));
    remove_missing->setToolTip(i18n("Remove Missing Physcial Volumes"));
    
    remove->setIconText(i18n("Delete"));
    reduce->setIconText(i18n("Reduce"));
    extend->setIconText(i18n("Extend"));
    rename->setIconText(i18n("Rename"));
    merge->setIconText(i18n("Merge"));
    split->setIconText(i18n("Split"));
    change->setIconText(i18n("Change Attributes"));
    create->setIconText(i18n("Create"));
    vgimport->setIconText(i18n("Import"));
    vgexport->setIconText(i18n("Export"));
    remove_missing->setIconText(i18n("Remove Missing"));
    
    remove->setIcon(KIcon("cross"));
    reduce->setIcon(KIcon("delete"));
    extend->setIcon(KIcon("add"));
    rename->setIcon(KIcon("edit-rename"));
    merge->setIcon(KIcon("arrow_join"));
    split->setIcon(KIcon("arrow_divide"));
    change->setIcon(KIcon("wrench")     );
    create->setIcon(KIcon("document-new"));
    vgimport->setIcon(KIcon("document-import"));
    vgexport->setIcon(KIcon("document-export"));
    remove_missing->setIcon(KIcon("error_go"));
    
    setVg(nullptr);

    connect(this, SIGNAL(actionTriggered(QAction *)),
            this, SLOT(callDialog(QAction *)));
}

void VGActions::setVg(VolGroup *vg)
{
    m_vg = vg;

    QAction *const remove = action("vgremove");
    QAction *const reduce = action("vgreduce");
    QAction *const extend = action("vgextend");
    QAction *const rename = action("vgrename");
    QAction *const merge  = action("vgmerge");
    QAction *const split  = action("vgsplit");
    QAction *const change = action("vgchange");
    QAction *const vgimport = action("vgimport");
    QAction *const vgexport = action("vgexport");
    QAction *const remove_missing = action("vgremovemissing");
    
    // only enable group removal if the tab is
    // a volume group with no logical volumes

    if (vg && !vg->openFailed()) {

        bool has_active = false;
        auto lvs = vg->getLogicalVolumes();

        for (auto lv : lvs) {
            if (lv->isActive()) {
                has_active = true;
                break;
            }
        }

        if (lvs.size() || vg->isPartial() || vg->isExported())
            remove->setEnabled(false);
        else
            remove->setEnabled(true);

        remove_missing->setEnabled(vg->isPartial());
        change->setEnabled(!vg->isExported());
        
        if (vg->isExported()) {
            split->setEnabled(false);
            merge->setEnabled(false);
            vgimport->setEnabled(true);
            vgexport->setEnabled(false);
            reduce->setEnabled(false);
            extend->setEnabled(false);
        } else if (!vg->isPartial()) {
            vgimport->setEnabled(false);
            reduce->setEnabled(true);
            split->setEnabled(true);
            merge->setEnabled(true);
            extend->setEnabled(true);

            if (has_active)
                vgexport->setEnabled(false);
            else
                vgexport->setEnabled(true);
        } else {
            split->setEnabled(false);
            merge->setEnabled(false);
            vgimport->setEnabled(false);
            vgexport->setEnabled(false);
            reduce->setEnabled(false);
            extend->setEnabled(false);
        }

        rename->setEnabled(true);
    } else {
        reduce->setEnabled(false);
        rename->setEnabled(false);
        remove->setEnabled(false);
        remove_missing->setEnabled(false);
        change->setEnabled(false);
        vgimport->setEnabled(false);
        split->setEnabled(false);
        merge->setEnabled(false);
        vgexport->setEnabled(false);
        extend->setEnabled(false);
    }
}

void VGActions::callDialog(QAction *action)
{
    const QString name = action->objectName();

    if (name == "vgremove" || name == "vgimport" || name == "vgexport") {
        if (name == "vgremove") {
            if (remove_vg(m_vg))
                g_top_window->reRun();
        } else if (name == "vgimport") {
            if (import_vg(m_vg))
                g_top_window->reRun();
        } else if (name == "vgexport") {
            if (export_vg(m_vg))
                g_top_window->reRun();
        }
    } else {
        KvpmDialog *dialog = nullptr;

        if (name == "vgchange")
            dialog = new VGChangeDialog(m_vg);
        else if (name == "vgcreate")
            dialog = new VGCreateDialog();
        else if (name == "vgreduce")
            dialog = new VGReduceDialog(m_vg);
        else if (name == "vgextend")
            dialog = new VGExtendDialog(m_vg);
        else if (name == "vgsplit")
            dialog = new VGSplitDialog(m_vg);
        else if (name == "vgmerge")
            dialog = new VGMergeDialog(m_vg);
        else if (name == "vgrename")
            dialog = new VGRenameDialog(m_vg);
        else if (name == "vgremovemissing")
            dialog = new VGRemoveMissingDialog(m_vg);
        
        if (dialog) {
            int result = dialog->run();
            
            if (result == QDialog::Accepted || result == KDialog::Yes)
                g_top_window->reRun();
        }
    }
}
