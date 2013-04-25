/*
 *
 *
 * Copyright (C) 2008, 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "lvactionsmenu.h"

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
#include <KActionCollection>
#include <KLocale>

#include <QDebug>
#include <QPoint>


LVActionsMenu::LVActionsMenu(LogVol *logicalVolume, int segment, VolGroup *volumeGroup, QWidget *parent) :
    KMenu(parent),
    m_vg(volumeGroup),
    m_lv(logicalVolume),
    m_segment(segment)
{
    if (m_vg->getSize() == 0)
        setEnabled(false);

    KActionCollection *lvs = new KActionCollection(this);
    lvs->addAssociatedWidget(this);

    KAction *const lv_create = lvs->addAction("lvcreate", this, SLOT(createLv()));
    lv_create->setText(i18n("Create logical volume..."));
    lv_create->setIcon(KIcon("document-new"));

    KAction *const thin_create = lvs->addAction("thincreate", this, SLOT(createThinVolume()));
    thin_create->setText(i18n("Create thin volume..."));
    thin_create->setIcon(KIcon("document-new"));

    KAction *const thin_pool = lvs->addAction("thinpool", this, SLOT(createThinPool()));
    thin_pool->setText(i18n("Create thin pool..."));
    thin_pool->setIcon(KIcon("document-new"));

    KAction *const lv_remove = lvs->addAction("lvremove", this, SLOT(removeLv()));
    lv_remove->setText(i18n("Remove logical volume..."));
    lv_remove->setIcon(KIcon("cross"));
    addSeparator();

    KAction *const lv_rename = lvs->addAction("lvrename", this, SLOT(renameLv()));
    lv_rename->setText(i18n("Rename logical volume..."));
    lv_rename->setIcon(KIcon("edit-rename"));

    KAction *const snap_create = lvs->addAction("snapcreate", this, SLOT(createSnapshot()));
    snap_create->setText(i18n("Create snapshot..."));
    snap_create->setIcon(KIcon("camera_add"));

    KAction *const thin_snap = lvs->addAction("thinsnap", this, SLOT(thinSnapshot()));
    thin_snap->setText(i18n("Create thin snapshot..."));
    thin_snap->setIcon(KIcon("camera_add"));

    KAction *const snap_merge = lvs->addAction("snapmerge", this, SLOT(mergeSnapshot()));
    snap_merge->setText(i18n("Merge snapshot..."));
    snap_merge->setIcon(KIcon("arrow_join"));

    KAction *const lv_reduce = lvs->addAction("lvreduce", this, SLOT(reduceLv()));
    lv_reduce->setText(i18n("Reduce logical volume..."));
    lv_reduce->setIcon(KIcon("delete"));

    KAction *const lv_extend = lvs->addAction("lvextend", this, SLOT(extendLv()));
    lv_extend->setText(i18n("Extend logical volume..."));
    lv_extend->setIcon(KIcon("add"));

    KAction *const pv_move = lvs->addAction("pvmove", this, SLOT(movePhysicalExtents()));
    pv_move->setText(i18n("Move physical extents..."));
    pv_move->setIcon(KIcon("lorry"));

    KAction *const lv_change = lvs->addAction("lvchange", this, SLOT(changeLv()));
    lv_change->setText(i18n("Change attributes or tags..."));
    lv_change->setIcon(KIcon("wrench"));
    addSeparator();

    KAction *const mkfs      = new KAction(KIcon("lightning_add"), i18n("Make or remove filesystem..."), this);
    KAction *const max_fs    = new KAction(KIcon("resultset_last"), i18n("Extend filesystem to fill volume..."), this);
    KAction *const fsck      = new KAction(i18n("Run 'fsck -fp' on filesystem..."), this);
    KAction *const mount     = new KAction(KIcon("emblem-mounted"), i18n("Mount filesystem..."), this);
    KAction *const unmount   = new KAction(KIcon("emblem-unmounted"), i18n("Unmount filesystem..."), this);

    connect(mkfs, SIGNAL(triggered()),
            this, SLOT(makeFs()));

    connect(fsck, SIGNAL(triggered()),
            this, SLOT(checkFs()));

    connect(max_fs, SIGNAL(triggered()),
            this, SLOT(maxFs()));

    connect(mount, SIGNAL(triggered()),
            this, SLOT(mountFs()));

    connect(unmount, SIGNAL(triggered()),
            this, SLOT(unmountFs()));

    addMenu(buildMirrorMenu());

    KMenu *const fs_menu = new KMenu(i18n("Filesystem operations"), this);
    addMenu(fs_menu);
    fs_menu->addAction(mount);
    fs_menu->addAction(unmount);
    fs_menu->addSeparator();
    fs_menu->addAction(max_fs);
    fs_menu->addAction(fsck);
    fs_menu->addSeparator();
    fs_menu->addAction(mkfs);
    fs_menu->setEnabled(false);
    thin_create->setEnabled(false);
    thin_snap->setEnabled(false);

    if (m_lv) {  // snap containers are replaced by the "real" lv before getting here, see: vg->getLvByName()

        if (!m_lv->isThinVolume() && m_lv->isCowSnap() && m_lv->isValid() && !m_lv->isMerging())
            snap_merge->setEnabled(true);
        else
            snap_merge->setEnabled(false);

        if(m_lv->isThinPool()){
            thin_pool->setText(i18n("Create thin pool..."));
            lv_extend->setText(i18n("Extend thin pool..."));
            lv_reduce->setText(i18n("Reduce thin pool..."));
            lv_rename->setText(i18n("Rename thin pool..."));
            lv_remove->setText(i18n("Remove thin pool..."));

            lv_create->setEnabled(false);
            thin_create->setEnabled(true);
            thin_pool->setEnabled(false);
            snap_merge->setEnabled(false);
            max_fs->setEnabled(false);
            mkfs->setEnabled(false);
            lv_remove->setEnabled(true);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            lv_change->setEnabled(true);
            lv_extend->setEnabled(true);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(true);
            pv_move->setEnabled(true);
            snap_create->setEnabled(false);
            fs_menu->setEnabled(false);
        } else if(m_lv->isMetadata() || m_lv->isThinPoolData()){
            snap_merge->setEnabled(false);
            max_fs->setEnabled(false);
            mkfs->setEnabled(false);
            lv_remove->setEnabled(false);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            lv_change->setEnabled(false);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(false);
            pv_move->setEnabled(true);
            snap_create->setEnabled(false);
            fs_menu->setEnabled(false);
        } else if(m_lv->isLvmMirrorLog() || m_lv->isTemporary()){
            snap_merge->setEnabled(false);
            max_fs->setEnabled(false);
            mkfs->setEnabled(false);

            if (m_lv->getParentMirror() == NULL)   // allow removal of bogus log
                lv_remove->setEnabled(true);
            else
                lv_remove->setEnabled(false);

            unmount->setEnabled(false);
            mount->setEnabled(false);
            lv_change->setEnabled(false);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(false);
            pv_move->setEnabled(false);
            snap_create->setEnabled(false);
            fs_menu->setEnabled(false);
        } else if (m_lv->isWritable()  && !m_lv->isLocked() && !m_lv->isVirtual() && !m_lv->isThinVolume() &&
                   !m_lv->isMirrorLeg() && !m_lv->isLvmMirrorLog() && !m_lv->isRaidImage() && !m_lv->isTemporary()) {

            if (m_lv->isMounted()) {
                fsck->setEnabled(false);
                mkfs->setEnabled(false);
                lv_reduce->setEnabled(false);
                lv_extend->setEnabled(true);
                lv_remove->setEnabled(false);
                unmount->setEnabled(true);
                mount->setEnabled(true);
            } else if (m_lv->isOpen() && m_lv->getFilesystem() == "swap") {
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

            if (m_lv->isCowOrigin()) {

                if (m_lv->isRaid())
                    snap_create->setEnabled(false);
                else
                    snap_create->setEnabled(true);

                if (m_lv->isMerging())
                    snap_create->setEnabled(false);
                else
                    lv_extend->setEnabled(true);
               
                lv_reduce->setEnabled(false);
                pv_move->setEnabled(false);

            } else if (m_lv->isCowSnap()) {
                max_fs->setEnabled(false);
                snap_create->setEnabled(false);
                pv_move->setEnabled(false);

                if (m_lv->isMerging() || !m_lv->isValid()) {
                    lv_extend->setEnabled(false);
                    lv_reduce->setEnabled(false);
                    mount->setEnabled(false);
                    fsck->setEnabled(false);
                    mkfs->setEnabled(false);

                    if (m_lv->isMounted())
                        fs_menu->setEnabled(true);

                    if (!m_lv->isValid())
                        lv_remove->setEnabled(true);
                    else
                        lv_remove->setEnabled(false);
                } else if (m_lv->isMounted()) {
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
            } else if (m_lv->isMirror()) {
                pv_move->setEnabled(false);

                if (m_lv->isUnderConversion()) {
                    lv_extend->setEnabled(false);
                    lv_reduce->setEnabled(false);
                } else {
                    lv_extend->setEnabled(true);
                    lv_reduce->setEnabled(true);
                }

            } else if (m_lv->isRaid()) {
                lv_change->setEnabled(false);
                lv_extend->setEnabled(true);
                lv_reduce->setEnabled(false);
                lv_rename->setEnabled(true);
                pv_move->setEnabled(false);
                snap_create->setEnabled(false);
            } else {
                snap_create->setEnabled(true);
            }

            if (m_lv->isCowSnap() && m_lv->isMerging()) {
                lv_rename->setEnabled(false);
                lv_change->setEnabled(false);
            } else {
                lv_rename->setEnabled(true);
                lv_change->setEnabled(true);
                fs_menu->setEnabled(true);
            }
        } else if (m_lv->isThinVolume()){

            thin_snap->setEnabled(true);
            fs_menu->setEnabled(true);
            pv_move->setEnabled(false);
            lv_reduce->setEnabled(true);

            if (m_lv->isMounted()) {
                fsck->setEnabled(false);
                mkfs->setEnabled(false);
                lv_reduce->setEnabled(false);
                lv_extend->setEnabled(true);
                lv_remove->setEnabled(false);
                unmount->setEnabled(true);
                mount->setEnabled(true);
            } else if (m_lv->isOpen() && m_lv->getFilesystem() == "swap") {
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

            if (!m_lv->isWritable()) {
                fsck->setEnabled(false);
                mkfs->setEnabled(false);
                lv_reduce->setEnabled(false);
                lv_extend->setEnabled(false);
                max_fs->setEnabled(false);
            } else if (m_lv->isCowOrigin())
                lv_reduce->setEnabled(false);

        } else if (m_lv->isOrphan()) {
            mkfs->setEnabled(false);
            max_fs->setEnabled(false);
            lv_remove->setEnabled(true);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            lv_change->setEnabled(false);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(false);
            pv_move->setEnabled(false);
            snap_create->setEnabled(false);
            fs_menu->setEnabled(false);
        } else if (m_lv->isRaidImage()) {
            mkfs->setEnabled(false);
            max_fs->setEnabled(false);
            lv_remove->setEnabled(false);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            lv_change->setEnabled(false);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(false);
            pv_move->setEnabled(true);
            snap_create->setEnabled(false);
            fs_menu->setEnabled(false);
        } else if (m_lv->isPvmove()) {
            mkfs->setEnabled(false);
            max_fs->setEnabled(false);
            lv_remove->setEnabled(false);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            lv_change->setEnabled(false);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            lv_rename->setEnabled(false);
            pv_move->setEnabled(false);
            snap_create->setEnabled(false);
            fs_menu->setEnabled(false);
        } else if (m_lv->isLvmMirrorLeg() && !m_lv->isLvmMirrorLog() && !m_lv->isTemporary()) {
            mkfs->setEnabled(false);
            max_fs->setEnabled(false);
            lv_remove->setEnabled(false);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            lv_change->setEnabled(false);
            lv_extend->setEnabled(false);
            lv_reduce->setEnabled(false);
            pv_move->setEnabled(false);
            lv_rename->setEnabled(false);
            snap_create->setEnabled(false);
            fs_menu->setEnabled(false);
        } else if (!(m_lv->isWritable()) && m_lv->isLocked()) {

            if (m_lv->isMounted())
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
            fs_menu->setEnabled(true);
        } else if (m_lv->isWritable() && m_lv->isLocked()) {

            if (m_lv->isMounted())
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
            fs_menu->setEnabled(true);
        } else {
            if (m_lv->isMounted()) {
                lv_remove->setEnabled(false);
                unmount->setEnabled(true);
            } else {
                lv_remove->setEnabled(true);
                unmount->setEnabled(false);
            }

            if (m_lv->isCowSnap() || m_lv->isCowOrigin()) {
                pv_move->setEnabled(false);

                if (m_lv->isCowSnap())
                    snap_create->setEnabled(false);
                else
                    snap_create->setEnabled(true);

            } else if (m_lv->isMirror()) {
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

            if (m_lv->isVirtual()) {
                lv_rename->setEnabled(false);
                lv_remove->setEnabled(false);
                mount->setEnabled(false);
                lv_change->setEnabled(false);
                fs_menu->setEnabled(false);
            } else {
                fs_menu->setEnabled(true);
                mount->setEnabled(true);
                lv_change->setEnabled(true);
            }
        }

        if (!m_lv->isActive()) {
            mkfs->setEnabled(false);
            unmount->setEnabled(false);
            mount->setEnabled(false);
            fs_menu->setEnabled(false);
            snap_create->setEnabled(false);
        }
    } else {
        snap_merge->setEnabled(false);
        max_fs->setEnabled(false);
        mkfs->setEnabled(false);
        lv_remove->setEnabled(false);
        unmount->setEnabled(false);
        mount->setEnabled(false);
        lv_change->setEnabled(false);
        lv_extend->setEnabled(false);
        lv_reduce->setEnabled(false);
        lv_rename->setEnabled(false);
        pv_move->setEnabled(false);
        snap_create->setEnabled(false);
        fs_menu->setEnabled(false);
    }
}

KMenu* LVActionsMenu::buildMirrorMenu()
{
    KAction *const add_legs       = new KAction(i18n("Add mirror legs to volume..."), this);
    KAction *const change_log     = new KAction(i18n("Change mirror log..."), this);
    KAction *const remove_mirror  = new KAction(i18n("Remove mirror legs..."), this);
    KAction *const remove_this    = new KAction(i18n("Remove this mirror leg..."), this);
    KAction *const repair_missing = new KAction(i18n("Repair volume..."), this);

    KMenu *const menu = new KMenu(i18n("Mirrors and RAID"), this);
    menu->setIcon(KIcon("document-multiple"));
    menu->addAction(add_legs);
    menu->addAction(change_log);
    menu->addAction(remove_mirror);
    menu->addAction(remove_this);
    menu->addSeparator();
    menu->addAction(repair_missing);

    bool partial = false;

    if (m_lv) {

        if ((m_lv->isRaidImage() || m_lv->isRaidMetadata()) && (m_lv->getParent()))
            partial = m_lv->getParent()->isPartial();
        else if ((m_lv->isLvmMirrorLog() || m_lv->isLvmMirrorLeg() || m_lv->isTemporary()) && (m_lv->getParentMirror()))
            partial = m_lv->getParentMirror()->isPartial();
        else
            partial = m_lv->isPartial();

        if (m_lv->isThinPool()) {
            add_legs->setEnabled(false);
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            remove_this->setEnabled(false);
            repair_missing->setEnabled(false);
        } else if (partial && (m_lv->isRaidImage() || m_lv->isRaid() || m_lv->isMirror() || m_lv->isRaidMetadata() ||
                               m_lv->isMirrorLeg() || m_lv->isLvmMirrorLog() || m_lv->isTemporary())) {

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
        } else if(m_lv->isThinMetadata() || m_lv->isThinPoolData()){
            add_legs->setEnabled(false);
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            remove_this->setEnabled(false);
            repair_missing->setEnabled(false);
        } else if(m_lv->isLvmMirrorLog() || m_lv->isTemporary()){
            remove_mirror->setEnabled(false);
            change_log->setEnabled(true);
            remove_this->setEnabled(false);
            repair_missing->setEnabled(false);

            if (m_lv->getParentMirror() != NULL) {
                if (m_lv->getParentMirror()->isUnderConversion() || m_lv->getParentMirror()->isCowOrigin())
                    add_legs->setEnabled(false);
                else
                    add_legs->setEnabled(true);
            } else {
                add_legs->setEnabled(false);
            }
        } else if (m_lv->isRaidImage()) {
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            repair_missing->setEnabled(false);
            
            if(m_lv->isMirrorLeg()) {
                remove_this->setEnabled(true);
                add_legs->setEnabled(true);
            } else {
                remove_this->setEnabled(false);
                add_legs->setEnabled(false);
            }
        } else if (m_lv->isRaidMetadata()) {
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            repair_missing->setEnabled(false);
            remove_this->setEnabled(false);

            if (m_lv->getParent() != NULL) {
                if (m_lv->getParent()->isMirror())
                    add_legs->setEnabled(true);
                else
                    add_legs->setEnabled(false);
            } else {
                add_legs->setEnabled(false);
            }
        } else if (!m_lv->isLocked() && !m_lv->isVirtual() && !m_lv->isThinVolume() &&
                   !m_lv->isMirrorLeg() && !m_lv->isLvmMirrorLog() && !m_lv->isRaidImage() && !m_lv->isTemporary()) {

            remove_this->setEnabled(false);
            repair_missing->setEnabled(false);

            if (m_lv->isCowOrigin()) {
                if (m_lv->isMirror()) {
                    if (m_lv->isRaid()) {
                        change_log->setEnabled(false);
                        add_legs->setEnabled(true);
                    } else {
                        change_log->setEnabled(true);
                        add_legs->setEnabled(false);
                    }
                    
                    remove_mirror->setEnabled(true);
                } else if (m_lv->isRaid()) {
                    add_legs->setEnabled(false);
                    change_log->setEnabled(false);
                    remove_mirror->setEnabled(false);
                } else {
                    add_legs->setEnabled(true);
                    change_log->setEnabled(false);
                    remove_mirror->setEnabled(false);
                }
                
                if (m_lv->isMerging()) {
                    add_legs->setEnabled(false);
                }
            } else if (m_lv->isCowSnap()) {
                add_legs->setEnabled(false);
                remove_mirror->setEnabled(false);
                change_log->setEnabled(false);
            } else if (m_lv->isMirror()) {
                remove_mirror->setEnabled(true);
                        
                if (m_lv->isRaid())
                    change_log->setEnabled(false);
                else
                    change_log->setEnabled(true);
                
                if (m_lv->isUnderConversion())
                    add_legs->setEnabled(false);
                else
                    add_legs->setEnabled(true);

            } else if (m_lv->isRaid()) {
                add_legs->setEnabled(false);
                change_log->setEnabled(false);
                remove_mirror->setEnabled(false);
            } else {
                add_legs->setEnabled(true);
                remove_mirror->setEnabled(false);
                change_log->setEnabled(false);
            }
        } else if (m_lv->isVirtual() ||m_lv->isThinVolume() || m_lv->isOrphan() || m_lv->isPvmove()) {
            add_legs->setEnabled(false);
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            remove_this->setEnabled(false);
            repair_missing->setEnabled(false);
        } else if (m_lv->isLvmMirrorLeg() && !m_lv->isLvmMirrorLog() && !m_lv->isTemporary()) {
            remove_mirror->setEnabled(false);
            change_log->setEnabled(true);
            remove_this->setEnabled(true);
            repair_missing->setEnabled(false);
            
            if (m_lv->getParentMirror() != NULL) {
                if (m_lv->getParentMirror()->isUnderConversion() || m_lv->getParentMirror()->isCowOrigin())
                    add_legs->setEnabled(false);
                else
                    add_legs->setEnabled(true);
            } else {
                add_legs->setEnabled(false);
            }
        } else if (m_lv->isLocked()) {
            remove_mirror->setEnabled(false);
            change_log->setEnabled(false);
            remove_this->setEnabled(false);
            add_legs->setEnabled(false);
            repair_missing->setEnabled(false);
        }

        if (!m_lv->isActive()) {
            change_log->setEnabled(false);
        }
    } else {
        add_legs->setEnabled(false);
        remove_mirror->setEnabled(false);
        change_log->setEnabled(false);
        remove_this->setEnabled(false);
        repair_missing->setEnabled(false);
    }

    connect(add_legs, SIGNAL(triggered()),
            this, SLOT(addLegs()));

    connect(change_log, SIGNAL(triggered()),
            this, SLOT(changeLog()));

    connect(remove_mirror, SIGNAL(triggered()),
            this, SLOT(removeMirror()));

    connect(remove_this, SIGNAL(triggered()),
            this, SLOT(removeLeg()));

    connect(repair_missing, SIGNAL(triggered()),
            this, SLOT(repairMissing()));

    return menu;
}

void LVActionsMenu::createLv()
{
    LVCreateDialog dialog(m_vg, false);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}

void LVActionsMenu::createThinVolume()
{
    ThinCreateDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}

void LVActionsMenu::createThinPool()
{
    LVCreateDialog dialog(m_vg, true);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}

void LVActionsMenu::reduceLv()
{
    LVReduceDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}

void LVActionsMenu::extendLv()
{
    if (m_lv->isThinVolume()) {
        ThinCreateDialog dialog(m_lv, false);
        
        if (!dialog.bailout()) {
            dialog.exec();
            if (dialog.result() == QDialog::Accepted)
                MainWindow->reRun();
        }
    } else {
        LVCreateDialog dialog(m_lv, false);
        
        if (!dialog.bailout()) {
            dialog.exec();
            if (dialog.result() == QDialog::Accepted)
                MainWindow->reRun();
        }
    }
}

void LVActionsMenu::addLegs()
{
    ChangeMirrorDialog dialog(m_lv, false);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}

void LVActionsMenu::changeLog()
{
    ChangeMirrorDialog dialog(m_lv, true);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}

void LVActionsMenu::removeMirror()
{
    if (remove_mirror(m_lv))
        MainWindow->reRun();
}

void LVActionsMenu::removeLeg()
{
    if (remove_mirror_leg(m_lv))
        MainWindow->reRun();
}

void LVActionsMenu::makeFs()
{
    MkfsDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}

void LVActionsMenu::checkFs()
{
    if (manual_fsck(m_lv))
        MainWindow->reRun();
}

void LVActionsMenu::maxFs()
{
    if (max_fs(m_lv))
        MainWindow->reRun();
}

void LVActionsMenu::mergeSnapshot()
{
    if (merge_snap(m_lv))
        MainWindow->reRun();
}

void LVActionsMenu::removeLv()
{
    LVRemoveDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == KDialog::Yes)
            MainWindow->reRun();
    }
}

void LVActionsMenu::repairMissing()
{
    RepairMissingDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}

void LVActionsMenu::renameLv()
{
    LVRenameDialog dialog(m_lv);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
        MainWindow->reRun();
}

void LVActionsMenu::createSnapshot()
{
    LVCreateDialog dialog(m_lv, true);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}


void LVActionsMenu::thinSnapshot()
{
    ThinCreateDialog dialog(m_lv, true);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}

void LVActionsMenu::changeLv()
{
    LVChangeDialog dialog(m_lv);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
        MainWindow->reRun();
}

void LVActionsMenu::mountFs()
{
    MountDialog dialog(m_lv);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
        MainWindow->reRun();
}

void LVActionsMenu::unmountFs()
{
    UnmountDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted || dialog.result() == KDialog::Yes)
            MainWindow->reRun();
    }
}

void LVActionsMenu::movePhysicalExtents()
{
    PVMoveDialog dialog(m_lv, m_segment);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}
