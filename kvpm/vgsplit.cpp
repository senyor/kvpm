/*
 *
 *
 * Copyright (C) 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "vgsplit.h"

#include "masterlist.h"
#include "misc.h"
#include "physvol.h"
#include "processprogress.h"
#include "volgroup.h"

#include <KDialog>
#include <KLineEdit>
#include <KListWidget>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>
#include <KTabWidget>

#include <QCheckBox>
#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QRegExpValidator>
#include <QTableWidget>
#include <QVBoxLayout>


VGSplitDialog::VGSplitDialog(VolGroup *volumeGroup, QWidget *parent) : KDialog(parent), m_vg(volumeGroup)
{
    QList<PhysVol *> pv_list = m_vg->getPhysicalVolumes();

    setWindowTitle(i18n("Split Volume Group"));

    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout();
    dialog_body->setLayout(layout);
    QLabel *const name_label = new QLabel(i18n("<b>Volume group to split: %1</b>", m_vg->getName()));
    name_label->setAlignment(Qt::AlignCenter);
    layout->addWidget(name_label);

    m_new_vg_name = new KLineEdit();
    QRegExp rx("[0-9a-zA-Z_\\.][-0-9a-zA-Z_\\.]*");
    m_validator = new QRegExpValidator(rx, m_new_vg_name);
    m_new_vg_name->setValidator(m_validator);
    QLabel *const new_name_label = new QLabel(i18n("New Volume Group Name"));
    new_name_label->setBuddy(m_new_vg_name);
    QHBoxLayout *const new_name_layout = new QHBoxLayout();
    new_name_layout->addStretch();
    new_name_layout->addWidget(new_name_label);
    new_name_layout->addWidget(m_new_vg_name);
    new_name_layout->addStretch();
    layout->addLayout(new_name_layout);

    connect(m_new_vg_name, SIGNAL(textEdited(QString)),
            this, SLOT(validateOK()));

    connect(this, SIGNAL(okClicked()),
            this, SLOT(deactivate()));

    KTabWidget *const tw = new KTabWidget();
    layout->addWidget(tw);

    QStringList mobile_lv_names, fixed_lv_names, mobile_pv_names, fixed_pv_names;

    volumeMobility(mobile_lv_names, fixed_lv_names, mobile_pv_names, fixed_pv_names);

    tw->addTab(buildLvLists(mobile_lv_names, fixed_lv_names), i18n("Logical volume view"));
    tw->addTab(buildPvLists(mobile_pv_names, fixed_pv_names), i18n("Physical volume view"));

    enableLvArrows();
    enablePvArrows();

    validateOK();
    setMinimumWidth(400);

    connect(this, SIGNAL(okClicked()),
            this, SLOT(commitChanges()));
}

void VGSplitDialog::validateOK()
{
    QString name = m_new_vg_name->text();
    int pos = 0;
    bool original_vg = false;
    bool new_vg = false;

    if (m_validator->validate(name, pos) == QValidator::Acceptable && name != "." && name != "..") {

        if (m_left_pv_list->count())  // Must have at least one pv in old group and one in new group
            original_vg = true;

        if (m_right_pv_list->count())
            new_vg = true;

        if (new_vg && original_vg)
            enableButtonOk(true);
        else
            enableButtonOk(false);
    } else
        enableButtonOk(false);
}

void VGSplitDialog::commitChanges()
{
    QStringList args = QStringList() << "vgsplit" << m_vg->getName() << m_new_vg_name->text(); 

    for (int x = m_right_pv_list->count() - 1; x >= 0; x--)
        args << m_right_pv_list->item(x)->data(Qt::DisplayRole).toString();

    ProcessProgress vgsplit(args);
}

void VGSplitDialog::deactivate()
{
    QStringList moving_lvs;
    const QByteArray vg_name = m_vg->getName().toLocal8Bit();
    lvm_t lvm = MasterList::getLvm();
    vg_t vg_dm;
    dm_list *lv_dm_list;
    lvm_lv_list *lv_list;
    QList<lv_t> lvs_to_deactivate;

    for (int x = m_right_lv_list->count() - 1; x >= 0; x--)
        moving_lvs << m_right_lv_list->item(x)->data(Qt::DisplayRole).toString();
    qDebug() << "Move --->" << moving_lvs;
    if ((vg_dm = lvm_vg_open(lvm, vg_name.data(), "w", 0x0))) {

        for (int x = 0; x < moving_lvs.size(); x++) {
            lv_dm_list = lvm_vg_list_lvs(vg_dm);
            dm_list_iterate_items(lv_list, lv_dm_list) {
                if (QString(lvm_lv_get_name(lv_list->lv)).trimmed() == moving_lvs[x])
                    lvs_to_deactivate.append(lv_list->lv);
            }
        }

        for (int x = 0; x < lvs_to_deactivate.size(); x++) {
            if (lvm_lv_is_active(lvs_to_deactivate[x])) {
                if (lvm_lv_deactivate(lvs_to_deactivate[x]))
                    KMessageBox::error(0, QString(lvm_errmsg(lvm)));
            }
        }
        lvm_vg_close(vg_dm);
        return;
    } else {
        KMessageBox::error(0, QString(lvm_errmsg(lvm)));
        return;
    }

    return;
}

QWidget *VGSplitDialog::buildLvLists(const QStringList mobileLvNames, const QStringList fixedLvNames)
{
    QWidget *const lv_list = new QWidget();
    QHBoxLayout *const layout = new QHBoxLayout;
    QVBoxLayout *const left_layout = new QVBoxLayout;
    QVBoxLayout *const center_layout = new QVBoxLayout;
    QVBoxLayout *const right_layout = new QVBoxLayout;
    m_lv_add = new KPushButton(KIcon("arrow-right"), i18n("Add"));
    m_lv_remove = new KPushButton(KIcon("arrow-left"), i18n("Remove"));

    m_left_lv_list = new KListWidget();
    m_right_lv_list = new KListWidget();
    m_left_lv_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_right_lv_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_left_lv_list->setSortingEnabled(true);
    m_right_lv_list->setSortingEnabled(true);

    QListWidgetItem *lv_item;

    for (int x = mobileLvNames.size() - 1; x >= 0; x--) {
        lv_item = new QListWidgetItem(mobileLvNames[x]);
        lv_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_left_lv_list->addItem(lv_item);
    }
    for (int x = fixedLvNames.size() - 1; x >= 0; x--) {
        lv_item = new QListWidgetItem(fixedLvNames[x]);
        lv_item->setFlags(Qt::NoItemFlags);
        m_left_lv_list->addItem(lv_item);
    }

    QLabel *temp_label = new QLabel(i18n("Original volume group"));
    temp_label->setAlignment(Qt::AlignCenter);
    left_layout->addWidget(temp_label);
    left_layout->addWidget(m_left_lv_list);
    layout->addLayout(left_layout);
    center_layout->addStretch();
    center_layout->addWidget(m_lv_add);
    center_layout->addWidget(m_lv_remove);
    center_layout->addStretch();
    layout->addLayout(center_layout);
    temp_label = new QLabel(i18n("New volume group"));
    temp_label->setAlignment(Qt::AlignCenter);
    right_layout->addWidget(temp_label);
    right_layout->addWidget(m_right_lv_list);
    layout->addLayout(right_layout);

    lv_list->setLayout(layout);

    connect(m_left_lv_list, SIGNAL(itemSelectionChanged()),
            this, SLOT(enableLvArrows()));

    connect(m_right_lv_list, SIGNAL(itemSelectionChanged()),
            this, SLOT(enableLvArrows()));

    connect(m_lv_add, SIGNAL(clicked()),
            this, SLOT(addLvList()));

    connect(m_lv_remove, SIGNAL(clicked()),
            this, SLOT(removeLvList()));

    return lv_list;
}

QWidget *VGSplitDialog::buildPvLists(const QStringList mobilePvNames, const QStringList fixedPvNames)
{
    QWidget *const pv_list = new QWidget();
    QHBoxLayout *const layout = new QHBoxLayout;
    QVBoxLayout *const left_layout = new QVBoxLayout;
    QVBoxLayout *const center_layout = new QVBoxLayout;
    QVBoxLayout *const right_layout = new QVBoxLayout;
    m_pv_add = new KPushButton(KIcon("arrow-right"), "Add");
    m_pv_remove = new KPushButton(KIcon("arrow-left"), "Remove");

    QListWidgetItem *pv_item;
    QStringList open_pv_names;
    QStringList closed_pv_names;

    m_left_pv_list = new KListWidget();
    m_right_pv_list = new KListWidget();
    m_left_pv_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_right_pv_list->setSelectionMode(QAbstractItemView::ExtendedSelection);
    m_left_pv_list->setSortingEnabled(true);
    m_right_pv_list->setSortingEnabled(true);

    for (int x = mobilePvNames.size() - 1; x >= 0; x--) {
        pv_item = new QListWidgetItem(mobilePvNames[x]);
        pv_item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
        m_left_pv_list->addItem(pv_item);
    }

    for (int x = fixedPvNames.size() - 1; x >= 0; x--) {
        pv_item = new QListWidgetItem(fixedPvNames[x]);
        pv_item->setFlags(Qt::NoItemFlags);
        m_left_pv_list->addItem(pv_item);
    }

    QLabel *temp_label = new QLabel(i18n("Original volume group"));
    temp_label->setAlignment(Qt::AlignCenter);
    left_layout->addWidget(temp_label);
    left_layout->addWidget(m_left_pv_list);
    layout->addLayout(left_layout);

    center_layout->addStretch();
    center_layout->addWidget(m_pv_add);
    center_layout->addWidget(m_pv_remove);
    center_layout->addStretch();
    layout->addLayout(center_layout);

    temp_label = new QLabel(i18n("New volume group"));
    temp_label->setAlignment(Qt::AlignCenter);
    right_layout->addWidget(temp_label);
    right_layout->addWidget(m_right_pv_list);
    layout->addLayout(right_layout);

    pv_list->setLayout(layout);

    connect(m_left_pv_list, SIGNAL(itemSelectionChanged()),
            this, SLOT(enablePvArrows()));

    connect(m_right_pv_list, SIGNAL(itemSelectionChanged()),
            this, SLOT(enablePvArrows()));

    connect(m_pv_add, SIGNAL(clicked()),
            this, SLOT(addPvList()));

    connect(m_pv_remove, SIGNAL(clicked()),
            this, SLOT(removePvList()));

    return pv_list;
}

void VGSplitDialog::enableLvArrows()
{
    if (m_left_lv_list->selectedItems().size())
        m_lv_add->setEnabled(true);
    else
        m_lv_add->setEnabled(false);

    if (m_right_lv_list->selectedItems().size())
        m_lv_remove->setEnabled(true);
    else
        m_lv_remove->setEnabled(false);
}

void VGSplitDialog::enablePvArrows()
{
    if (m_left_pv_list->selectedItems().size())
        m_pv_add->setEnabled(true);
    else
        m_pv_add->setEnabled(false);

    if (m_right_pv_list->selectedItems().size())
        m_pv_remove->setEnabled(true);
    else
        m_pv_remove->setEnabled(false);
}

void VGSplitDialog::addLvList()
{
    moveNames(true, m_left_lv_list, m_right_lv_list, m_left_pv_list, m_right_pv_list);
}

void VGSplitDialog::removeLvList()
{
    moveNames(true, m_right_lv_list, m_left_lv_list, m_right_pv_list, m_left_pv_list);
}

void VGSplitDialog::addPvList()
{
    moveNames(false, m_left_lv_list, m_right_lv_list, m_left_pv_list, m_right_pv_list);
}

void VGSplitDialog::removePvList()
{
    moveNames(false, m_right_lv_list, m_left_lv_list, m_right_pv_list, m_left_pv_list);
}

void VGSplitDialog::moveNames(const bool isLvMove,
                              KListWidget *const lvSource, KListWidget *const lvTarget,
                              KListWidget *const pvSource, KListWidget *const pvTarget)
{
    QList<QListWidgetItem *> selected_items;
    QList<QListWidgetItem *> moving_items;
    QStringList moving_pv_names, moving_lv_names;
    QStringList pv_names, lv_names;
    QString name;

    if (isLvMove) {
        selected_items = lvSource->selectedItems();

        for (int x = selected_items.size() - 1; x >= 0; x--) {
            name = lvSource->item(lvSource->row(selected_items[x]))->data(Qt::DisplayRole).toString();
            movesWithVolume(true, name, pv_names, lv_names);
            moving_pv_names.append(pv_names);
            moving_lv_names.append(lv_names);
        }

        moving_pv_names.removeDuplicates();
        moving_lv_names.removeDuplicates();
    } else {
        selected_items = pvSource->selectedItems();

        for (int x = selected_items.size() - 1; x >= 0; x--) {
            name = pvSource->item(pvSource->row(selected_items[x]))->data(Qt::DisplayRole).toString();
            movesWithVolume(false, name, pv_names, lv_names);
            moving_pv_names.append(pv_names);
            moving_lv_names.append(lv_names);
        }

        moving_pv_names.removeDuplicates();
        moving_lv_names.removeDuplicates();
    }

    for (int x = moving_lv_names.size() - 1; x >= 0; x--) {

        moving_items = lvSource->findItems(moving_lv_names[x], Qt::MatchExactly);

        for (int y = moving_items.size() - 1; y >= 0; y--) {
            lvSource->takeItem(lvSource->row(moving_items[y]));
            lvTarget->addItem(moving_items[y]);
        }
    }

    for (int x = moving_pv_names.size() - 1; x >= 0; x--) {

        moving_items = pvSource->findItems(moving_pv_names[x], Qt::MatchExactly);

        for (int y = moving_items.size() - 1; y >= 0; y--) {
            pvSource->takeItem(pvSource->row(moving_items[y]));
            pvTarget->addItem(moving_items[y]);
        }
    }

    validateOK();
}

void VGSplitDialog::volumeMobility(QStringList &mobileLvNames, QStringList &fixedLvNames,
                                   QStringList &mobilePvNames, QStringList &fixedPvNames)
{
    const LogVolList lvs = getFullLvList();
    const QList<PhysVol *> pvs = m_vg->getPhysicalVolumes();

    bool growing = true;
    int list_size = 0;

    mobileLvNames.clear();
    mobilePvNames.clear();
    fixedLvNames.clear();
    fixedPvNames.clear();

    pvState(fixedPvNames, mobilePvNames);

    QStringList lv_pv_names;

    while (growing) {

        growing = false;

        for (int x = lvs.size() - 1; x >= 0 ; x--) {
            lv_pv_names = getPvs(lvs[x]);

            for (int y = lv_pv_names.size() - 1; y >= 0; y--) {
                for (int z = fixedPvNames.size() - 1; z >= 0; z--) {
                    if (fixedPvNames[z] == lv_pv_names[y]) {
                        fixedPvNames.append(lv_pv_names);
                        fixedLvNames.append(lvs[x]->getName());
                        break;
                    }
                }
            }
        }

        fixedLvNames.removeDuplicates();
        fixedPvNames.removeDuplicates();

        if (fixedLvNames.size() + fixedPvNames.size() > list_size) {
            growing = true;
            list_size = fixedLvNames.size() + fixedPvNames.size();
        }
    }

    fixedLvNames.sort();
    fixedPvNames.sort();
    mobileLvNames.clear();
    mobilePvNames.clear();

    for (int x = lvs.size() - 1; x >= 0; x--)
        mobileLvNames.append(lvs[x]->getName());

    for (int x = mobileLvNames.size() - 1; x >= 0; x--) {
        for (int y = fixedLvNames.size() - 1; y >= 0; y--) {
            if (mobileLvNames[x] == fixedLvNames[y]) {
                mobileLvNames.removeAt(x);
                break;
            }
        }
    }

    for (int x = pvs.size() - 1; x >= 0; x--)
        mobilePvNames.append(pvs[x]->getName());

    for (int x = mobilePvNames.size() - 1; x >= 0; x--) {
        for (int y = fixedPvNames.size() - 1; y >= 0; y--) {
            if (mobilePvNames[x] == fixedPvNames[y]) {
                mobilePvNames.removeAt(x);
                break;
            }
        }
    }

    mobileLvNames.removeDuplicates();
    mobilePvNames.removeDuplicates();
    mobileLvNames.sort();
    mobilePvNames.sort();
}

void VGSplitDialog::pvState(QStringList &open, QStringList &closed)
{
    const LogVolList  lvs = getFullLvList();

    for (int x = lvs.size() - 1; x >= 0; x--) {
        if (lvs[x]->isOpen()) {
            open.append(lvs[x]->getPvNamesAllFlat());

            if (lvs[x]->isSnapContainer()) {
                const LogVolList snaps(lvs[x]->getSnapshots());

                for (int y = snaps.size() - 1; y >= 0; y--) {
                    if (snaps[y]->isOpen()) {
                        open.append(lvs[x]->getPvNamesAllFlat());   // if any snap is open the whole container is open
                        break;
                    }
                }
            }

            if (lvs[x]->isThinVolume()) {
                LogVol *const pool = m_vg->getLvByName(lvs[x]->getPoolName());
                const LogVolList thinvols(pool->getThinVolumes());

                for (int y = thinvols.size() - 1; y >= 0; y--) {
                    if (thinvols[y]->isOpen()) {  // if any thin volume is open the whole pool is open
                        open.append(m_vg->getLvByName(lvs[x]->getPoolName())->getPvNamesAllFlat()); 
                        break;
                    }
                }
            }
        }
    }

    open.removeDuplicates();

    const QList<PhysVol *> pvs = m_vg->getPhysicalVolumes();
    QStringList all_pv_names;

    for (int x = pvs.size() - 1; x >= 0; x--)
        all_pv_names.append(pvs[x]->getName());

    for (int x = all_pv_names.size() - 1; x >= 0; x--) {
        for (int y = open.size() - 1; y >= 0; y--) {
            if (all_pv_names[x] == open[y]) {
                all_pv_names.removeAt(x);
                break;
            }
        }
    }

    closed = all_pv_names;
}

void VGSplitDialog::movesWithVolume(const bool isLV, const QString name,
                                    QStringList &movingPvNames, QStringList &movingLvNames)
{
    const LogVolList  lvs = getFullLvList();
    const QList<PhysVol *> pvs = m_vg->getPhysicalVolumes();
    LogVol *temp;
    bool growing = true;
    bool moving = true;
    int moving_lv_count;
    int moving_pv_count;
    movingPvNames.clear();
    movingLvNames.clear();

    if (isLV) {
        moving_lv_count = 1;
        moving_pv_count = 0;
        temp = m_vg->getLvByName(name);

        if (temp->isCowOrigin() && (temp->getParent() != NULL))
            movingPvNames = temp->getParent()->getPvNamesAllFlat();
        else if (temp->isThinVolume())
            movingPvNames = m_vg->getLvByName(temp->getPoolName())->getPvNamesAllFlat();
        else
            movingPvNames = temp->getPvNamesAllFlat();
    } else {
        moving_lv_count = 0;
        moving_pv_count = 1;
        movingPvNames.append(name);
    }

    QStringList lv_pv_names;

    while (growing) {
        for (int x = lvs.size() - 1; x >= 0 ; x--) {
            lv_pv_names = getPvs(lvs[x]);
            moving = false;

            for (int y = lv_pv_names.size() - 1; y >= 0; y--) {
                for (int z = movingPvNames.size() - 1; z >= 0; z--) {
                    if (movingPvNames[z] == lv_pv_names[y]) {
                        moving = true;
                        break;
                    }
                }
            }
            if (moving)
                movingLvNames.append(lvs[x]->getName());
        }

        for (int x = movingLvNames.size() - 1; x >= 0; x--) {
            temp = m_vg->getLvByName(movingLvNames[x]);

            if (temp->isCowOrigin() && (temp->getParent() != NULL))
                movingPvNames.append(temp->getParent()->getPvNamesAllFlat());
            else
                movingPvNames.append(temp->getPvNamesAllFlat());
        }

        movingLvNames.removeDuplicates();
        movingPvNames.removeDuplicates();

        if ((movingLvNames.size() > moving_lv_count) || (movingPvNames.size() > moving_pv_count))
            growing = true;
        else
            growing = false;

        moving_lv_count = movingLvNames.size();
        moving_pv_count = movingPvNames.size();
    }
}

bool VGSplitDialog::bailout()
{
    if (m_vg->getPhysicalVolumes().size() < 2) {
        KMessageBox::error(0, i18n("A volume group must have at least two physical volumes to split group"));
        return true;
    } else {
        return false;
    }
}

LogVolList VGSplitDialog::getFullLvList()
{
    LogVolList  lvs = m_vg->getLogicalVolumes();

    for (int x = lvs.size() - 1; x >= 0; x--) { // find and list any thin volumes 
        if (lvs[x]->isThinPool())
            lvs.append(lvs[x]->getThinVolumes());
    }

    for (int x = lvs.size() - 1; x >= 0; x--) { // find and list any snapshots 
        if (lvs[x]->isCowOrigin()) {
            LogVolList  snaps(lvs[x]->getSnapshots());
            for (int y = snaps.size() - 1; y >= 0; y--) {
                if (snaps[y]->isCowSnap())
                    lvs.append(snaps[y]);
            }
        }
    }

    return lvs;
}

// Returns all the pvs associated with this volume including the pvs
// of any snapshots under it and for the pool if it is a thin volume.

QStringList VGSplitDialog::getPvs(LogVol *const lv)
{
    QStringList names;

    if (lv->isThinVolume())
        names = m_vg->getLvByName(lv->getPoolName())->getPvNamesAllFlat();
    else
        names = lv->getPvNamesAllFlat();
    
    if (lv->isCowOrigin()) {
        if (lv->getParent() != NULL && !lv->isSnapContainer())
            names.append(lv->getParent()->getPvNamesAllFlat());
        else
            names.append(lv->getPvNamesAllFlat());
    }
    
    names.removeDuplicates();

    return names;
}
