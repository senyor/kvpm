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


#include "lvactions.h"

#include "fsck.h"
#include "logvol.h"
#include "vgtree.h"
#include "lvsizechartseg.h"
#include "topwindow.h"
#include "changemirror.h"
#include "lvsizechartseg.h"
#include "lvcreate.h"
#include "lvchange.h"
#include "lvreduce.h"
#include "lvremove.h"
#include "lvrename.h"
#include "mkfs.h"
#include "maxfs.h"
#include "mount.h"
#include "pvmove.h"
#include "repairmissing.h"
#include "removemirror.h"
#include "removemirrorleg.h"
#include "snapmerge.h"
#include "thincreate.h"
#include "unmount.h"
#include "volgroup.h"

#include <KAction>
#include <KLocale>

#include <QDebug>
#include <QTreeWidgetItem>


LVActions::LVActions(VolGroup *const group, QWidget *parent) :
    KActionCollection(parent),
    m_vg(group)
{
    KAction *const lv_create = addAction("lvcreate", this, SLOT(createLv()));
    lv_create->setText(i18n("Create logical volume..."));
    lv_create->setIconText(i18n("New volume"));
    lv_create->setToolTip(i18n("Create a new logical volume"));
    lv_create->setIcon(KIcon("document-new"));

    KAction *const thin_pool = addAction("thinpool", this, SLOT(createThinPool()));
    thin_pool->setText(i18n("Create thin pool..."));
    thin_pool->setToolTip(i18n("Create a new thin pool"));
    thin_pool->setIconText(i18n("New pool"));
    thin_pool->setIcon(KIcon("object-group"));

    KAction *const thin_create = addAction("thincreate", this, SLOT(createThinVolume()));
    thin_create->setText(i18n("Create thin volume..."));
    thin_create->setToolTip(i18n("Create new thin volume"));
    thin_create->setIconText(i18n("Thin volume"));
    thin_create->setIcon(KIcon("page_white_add"));

    KAction *const lv_rename = addAction("lvrename", this, SLOT(renameLv()));
    lv_rename->setText(i18n("Rename logical volume..."));
    lv_rename->setToolTip(i18n("Rename a logical volume or thin pool"));
    lv_rename->setIconText(i18n("Rename"));
    lv_rename->setIcon(KIcon("edit-rename"));

    KAction *const snap_create = addAction("snapcreate", this, SLOT(createSnapshot()));
    snap_create->setText(i18n("Create snapshot..."));
    snap_create->setToolTip(i18n("Create a snapshot of a volume"));
    snap_create->setIconText(i18n("Snapshot"));
    snap_create->setIcon(KIcon("camera_add"));

    KAction *const thin_snap = addAction("thinsnap", this, SLOT(thinSnapshot()));
    thin_snap->setText(i18n("Create thin snapshot..."));
    thin_snap->setToolTip(i18n("Create a thin snapshot of a volume"));
    thin_snap->setText(i18n("Thin snapshot"));
    thin_snap->setIcon(KIcon("page_white_camera"));

    KAction *const snap_merge = addAction("snapmerge", this, SLOT(mergeSnapshot()));
    snap_merge->setText(i18n("Merge snapshot..."));
    snap_merge->setToolTip(i18n("Merge a snapshot with its origin"));
    snap_merge->setIconText(i18n("Merge"));
    snap_merge->setIcon(KIcon("arrow_join"));

    KAction *const lv_reduce = addAction("lvreduce", this, SLOT(reduceLv()));
    lv_reduce->setText(i18n("Reduce logical volume..."));
    lv_reduce->setToolTip(i18n("Reduce the size of a logical volume or thin pool"));
    lv_reduce->setText(i18n("Reduce"));
    lv_reduce->setIcon(KIcon("delete"));

    KAction *const lv_extend = addAction("lvextend", this, SLOT(extendLv()));
    lv_extend->setText(i18n("Extend logical volume..."));
    lv_extend->setToolTip(i18n("Increase the size of a logical volume or thin pool"));
    lv_extend->setIconText(i18n("Extend"));
    lv_extend->setIcon(KIcon("add"));

    KAction *const pv_move = addAction("pvmove", this, SLOT(movePhysicalExtents()));
    pv_move->setText(i18n("Move physical extents..."));
    pv_move->setToolTip(i18n("Move extents to another physical volume"));
    pv_move->setIconText(i18n("Move"));
    pv_move->setIcon(KIcon("lorry"));

    KAction *const lv_change = addAction("lvchange", this, SLOT(changeLv()));
    lv_change->setText(i18n("Change attributes or tags..."));
    lv_change->setToolTip(i18n("Change attributes or tags of a volume or thin pool"));
    lv_change->setIconText(i18n("Attributes"));
    lv_change->setIcon(KIcon("wrench"));

    KAction *const lv_remove = addAction("lvremove", this, SLOT(removeLv()));
    lv_remove->setText(i18n("Delete logical volume..."));
    lv_remove->setToolTip(i18n("Delete logical volume or thin pool"));
    lv_remove->setIconText(i18n("Delete"));
    lv_remove->setIcon(KIcon("cross"));

    KAction *const mkfs =  addAction("mkfs", this, SLOT(makeFs()));
    mkfs->setIcon(KIcon("lightning_add"));
    mkfs->setText(i18n("Make or remove filesystem..."));
    mkfs->setIconText(i18n("mkfs"));
    mkfs->setToolTip(i18n("Make or remove a filesystem"));

    KAction *const max_fs = addAction("maxfs", this, SLOT(maxFs()));
    max_fs->setText(i18n("Extend filesystem to fill volume..."));
    max_fs->setIcon(KIcon("resultset_last")); 
    max_fs->setIconText(i18n("Extend fs"));
    max_fs->setToolTip(i18n("Extend filesystem to fill volume"));

    KAction *const fsck = addAction("fsck", this, SLOT(checkFs()));
    fsck->setText(i18n("Run 'fsck -fp' on filesystem..."));
    fsck->setIcon(KIcon("checkmark")); 
    fsck->setIconText(i18n("fsck -fp"));
    fsck->setToolTip(i18n("Run 'fsck -fp' on the filesystem"));

    KAction *const mount = addAction("mount", this, SLOT(mountFs()));
    mount->setText(i18n("Mount filesystem..."));
    mount->setIconText(i18n("mount fs"));
    mount->setToolTip(i18n("Mount filesystem"));
    mount->setIcon(KIcon("emblem-mounted"));

    KAction *const unmount = addAction("unmount", this, SLOT(unmountFs()));
    unmount->setText(i18n("Unmount filesystem..."));
    unmount->setIconText(i18n("Unmount fs"));
    unmount->setToolTip(i18n("Unmount filesystem"));
    unmount->setIcon(KIcon("emblem-unmounted"));

    KAction *const add_legs = addAction("addlegs", this , SLOT(addLegs()));
    add_legs->setText(i18n("Add mirror legs to volume..."));
    add_legs->setIconText(i18n("Add mirrors"));
    add_legs->setToolTip(i18n("Add mirror legs to volume"));
    add_legs->setIcon(KIcon("edit-copy"));

    KAction *const change_log = addAction("changelog", this, SLOT(changeLog())); 
    change_log->setText(i18n("Change mirror log..."));
    change_log->setIconText(i18n("Change log"));
    change_log->setToolTip(i18n("Change mirror log"));
    change_log->setIcon(KIcon("preferences-other"));     

    KAction *const remove_mirror = addAction("removemirror", this, SLOT(removeMirror()));
    remove_mirror->setText(i18n("Remove mirror legs..."));
    remove_mirror->setToolTip(i18n("Remove mirror legs"));
    remove_mirror->setIconText(i18n("Remove legs"));
    remove_mirror->setIcon(KIcon("document-close"));    

    KAction *const remove_this = addAction("removethis", this, SLOT(removeLeg()));
    remove_this->setIcon(KIcon("document-close"));
    remove_this->setText(i18n("Remove this mirror leg..."));
    remove_this->setIconText(i18n("Remove leg..."));
    remove_this->setText(i18n("Remove this mirror leg"));

    KAction *const repair_missing = addAction("repairmissing", this, SLOT(repairMissing()));
    repair_missing->setIcon(KIcon("edit-bomb"));
    repair_missing->setText(i18n("Repair volume..."));
    repair_missing->setIconText(i18n("Repair"));
    repair_missing->setToolTip(i18n("Repair raid or mirror volume"));

    setActions(nullptr, 0); 
}

void LVActions::setActions(LogVol *const lv, const int segment)
{
    m_lv = lv;
    m_segment = segment;

    QAction *const lv_create   = action("lvcreate");
    QAction *const thin_pool   = action("thinpool");
    QAction *const thin_create = action("thincreate");
    QAction *const lv_rename   = action("lvrename");
    QAction *const snap_create = action("snapcreate");
    QAction *const thin_snap   = action("thinsnap");
    QAction *const snap_merge  = action("snapmerge");
    QAction *const lv_reduce   = action("lvreduce");
    QAction *const lv_extend   = action("lvextend");
    QAction *const pv_move     = action("pvmove");
    QAction *const lv_change   = action("lvchange");
    QAction *const lv_remove   = action("lvremove");
    QAction *const mount   = action("mount");
    QAction *const unmount = action("unmount");
    QAction *const max_fs  = action("maxfs");
    QAction *const fsck    = action("fsck");
    QAction *const mkfs    = action("mkfs");

    lv_create->setEnabled(true);
    thin_pool->setEnabled(true);
    thin_create->setEnabled(true);
    lv_rename->setEnabled(true);
    snap_create->setEnabled(true);
    thin_snap->setEnabled(true);
    snap_merge->setEnabled(true);
    lv_reduce->setEnabled(true);
    lv_extend->setEnabled(true);
    pv_move->setEnabled(true);
    lv_change->setEnabled(true);
    lv_remove->setEnabled(true);
    mount->setEnabled(true);
    unmount->setEnabled(true);
    max_fs->setEnabled(true);
    fsck->setEnabled(true);
    mkfs->setEnabled(true);
    
    lv_extend->setText(i18n("Extend logical volume..."));
    lv_reduce->setText(i18n("Reduce logical volume..."));
    lv_rename->setText(i18n("Rename logical volume..."));
    lv_remove->setText(i18n("Delete logical volume..."));

    if (m_vg->isExported()) {
        lv_create->setEnabled(false);
        thin_pool->setEnabled(false);
        thin_create->setEnabled(false);
        lv_rename->setEnabled(false);
        snap_create->setEnabled(false);
        thin_snap->setEnabled(false);
        snap_merge->setEnabled(false);
        lv_reduce->setEnabled(false);
        lv_extend->setEnabled(false);
        pv_move->setEnabled(false);
        lv_change->setEnabled(false);
        lv_remove->setEnabled(false);
        mount->setEnabled(false);
        unmount->setEnabled(false);
        max_fs->setEnabled(false);
        fsck->setEnabled(false);
        mkfs->setEnabled(false);
    } else if (lv) {  // snap containers are replaced by the "real" lv before getting here, see: m_vg->getLvByName()

        thin_create->setEnabled(false);
        thin_snap->setEnabled(false);
        
        if (!lv->isThinVolume() && lv->isCowSnap() && lv->isValid() && !lv->isMerging())
            snap_merge->setEnabled(true);
        else
            snap_merge->setEnabled(false);

        if(lv->isThinPool()){
            lv_extend->setText(i18n("Extend thin pool..."));
            lv_reduce->setText(i18n("Reduce thin pool..."));
            lv_rename->setText(i18n("Rename thin pool..."));
            lv_remove->setText(i18n("Delete thin pool..."));

            lv_create->setEnabled(false);
            lv_remove->setEnabled(true);
            thin_create->setEnabled(true);
            thin_pool->setEnabled(false);
            snap_merge->setEnabled(false);
            max_fs->setEnabled(false);
            mkfs->setEnabled(false);
            fsck->setEnabled(false);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            lv_change->setEnabled(true);
            lv_extend->setEnabled(true);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(true);
            pv_move->setEnabled(true);
            snap_create->setEnabled(false);
        } else if(lv->isThinMetadata() || lv->isThinPoolData()){
            thin_create->setEnabled(true);
            snap_merge->setEnabled(false);
            lv_remove->setEnabled(false);
            max_fs->setEnabled(false);
            mkfs->setEnabled(false);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            fsck->setEnabled(false);
            lv_change->setEnabled(false);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(false);
            
            if (lv->isRaid() || lv->isLvmMirror())
                pv_move->setEnabled(false);
            else
                pv_move->setEnabled(true);
            
            snap_create->setEnabled(false);
        } else if(lv->isLvmMirrorLog() || lv->isTemporary()){
            snap_merge->setEnabled(false);
            max_fs->setEnabled(false);
            mkfs->setEnabled(false);
            
            if (lv->getParentMirror() == nullptr)  //  allow removal of bogus log
                lv_remove->setEnabled(true);
            else
                lv_remove->setEnabled(false);
            
            unmount->setEnabled(false);
            mount->setEnabled(false);
            fsck->setEnabled(false);
            lv_change->setEnabled(false);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(false);
            pv_move->setEnabled(false);
            snap_create->setEnabled(false);
        } else if (lv->isWritable()    && !lv->isLocked()    && !lv->isVirtual() &&
                   !lv->isThinVolume() && !lv->isMirrorLeg() && !lv->isLvmMirrorLog() &&
                   !lv->isRaidImage()  && !lv->isTemporary() && !lv->isRaidMetadata()) {

            if (lv->isMounted()) {
                fsck->setEnabled(false);
                mkfs->setEnabled(false);
                lv_reduce->setEnabled(false);
                lv_extend->setEnabled(true);
                lv_remove->setEnabled(false);
                unmount->setEnabled(true);
                mount->setEnabled(true);
            } else if (lv->isOpen() && lv->getFilesystem() == "swap") {
                fsck->setEnabled(false);
                mkfs->setEnabled(false);
                lv_reduce->setEnabled(false);
                lv_extend->setEnabled(false);
                lv_remove->setEnabled(false);
                max_fs->setEnabled(false);
                unmount->setEnabled(false);
                mount->setEnabled(false);
            } else {
                mkfs->setEnabled(true);
                lv_reduce->setEnabled(true);
                lv_extend->setEnabled(true);
                lv_remove->setEnabled(true);
                unmount->setEnabled(false);
                mount->setEnabled(true);
            }
            
            if (lv->isCowOrigin()) {
                snap_create->setEnabled(true);
                
                if (lv->isMerging()) {
                    snap_create->setEnabled(false);
                    lv_extend->setEnabled(false);
                } else {
                    lv_extend->setEnabled(true);
                }

                lv_reduce->setEnabled(false);
                pv_move->setEnabled(false);
            } else if (lv->isCowSnap()) {
                max_fs->setEnabled(false);
                snap_create->setEnabled(false);
                pv_move->setEnabled(false);
                
                if (lv->isMerging() || !lv->isValid()) {
                    lv_extend->setEnabled(false);
                    lv_reduce->setEnabled(false);
                    mount->setEnabled(false);
                    fsck->setEnabled(false);
                    mkfs->setEnabled(false);

                    if (!lv->isValid())
                        lv_remove->setEnabled(true);
                    else
                        lv_remove->setEnabled(false);
                } else if (lv->isMounted()) {
                    lv_extend->setEnabled(true);
                    lv_reduce->setEnabled(false);
                    fsck->setEnabled(false);
                    mkfs->setEnabled(false);
                } else {
                    lv_extend->setEnabled(true);
                    lv_reduce->setEnabled(true);
                    fsck->setEnabled(true);
                    mkfs->setEnabled(true);
                }
            } else if (lv->isLvmMirror()) {
                pv_move->setEnabled(false);
                
                if (lv->isUnderConversion()) {
                    lv_extend->setEnabled(false);
                    lv_reduce->setEnabled(false);
                } else {
                    lv_extend->setEnabled(true);
                    lv_reduce->setEnabled(true);
                }
                
            } else if (lv->isRaid()) {
                lv_change->setEnabled(false);
                lv_extend->setEnabled(true);
                lv_reduce->setEnabled(false);
                lv_rename->setEnabled(true);
                pv_move->setEnabled(false);
                snap_create->setEnabled(true);
            } else {
                snap_create->setEnabled(true);
            }
            
            if (lv->isCowSnap() && lv->isMerging()) {
                lv_rename->setEnabled(false);
                lv_change->setEnabled(false);
            } else {
                lv_rename->setEnabled(true);
                lv_change->setEnabled(true);
            }
        } else if (lv->isThinVolume()){
            lv_extend->setText(i18n("Extend thin volume..."));
            lv_reduce->setText(i18n("Reduce thin volume..."));
            lv_rename->setText(i18n("Rename thin volume..."));
            lv_remove->setText(i18n("Delete thin volume..."));
            
            thin_create->setEnabled(true);
            thin_snap->setEnabled(true);
            pv_move->setEnabled(false);
            lv_reduce->setEnabled(true);
            
            if (lv->isMounted()) {
                fsck->setEnabled(false);
                mkfs->setEnabled(false);
                lv_reduce->setEnabled(false);
                lv_extend->setEnabled(true);
                lv_remove->setEnabled(false);
                unmount->setEnabled(true);
                mount->setEnabled(true);
            } else if (lv->isOpen() && lv->getFilesystem() == "swap") {
                fsck->setEnabled(false);
                mkfs->setEnabled(false);
                lv_reduce->setEnabled(false);
                lv_extend->setEnabled(false);
                lv_remove->setEnabled(false);
                max_fs->setEnabled(false);
                unmount->setEnabled(false);
                mount->setEnabled(false);
            } else {
                mkfs->setEnabled(true);
                lv_extend->setEnabled(true);
                lv_remove->setEnabled(true);
                unmount->setEnabled(false);
                mount->setEnabled(true);
            }
            
            if (!lv->isWritable()) {
                fsck->setEnabled(false);
                mkfs->setEnabled(false);
                lv_reduce->setEnabled(false);
                lv_extend->setEnabled(false);
                max_fs->setEnabled(false);
            } else if (lv->isCowOrigin()) {
                lv_reduce->setEnabled(false);
            }
        } else if (lv->isOrphan()) {
            mkfs->setEnabled(false);
            max_fs->setEnabled(false);
            lv_remove->setEnabled(true);
            unmount->setEnabled(false);
            mount->setEnabled(false);   
            fsck->setEnabled(false);
            lv_change->setEnabled(false);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(false);
            pv_move->setEnabled(false);
            snap_create->setEnabled(false);
        } else if (lv->isRaidImage() || lv->isRaidMetadata()) {
            mkfs->setEnabled(false);
            max_fs->setEnabled(false);
            fsck->setEnabled(false);
            lv_remove->setEnabled(false);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            lv_change->setEnabled(false);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(false);
            pv_move->setEnabled(false);
            snap_create->setEnabled(false);
        } else if (lv->isPvmove()) {
            mkfs->setEnabled(false);
            max_fs->setEnabled(false);
            fsck->setEnabled(false);
            lv_remove->setEnabled(false);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            lv_change->setEnabled(false);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(false);
            pv_move->setEnabled(false);
            snap_create->setEnabled(false);
        } else if (lv->isLvmMirrorLeg() && !lv->isLvmMirrorLog() && !lv->isTemporary()) {
            mkfs->setEnabled(false);
            max_fs->setEnabled(false);
            fsck->setEnabled(false);
            lv_remove->setEnabled(false);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            lv_change->setEnabled(false);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            pv_move->setEnabled(false);
            lv_rename->setEnabled(false);
            snap_create->setEnabled(false);
        } else if (!(lv->isWritable()) && lv->isLocked()) {
  
            if (lv->isMounted())
                unmount->setEnabled(true);
            else
                unmount->setEnabled(false);
            
            mount->setEnabled(true);
            mkfs->setEnabled(false);
            max_fs->setEnabled(false);
            lv_remove->setEnabled(false);
            lv_change->setEnabled(true);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(false);
            pv_move->setEnabled(false);
            snap_create->setEnabled(false);
        } else if (lv->isWritable() && lv->isLocked()) {
    
            if (lv->isMounted())
                  unmount->setEnabled(true);
            else
                  unmount->setEnabled(false);
    
            mount->setEnabled(true);
            mkfs->setEnabled(true);
            lv_remove->setEnabled(false);
            lv_rename->setEnabled(false);
            lv_change->setEnabled(true);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            pv_move->setEnabled(false);
            snap_create->setEnabled(false);
        } else {
            if (lv->isMounted()) {
                lv_remove->setEnabled(false);
                unmount->setEnabled(true);
            } else {
                lv_remove->setEnabled(true);
                unmount->setEnabled(false);
            }
            
            if (lv->isCowSnap() || lv->isCowOrigin()) {
                pv_move->setEnabled(false);
                
                if (lv->isCowSnap())
                    snap_create->setEnabled(false);
                else
                    snap_create->setEnabled(true);
                
            } else if (lv->isMirror()) {
                pv_move->setEnabled(false);
                snap_create->setEnabled(false);
            } else {
                pv_move->setEnabled(true);
                snap_create->setEnabled(true);
            }

            mkfs->setEnabled(false);
            fsck->setEnabled(false);
            max_fs->setEnabled(false);
            lv_reduce->setEnabled(false);
            lv_extend->setEnabled(false);
            
            if (lv->isVirtual()) {
                lv_rename->setEnabled(false);
                lv_remove->setEnabled(false);
                mount->setEnabled(false);
                lv_change->setEnabled(false);
            } else {
                mount->setEnabled(true);
                lv_change->setEnabled(true);
            }
        }
        
        if (!lv->isActive()) {
            mkfs->setEnabled(false);
            fsck->setEnabled(false);
            max_fs->setEnabled(false);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            snap_create->setEnabled(false);
        }
    } else {
        snap_merge->setEnabled(false);
        max_fs->setEnabled(false);
        mkfs->setEnabled(false);
        lv_remove->setEnabled(false);
        unmount->setEnabled(false);
        mount->setEnabled(false);
        fsck->setEnabled(false);
        lv_change->setEnabled(false);
        lv_extend->setEnabled(false);
        lv_reduce->setEnabled(false);
        lv_rename->setEnabled(false);
        pv_move->setEnabled(false);
        snap_create->setEnabled(false);
        thin_snap->setEnabled(false);
        thin_create->setEnabled(false);
    }
}

void LVActions::setMirrorActions(LogVol *const lv)
{
    bool partial = false;

    QAction *const add_legs = action("addlegs");
    QAction *const remove_mirror = action("removemirror");
    QAction *const change_log = action("changelog");
    QAction *const remove_this = action("removethis");
    QAction *const repair_missing = action("repairmissing");

    add_legs->setEnabled(true);
    change_log->setEnabled(true);
    remove_mirror->setEnabled(true);
    remove_this->setEnabled(true);
    repair_missing->setEnabled(true);

    if (lv && !m_vg->isExported()) {

        if ((lv->isRaidImage() || lv->isRaidMetadata()) && (lv->getParent()))
            partial = lv->getParent()->isPartial();
        else if ((lv->isLvmMirrorLog() || lv->isLvmMirrorLeg() || lv->isTemporary()) && (lv->getParentMirror()))
            partial = lv->getParentMirror()->isPartial();
        else
            partial = lv->isPartial();

        if (lv->isThinPool()) {
            add_legs->setEnabled(false);
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            remove_this->setEnabled(false);
            repair_missing->setEnabled(false);
        } else if (partial && (lv->isRaidImage() || lv->isRaid() || lv->isMirror() || lv->isRaidMetadata() ||
                               lv->isMirrorLeg() || lv->isLvmMirrorLog() || lv->isTemporary())) {

            add_legs->setEnabled(false);
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            remove_this->setEnabled(false);
            repair_missing->setEnabled(true);
        } else if (partial) {
            add_legs->setEnabled(false);
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            remove_this->setEnabled(false);
            repair_missing->setEnabled(false);
        } else if(lv->isThinMetadata() || lv->isThinPoolData()){
            add_legs->setEnabled(false);
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            remove_this->setEnabled(false);
            repair_missing->setEnabled(false);
        } else if(lv->isLvmMirrorLog() || lv->isTemporary()){
            remove_mirror->setEnabled(true);
            change_log->setEnabled(true);
            remove_this->setEnabled(false);
            repair_missing->setEnabled(false);

            if (lv->getParentMirror()) {
                if (lv->getParentMirror()->isUnderConversion() || lv->getParentMirror()->isCowOrigin())
                    add_legs->setEnabled(false);
                else
                    add_legs->setEnabled(true);
            } else {
                add_legs->setEnabled(false);
            }
        } else if (lv->isRaidImage()) {
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            repair_missing->setEnabled(false);
            
            if(lv->isMirrorLeg()) {
                remove_this->setEnabled(true);
                remove_this->setVisible(true);
                remove_mirror->setVisible(false);
                add_legs->setEnabled(true);
            } else {
                remove_this->setEnabled(false);
                add_legs->setEnabled(false);
            }
        } else if (lv->isRaidMetadata()) {
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            repair_missing->setEnabled(false);
            remove_this->setEnabled(false);

            if (lv->getParent()) {
                if (lv->getParent()->isMirror())
                    add_legs->setEnabled(true);
                else
                    add_legs->setEnabled(false);
            } else {
                add_legs->setEnabled(false);
            }
        } else if (!lv->isLocked() && !lv->isVirtual() && !lv->isThinVolume() &&
                   !lv->isMirrorLeg() && !lv->isLvmMirrorLog() && !lv->isRaidImage() && !lv->isTemporary()) {

            remove_this->setEnabled(false);
            repair_missing->setEnabled(false);

            if (lv->isCowOrigin()) {
                if (lv->isMirror()) {
                    if (lv->isRaid()) {
                        change_log->setEnabled(false);
                        add_legs->setEnabled(true);
                    } else {
                        change_log->setEnabled(true);
                        add_legs->setEnabled(false);
                    }
                    
                    remove_mirror->setEnabled(true);
                } else if (lv->isRaid()) {
                    add_legs->setEnabled(false);
                    change_log->setEnabled(false);
                    remove_mirror->setEnabled(false);
                } else {
                    add_legs->setEnabled(true);
                    change_log->setEnabled(false);
                    remove_mirror->setEnabled(false);
                }
                
                if (lv->isMerging()) {
                    add_legs->setEnabled(false);
                }
            } else if (lv->isCowSnap()) {
                add_legs->setEnabled(false);
                remove_mirror->setEnabled(false);
                change_log->setEnabled(false);
            } else if (lv->isMirror()) {
                remove_mirror->setEnabled(true);
                        
                if (lv->isRaid())
                    change_log->setEnabled(false);
                else
                    change_log->setEnabled(true);
                
                if (lv->isUnderConversion())
                    add_legs->setEnabled(false);
                else
                    add_legs->setEnabled(true);

            } else if (lv->isRaid()) {
                add_legs->setEnabled(false);
                change_log->setEnabled(false);
                remove_mirror->setEnabled(false);
            } else {
                add_legs->setEnabled(true);
                remove_mirror->setEnabled(false);
                change_log->setEnabled(false);
            }
        } else if (lv->isVirtual() ||lv->isThinVolume() || lv->isOrphan() || lv->isPvmove()) {
            add_legs->setEnabled(false);
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            remove_this->setEnabled(false);
            repair_missing->setEnabled(false);
        } else if (lv->isLvmMirrorLeg() && !lv->isLvmMirrorLog() && !lv->isTemporary()) {
            remove_mirror->setEnabled(false);
            change_log->setEnabled(true);
            remove_this->setEnabled(true);
            remove_this->setVisible(true);
            remove_mirror->setVisible(false);
            repair_missing->setEnabled(false);
            
            if (lv->getParentMirror()) {
                if (lv->getParentMirror()->isUnderConversion() || lv->getParentMirror()->isCowOrigin())
                    add_legs->setEnabled(false);
                else
                    add_legs->setEnabled(true);
            } else {
                add_legs->setEnabled(false);
            }
        } else if (lv->isLocked()) {
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            remove_this->setEnabled(false);
            add_legs->setEnabled(false);
            repair_missing->setEnabled(false);
        }

        if (!lv->isActive()) {
            change_log->setEnabled(false);
        }
    } else {
        add_legs->setEnabled(false);
        remove_mirror->setEnabled(false);
        change_log->setEnabled(false);
        remove_this->setEnabled(false);
        repair_missing->setEnabled(false);
    }

    remove_this->setVisible(remove_this->isEnabled());
    remove_mirror->setVisible(!remove_this->isEnabled());
}

void LVActions::changeLv(LogVol *lv, int segment)
{
    setActions(lv, segment);     // segment = -1 means whole lv
    setMirrorActions(lv);
}

void LVActions::changeLv(QTreeWidgetItem *item)
{
    LogVol *target = nullptr;
    int segment = -1;

    if (item) {
        for (auto lv : m_vg->getLogicalVolumesFlat()) {
            if (QVariant(item->data(2, Qt::UserRole)).toString() == lv->getUuid()) {
                
                target = lv;
                if (QVariant(item->data(3, Qt::UserRole)).toString() == "segment")
                    segment = QVariant(item->data(1, Qt::UserRole)).toInt();
                
                break;
            }
        }
    }
    
    changeLv(target, segment);
}

void LVActions::mountFs()
{
    MountDialog dialog(m_lv);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
        g_top_window->reRun();
}

void LVActions::unmountFs()
{
    UnmountDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted || dialog.result() == KDialog::Yes)
            g_top_window->reRun();
    }
}

void LVActions::makeFs()
{
    MkfsDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

void LVActions::checkFs()
{
    if (manual_fsck(m_lv))
        g_top_window->reRun();
}

void LVActions::maxFs()
{
    if (max_fs(m_lv))
        g_top_window->reRun();
}

void LVActions::createLv()
{
    LVCreateDialog dialog(m_vg, false);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

void LVActions::createThinVolume()
{
    auto pool = m_lv;
    while (pool->getParent() && !pool->isThinPool())
        pool = pool->getParent();

    ThinCreateDialog dialog(pool);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

void LVActions::createThinPool()
{
    LVCreateDialog dialog(m_vg, true);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

void LVActions::reduceLv()
{
    LVReduceDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

void LVActions::extendLv()
{
    if (m_lv->isThinVolume()) {
        ThinCreateDialog dialog(m_lv, false);
        
        if (!dialog.bailout()) {
            dialog.exec();
            if (dialog.result() == QDialog::Accepted)
                g_top_window->reRun();
        }
    } else {
        LVCreateDialog dialog(m_lv, false);
        
        if (!dialog.bailout()) {
            dialog.exec();
            if (dialog.result() == QDialog::Accepted)
                g_top_window->reRun();
        }
    }
}

void LVActions::mergeSnapshot()
{
    if (merge_snap(m_lv))
        g_top_window->reRun();
}

void LVActions::removeLv()
{
    LVRemoveDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == KDialog::Yes)
            g_top_window->reRun();
    }
}

void LVActions::renameLv()
{
    LVRenameDialog dialog(m_lv);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
        g_top_window->reRun();
}

void LVActions::createSnapshot()
{
    LVCreateDialog dialog(m_lv, true);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}


void LVActions::thinSnapshot()
{
    ThinCreateDialog dialog(m_lv, true);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

void LVActions::changeLv()
{
    LVChangeDialog dialog(m_lv);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
        g_top_window->reRun();
}

void LVActions::movePhysicalExtents()
{
    PVMoveDialog dialog(m_lv, m_segment);

    if (dialog.run() == QDialog::Accepted)
            g_top_window->reRun();
}

void LVActions::addLegs()
{
    ChangeMirrorDialog dialog(m_lv, false);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

void LVActions::changeLog()
{
    ChangeMirrorDialog dialog(m_lv, true);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

void LVActions::removeMirror()
{
    RemoveMirrorDialog dialog(m_lv);

    if (dialog.run() == QDialog::Accepted)
        g_top_window->reRun();
}

void LVActions::removeLeg()
{
    if (remove_mirror_leg(m_lv))
        g_top_window->reRun();
}

void LVActions::repairMissing()
{
    RepairMissingDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

