/*
 *
 *
 * Copyright (C) 2009, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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

#include "dualselectorbox.h"
#include "fsextend.h"
#include "fsreduce.h"
#include "masterlist.h"
#include "pedexceptions.h"
#include "partitiongraphic.h"
#include "physvol.h"
#include "processprogress.h"
#include "progressbox.h"
#include "pvextend.h"
#include "pvreduce.h"
#include "storagepartition.h"
#include "topwindow.h"
#include "volgroup.h"

#include <math.h>

#include <KComboBox>
#include <KLineEdit>
#include <KApplication>
#include <KButtonGroup>
#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>

#include <QLabel>
#include <QGroupBox>
#include <QVBoxLayout>



PartitionChangeDialog::PartitionChangeDialog(StoragePartition *const partition, QWidget *parent)
    : KDialog(parent),
      m_old_storage_part(partition)
{
    m_bailout = false;
    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);

    setup();

    const QString fs = m_old_storage_part->getFilesystem();
    const long long existing_offset = m_existing_part->geom.start - m_max_part_start;

    const long long max_offset = m_max_part_size - m_min_shrink_size;
    long long max_size;

    if (!(fs_can_extend(fs) || partition->isPhysicalVolume()))
        max_size = m_existing_part->geom.length;
    else
        max_size = m_max_part_size;

    setButtons(KDialog::Ok | KDialog::Cancel | KDialog::Reset);
    setWindowTitle(i18n("Move or resize a partition"));

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    QLabel *const label = new QLabel(i18n("<b>Resize Or Move A Partition</b>"));
    label->setAlignment(Qt::AlignCenter);
    layout->addSpacing(5);
    layout->addWidget(label);
    layout->addSpacing(5);

    layout->addWidget(buildInfoGroup(max_size));

    if (m_old_storage_part->isPhysicalVolume()) {
        if (m_old_storage_part->getPhysicalVolume()->isActive()) {
            max_size -= existing_offset;
            m_dual_selector = new DualSelectorBox(m_sector_size, m_max_part_size,
                                                  m_min_shrink_size, max_size, m_existing_part->geom.length,
                                                  existing_offset, existing_offset, existing_offset);
        } else {
            m_dual_selector = new DualSelectorBox(m_sector_size, m_max_part_size,
                                                  m_min_shrink_size, max_size, m_existing_part->geom.length,
                                                  0, max_offset, existing_offset);
        }
    } else {
        m_dual_selector = new DualSelectorBox(m_sector_size, m_max_part_size,
                                              m_min_shrink_size, max_size, m_existing_part->geom.length,
                                              0, max_offset, existing_offset);
    }


    layout->addWidget(m_dual_selector);
    m_dual_selector->resetSelectors();
    validateChange();

    connect(m_dual_selector, SIGNAL(changed()),
            this, SLOT(validateChange()));

    connect(this, SIGNAL(resetClicked()),
            m_dual_selector, SLOT(resetSelectors()));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(commitPartition()));

    setDefaultButton(KDialog::Cancel);
}

void PartitionChangeDialog::commitPartition()
{
    long long new_size   = m_dual_selector->getCurrentSize();
    long long new_offset = m_dual_selector->getCurrentOffset();
    bool grow   = false;
    bool shrink = false;
    bool move   = ((m_max_part_start + new_offset) != m_existing_part->geom.start);

    if (new_size < m_existing_part->geom.length) {
        grow   = false;
        shrink = true;
    } else if (new_size > m_existing_part->geom.length) {
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
    if (!m_dual_selector->isValid()) {
        button(KDialog::Ok)->setEnabled(false);
        return;
    }

    const long long preceding_sectors = m_dual_selector->getCurrentOffset();
    const long long following_sectors = m_max_part_size - (m_dual_selector->getCurrentOffset() + m_dual_selector->getCurrentSize());

    if (preceding_sectors < 0 || following_sectors < 0) {
        (button(KDialog::Ok))->setEnabled(false);
        return;
    }

    updateGraphicAndLabels();

    button(KDialog::Ok)->setEnabled(true);
}

void PartitionChangeDialog::setup()
{
    const QString fs = m_old_storage_part->getFilesystem();

    m_existing_part = m_old_storage_part->getPedPartition();
    m_ped_disk = m_existing_part->disk;

    PedDevice *const ped_device = m_ped_disk->dev;

    QString message = i18n("Currently only the ext2, ext3 and ext4 file systems "
                           "are supported for file system shrinking. "
                           "Growing is supported for ext2/3/4, jfs, xfs, ntfs and Reiserfs. "
                           "Moving a partition is supported for any filesystem. "
                           "Physical volumes may also be grown, shrunk or moved");

    if (!(fs == "ext2" || fs == "ext3" || fs == "ext4" || m_old_storage_part->isPhysicalVolume()))
        KMessageBox::information(0, message);

    message = i18n("This partition is on the same device with partitions that are busy or mounted. "
                   "If at all possible they should be unmounted before proceeding. Otherise "
                   "changes to the partition table may not be recognized by the kernel.");

    if (ped_device_is_busy(ped_device)) {
        if (KMessageBox::warningContinueCancel(NULL,
                                               message,
                                               QString(),
                                               KStandardGuiItem::cont(),
                                               KStandardGuiItem::cancel(),
                                               QString(),
                                               KMessageBox::Dangerous) != KMessageBox::Continue) {
            m_bailout = true;
        }
    }

    /* Switch off cylinder alignment */
    PedDiskFlag cylinder_flag = ped_disk_flag_get_by_name("cylinder_alignment");
    if (ped_disk_is_flag_available(m_ped_disk, cylinder_flag))
        ped_disk_set_flag(m_ped_disk, cylinder_flag, 0);

    PedSector max_part_end;
    getMaximumPartition(m_max_part_start, max_part_end, m_sector_size);
    m_max_part_size = max_part_end - m_max_part_start + 1;

    if (m_existing_part->type & PED_PARTITION_LOGICAL)
        m_logical = true;
    else
        m_logical = false;

    if (m_old_storage_part->isPhysicalVolume()) {

        PhysVol *const pv = m_old_storage_part->getPhysicalVolume();
        const long mda_count = pv->getMdaCount();
        const long long extent_size = pv->getVg()->getExtentSize();
        const long long mda_extents = (pv->getMdaSize() / extent_size) + 1;

        m_min_shrink_size = 1 + (mda_extents * mda_count) + pv->getLastUsedExtent();
        m_min_shrink_size *= extent_size;
        m_min_shrink_size /= m_sector_size;
    } else {
        if (fs_can_reduce(fs))
            m_min_shrink_size = get_min_fs_size(ped_partition_get_path(m_existing_part), fs) / m_sector_size;
        else
            m_min_shrink_size = m_existing_part->geom.length;
    }

    const PedSector TWO_MIB = 0x200000 / m_sector_size;

    if (m_min_shrink_size == 0)                            // 0 means we can't shrink it
        m_min_shrink_size = m_existing_part->geom.length;
    else if (m_min_shrink_size <= TWO_MIB) {               // Don't allow shrinking below 2 MiB
        if (m_existing_part->geom.length > TWO_MIB)
            m_min_shrink_size  = TWO_MIB;
        else
            m_min_shrink_size  = m_existing_part->geom.length;
    }
}

bool PartitionChangeDialog::movefs(long long from_start, long long to_start, long long length)
{
    const long long blocksize = 8000;    // sectors moved in each block

    PedDevice *const device = m_ped_disk->dev;

    char *const buff = static_cast<char *>(malloc(blocksize * m_sector_size)) ;

    const long long blockcount = length / blocksize;
    const long long extra = length % blocksize;

    ped_device_open(device);

    ProgressBox *const progress_box = TopWindow::getProgressBox();
    progress_box->setRange(0, blockcount);
    progress_box->setText(i18n("Moving data"));
    int event_timer = 0;
    qApp->setOverrideCursor(Qt::WaitCursor);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    if (to_start < from_start) {                       // moving left
        for (long long x = 0; x < blockcount; x++) {
            event_timer++;
            if (event_timer > 5) {
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                event_timer = 0;
            }
            if (! ped_device_read(device, buff, from_start + (x * blocksize), blocksize)) {
                qApp->restoreOverrideCursor();
                KMessageBox::error(0, i18n("Move failed: could not read from device"));
                return false;
            }
            if (! ped_device_write(device, buff, to_start   + (x * blocksize), blocksize)) {
                qApp->restoreOverrideCursor();
                KMessageBox::error(0, i18n("Move failed: could not write to device"));
                return false;
            }
            progress_box->setValue(x);
        }
        if (! ped_device_read(device,  buff, from_start + (blockcount * blocksize), extra)) {
            qApp->restoreOverrideCursor();
            KMessageBox::error(0, i18n("Move failed: could not read from device"));
            return false;
        }
        if (! ped_device_write(device, buff, to_start   + (blockcount * blocksize), extra)) {
            qApp->restoreOverrideCursor();
            KMessageBox::error(0, i18n("Move failed: could not write to device"));
            return false;
        }
    } else {                                           // moving right
        if (! ped_device_read(device,  buff, from_start + (blockcount * blocksize), extra)) {
            qApp->restoreOverrideCursor();
            KMessageBox::error(0, i18n("Move failed: could not read from device"));
            return false;
        }
        if (! ped_device_write(device, buff, to_start   + (blockcount * blocksize), extra)) {
            qApp->restoreOverrideCursor();
            KMessageBox::error(0, i18n("Move failed: could not write to device"));
            return false;
        }
        for (long long x = blockcount - 1; x >= 0 ; x--) {
            event_timer++;
            if (event_timer > 5) {
                qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
                event_timer = 0;
            }
            if (! ped_device_read(device, buff, from_start + (x * blocksize), blocksize)) {
                qApp->restoreOverrideCursor();
                KMessageBox::error(0, i18n("Move failed: could not read from device"));
                return false;
            }
            if (! ped_device_write(device, buff, to_start   + (x * blocksize), blocksize)) {
                qApp->restoreOverrideCursor();
                KMessageBox::error(0, i18n("Move failed: could not write to device"));
                return false;
            }
            progress_box->setValue(blockcount - x);
        }
    }

    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    ped_device_sync(device);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    ped_device_close(device);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    qApp->restoreOverrideCursor();

    return true;
}

bool PartitionChangeDialog::shrinkPartition()
{
    hide();

    const PedSector ONE_MIB  = 0x100000 / m_sector_size;   // sectors per megabyte
    PedDevice *const device = m_ped_disk->dev;
    const PedSector current_start = m_existing_part->geom.start;
    const PedSector current_size  = m_existing_part->geom.length;
    const QString fs   = m_old_storage_part->getFilesystem();
    const QString path = ped_partition_get_path(m_existing_part);
    const bool is_pv   = m_old_storage_part->isPhysicalVolume();
    PedSector new_size = m_dual_selector->getCurrentSize();

    if (new_size >= current_size)
        return false;

    if (new_size < m_min_shrink_size)
        new_size = m_min_shrink_size;
    else if (new_size > m_max_part_size)
        new_size = m_max_part_size;

    PedSector reduced_size = m_dual_selector->getCurrentSize();
    if (is_pv)
        reduced_size = pv_reduce(path, new_size * m_sector_size);
    else
        reduced_size = fs_reduce(path, new_size * m_sector_size, fs);

    new_size = reduced_size / m_sector_size;
    if (reduced_size % m_sector_size)
        new_size++;

    if (new_size == 0)   // The shrink failed
        return false;

    // This constraint assures we have a new partition at least as long as the fs can shrink it
    // We allow up to an extra 1MiB sectors for the end of the partition

    PedAlignment *const start_alignment = ped_alignment_new(0, 1);
    PedGeometry  *const start_range = ped_geometry_new(device, current_start, 1);

    PedGeometry  *end_range;
    PedAlignment *end_alignment;
    PedSector maximum_size;
    PedSector minimum_size = new_size;

    if (current_start + new_size - 1 > m_max_part_start + m_max_part_size - 1) {
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

    int success = ped_disk_set_partition_geom(m_ped_disk, m_existing_part, constraint,
                  current_start, current_start + maximum_size);

    qApp->restoreOverrideCursor();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    ped_constraint_destroy(constraint);
    ped_alignment_destroy(start_alignment);
    ped_alignment_destroy(end_alignment);
    ped_geometry_destroy(start_range);
    ped_geometry_destroy(end_range);

    if (!success) {
        KMessageBox::error(0, i18n("Partition shrink failed"));
        return false;
    } else {
        pedCommitAndWait(m_ped_disk);
        return true;
    }
}

bool PartitionChangeDialog::growPartition()
{
    hide();

    const PedSector ONE_MIB  = 0x100000 / m_sector_size;   // sectors per megabyte
    PedDevice *const device = m_ped_disk->dev;
    const QString fs = m_old_storage_part->getFilesystem();
    const bool is_pv = m_old_storage_part->isPhysicalVolume();
    const PedSector current_start = m_existing_part->geom.start;
    const PedSector current_size  = m_existing_part->geom.length;
    const PedSector max_end   = m_max_part_start + m_max_part_size - 1;
    const PedSector max_start = m_max_part_start;

    int success;
    PedSector min_new_size,
              max_new_size;  // max desired size

    PedSector proposed_new_size = m_dual_selector->getCurrentSize();

    if (proposed_new_size - 1 + current_start > max_end)
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

    /* if constraint solves to NULL then the new part will fail, so just bail out */
    if (ped_constraint_solve_max(constraint) == NULL) {
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        qApp->restoreOverrideCursor();
        KMessageBox::error(0, i18n("Partition extension failed"));

        ped_constraint_destroy(constraint);
        ped_geometry_destroy(start_range);
        ped_geometry_destroy(end_range);

        return false;
    }

    success = ped_disk_set_partition_geom(m_ped_disk, m_existing_part, constraint, max_start, max_end);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
    qApp->restoreOverrideCursor();

    ped_constraint_destroy(constraint);
    ped_geometry_destroy(start_range);
    ped_geometry_destroy(end_range);

    if (!success) {
        KMessageBox::error(0, i18n("Partition extension failed"));
        return false;
    } else {
        // Here we wait for linux and udev to re-read the partition table before doing anything else.
        // Otherwise the resize program will fail.

        pedCommitAndWait(m_ped_disk);

        if (is_pv) {
            if (pv_extend(ped_partition_get_path(m_existing_part)))
                return true;
            else
                return false;
        } else {
            if (fs_extend(ped_partition_get_path(m_existing_part), fs, m_old_storage_part->getMountPoints()))
                return true;
            else
                return false;
        }
    }
}

void PartitionChangeDialog::updateGraphicAndLabels()
{
    const PedSector preceding_sectors = m_dual_selector->getCurrentOffset();
    const PedSector following_sectors = m_max_part_size - (preceding_sectors + m_dual_selector->getCurrentSize());
    const long long change_size = m_sector_size * (m_dual_selector->getCurrentSize() - m_existing_part->geom.length);

    m_display_graphic->setPrecedingSectors(preceding_sectors);
    m_display_graphic->setPartitionSectors(m_dual_selector->getCurrentSize());
    m_display_graphic->setFollowingSectors(following_sectors);
    m_display_graphic->repaint();

    KLocale *const locale = KGlobal::locale();
    if (m_use_si_units)
        locale->setBinaryUnitDialect(KLocale::MetricBinaryDialect);
    else
        locale->setBinaryUnitDialect(KLocale::IECBinaryDialect);

    if (change_size < 0) {
        QString change = locale->formatByteSize(qAbs(change_size));
        m_change_by_label->setText(i18n("<b>Shrink by : -%1</b>", change));
    } else {
        QString change = locale->formatByteSize(change_size);
        m_change_by_label->setText(i18n("<b>Grow by : %1</b>", change));
    }

    QString preceding_bytes_string = locale->formatByteSize(preceding_sectors * m_sector_size);
    m_preceding_label->setText(i18n("Preceding space: %1", preceding_bytes_string));

    long long following_space = following_sectors * m_sector_size;

    if (following_space < 0)
        following_space = 0;

    QString following_bytes_string = locale->formatByteSize(following_space);
    m_following_label->setText(i18n("Following space: %1", following_bytes_string));
}

bool PartitionChangeDialog::movePartition()
{
    hide();

    PedDevice *const device = m_ped_disk->dev;

    const PedSector ONE_MIB   = 0x100000 / m_sector_size;   // sectors per megabyte
    const PedSector max_start = m_max_part_start;
    const PedSector max_end   = max_start + m_max_part_size - 1;
    const PedSector current_size  = m_existing_part->geom.length;
    PedSector current_start = m_existing_part->geom.start;
    PedSector new_start     = m_max_part_start + m_dual_selector->getCurrentOffset();

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
        success = ped_disk_set_partition_geom(m_ped_disk, m_existing_part, constraint_1MiB,
                                              max_start, max_end);
    }

    ped_exception_set_handler(my_handler);
    current_start = m_existing_part->geom.start;
    qApp->restoreOverrideCursor();
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    ped_constraint_destroy(constraint_1MiB);
    ped_alignment_destroy(start_alignment);
    ped_alignment_destroy(end_alignment);
    ped_geometry_destroy(start_range);
    ped_geometry_destroy(end_range);

    if (!success) {
        KMessageBox::error(0, i18n("Repartitioning failed: data not moved"));
        return false;
    } else {
        if (!movefs(old_start, current_start, old_size)) {
            return false;
        } else {
            pedCommitAndWait(m_ped_disk);
            return true;
        }
    }
}

/* The following function waits for udev to acknowledge the partion changes before exiting */

bool PartitionChangeDialog::pedCommitAndWait(PedDisk *disk)
{
    QStringList args;

    qApp->setOverrideCursor(Qt::WaitCursor);
    qApp->processEvents(QEventLoop::ExcludeUserInputEvents);

    if (!ped_disk_commit(disk)) {
        qApp->restoreOverrideCursor();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        return false;
    } else {
        args << "udevadm" << "settle";
        ProcessProgress wait_settle(args);
        qApp->restoreOverrideCursor();
        qApp->processEvents(QEventLoop::ExcludeUserInputEvents);
        return true;
    }
}

QGroupBox *PartitionChangeDialog::buildInfoGroup(const long long maxSize)
{
    QGroupBox   *const group = new QGroupBox();
    QVBoxLayout *const layout = new QVBoxLayout();
    group->setLayout(layout);
    layout->addSpacing(10);

    m_display_graphic = new PartitionGraphic();
    layout->addWidget(m_display_graphic, 0, Qt::AlignCenter);
    QLabel *const path = new QLabel(i18n("<b>Device: %1</b>", m_old_storage_part->getName()));
    path->setAlignment(Qt::AlignHCenter);
    layout->addWidget(path);
    layout->addSpacing(10);

    KLocale *const locale = KGlobal::locale();
    if (m_use_si_units)
        locale->setBinaryUnitDialect(KLocale::MetricBinaryDialect);
    else
        locale->setBinaryUnitDialect(KLocale::IECBinaryDialect);

    m_change_by_label = new QLabel();
    m_preceding_label = new QLabel();
    m_following_label = new QLabel();
    layout->addWidget(m_preceding_label);
    layout->addWidget(m_following_label);
    layout->addStretch();
    layout->addWidget(new QLabel(i18n("Minimum size: %1", locale->formatByteSize(m_min_shrink_size * m_sector_size))));
    layout->addWidget(new QLabel(i18n("Maximum size: %1", locale->formatByteSize(maxSize * m_sector_size))));
    layout->addStretch();
    layout->addSpacing(10);
    layout->addWidget(m_change_by_label);

    return group;
}

bool PartitionChangeDialog::bailout()
{
    return m_bailout;
}

void PartitionChangeDialog::getMaximumPartition(PedSector &start, PedSector &end, PedSector &sectorSize)
{
    PedPartition *const part = m_old_storage_part->getPedPartition();
    PedDevice  *const device = part->disk->dev;
    PedDisk    *const disk   = part->disk;
    sectorSize = device->sector_size;
    const PedSector ONE_MIB = 0x100000 / m_sector_size;

    PedSector const old_start = part->geom.start;
    PedSector const old_end = part->geom.length + old_start - 1;

    PedDiskFlag cylinder_flag = ped_disk_flag_get_by_name("cylinder_alignment");
    if (ped_disk_is_flag_available(disk, cylinder_flag))
        ped_disk_set_flag(disk, cylinder_flag, 0);

    PedConstraint *constraint = ped_constraint_any(device);
    PedGeometry *max_geometry = ped_disk_get_max_partition_geometry(disk, part, constraint);
    start = max_geometry->start;
    end = max_geometry->length + max_geometry->start - 1;
    ped_constraint_destroy(constraint);

    PedAlignment *const start_align  = ped_alignment_new(0, ONE_MIB);
    PedAlignment *const end_align    = ped_alignment_new(-1, ONE_MIB);
    PedGeometry  *const start_range  = ped_geometry_new(device, start, max_geometry->length);
    PedGeometry  *const end_range    = ped_geometry_new(device, start, max_geometry->length);

    constraint = ped_constraint_new(start_align, end_align,
                                    start_range, end_range,
                                    1, max_geometry->length);

    ped_geometry_destroy(max_geometry);
    max_geometry = ped_disk_get_max_partition_geometry(disk, part, constraint);
    start = max_geometry->start;
    end = max_geometry->length + max_geometry->start - 1;

    // Don't return a size smaller than partition already is
    if (start > old_start)
        start = old_start;
    if (end < old_end)
        end = old_end;

    ped_constraint_destroy(constraint);
    ped_geometry_destroy(max_geometry);
}

