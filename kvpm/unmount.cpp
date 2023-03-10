/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "unmount.h"

#include "logvol.h"
#include "mountentry.h"
#include "mounttables.h"
#include "misc.h"
#include "storagepartition.h"
#include "volgroup.h"

#include <sys/mount.h>
#include <errno.h>
#include <string.h>

#include <KLocalizedString>
#include <KMessageBox>

#include <QLabel>
#include <QVBoxLayout>



UnmountDialog::UnmountDialog(LogVol *const volume, QWidget *parent) : 
    KvpmDialog(parent)
{
    const MountList entries = volume->getMountEntries();
    const QString name = volume->getName();

    buildDialog(name, entries);
}

UnmountDialog::UnmountDialog(StoragePartition *const partition, QWidget *parent) : 
    KvpmDialog(parent)
{
    const MountList entries = partition->getMountEntries();
    const QString name = partition->getName();

    buildDialog(name, entries);
}

void UnmountDialog::buildDialog(QString const device, const MountList entries)
{
    if (entries.isEmpty()) {
        preventExec();
        hide();
        KMessageBox::sorry(nullptr, i18n("Can not unmount: <b>%1</b>, it does not seem to be mounted", device));
        return;
    } else if (entries.size() == 1) {
        m_single = true;
        if (entries[0]->getMountPosition() > 1) {
            preventExec();
            hide();
            KMessageBox::sorry(nullptr, i18n("Can not unmount: <b>%1</b>, another volume or "
                                             "device is mounted over the same "
                                             "mount point and must be unmounted first", device));
            return;
        }
    } else {
        m_single = false;
        bool can_unmount = false;
        QListIterator<MountPtr> entry(entries);
        while (entry.hasNext()) {
            if (entry.next()->getMountPosition() < 2)
                can_unmount = true;
        }
        if (!can_unmount) {
            preventExec();
            KMessageBox::sorry(nullptr, i18n("Can not unmount: <b>%1</b>, another volume or "
                                             "device is mounted over the same "
                                             "mount point and must be unmounted first", device));
            return;
        }
    }

    setCaption(i18n("Unmount Filesystem"));

    m_mp = entries[0]->getMountPoint();
    NoMungeCheck *check;
    bool checks_disabled = false;
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QLabel *label;
    QVBoxLayout *layout = new QVBoxLayout();
    dialog_body->setLayout(layout);

    label = new QLabel(i18n("<b>Unmount Filesystem</b>"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    layout->addSpacing(10);

    if (m_single) {
        setButtons(KDialog::Yes | KDialog::No);

        layout->addWidget(new QLabel(i18n("<b>%1</b> is mounted on: <b>%2</b>", device, m_mp)));
        layout->addWidget(new QLabel(i18n("Do you wish to unmount it?")));
    } else {
        setButtons(KDialog::Ok | KDialog::Cancel);
        enableButtonOk(false);

        label = new QLabel(i18n("<b>%1</b> is mounted at multiple locatations.", device));
        layout->addWidget(label);
        label = new QLabel(i18n("Select the ones to unmount:"));
        layout->addWidget(label);

        for (int x = 0; x < entries.size(); ++x) {
            check = new NoMungeCheck(entries[x]->getMountPoint());
            if (entries[x]->getMountPosition() > 1) {
                check->setChecked(false);
                check->setEnabled(false);
                checks_disabled = true;
            }
            layout->addWidget(check);
            m_check_list.append(check);
            connect(check, SIGNAL(clicked()), this, SLOT(resetOkButton()));
        }

        if (checks_disabled) {

            label = new QLabel(i18n("Some selections have been disabled. Another "
                                    "device or volume is mounted over the same "
                                    "mount point and must be unmounted first"));

            label->setWordWrap(true);
            layout->addWidget(label);
        }
    }
}

void UnmountDialog::resetOkButton()
{
    bool enable = false;

    QListIterator<NoMungeCheck *> itr(m_check_list);
    while (itr.hasNext()) {
        if (itr.next()->isChecked())
            enable = true;
    }

    enableButtonOk(enable);
}

void UnmountDialog::commit()
{
    NoMungeCheck *cb;
    QListIterator<NoMungeCheck *> cb_itr(m_check_list);
    QByteArray mp_qba;

    hide();

    if (m_single) {

        mp_qba = m_mp.toLocal8Bit();

        if (umount2(mp_qba.data(), 0)) {
            KMessageBox::error(nullptr, i18n("Unmounting <b>%1</b> failed with error number: %2 %3",
                                             m_mp, errno, QString(strerror(errno))));
        } else {
            MountTables::removeEntry(m_mp);
        }
    } else {
        while (cb_itr.hasNext()) {
            cb = cb_itr.next();

            if (cb->isChecked()) {
                mp_qba = cb->getUnmungedText().toLocal8Bit();

                if (umount2(mp_qba.data(), 0)) {
                    KMessageBox::error(nullptr, i18n("Unmounting <b>%1</b> failed with error number: %2 %3",
                                                     m_mp, errno, QString(strerror(errno))));
                } else {
                    MountTables::removeEntry(cb->getUnmungedText());
                }
            }
        }
    }
}

