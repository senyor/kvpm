/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "lvremove.h"

#include <KMessageBox>
#include <KLocale>

#include <QDebug>
#include <QLabel>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QVBoxLayout>

#include "logvol.h"
#include "processprogress.h"


LVRemoveDialog::LVRemoveDialog(LogVol *const lv, QWidget *parent) : KDialog(parent), m_lv(lv)
{
    setButtons(KDialog::Yes | KDialog::No);
    setDefaultButton(KDialog::No);
    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);

    m_name = m_lv->getName();
    m_bailout = false;

    QHBoxLayout *const layout = new QHBoxLayout();
    QVBoxLayout *const right_layout = new QVBoxLayout();

    QLabel *const icon_label = new QLabel();
    icon_label->setPixmap(KIcon("dialog-warning").pixmap(64, 64));
    layout->addWidget(icon_label);
    layout->addLayout(right_layout);

    QLabel *label;

    if(m_lv->isThinPool()) {
        setCaption(i18n("Delete Thin Pool"));
        label = new QLabel("<b>Confirm Thin Pool Deletion</b>");
    } else {
        setCaption(i18n("Delete Volume"));
        label = new QLabel("<b>Confirm Volume Deletion</b>");
    }

    label->setAlignment(Qt::AlignCenter);
    right_layout->addWidget(label);
    right_layout->addSpacing(20);

    QStringList children(getDependentChildren(m_lv));
    children.sort();
    children.removeDuplicates(); 

    if (children.isEmpty()) {
        if(m_lv->isThinPool()) {
            right_layout->addWidget(new QLabel(i18n("Delete the thin pool named: %1?", "<b>" + m_name + "</b>")));
        } else {
            right_layout->addWidget(new QLabel(i18n("Delete the volume named: %1?", "<b>" + m_name + "</b>")));
            right_layout->addWidget(new QLabel(i18n("Any data on it will be lost.")));
        }
    } else {
        if(m_lv->isThinPool())
            right_layout->addWidget(new QLabel(i18n("The thin pool: <b>%1</b> has dependent volumes.", m_name)));
        else
            right_layout->addWidget(new QLabel(i18n("The volume: <b>%1</b> has dependent volumes.", m_name)));

        right_layout->addWidget(new QLabel(i18n("The following volumes will all be deleted:")));
        right_layout->addSpacing(10);

        QWidget *const list = new QWidget();
        QVBoxLayout *const list_layout = new QVBoxLayout();
        list->setLayout(list_layout);

        label = new QLabel("<b>" + m_name + "</b>");
        label->setAlignment(Qt::AlignLeft);
        list_layout->addWidget(label);
        list_layout->addSpacing(10);

        for (int x = 0; x < children.size(); x++) {
            label = new QLabel("<b>" + children[x] + "</b>");
            label->setAlignment(Qt::AlignLeft);
            list_layout->addWidget(label);
        }

        QScrollArea *const scroll = new QScrollArea();
        scroll->setWidget(list);
        right_layout->addWidget(scroll);

        right_layout->addSpacing(10);
        right_layout->addWidget(new QLabel(i18n("Are you certain you want to delete these volumes?")));
        right_layout->addWidget(new QLabel(i18n("Any data on them will be lost.")));
    }

    dialog_body->setLayout(layout);

    connect(this, SIGNAL(yesClicked()),
            this, SLOT(commitChanges()));

    if (m_bailout) {
        hide();
        KMessageBox::error(this, i18n("A snapshot of this origin is busy or mounted. It can not be deleted."));
    }
}

QStringList LVRemoveDialog::getDependentChildren(LogVol *const lv)
{
    QStringList children;

    if (lv->isCowOrigin()) {
        QListIterator<LogVol *> snap_itr(lv->getSnapshots());
        LogVol *snap;
        while (snap_itr.hasNext()) {
            snap = snap_itr.next();

            if (snap->isMounted())
                m_bailout = true;

            children.append(snap->getName());
        }
    } else if (lv->isThinPool()){
        QListIterator<LogVol *> thin_itr(lv->getThinVolumes());
        LogVol *thin;
        while (thin_itr.hasNext()) {
            thin = thin_itr.next();

            if (thin->isMounted())
                m_bailout = true;

            children.append(thin->getName());

            if (thin->isCowOrigin())
                children.append(getDependentChildren(thin));
        }
    }

    return children;
}

bool LVRemoveDialog::bailout()
{
    return m_bailout;
}

void LVRemoveDialog::commitChanges()
{
    const QString full_name = m_lv->getFullName().remove('[').remove(']');
    QStringList args;

    if (m_lv->isActive() && !m_lv->isCowSnap()) {
        args << "lvchange"
             << "-an"
             << full_name;

        ProcessProgress deactivate(args);
    }

    args.clear();
    args << "lvremove"
         << "--force"
         << full_name;

    ProcessProgress remove(args);

    return;
}

