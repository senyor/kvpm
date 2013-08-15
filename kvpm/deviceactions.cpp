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


#include "deviceactions.h"

#include "fsck.h"
#include "devicesizechartseg.h"
#include "mkfs.h"
#include "maxfs.h"
#include "mount.h"
#include "unmount.h"
#include "masterlist.h"
#include "partremove.h"
#include "partadd.h"
#include "partchange.h"
#include "partflag.h"
#include "physvol.h"
#include "storagedevice.h"
#include "storagepartition.h"
#include "tablecreate.h"
#include "topwindow.h"
#include "vgreduce.h"
#include "vgcreate.h"
#include "vgextend.h"

#include <KLocale>
#include <KIcon>

#include <QActionGroup>
#include <QDebug>
#include <QTreeWidgetItem>



DeviceActions::DeviceActions(QWidget *parent) : 
    KActionCollection(parent)
{
    m_act_grp = new QActionGroup(this);

    connect(m_act_grp, SIGNAL(triggered(QAction *)), 
            this, SLOT(extendVg(QAction *)) );

    KAction *const partchange = addAction("partchange", this, SLOT(changePartition()));
    //   mkfs->setIcon(KIcon("lightning_add"));
    partchange->setText(i18n("Move or resize disk partition"));
    partchange->setIconText(i18n("change"));
    partchange->setToolTip(i18n("Move or resize disk partition"));
    
    KAction *const partflag = addAction("changeflags", this, SLOT(changeFlags())); 
    partflag->setIcon(KIcon("flag-blue"));
    partflag->setText(i18n("Change partition flags"));
    partflag->setIconText(i18n("flags"));
    partflag->setToolTip(i18n("Change partition flags"));
    
    KAction *const maxpv = addAction("max_pv", this, SLOT(maxFs()));
    maxpv->setIcon(KIcon("resultset_last"));
    maxpv->setText(i18n("Extend physical volume to fill device"));
    maxpv->setIconText(i18n("Extend physical volume to fill device"));
    maxpv->setToolTip(i18n("Extend physical volume to fill device"));
    
    KAction *const partadd = addAction("partadd", this, SLOT(addPartition()));
    //  partadd->setIcon(i18n("Add disk partition"));
    partadd->setText(i18n("Add disk partition"));
    partadd->setIconText(i18n("Add"));
    partadd->setToolTip(i18n("Add disk partition"));
    
    KAction *const partremove = addAction("partremove", this, SLOT(removePartition())); 
    partremove->setIcon(KIcon("cross"));
    partremove->setText(i18n("Remove disk partition"));
    partremove->setIconText(i18n("Remove"));
    partremove->setToolTip(i18n("Remove disk partition"));
    
    KAction *const vgcreate = addAction("vgcreate", this, SLOT(createVg()));
    vgcreate->setIcon(KIcon("document-new"));
    vgcreate->setText(i18n("Create volume group"));
    vgcreate->setIconText(i18n("New vg"));
    vgcreate->setToolTip(i18n("Create volume group"));
    
    KAction *const tablecreate = addAction("tablecreate", this, SLOT(createTable())); 
    tablecreate->setIcon(KIcon("exclamation"));
    tablecreate->setText(i18n("Create or remove a partition table"));
    tablecreate->setIconText(i18n("Table"));
    tablecreate->setToolTip(i18n("Create or remove a partition table"));

    KAction *const vgreduce = addAction("vgreduce", this, SLOT(reduceVg()));
    vgreduce->setIcon(KIcon("delete"));
    vgreduce->setText(i18n("Remove from volume group"));
    vgreduce->setIconText(i18n("Remove from volume group"));
    vgreduce->setToolTip(i18n("Remove from volume group"));
    
    KAction *const mkfs =  addAction("mkfs", this, SLOT(makeFs()));
    mkfs->setIcon(KIcon("lightning_add"));
    mkfs->setText(i18n("Make or remove filesystem..."));
    mkfs->setIconText(i18n("mkfs"));
    mkfs->setToolTip(i18n("Make or remove a filesystem"));
    
    KAction *const max_fs = addAction("max_fs", this, SLOT(maxFs()));
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

    changeDevice(nullptr);
}

void DeviceActions::changeDevice(QTreeWidgetItem *item)
{
    QAction *const mkfs = action("mkfs");
    QAction *const fsck = action("fsck");
    QAction *const max_fs = action("max_fs");
    QAction *const max_pv = action("max_pv");
    QAction *const partflag = action("changeflags");
    QAction *const partremove = action("partremove");
    QAction *const partadd = action("partadd");
    QAction *const partchange = action("partchange");
    QAction *const vgcreate = action("vgcreate");
    QAction *const tablecreate = action("tablecreate");
    QAction *const vgreduce = action("vgreduce");
    QAction *const mount = action("mount");
    QAction *const unmount = action("unmount");

    QList<QAction *> extend_actions = m_act_grp->actions();
    for (auto action : extend_actions) {
        m_act_grp->removeAction(action);
        removeAction(action);  // also deletes the action
    }

    for (auto vg : MasterList::getVgNames()) {
        KAction *const action = addAction(vg, this);
        m_act_grp->addAction(action);
        action->setText(vg);
    }

    if (item) {

        m_dev = (StorageDevice *)((item->data(1, Qt::UserRole)).value<void *>());

        if ((item->data(0, Qt::UserRole)).canConvert<void *>()) {   // its a partition
            m_part = (StoragePartition *)((item->data(0, Qt::UserRole)).value<void *>());

            tablecreate->setEnabled(false);
            mount->setEnabled(m_part->isMountable());
            unmount->setEnabled(m_part->isMounted());
            fsck->setEnabled(!m_part->isMounted() && !m_part->isBusy());

            if (m_part->getPedType() & 0x04) {    // freespace
                max_fs->setEnabled(false);
                max_pv->setEnabled(false);
                mkfs->setEnabled(false);
                partremove->setEnabled(false);
                partchange->setEnabled(false);
                partadd->setEnabled(true);
                vgcreate->setEnabled(false);
                vgextendEnable(false);
                vgreduce->setEnabled(false);
                partflag->setEnabled(false);
            } else if (m_part->getPedType() & 0x02) { // extended partition
                max_fs->setEnabled(false);
                max_pv->setEnabled(false);
                mkfs->setEnabled(false);

                if (m_part->isEmptyExtended()) {
                    partadd->setEnabled(true);
                    partremove->setEnabled(true);
                } else {
                    partadd->setEnabled(false);
                    partremove->setEnabled(false);
                }

                partchange->setEnabled(false);
                vgcreate->setEnabled(false);
                vgextendEnable(false);
                vgreduce->setEnabled(false);
                partflag->setEnabled(true);
            } else if (m_part->isPhysicalVolume()) {
                max_fs->setEnabled(false);
                mkfs->setEnabled(false);
                partremove->setEnabled(false);

                if (m_part->getPhysicalVolume()->isActive())
                    partchange->setEnabled(false);
                else
                    partchange->setEnabled(true);
                
                max_pv->setEnabled(true);
                partadd->setEnabled(false);
                vgcreate->setEnabled(false);
                vgextendEnable(false);

                if (m_part->getPhysicalVolume()->getPercentUsed() == 0)
                    vgreduce->setEnabled(true);
                else
                    vgreduce->setEnabled(false);

                partflag->setEnabled(true);
            } else if (m_part->isNormal() || m_part->isLogical()) {
                max_pv->setEnabled(false);

                if (m_part->isMounted() || m_part->isBusy()) {
                    partremove->setEnabled(false);
                    partchange->setEnabled(false);
                    mkfs->setEnabled(false);
                    max_fs->setEnabled(false);
                    vgcreate->setEnabled(false);
                    vgextendEnable(false);
                } else {                                            // not mounted or busy
                    partremove->setEnabled(true);
                    partchange->setEnabled(true);
                    max_fs->setEnabled(true);
                    mkfs->setEnabled(true);
                    vgcreate->setEnabled(true);
                }
                partadd->setEnabled(false);
                vgreduce->setEnabled(false);
                partflag->setEnabled(true);
            }
        } else { // its a whole device
            m_part = nullptr;
            mount->setEnabled(false);
            unmount->setEnabled(false);
            max_fs->setEnabled(false);
            fsck->setEnabled(false);
            partflag->setEnabled(false);

            if (m_dev->isPhysicalVolume()) {
                tablecreate->setEnabled(false);
                mkfs->setEnabled(false);
                max_pv->setEnabled(true);
                partremove->setEnabled(false);
                partchange->setEnabled(false);
                partadd->setEnabled(false);
                vgcreate->setEnabled(false);
                vgextendEnable(false);
                if (m_dev->getPhysicalVolume()->getPercentUsed() == 0)
                    vgreduce->setEnabled(true);
                else
                    vgreduce->setEnabled(false);
            } else { // not a pv
                partremove->setEnabled(false);
                partchange->setEnabled(false);
                partadd->setEnabled(false);
                mkfs->setEnabled(false);
                vgreduce->setEnabled(false);
                max_pv->setEnabled(false);
                if (m_dev->isBusy() || m_dev->getRealPartitionCount() != 0) {
                    tablecreate->setEnabled(false);
                    vgcreate->setEnabled(false);
                    vgextendEnable(false);
                } else {
                    tablecreate->setEnabled(true);
                    vgcreate->setEnabled(true);
                    vgextendEnable(true);
                }
            }
        }
    } else {
        mkfs->setEnabled(false);
        fsck->setEnabled(false);
        max_fs->setEnabled(false);
        max_pv->setEnabled(false);
        partflag->setEnabled(false);
        partremove->setEnabled(false);
        partadd->setEnabled(false);
        partchange->setEnabled(false);
        vgcreate->setEnabled(false);
        tablecreate->setEnabled(false);
        vgreduce->setEnabled(false);
        mount->setEnabled(false);
        unmount->setEnabled(false);
    }
}

void DeviceActions::vgextendEnable(bool enable)
{
    for (auto action : m_act_grp->actions())
        action->setEnabled(enable);
}

void DeviceActions::makeFs()
{
    MkfsDialog dialog(m_part);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

void DeviceActions::checkFs()
{
    if (manual_fsck(m_part))
        g_top_window->reRun();
}

void DeviceActions::maxFs()
{
    if (m_part) {        // m_part == nullptr if not a partition
        if (max_fs(m_part))
            g_top_window->reRun();
    } else {
        if (max_fs(m_dev))
            g_top_window->reRun();
    }
}

void DeviceActions::removePartition()
{
    if (remove_partition(m_part))
        g_top_window->reRun();
}

void DeviceActions::addPartition()
{
    PartitionAddDialog dialog(m_part);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

void DeviceActions::changePartition()
{
    PartitionChangeDialog dialog(m_part);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

void DeviceActions::changeFlags()
{
    PartitionFlagDialog dialog(m_part);

    if (!dialog.bailout()) {
        dialog.exec();

        if (dialog.result() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

void DeviceActions::createVg()
{
    VGCreateDialog *dialog;

    if (m_part)
        dialog = new VGCreateDialog(m_part);
    else       
        dialog = new VGCreateDialog(m_dev);

    if (dialog->run() == QDialog::Accepted)
        g_top_window->reRun();

    dialog->deleteLater();
}

void DeviceActions::createTable()
{
    if (create_table(m_dev->getName()))
        g_top_window->reRun();
}

void DeviceActions::reduceVg()   // pvs can also be whole devices
{
    PhysVol *pv = nullptr;

    if (m_part)
        pv = m_part->getPhysicalVolume();
    else
        pv = m_dev->getPhysicalVolume();

    VGReduceDialog  dialog(pv);

    if (dialog.run() == KDialog::Yes)
        g_top_window->reRun();
}

void DeviceActions::mountFs()
{
    MountDialog dialog(m_part);
    dialog.exec();

    if (dialog.result() == QDialog::Accepted)
        g_top_window->reRun();
}

void DeviceActions::unmountFs()
{
    UnmountDialog dialog(m_part);

    if (!dialog.bailout()) {
        dialog.exec();
        if (dialog.result() == QDialog::Accepted || dialog.result() == KDialog::Yes)
            g_top_window->reRun();
    }
}

void DeviceActions::extendVg(QAction *action)
{
    const QString vg_name = action->objectName();
    VolGroup *const vg = MasterList::getVgByName(vg_name);

    if (m_part) {
        VGExtendDialog dialog(vg, nullptr, m_part);

        if (dialog.run() == QDialog::Accepted)
            g_top_window->reRun();
    } else {                          // whole device, not partition
        VGExtendDialog dialog(vg, m_dev, nullptr);

        if (dialog.run() == QDialog::Accepted)
            g_top_window->reRun();
    }
}

