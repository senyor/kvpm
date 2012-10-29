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
#include "removefs.h"
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

    KActionCollection *lv_actions = new KActionCollection(this);
    lv_actions->addAssociatedWidget(this);
    lv_create_action = lv_actions->addAction("lvcreate", this, SLOT(createLogicalVolume()));
    lv_create_action->setText(i18n("Create logical volume..."));
    lv_create_action->setIcon(KIcon("document-new"));
    thin_create_action = lv_actions->addAction("thincreate", this, SLOT(createThinVolume()));
    thin_create_action->setText(i18n("Create thin volume..."));
    thin_create_action->setIcon(KIcon("document-new"));
    thin_pool_action = lv_actions->addAction("thinpool", this, SLOT(createThinPool()));
    thin_pool_action->setText(i18n("Create thin pool..."));
    thin_pool_action->setIcon(KIcon("document-new"));
    lv_remove_action = lv_actions->addAction("lvremove", this, SLOT(removeLogicalVolume()));
    lv_remove_action->setText(i18n("Remove logical volume..."));
    lv_remove_action->setIcon(KIcon("cross"));
    addSeparator();
    lv_rename_action = lv_actions->addAction("lvrename", this, SLOT(renameLogicalVolume()));
    lv_rename_action->setText(i18n("Rename logical volume..."));
    lv_rename_action->setIcon(KIcon("edit-rename"));

    snap_create_action = lv_actions->addAction("snapcreate", this, SLOT(createSnapshot()));
    snap_create_action->setText(i18n("Create snapshot..."));
    snap_create_action->setIcon(KIcon("camera_add"));

    thin_snap_action = lv_actions->addAction("thinsnap", this, SLOT(thinSnapshot()));
    thin_snap_action->setText(i18n("Create thin snapshot..."));
    thin_snap_action->setIcon(KIcon("camera_add"));

    snap_merge_action = lv_actions->addAction("snapmerge", this, SLOT(mergeSnapshot()));
    snap_merge_action->setText(i18n("Merge snapshot..."));
    snap_merge_action->setIcon(KIcon("arrow_join"));
    lv_reduce_action = lv_actions->addAction("lvreduce", this, SLOT(reduceLogicalVolume()));
    lv_reduce_action->setText(i18n("Reduce logical volume..."));
    lv_reduce_action->setIcon(KIcon("delete"));
    lv_extend_action = lv_actions->addAction("lvextend", this, SLOT(extendLogicalVolume()));
    lv_extend_action->setText(i18n("Extend logical volume..."));
    lv_extend_action->setIcon(KIcon("add"));
    pv_move_action = lv_actions->addAction("pvmove", this, SLOT(movePhysicalExtents()));
    pv_move_action->setText(i18n("Move physical extents..."));
    pv_move_action->setIcon(KIcon("lorry"));
    lv_change_action = lv_actions->addAction("lvchange", this, SLOT(changeLogicalVolume()));
    lv_change_action->setText(i18n("Change attributes or tags..."));
    lv_change_action->setIcon(KIcon("wrench"));
    addSeparator();

    lv_mkfs_action     = new KAction(KIcon("lightning_add"), i18n("Make filesystem..."), this);
    lv_removefs_action = new KAction(KIcon("lightning_delete"), i18n("Remove filesystem..."), this);
    lv_maxfs_action    = new KAction(KIcon("resultset_last"), i18n("Extend filesystem to fill volume..."), this);
    lv_fsck_action     = new KAction(i18n("Run 'fsck -fp' on filesystem..."), this);
    mount_filesystem_action   = new KAction(KIcon("emblem-mounted"), i18n("Mount filesystem..."), this);
    unmount_filesystem_action = new KAction(KIcon("emblem-unmounted"), i18n("Unmount filesystem..."), this);
    add_mirror_legs_action    = new KAction(i18n("Add mirror legs to volume..."), this);
    change_mirror_log_action  = new KAction(i18n("Change mirror log..."), this);
    remove_mirror_action      = new KAction(i18n("Remove mirror leg..."), this);
    remove_mirror_leg_action  = new KAction(i18n("Remove this mirror leg..."), this);

    connect(lv_mkfs_action, SIGNAL(triggered()),
            this, SLOT(mkfsLogicalVolume()));

    connect(lv_fsck_action, SIGNAL(triggered()),
            this, SLOT(fsckLogicalVolume()));

    connect(lv_removefs_action, SIGNAL(triggered()),
            this, SLOT(removefsLogicalVolume()));

    connect(lv_maxfs_action, SIGNAL(triggered()),
            this, SLOT(maxfsLogicalVolume()));

    connect(add_mirror_legs_action, SIGNAL(triggered()),
            this, SLOT(addMirrorLegs()));

    connect(change_mirror_log_action, SIGNAL(triggered()),
            this, SLOT(changeMirrorLog()));

    connect(mount_filesystem_action, SIGNAL(triggered()),
            this, SLOT(mountFilesystem()));

    connect(unmount_filesystem_action, SIGNAL(triggered()),
            this, SLOT(unmountFilesystem()));

    connect(remove_mirror_action, SIGNAL(triggered()),
            this, SLOT(removeMirror()));

    connect(remove_mirror_leg_action, SIGNAL(triggered()),
            this, SLOT(removeMirrorLeg()));

    KMenu *mirror_menu = new KMenu(i18n("Mirror operations"), this);
    mirror_menu->setIcon(KIcon("document-multiple"));
    addMenu(mirror_menu);
    mirror_menu->addAction(add_mirror_legs_action);
    mirror_menu->addAction(change_mirror_log_action);
    mirror_menu->addAction(remove_mirror_action);
    mirror_menu->addAction(remove_mirror_leg_action);

    filesystem_menu = new KMenu(i18n("Filesystem operations"), this);
    addMenu(filesystem_menu);
    filesystem_menu->addAction(mount_filesystem_action);
    filesystem_menu->addAction(unmount_filesystem_action);
    filesystem_menu->addSeparator();
    filesystem_menu->addAction(lv_maxfs_action);
    filesystem_menu->addAction(lv_fsck_action);
    filesystem_menu->addSeparator();
    filesystem_menu->addAction(lv_mkfs_action);
    filesystem_menu->addAction(lv_removefs_action);
    filesystem_menu->setEnabled(false);
    thin_create_action->setEnabled(false);
    thin_snap_action->setEnabled(false);

    if (m_lv) {  // snap containers are replaced by the "real" lv before getting here, see: vg->getLvByName()

        if (!m_lv->isThinVolume() && m_lv->isCowSnap() && m_lv->isValid() && !m_lv->isMerging())
            snap_merge_action->setEnabled(true);
        else
            snap_merge_action->setEnabled(false);

        if(m_lv->isThinPool()){
            thin_pool_action->setText(i18n("Create thin pool..."));
            lv_extend_action->setText(i18n("Extend thin pool..."));
            lv_reduce_action->setText(i18n("Reduce thin pool..."));
            lv_rename_action->setText(i18n("Rename thin pool..."));
            lv_remove_action->setText(i18n("Remove thin pool..."));

            lv_create_action->setEnabled(false);
            thin_create_action->setEnabled(true);
            thin_pool_action->setEnabled(false);
            snap_merge_action->setEnabled(false);
            lv_maxfs_action->setEnabled(false);
            lv_mkfs_action->setEnabled(false);
            lv_removefs_action->setEnabled(false);
            lv_remove_action->setEnabled(true);
            unmount_filesystem_action->setEnabled(false);
            mount_filesystem_action->setEnabled(false);
            add_mirror_legs_action->setEnabled(false);
            lv_change_action->setEnabled(true);
            lv_extend_action->setEnabled(true);
            lv_reduce_action->setEnabled(false);
            lv_rename_action->setEnabled(true);
            pv_move_action->setEnabled(true);
            remove_mirror_action->setEnabled(false);
            change_mirror_log_action->setEnabled(false);
            remove_mirror_leg_action->setEnabled(false);
            snap_create_action->setEnabled(false);
            filesystem_menu->setEnabled(false);
        } else if(m_lv->isMetadata() || m_lv->isThinPoolData()){
            snap_merge_action->setEnabled(false);
            lv_maxfs_action->setEnabled(false);
            lv_mkfs_action->setEnabled(false);
            lv_removefs_action->setEnabled(false);
            lv_remove_action->setEnabled(false);
            unmount_filesystem_action->setEnabled(false);
            mount_filesystem_action->setEnabled(false);
            add_mirror_legs_action->setEnabled(false);
            lv_change_action->setEnabled(false);
            lv_extend_action->setEnabled(false);
            lv_reduce_action->setEnabled(false);
            lv_rename_action->setEnabled(false);
            pv_move_action->setEnabled(true);
            remove_mirror_action->setEnabled(false);
            change_mirror_log_action->setEnabled(false);
            remove_mirror_leg_action->setEnabled(false);
            snap_create_action->setEnabled(false);
            filesystem_menu->setEnabled(false);

        } else if (m_lv->isWritable()  && !m_lv->isLocked() && !m_lv->isVirtual() &&
            !m_lv->isMirrorLeg() && !m_lv->isLvmMirrorLog() && !m_lv->isRaidImage()) {

            if (m_lv->isMounted()) {
                lv_fsck_action->setEnabled(false);
                lv_mkfs_action->setEnabled(false);
                lv_removefs_action->setEnabled(false);
                lv_reduce_action->setEnabled(false);
                lv_extend_action->setEnabled(true);
                lv_remove_action->setEnabled(false);
                unmount_filesystem_action->setEnabled(true);
                mount_filesystem_action->setEnabled(true);
            } else if (m_lv->isOpen() && m_lv->getFilesystem() == "swap") {
                lv_fsck_action->setEnabled(false);
                lv_mkfs_action->setEnabled(false);
                lv_removefs_action->setEnabled(false);
                lv_reduce_action->setEnabled(false);
                lv_extend_action->setEnabled(false);
                lv_remove_action->setEnabled(false);
                lv_maxfs_action->setEnabled(false);
                unmount_filesystem_action->setEnabled(false);
                mount_filesystem_action->setEnabled(false);
            } else {
                lv_mkfs_action->setEnabled(true);
                lv_removefs_action->setEnabled(true);
                lv_reduce_action->setEnabled(true);
                lv_extend_action->setEnabled(true);
                lv_remove_action->setEnabled(true);
                unmount_filesystem_action->setEnabled(false);
                mount_filesystem_action->setEnabled(true);
            }

            if (m_lv->isCowOrigin()) {
                snap_create_action->setEnabled(true);

                if (m_lv->isThinVolume())
                    thin_snap_action->setEnabled(true);

                if (m_lv->isMirror()) {
                    add_mirror_legs_action->setEnabled(false);

                    if (m_lv->isRaid()) 
                        change_mirror_log_action->setEnabled(false);
                    else
                        change_mirror_log_action->setEnabled(true);

                    remove_mirror_action->setEnabled(true);
                } else if (m_lv->isRaid()) {
                    add_mirror_legs_action->setEnabled(false);
                    change_mirror_log_action->setEnabled(false);
                    remove_mirror_action->setEnabled(false);
                    snap_create_action->setEnabled(false);
                } else {
                    add_mirror_legs_action->setEnabled(true);
                    change_mirror_log_action->setEnabled(false);
                    remove_mirror_action->setEnabled(false);
                }

                if (m_lv->isMerging()) {
                    snap_create_action->setEnabled(false);
                    add_mirror_legs_action->setEnabled(false);
                } else {
                    lv_extend_action->setEnabled(true);
                }

                lv_reduce_action->setEnabled(false);
                pv_move_action->setEnabled(false);

            } else if (m_lv->isCowSnap()) {
                add_mirror_legs_action->setEnabled(false);
                remove_mirror_action->setEnabled(false);
                change_mirror_log_action->setEnabled(false);
                mirror_menu->setEnabled(false);
                lv_maxfs_action->setEnabled(false);
                snap_create_action->setEnabled(false);
                pv_move_action->setEnabled(false);

                if (m_lv->isMerging() || !m_lv->isValid()) {
                    lv_extend_action->setEnabled(false);
                    lv_reduce_action->setEnabled(false);
                    mount_filesystem_action->setEnabled(false);
                    lv_fsck_action->setEnabled(false);
                    lv_mkfs_action->setEnabled(false);
                    lv_removefs_action->setEnabled(false);

                    if (m_lv->isMounted())
                        filesystem_menu->setEnabled(true);

                    if (!m_lv->isValid())
                        lv_remove_action->setEnabled(true);
                    else
                        lv_remove_action->setEnabled(false);
                } else if (m_lv->isMounted()) {
                    lv_extend_action->setEnabled(true);
                    lv_reduce_action->setEnabled(false);
                    lv_fsck_action->setEnabled(false);
                    lv_mkfs_action->setEnabled(false);
                    lv_removefs_action->setEnabled(false);
                } else {
                    lv_extend_action->setEnabled(true);
                    lv_reduce_action->setEnabled(true);
                    lv_fsck_action->setEnabled(true);
                    lv_mkfs_action->setEnabled(true);
                    lv_removefs_action->setEnabled(true);
                }
            } else if (m_lv->isMirror()) {
                remove_mirror_action->setEnabled(true);
                pv_move_action->setEnabled(false);

                if (m_lv->isRaid()){
                    snap_create_action->setEnabled(true);
                    change_mirror_log_action->setEnabled(false);
                } else {
                    change_mirror_log_action->setEnabled(true);
                }

                if (m_lv->isUnderConversion()) {
                    add_mirror_legs_action->setEnabled(false);
                    lv_extend_action->setEnabled(false);
                    lv_reduce_action->setEnabled(false);
                } else {
                    add_mirror_legs_action->setEnabled(true);
                    lv_extend_action->setEnabled(true);
                    lv_reduce_action->setEnabled(true);
                }

            } else if (m_lv->isRaid()) {
                add_mirror_legs_action->setEnabled(false);
                lv_change_action->setEnabled(false);
                lv_extend_action->setEnabled(true);
                lv_reduce_action->setEnabled(false);
                lv_rename_action->setEnabled(true);
                pv_move_action->setEnabled(false);
                remove_mirror_action->setEnabled(false);
                change_mirror_log_action->setEnabled(false);
                remove_mirror_leg_action->setEnabled(false);
                snap_create_action->setEnabled(false);
            } else {
                if (m_lv->isThinVolume()) {
                    add_mirror_legs_action->setEnabled(false);
                    mirror_menu->setEnabled(false);
                    thin_snap_action->setEnabled(true);
                    pv_move_action->setEnabled(false);
                } else
                    add_mirror_legs_action->setEnabled(true);

                remove_mirror_action->setEnabled(false);
                change_mirror_log_action->setEnabled(false);
                snap_create_action->setEnabled(true);
            }

            remove_mirror_leg_action->setEnabled(false);

            if (m_lv->isCowSnap() && m_lv->isMerging()) {
                lv_rename_action->setEnabled(false);
                lv_change_action->setEnabled(false);
            } else {
                lv_rename_action->setEnabled(true);
                lv_change_action->setEnabled(true);
                filesystem_menu->setEnabled(true);
            }
        } else if (m_lv->isOrphan()) {
            lv_mkfs_action->setEnabled(false);
            lv_removefs_action->setEnabled(false);
            lv_maxfs_action->setEnabled(false);
            lv_remove_action->setEnabled(true);
            unmount_filesystem_action->setEnabled(false);
            mount_filesystem_action->setEnabled(false);
            add_mirror_legs_action->setEnabled(false);
            lv_change_action->setEnabled(false);
            lv_extend_action->setEnabled(false);
            lv_reduce_action->setEnabled(false);
            lv_rename_action->setEnabled(false);
            pv_move_action->setEnabled(false);
            remove_mirror_action->setEnabled(false);
            change_mirror_log_action->setEnabled(false);
            remove_mirror_leg_action->setEnabled(false);
            snap_create_action->setEnabled(false);
            filesystem_menu->setEnabled(false);
        } else if (m_lv->isRaidImage()) {
            lv_mkfs_action->setEnabled(false);
            lv_removefs_action->setEnabled(false);
            lv_maxfs_action->setEnabled(false);
            lv_remove_action->setEnabled(false);
            unmount_filesystem_action->setEnabled(false);
            mount_filesystem_action->setEnabled(false);
            add_mirror_legs_action->setEnabled(false);
            lv_change_action->setEnabled(false);
            lv_extend_action->setEnabled(false);
            lv_reduce_action->setEnabled(false);
            lv_rename_action->setEnabled(false);
            pv_move_action->setEnabled(true);
            remove_mirror_action->setEnabled(false);
            change_mirror_log_action->setEnabled(false);

            if(m_lv->isMirrorLeg())
                remove_mirror_leg_action->setEnabled(true);
            else
                remove_mirror_leg_action->setEnabled(false);

            snap_create_action->setEnabled(false);
            filesystem_menu->setEnabled(false);
        } else if (m_lv->isPvmove()) {
            lv_mkfs_action->setEnabled(false);
            lv_removefs_action->setEnabled(false);
            lv_maxfs_action->setEnabled(false);
            lv_remove_action->setEnabled(false);
            unmount_filesystem_action->setEnabled(false);
            mount_filesystem_action->setEnabled(false);
            add_mirror_legs_action->setEnabled(false);
            lv_change_action->setEnabled(false);
            lv_extend_action->setEnabled(false);
            lv_reduce_action->setEnabled(false);
            lv_rename_action->setEnabled(false);
            pv_move_action->setEnabled(false);
            remove_mirror_action->setEnabled(false);
            change_mirror_log_action->setEnabled(false);
            remove_mirror_leg_action->setEnabled(false);
            snap_create_action->setEnabled(false);
            filesystem_menu->setEnabled(false);
        } else if (m_lv->isMirrorLeg() || m_lv->isLvmMirrorLog()) {
            lv_mkfs_action->setEnabled(false);
            lv_removefs_action->setEnabled(false);
            lv_maxfs_action->setEnabled(false);
            lv_remove_action->setEnabled(false);
            unmount_filesystem_action->setEnabled(false);
            mount_filesystem_action->setEnabled(false);
            add_mirror_legs_action->setEnabled(false);
            lv_change_action->setEnabled(false);
            lv_extend_action->setEnabled(false);
            lv_reduce_action->setEnabled(false);
            pv_move_action->setEnabled(false);
            remove_mirror_action->setEnabled(false);
            change_mirror_log_action->setEnabled(false);
            lv_rename_action->setEnabled(false);
            snap_create_action->setEnabled(false);
            filesystem_menu->setEnabled(false);

            if (!m_lv->isLvmMirrorLog())
                remove_mirror_leg_action->setEnabled(true);
            else {
                remove_mirror_leg_action->setEnabled(false);
                mirror_menu->setEnabled(false);
            }
        } else if (!(m_lv->isWritable()) && m_lv->isLocked()) {

            if (m_lv->isMounted())
                unmount_filesystem_action->setEnabled(true);
            else
                unmount_filesystem_action->setEnabled(false);

            mount_filesystem_action->setEnabled(true);
            lv_removefs_action->setEnabled(false);
            lv_mkfs_action->setEnabled(false);
            lv_maxfs_action->setEnabled(false);
            lv_remove_action->setEnabled(false);
            add_mirror_legs_action->setEnabled(false);
            lv_change_action->setEnabled(true);
            lv_extend_action->setEnabled(false);
            lv_reduce_action->setEnabled(false);
            lv_rename_action->setEnabled(false);
            pv_move_action->setEnabled(false);
            remove_mirror_action->setEnabled(false);
            change_mirror_log_action->setEnabled(false);
            remove_mirror_leg_action->setEnabled(false);
            snap_create_action->setEnabled(false);
            filesystem_menu->setEnabled(true);
        } else if (m_lv->isWritable() && m_lv->isLocked()) {

            if (m_lv->isMounted())
                unmount_filesystem_action->setEnabled(true);
            else
                unmount_filesystem_action->setEnabled(false);

            mount_filesystem_action->setEnabled(true);
            lv_removefs_action->setEnabled(true);
            lv_mkfs_action->setEnabled(true);
            lv_remove_action->setEnabled(false);
            lv_rename_action->setEnabled(false);
            add_mirror_legs_action->setEnabled(false);
            lv_change_action->setEnabled(true);
            lv_extend_action->setEnabled(false);
            lv_reduce_action->setEnabled(false);
            pv_move_action->setEnabled(false);
            remove_mirror_action->setEnabled(false);
            change_mirror_log_action->setEnabled(false);
            remove_mirror_leg_action->setEnabled(false);
            snap_create_action->setEnabled(false);
            filesystem_menu->setEnabled(true);
        } else {
            if (m_lv->isMounted()) {
                lv_remove_action->setEnabled(false);
                unmount_filesystem_action->setEnabled(true);
            } else {
                lv_remove_action->setEnabled(true);
                unmount_filesystem_action->setEnabled(false);
            }

            if (m_lv->isCowSnap() || m_lv->isCowOrigin()) {
                add_mirror_legs_action->setEnabled(false);
                remove_mirror_action->setEnabled(false);
                change_mirror_log_action->setEnabled(false);
                pv_move_action->setEnabled(false);

                if (m_lv->isCowSnap())
                    snap_create_action->setEnabled(false);
                else
                    snap_create_action->setEnabled(true);

                mirror_menu->setEnabled(false);
            } else if (m_lv->isMirror()) {
                add_mirror_legs_action->setEnabled(true);
                remove_mirror_action->setEnabled(true);

                if (m_lv->isRaid())
                    change_mirror_log_action->setEnabled(false);
                else
                    change_mirror_log_action->setEnabled(true);

                pv_move_action->setEnabled(false);
                snap_create_action->setEnabled(false);
            } else {
                add_mirror_legs_action->setEnabled(true);
                remove_mirror_action->setEnabled(false);
                change_mirror_log_action->setEnabled(false);
                pv_move_action->setEnabled(true);
                snap_create_action->setEnabled(true);
            }

            lv_removefs_action->setEnabled(false);
            lv_mkfs_action->setEnabled(false);
            lv_fsck_action->setEnabled(false);
            lv_maxfs_action->setEnabled(false);
            lv_reduce_action->setEnabled(false);
            lv_extend_action->setEnabled(false);
            remove_mirror_leg_action->setEnabled(false);

            if (!m_lv->isVirtual()) {
                filesystem_menu->setEnabled(true);
                mount_filesystem_action->setEnabled(true);
                lv_change_action->setEnabled(true);
            } else {
                lv_rename_action->setEnabled(false);
                lv_remove_action->setEnabled(false);
                mount_filesystem_action->setEnabled(false);
                lv_change_action->setEnabled(false);
                filesystem_menu->setEnabled(false);
                mirror_menu->setEnabled(false);
            }
        }

        if (!m_lv->isActive()) {
            lv_removefs_action->setEnabled(false);
            lv_mkfs_action->setEnabled(false);
            unmount_filesystem_action->setEnabled(false);
            mount_filesystem_action->setEnabled(false);
            filesystem_menu->setEnabled(false);
            snap_create_action->setEnabled(false);
        }
    } else {
        snap_merge_action->setEnabled(false);
        lv_maxfs_action->setEnabled(false);
        lv_mkfs_action->setEnabled(false);
        lv_removefs_action->setEnabled(false);
        lv_remove_action->setEnabled(false);
        unmount_filesystem_action->setEnabled(false);
        mount_filesystem_action->setEnabled(false);
        add_mirror_legs_action->setEnabled(false);
        lv_change_action->setEnabled(false);
        lv_extend_action->setEnabled(false);
        lv_reduce_action->setEnabled(false);
        lv_rename_action->setEnabled(false);
        pv_move_action->setEnabled(false);
        remove_mirror_action->setEnabled(false);
        change_mirror_log_action->setEnabled(false);
        remove_mirror_leg_action->setEnabled(false);
        snap_create_action->setEnabled(false);
        filesystem_menu->setEnabled(false);
    }

    if (!add_mirror_legs_action->isEnabled()   && !remove_mirror_action->isEnabled() &&
            !remove_mirror_leg_action->isEnabled() && !change_mirror_log_action->isEnabled())
        mirror_menu->setEnabled(false);
}

void LVActionsMenu::createLogicalVolume()
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

void LVActionsMenu::reduceLogicalVolume()
{
    LVReduceDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}

void LVActionsMenu::extendLogicalVolume()
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

void LVActionsMenu::addMirrorLegs()
{
    ChangeMirrorDialog dialog(m_lv, false);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
        MainWindow->reRun();
}

void LVActionsMenu::changeMirrorLog()
{
    ChangeMirrorDialog dialog(m_lv, true);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
        MainWindow->reRun();
}

void LVActionsMenu::removeMirror()
{
    if (remove_mirror(m_lv))
        MainWindow->reRun();
}

void LVActionsMenu::removeMirrorLeg()
{
    if (remove_mirror_leg(m_lv))
        MainWindow->reRun();
}

void LVActionsMenu::mkfsLogicalVolume()
{
    MkfsDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            MainWindow->reRun();
    }
}

void LVActionsMenu::fsckLogicalVolume()
{
    if (manual_fsck(m_lv))
        MainWindow->reRun();
}

void LVActionsMenu::removefsLogicalVolume()
{
    if (remove_fs(m_lv->getMapperPath()))
        MainWindow->reRun();
}

void LVActionsMenu::maxfsLogicalVolume()
{
    if (max_fs(m_lv))
        MainWindow->reRun();
}

void LVActionsMenu::mergeSnapshot()
{
    if (merge_snap(m_lv))
        MainWindow->reRun();
}

void LVActionsMenu::removeLogicalVolume()
{
    LVRemoveDialog dialog(m_lv);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == KDialog::Yes)
            MainWindow->reRun();
    }
}

void LVActionsMenu::renameLogicalVolume()
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

void LVActionsMenu::changeLogicalVolume()
{
    LVChangeDialog dialog(m_lv);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
        MainWindow->reRun();
}

void LVActionsMenu::mountFilesystem()
{
    MountDialog dialog(m_lv);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
        MainWindow->reRun();
}

void LVActionsMenu::unmountFilesystem()
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
