/*
 *
 *
 * Copyright (C) 2009, 2010, 2011, 2012, 2013, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "partchange.h"

#include "fsextend.h"
#include "fsreduce.h"
#include "pedexceptions.h"
#include "physvol.h"
#include "processprogress.h"
#include "progressbox.h"
#include "pvextend.h"
#include "pvreduce.h"
#include "storagepartition.h"
#include "topwindow.h"

#include <cmath>

#include <QApplication>
#include <QPushButton>

#include <KLocalizedString>
#include <KMessageBox>


PartitionChangeDialog::PartitionChangeDialog(StoragePartition *const partition, QWidget *parent)
    : PartitionDialogBase(partition, parent),
      m_old_storage_part(partition)
{
    m_bailout = true;

    if (!hasInitialErrors()) {
        if (continueBackup()) {
            if (continueResize()) {
                if (continueBusy()) {
                    validateChange();
        
                    connect(this, SIGNAL(changed()),
                            this, SLOT(validateChange()));
                    
                    connect(this, SIGNAL(okClicked()),
                            this, SLOT(commitPartition()));

                    m_bailout = false;
                }
            }
        }
    }
}

bool PartitionChangeDialog::continueBackup()
{
    const QString warning2 = i18n("Changes to the partition table can cause unintentional and permanent data"
                                  " loss. If the partition holds important data, you really should back it"
                                  " up before continuing.");

    if (KMessageBox::warningContinueCancel(nullptr,
                                           warning2,
                                           QString(),
                                           KStandardGuiItem::cont(),
                                           KStandardGuiItem::cancel(),
                                           QString(),
                                           KMessageBox::Dangerous) != KMessageBox::Continue) {
        return false;
    }

    return true;
}

bool PartitionChangeDialog::continueResize()
{
    const QString message = i18n("Shrinking this filesystem is not supported, only growing or"
                                 " moving it is possible."
                                 "\n\n"
                                 "Note: currently only ext2, ext3, ext4 and physical volumes can be shrunk.");

    const QString warning = i18n("If this partition is enlarged, any filesystem or data on it"
                                 " will need to be extended separately. Shrinking this"
                                 " partition is not supported."
                                 "\n\n"
                                 "Note: currently only ext2, ext3, ext4 and physical volumes"
                                 " can be both shrunk and grown,"
                                 " while Reiserfs, ntfs, jfs and xfs can be grown only.");


    const QString fs = m_old_storage_part->getFilesystem();

    if (!(fs_can_reduce(fs) || m_old_storage_part->isPhysicalVolume())) {
        if(fs_can_extend(fs, m_old_storage_part->isMounted())) {
            KMessageBox::information(nullptr, message);
        } else if (KMessageBox::warningContinueCancel(nullptr,
                                                      warning,
                                                      QString(),
                                                      KStandardGuiItem::cont(),
                                                      KStandardGuiItem::cancel(),
                                                      QString(),
                                                      KMessageBox::Dangerous) != KMessageBox::Continue) {
            return false;
        }
    }
    
    return true;
}

bool PartitionChangeDialog::continueBusy()
{
    const QString warning = i18n("This partition is on the same device with partitions that are busy or mounted. "
                                 "If at all possible they should be unmounted before proceeding. Otherise "
                                 "changes to the partition table may not be recognized by the kernel.");

    if (ped_device_is_busy(getPedPartition()->disk->dev)) {
        if (KMessageBox::warningContinueCancel(nullptr,
                                               warning,
                                               QString(),
                                               KStandardGuiItem::cont(),
                                               KStandardGuiItem::cancel(),
                                               QString(),
                                               KMessageBox::Dangerous) != KMessageBox::Continue) {
            return false;
        }
    }

    return true;
}

void PartitionChangeDialog::commitPartition()
{
    const PedSector new_size   = getNewSize();
    const PedSector new_offset = getNewOffset();
    bool grow   = false;
    bool shrink = false;
    bool move   = ((getMaxStart() + new_offset) != getCurrentStart());

    if (new_size < getCurrentSize()) {
        grow   = false;
        shrink = true;
    } else if (new_size > getCurrentSize()) {
        grow   = true;
        shrink = false;
    } else {
        grow   = false;
        shrink = false;
    }

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    if (grow && move) {
        if (movePartition()) {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            growPartition();
        }
    } else if (grow) {
        growPartition();
    } else if (shrink && move) {
        if (shrinkPartition()) {
            qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
            movePartition();
        }
    } else if (shrink) {
        shrinkPartition();
    } else if (move) {
        movePartition();
    }

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
}

void PartitionChangeDialog::validateChange()
{
    const PedSector ONE_MIB   = 0x100000 / getSectorSize();   // sectors per megabyte
    const PedSector max_start = getMaxStart();
    const PedSector max_end   = getMaxEnd();
    const PedSector current_size  = getCurrentSize();
    const PedSector current_start = getCurrentStart();
    const PedSector preceding_sectors = getNewOffset();
    const PedSector new_start = max_start + preceding_sectors;
    const PedSector new_size  = getNewSize();
    const PedSector following_sectors = getMaxSize() - (preceding_sectors + new_size);

    if (!isValid() || (preceding_sectors < 0) || (following_sectors < 0)) {
        (button(KDialog::Ok))->setEnabled(false);
        return;
    } else {
        updateGraphicAndLabels();

        bool moving = false;
        bool resize = false;
        
        // we don't move if the move is less than 1 megabyte
        // and check that we have at least 1 meg to spare
        
        if (fabs(current_start - new_start) >= ONE_MIB){  // moving more than 1 MiB
            if ((new_start < current_start) && ((current_start - max_start) >= ONE_MIB)) {  // moving left
                moving = true;
            } else if ((max_end - (current_start + current_size - 1)) >= ONE_MIB){          // moving right
                moving = true;
            }
        }
        
        if (fabs(current_size - new_size) >= ONE_MIB){  // resizing by more than 1 MiB?
            resize = true;
        }
        
        if (resize || moving)
            button(KDialog::Ok)->setEnabled(true);
        else
            button(KDialog::Ok)->setEnabled(false);
    }
}

bool PartitionChangeDialog::movefs(PedSector from_start, PedSector to_start, PedSector length)
{
    const long long blocksize = 8000;    // sectors moved in each block
    PedDevice *const device = getPedPartition()->disk->dev;

    const long long blockcount = length / blocksize;
    const long long extra = length % blocksize;

    bool success = true;

    if (ped_device_open(device)) {

        void *const buff = malloc(blocksize * getSectorSize());
        
        ProgressBox *const progress_box = TopWindow::getProgressBox();
        progress_box->setRange(0, blockcount);
        progress_box->setText(i18n("Moving data"));
        qApp->setOverrideCursor(Qt::WaitCursor);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        
        if (to_start < from_start) {                       // moving left
            for (long long x = 0; x < blockcount; x++) {
                if ( !(x % 5) )
                    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                if (!ped_device_read(device, buff, from_start + (x * blocksize), blocksize)) {
                    qApp->restoreOverrideCursor();
                    KMessageBox::error(nullptr, i18n("Move failed: could not read from device"));
                    success = false;
                    break;
                } else if (!ped_device_write(device, buff, to_start + (x * blocksize), blocksize)) {
                    qApp->restoreOverrideCursor();
                    KMessageBox::error(nullptr, i18n("Move failed: could not write to device"));
                    success = false;
                    break;
                }
                progress_box->setValue(x);
            }

            if (success) {
                if (!ped_device_read(device,  buff, from_start + (blockcount * blocksize), extra)) {
                    qApp->restoreOverrideCursor();
                    KMessageBox::error(nullptr, i18n("Move failed: could not read from device"));
                    success = false;
                } else if (!ped_device_write(device, buff, to_start + (blockcount * blocksize), extra)) {
                    qApp->restoreOverrideCursor();
                    KMessageBox::error(nullptr, i18n("Move failed: could not write to device"));
                    success = false;
                }
            }
        } else {                                           // moving right
            if (!ped_device_read(device,  buff, from_start + (blockcount * blocksize), extra)) {
                qApp->restoreOverrideCursor();
                KMessageBox::error(nullptr, i18n("Move failed: could not read from device"));
                success = false;
            } else if (!ped_device_write(device, buff, to_start   + (blockcount * blocksize), extra)) {
                qApp->restoreOverrideCursor();
                KMessageBox::error(nullptr, i18n("Move failed: could not write to device"));
                success = false;
            }

            if (success) {
                for (long long x = blockcount - 1; x >= 0 ; x--) {
                    if ( !(x % 5) )
                        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

                    if (!ped_device_read(device, buff, from_start + (x * blocksize), blocksize)) {
                        qApp->restoreOverrideCursor();
                        KMessageBox::error(nullptr, i18n("Move failed: could not read from device"));
                        success = false;
                        break;
                    } else if (!ped_device_write(device, buff, to_start   + (x * blocksize), blocksize)) {
                        qApp->restoreOverrideCursor();
                        KMessageBox::error(nullptr, i18n("Move failed: could not write to device"));
                        success = false;
                        break;
                    }
                    progress_box->setValue(blockcount - x);
                }
            }
        }
        
        free(buff);

        progress_box->reset();
        progress_box->setRange(0, 0);
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

        progress_box->setText(i18n("Syncing device"));
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        if (!ped_device_sync(device))
            success = false;

        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        if (!ped_device_close(device))
            success = false;

        progress_box->reset();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        qApp->restoreOverrideCursor();
    } else {
        success = false;
    }

    return success;
}

bool PartitionChangeDialog::shrinkPartition()
{
    hide();

    const PedSector ONE_MIB  = 0x100000 / getSectorSize();   // sectors per megabyte
    PedDisk *const disk = getPedPartition()->disk;
    PedDevice *const device = disk->dev;
    const PedSector current_start = getCurrentStart();
    const PedSector current_size  = getCurrentSize();
    const QString fs   = m_old_storage_part->getFilesystem();
    const bool is_pv   = m_old_storage_part->isPhysicalVolume();
    PedSector new_size = getNewSize();

    if (new_size >= current_size)
        return false;

    if (new_size < getMinSize())
        new_size = getMinSize();
    else if (new_size > getMaxSize())
        new_size = 1 + getMaxSize();

    PedSector reduced_size = getNewSize();
    if (is_pv)
        reduced_size = pv_reduce(getPath(), new_size * getSectorSize());
    else
        reduced_size = fs_reduce(getPath(), new_size * getSectorSize(), fs);

    new_size = reduced_size / getSectorSize();
    if (reduced_size % getSectorSize())
        new_size++;

    if (new_size == 0) {  // The shrink failed
        KMessageBox::error(nullptr, i18n("Filesystem shrink failed"));
        return false;
    }

    // This constraint assures we have a new partition at least as long as the fs can shrink it
    // We allow up to an extra 1MiB sectors for the end of the partition

    PedAlignment *const start_alignment = ped_alignment_new(0, 1);
    PedGeometry  *const start_range = ped_geometry_new(device, current_start, 1);

    PedGeometry  *end_range;
    PedAlignment *end_alignment;
    PedSector maximum_size;
    PedSector minimum_size = new_size;

    if (current_start + new_size - 1 > getMaxEnd()) {
        end_alignment = ped_alignment_new(0, 1);
        end_range = ped_geometry_new(device, current_start + new_size - 1, 1);
        maximum_size = minimum_size;
    } else {
        end_alignment = ped_alignment_new(-1, ONE_MIB);
        end_range = ped_geometry_new(device, current_start + new_size - 1, ONE_MIB);
        maximum_size = new_size + ONE_MIB - 1;
    }

    qApp->setOverrideCursor(Qt::WaitCursor);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    PedConstraint *constraint = ped_constraint_new(start_alignment, end_alignment,
                                                   start_range, end_range,
                                                   minimum_size, maximum_size);

    int success = ped_disk_set_partition_geom(disk, getPedPartition(), constraint,
                                              current_start, current_start + maximum_size);

    qApp->restoreOverrideCursor();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    ped_constraint_destroy(constraint);
    ped_alignment_destroy(start_alignment);
    ped_alignment_destroy(end_alignment);
    ped_geometry_destroy(start_range);
    ped_geometry_destroy(end_range);

    if (!success) {
        KMessageBox::error(nullptr, i18n("Partition shrink failed"));
        return false;
    } else {
        pedCommitAndWait(disk);
        return true;
    }
}

bool PartitionChangeDialog::growPartition()
{
    hide();

    const PedSector ONE_MIB  = 0x100000 / getSectorSize();   // sectors per megabyte
    PedDisk *const disk = getPedPartition()->disk;
    PedDevice *const device = disk->dev;
    const QString fs = m_old_storage_part->getFilesystem();
    const bool is_pv = m_old_storage_part->isPhysicalVolume();
    const PedSector current_start = getCurrentStart();
    const PedSector current_size  = getCurrentSize();
    const PedSector max_end   = getMaxEnd();
    const PedSector max_start = getMaxStart();

    int success;
    PedSector min_new_size,
              max_new_size;  // max desired size

    PedSector proposed_new_size = getNewSize();

    if ((proposed_new_size - 1 + current_start) > max_end)
        proposed_new_size = 1 + max_end - current_start;

    if (proposed_new_size <= current_size)
        return true;

    if (proposed_new_size < current_size + ONE_MIB) {
        if (current_start + ONE_MIB <= max_end) {
            max_new_size = current_size + ONE_MIB;
            min_new_size = current_size;
        } else {
            max_new_size = max_end - current_start + 1;
            min_new_size = current_size;
        }
    } else if (proposed_new_size + ONE_MIB >= 1 + max_end - current_start) {
        max_new_size = 1 + max_end - current_start;
        min_new_size = max_new_size - ONE_MIB;
    } else {
        max_new_size = proposed_new_size + ONE_MIB - 1;
        min_new_size = proposed_new_size;
    }

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    qApp->setOverrideCursor(Qt::WaitCursor);

    PedGeometry *start_range = ped_geometry_new(device, current_start, 1);
    PedGeometry *end_range   = ped_geometry_new(device, current_start, max_new_size);

    PedConstraint *constraint = ped_constraint_new(ped_alignment_new(0, 1), ped_alignment_new(-1, ONE_MIB),
                                start_range, end_range,
                                min_new_size, max_new_size);

    /* if constraint solves to nullptr then the new part will fail, so just bail out */
    if (ped_constraint_solve_max(constraint) == nullptr) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        qApp->restoreOverrideCursor();
        KMessageBox::error(nullptr, i18n("Partition extension failed"));

        ped_constraint_destroy(constraint);
        ped_geometry_destroy(start_range);
        ped_geometry_destroy(end_range);

        return false;
    }

    success = ped_disk_set_partition_geom(disk, getPedPartition(), constraint, max_start, max_end);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    qApp->restoreOverrideCursor();

    ped_constraint_destroy(constraint);
    ped_geometry_destroy(start_range);
    ped_geometry_destroy(end_range);

    if (!success) {
        KMessageBox::error(nullptr, i18n("Partition extension failed"));
        return false;
    } else {
        // Here we wait for linux and udev to re-read the partition table before doing anything else.
        // Otherwise the resize program will fail.

        pedCommitAndWait(disk);

        if (is_pv)
            return pv_extend(getPath());
        else if (fs_can_extend(fs, m_old_storage_part->isMounted()))
            return fs_extend(getPath(), fs, m_old_storage_part->getMountPoints());
        else
            return true;
    }
}

bool PartitionChangeDialog::movePartition()
{
    hide();

    PedDisk *const disk = getPedPartition()->disk;
    PedDevice *const device = disk->dev;

    const PedSector ONE_MIB   = 0x100000 / getSectorSize();   // sectors per megabyte
    const PedSector max_start = getMaxStart();
    const PedSector max_end   = getMaxEnd();
    const PedSector current_size  = getCurrentSize();
    PedSector current_start = getCurrentStart();
    PedSector new_start     = max_start + getNewOffset();

    // don't move if the move is less than 1 megabyte
    // and check that we have at least 1 meg to spare

    if (fabs(current_start - new_start) < ONE_MIB)
        return true;                       // pretend we moved since it wasn't worth doing
    else if (new_start < current_start) {  // moving left

        if ((current_start - max_start) < ONE_MIB)
            return false;
        else if ((new_start - max_start) < (ONE_MIB / 2))
            new_start = max_start + (ONE_MIB / 2);
    } else {                               // moving right

        if ((max_end - (current_start + current_size - 1)) < ONE_MIB)
            return false;
        else if ((new_start + current_size - 1) > (max_end - ONE_MIB / 2))
            new_start = 2 + max_end - ((ONE_MIB / 2) + current_size);
    }

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    qApp->setOverrideCursor(Qt::WaitCursor);

    PedAlignment *start_alignment = ped_alignment_new(0, ONE_MIB);
    PedAlignment *end_alignment   = ped_alignment_new(0, 1);

    PedGeometry *start_range = ped_geometry_new(device, new_start - (ONE_MIB / 2), ONE_MIB);

    PedGeometry *end_range = ped_geometry_new(device, new_start + (current_size - 1) - (ONE_MIB / 2), ONE_MIB);

    PedConstraint *constraint_1MiB = ped_constraint_new(start_alignment, end_alignment,
                                     start_range, end_range,
                                     current_size, current_size);

    PedSector old_start = current_start;
    PedSector old_size  = current_size;

    int success = 0;

    if (constraint_1MiB) {
        success = ped_disk_set_partition_geom(disk, getPedPartition(), constraint_1MiB,
                                              max_start, max_end);
    }

    ped_exception_set_handler(my_handler);
    current_start = getPedPartition()->geom.start;  // getCurrentStart() won't return the correct value here
    qApp->restoreOverrideCursor();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    ped_constraint_destroy(constraint_1MiB);
    ped_alignment_destroy(start_alignment);
    ped_alignment_destroy(end_alignment);
    ped_geometry_destroy(start_range);
    ped_geometry_destroy(end_range);

    if (!success) {
        KMessageBox::error(nullptr, i18n("Repartitioning failed: data not moved"));
        return false;
    } else {
        pedCommitAndWait(disk);
        return(movefs(old_start, current_start, old_size));
    }
}

bool PartitionChangeDialog::bailout()
{
    return m_bailout;
}
