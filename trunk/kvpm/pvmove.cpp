/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "pvmove.h"

#include "logvol.h"
#include "masterlist.h"
#include "misc.h"
#include "processprogress.h"
#include "physvol.h"
#include "pvgroupbox.h"
#include "volgroup.h"

#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>

#include <QCheckBox>
#include <QDebug>
#include <QGroupBox>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QString>
#include <QStringList>
#include <QRadioButton>
#include <QVBoxLayout>


struct NameAndRange {
    QString   name;        // Physical volume name
    QString   name_range;  // name + range of extents  ie: /dev/sda1:10-100 and just name if no range specified
    uint64_t  start;       // Starting extent
    uint64_t  end;         // Last extent
    long long used;        // extents in use
};


bool restart_pvmove()
{
    const QStringList args = QStringList() << "pvmove";
    const QString message = i18n("Do you wish to restart all interrupted physical volume moves?");

    if (KMessageBox::questionYesNo(nullptr, message) == KMessageBox::Yes) {
        ProcessProgress resize(args);
        return true;
    } else {
        return false;
    }
}

bool stop_pvmove()
{
    const QStringList args = QStringList() << "pvmove" << "--abort";
    const QString message = i18n("Do you wish to abort all physical volume moves currently in progress?");

    if (KMessageBox::questionYesNo(nullptr, message) == KMessageBox::Yes) {
        ProcessProgress resize(args);
        return true;
    } else {
        return false;
    }
}


// Whole pv move

PVMoveDialog::PVMoveDialog(PhysVol *const physicalVolume, QWidget *parent) : 
    KvpmDialog(parent)
{
    m_vg = physicalVolume->getVg();
    m_move_segment = false;

    const QString name = physicalVolume->getName();
    const LogVolList lvs = m_vg->getLogicalVolumes();

    NameAndRange *nar = new NameAndRange;
    nar->name = name;
    nar->name_range = name;
    nar->used = (physicalVolume->getSize() - physicalVolume->getRemaining()) / m_vg->getExtentSize();
    m_sources.append(nar);

    QList<PhysVol *> target_pvs = removeFullTargets(m_vg->getPhysicalVolumes());

    if (m_sources.size() == 1)
        target_pvs = removeForbiddenTargets(target_pvs, m_sources[0]->name);

    if (!hasMovableExtents()){
        preventExec();
        KMessageBox::sorry(nullptr, i18n("None of the extents on this volume can be moved"));
    }

    if (willExec())
        buildDialog(target_pvs);
}


// pv move only on one lv

PVMoveDialog::PVMoveDialog(LogVol *const logicalVolume, int const segment, QWidget *parent) :
    KvpmDialog(parent),
    m_lv(logicalVolume),
    m_segment(segment)
{
    m_vg = m_lv->getVg();

    if (!hasMovableExtents()){
        preventExec();
        KMessageBox::sorry(nullptr, i18n("None of the extents on this volume can be moved"));
    } else {
        QList<PhysVol *> target_pvs = removeFullTargets(m_vg->getPhysicalVolumes());
        
        if (segment >= 0) {
            setupSegmentMove(segment);
            m_move_segment = true;
            if (m_sources.size() == 1)
                target_pvs = removeForbiddenTargets(target_pvs, m_sources[0]->name);
        } else {
            setupFullMove();
            m_move_segment = false;
            if (m_sources.size() == 1)
                target_pvs = removeForbiddenTargets(target_pvs, m_sources[0]->name);
        }

        if (willExec()) {
            buildDialog(target_pvs);

            connect(this, SIGNAL(okClicked()),
                    this, SLOT(commit()));
        }
    }
}

PVMoveDialog::~PVMoveDialog()
{
    for (int x = 0; x < m_sources.size(); x++)
        delete m_sources[x];
}

QList<PhysVol *> PVMoveDialog::removeFullTargets(QList<PhysVol *> targets)
{
    for (int x = targets.size() - 1; x >= 0; --x) {
        if (targets[x]->getRemaining() <= 0 || !targets[x]->isAllocatable())
            targets.removeAt(x);
    }

    /* If there is only one physical volume in the group or they are
       all full then a pv move will have no place to go */

    if (targets.isEmpty()) {
        KMessageBox::sorry(nullptr, i18n("There are no allocatable physical volumes with space to move to"));
        preventExec();
    }

    return targets;
}

void PVMoveDialog::buildDialog(QList<PhysVol *> targets)
{
    bool use_si_units;
    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", use_si_units, false);

    KLocale::BinaryUnitDialect dialect;
    KLocale *const locale = KGlobal::locale();

    if (use_si_units)
        dialect = KLocale::MetricBinaryDialect;
    else
        dialect = KLocale::IECBinaryDialect;

    setCaption(i18n("Move Physical Extents"));
    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout;

    layout->addWidget(bannerWidget());
    layout->addSpacing(5);

    QGroupBox *const source_group = new QGroupBox(i18n("Source Physical Volumes"));
    QVBoxLayout *const source_layout = new QVBoxLayout;
    QGridLayout *const radio_layout = new QGridLayout();
    source_layout->addLayout(radio_layout);
    source_group->setLayout(source_layout);
    layout->addWidget(source_group);
    QHBoxLayout *const lower_layout = new QHBoxLayout;
    layout->addLayout(lower_layout);

    QList<QSharedPointer<PvSpace>> pv_space_list;
    for (auto pv : targets) {
        pv_space_list << QSharedPointer<PvSpace>(new PvSpace(pv, pv->getRemaining(), pv->getContiguous()));
    }

    m_pv_box = new PvGroupBox(pv_space_list, m_vg->getPolicy(), NO_POLICY, true);
    lower_layout->addWidget(m_pv_box);

    const int radio_count = m_sources.size();
    NoMungeRadioButton *radio_button = nullptr;

    if (radio_count > 1) {

        m_radio_label = new QLabel();
        source_layout->addSpacing(10);
        source_layout->addWidget(m_radio_label);

        for (int x = 0; x < radio_count; x++) {

            if (m_move_segment) {
                m_pv_used_space = (1 + m_sources[x]->end - m_sources[x]->start) * m_vg->getExtentSize();
                radio_button = new NoMungeRadioButton(QString("%1  %2").arg(m_sources[x]->name_range).arg(locale->formatByteSize(m_pv_used_space, 1, dialect)));
                radio_button->setAlternateText(m_sources[x]->name);
            } else if (m_lv) {
                m_pv_used_space = m_lv->getSpaceUsedOnPv(m_sources[x]->name);
                radio_button = new NoMungeRadioButton(QString("%1  %2").arg(m_sources[x]->name).arg(locale->formatByteSize(m_pv_used_space, 1, dialect)));
                radio_button->setAlternateText(m_sources[x]->name);
            } else {
                m_pv_used_space = m_sources[x]->used;
                radio_button = new NoMungeRadioButton(QString("%1  %2").arg(m_sources[x]->name).arg(locale->formatByteSize(m_pv_used_space, 1, dialect)));
                radio_button->setAlternateText(m_sources[x]->name);
            }

            if (radio_count < 11)
                radio_layout->addWidget(radio_button, x % 5, x / 5);
            else if (radio_count % 3 == 0)
                radio_layout->addWidget(radio_button, x % (radio_count / 3), x / (radio_count / 3));
            else
                radio_layout->addWidget(radio_button, x % ((radio_count + 2) / 3), x / ((radio_count + 2) / 3));

            m_radio_buttons.append(radio_button);

            connect(radio_button, SIGNAL(toggled(bool)),
                    this, SLOT(disableTargets()));

            connect(radio_button, SIGNAL(toggled(bool)),
                    this, SLOT(setRadioExtents()));

            if (!x)
                radio_button->setChecked(true);
        }
    } else {
        source_group->setTitle(i18n("Source Physical Volume"));
        auto src = m_sources[0];
        
        if (m_move_segment) {
            m_pv_used_space = (1 + src->end - src->start) * m_vg->getExtentSize();
            radio_layout->addWidget(new QLabel(QString("%1  %2").arg(src->name_range).arg(locale->formatByteSize(m_pv_used_space, 1, dialect))));
        } else if (m_lv) {
            m_pv_used_space = m_lv->getSpaceUsedOnPv(src->name);
            radio_layout->addWidget(new QLabel(QString("%1  %2").arg(src->name).arg(locale->formatByteSize(m_pv_used_space, 1, dialect))));
        } else {
            m_pv_used_space = movableExtents() * m_vg->getExtentSize();
            radio_layout->addWidget(new QLabel(QString("%1  %2").arg(src->name).arg(locale->formatByteSize(m_pv_used_space, 1, dialect))));
            source_layout->addWidget(singleSourceWidget());
        }
    }

    dialog_body->setLayout(layout);

    connect(m_pv_box, SIGNAL(stateChanged()),
            this, SLOT(disableTargets()));

    disableTargets();
    resetOkButton();
}

void PVMoveDialog::resetOkButton()
{
    long long free_space_total = 0;

    if(m_pv_box->getEffectivePolicy() == CONTIGUOUS) {
        for (auto space : m_pv_box->getRemainingSpaceList()) {
            if (space > free_space_total)
                free_space_total = space;
        }
    } else {
        free_space_total = m_pv_box->getRemainingSpace();
    }

    long long needed_space_total = 0;
    QString pv_name;

    if (m_lv) {
        if (m_radio_buttons.size() > 1) {
            for (auto radio : m_radio_buttons) {
                if (radio->isChecked()) {
                    pv_name = radio->getAlternateText();
                    needed_space_total = m_lv->getSpaceUsedOnPv(pv_name);
                }
            }
        } else {
            pv_name = m_sources[0]->name;
            needed_space_total = m_lv->getSpaceUsedOnPv(pv_name);
        }
    } else {
        needed_space_total = m_pv_used_space;
    }

    if (free_space_total < needed_space_total)
        enableButtonOk(false);
    else
        enableButtonOk(true);
}


QStringList PVMoveDialog::arguments()
{
    QStringList args = QStringList() << "pvmove" << "--background";
    QString source;

    if (m_lv) {
        args << "--name";
        args << m_lv->getFullName();
    }

    if (m_pv_box->getPolicy() <= ANYWHERE) // don't pass INHERITED_*
        args << "--alloc" << policyToString(m_pv_box->getEffectivePolicy());

    if (m_sources.size() > 1) {
        for (int x = m_sources.size() - 1; x >= 0; x--) {
            if (m_radio_buttons[x]->isChecked())
                source = m_sources[x]->name_range;
        }
    } else {
        source = m_sources[0]->name_range;
    }

    args << source;
    args << m_pv_box->getNames(); // target(s)

    return args;
}

void PVMoveDialog::setupSegmentMove(int segment)
{
    const QStringList names = m_lv->getPvNames(segment);                      // source pv name
    const int stripes = m_lv->getSegmentStripes(segment);                     // source pv stripe count
    const long long extents = m_lv->getSegmentExtents(segment);               // extent count
    const QList<long long> starts = m_lv->getSegmentStartingExtent(segment);  // lv's first extent on pv
    NameAndRange *nar = nullptr;

    for (int x = 0; x < names.size(); ++x) {
        nar = new NameAndRange;
        nar->name  = names[x];
        nar->start = starts[x];
        nar->end   = starts[x] + (extents / stripes) - 1;
        nar->name_range = QString("%1:%2-%3").arg(nar->name).arg(nar->start).arg(nar->end);
        nar->used = 1 + (nar->end - nar->start);
        m_sources.append(nar);
    }
}

void PVMoveDialog::setupFullMove()
{
    NameAndRange *nar = nullptr;
    PhysVol *pv = nullptr;

    for (auto pvname : m_lv->getPvNamesAllFlat()) {
        nar = new NameAndRange();
        nar->name = pvname;
        nar->name_range = pvname;
        pv = m_vg->getPvByName(pvname);
        nar->used = (pv->getSize() - pv->getRemaining()) / m_vg->getExtentSize();
        m_sources.append(nar);
    }
}

void PVMoveDialog::commit()
{
    hide();
    ProcessProgress move(arguments());
    return;
}

QWidget* PVMoveDialog::bannerWidget()
{
    QWidget *const banner = new QWidget;
    QVBoxLayout *const layout = new QVBoxLayout;

    QLabel *label = new QLabel;
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);

    if (m_lv) {
        label->setText(i18n("Move only physical extents on:"));
        label = new QLabel("<b>" + m_lv->getFullName() + "</b>");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    } else {
        label->setText(i18n("Move physical extents"));
    }

    banner->setLayout(layout);

    return banner;
}

QWidget* PVMoveDialog::singleSourceWidget()
{
    bool use_si_units;
    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", use_si_units, false);
    QLabel *label = nullptr;

    KLocale::BinaryUnitDialect dialect;
    KLocale *const locale = KGlobal::locale();

    if (use_si_units)
        dialect = KLocale::MetricBinaryDialect;
    else
        dialect = KLocale::IECBinaryDialect;

    QWidget *const widget = new QWidget();
    QVBoxLayout *const layout = new QVBoxLayout();
    QGridLayout *const grid = new QGridLayout();
    grid->setColumnStretch(1,1);
    grid->setColumnStretch(2,2);
    layout->addLayout(grid);
    widget->setLayout(layout);

    label = new QLabel(i18n("Logical Volumes To Move"));
    label->setAlignment(Qt::AlignCenter);
    grid->addWidget(label, 0, 0, 1, -1);

    const QStringList lv_names = getLvNames();

    for (int x = 0; x < lv_names.size(); ++x) {
        LogVol *const lv = m_vg->getLvByName(lv_names[x]); 
        
        if (lv) {
            label = new QLabel(lv_names[x]);
            grid->addWidget(label, x + 1, 0);
            
            label = new QLabel(QString("%1").arg(locale->formatByteSize(lv->getSpaceUsedOnPv(m_sources[0]->name), 1, dialect)));
            label->setAlignment(Qt::AlignRight);
            grid->addWidget(label, x + 1, 1); 
            
            if (!isMovable(lv)) {
                label = new QLabel(i18n("<Not movable>"));
                label->setAlignment(Qt::AlignCenter);
                grid->addWidget(label, x + 1, 2);
            }
        }
    }

    layout->addSpacing(10);
    layout->addWidget(new QLabel(i18n("Movable space: %1", locale->formatByteSize(movableExtents() * m_vg->getExtentSize(), 1, dialect))));
    layout->addWidget(new QLabel(i18n("Movable extents: %1", movableExtents())));

    return widget;
}

bool PVMoveDialog::hasMovableExtents()
{
    bool movable = false;

    if (m_lv) {                       // move only lv
        movable = isMovable(m_lv);
    } else {                          // move whole pv
        for (auto name : getLvNames()) {
            LogVol *const lv = m_vg->getLvByName(name); 
            if (lv && isMovable(lv))
                movable = true;
        }
    }
    
    return movable;
}

// The names of each lv on the source pv when moving a whole pv
QStringList PVMoveDialog::getLvNames() 
{
    QList<LVSegmentExtent *> segs;
    segs = m_vg->getPvByName(m_sources[0]->name)->sortByExtent();
    QStringList lv_names;

    for (int x = 0; x < segs.size(); ++x) {
        lv_names.append(segs[x]->lv_name);
        delete segs[x];
    }

    lv_names.removeDuplicates();
    lv_names.sort();

    return lv_names;
}

void PVMoveDialog::setRadioExtents()
{
    if (m_radio_label) {
        for (auto radio : m_radio_buttons) {
            if (radio->isChecked()) {
                PhysVol *const pv = m_vg->getPvByName(radio->getAlternateText());
                const long long used = (pv->getSize() - pv->getRemaining()) / m_vg->getExtentSize();
                m_radio_label->setText(i18n("Extents: %1", used));
            }
        }
    }
}

bool PVMoveDialog::isMovable(LogVol *lv)
{
    if (lv->isThinVolume() || lv->isLvmMirror() || lv->isLvmMirrorLeg() || 
        lv->isLvmMirrorLog() || lv->isCowSnap() || lv->isCowOrigin()) {
        return false;
    } else {
        return true;
    }
}

// Number of movable extents when moving a *whole* pv
long long PVMoveDialog::movableExtents()
{
    long long extents = 0;

    for (auto name : getLvNames()) {
        LogVol *const lv = m_vg->getLvByName(name);
        if (lv && isMovable(lv))
            extents += lv->getSpaceUsedOnPv(m_sources[0]->name) / m_vg->getExtentSize();
    }

    return extents;
}

void PVMoveDialog::disableTargets()
{
    QString source;

    if (!m_radio_buttons.isEmpty()) {
        for (auto radio : m_radio_buttons) {
            if (radio->isChecked())
                source = radio->getAlternateText();
        }
    } else {
        source = m_sources[0]->name;
    }

    m_pv_box->disableChecks(getForbiddenTargets(m_lv, source));
    resetOkButton();
}

/* don't allow source and target to be the same pv
   or move a pv to another it is striped with */

QStringList PVMoveDialog::getForbiddenTargets(LogVol *const lv, const QString source)
{
    /* Once lvm2 2.02.99 or higher is released this will need
       to be tested again. What happens if parts can be moved to 
       another pv but some parts cannot? How will allocation
       policy work with raid legs during pvmove? */

    QStringList forbidden = QStringList() << source;

    if(m_pv_box->getEffectivePolicy() == ANYWHERE)
        return forbidden;

    if (lv) {
        if (lv->isRaid()) {
            if (lv->getPvNamesAllFlat().contains(source)) {

                for (auto image : lv->getRaidImageVolumes()) {
                    QStringList pvs = QStringList() << image->getPvNamesAllFlat();
                    LogVolPointer meta = image->getRaidImageMetadata();
                    
                    if (meta)
                        pvs << meta->getPvNamesAllFlat();
                    
                if (!pvs.contains(source))
                    forbidden << pvs;
                }
            }
        } else if  (lv->isThinPool()) {
            for (auto data : lv->getThinDataVolumes())
                forbidden << getForbiddenTargets(data, source);

            for (auto meta : lv->getThinMetadataVolumes())
                forbidden << getForbiddenTargets(meta, source);
        } else {
            if (lv->isRaidImage() || lv->isRaidMetadata()) { // don't allow moving pvs from one raid leg to another
                LogVolPointer image, meta;
                
                if (lv->isRaidImage()) {
                    meta = lv->getRaidImageMetadata();
                    image = lv;
                } else {
                    image = lv->getRaidMetadataImage();
                    meta = lv;
                }
                
                auto parent = lv->getParentRaid();
                if (parent) {
                    for (auto leg : parent->getRaidImageVolumes()) {
                        if (image != leg)
                            forbidden << leg->getPvNamesAllFlat();
                    }
                    
                    for (auto leg : parent->getRaidMetadataVolumes()) {
                        if (meta != leg)
                            forbidden << leg->getPvNamesAllFlat();
                    }
                }
            }

            if (m_move_segment) {
                forbidden << lv->getPvNames(m_segment);
            } else {
                for (int seg = lv->getSegmentCount() - 1; seg >= 0; --seg) {
                    if (lv->getSegmentStripes(seg) > 1 && lv->getPvNames(seg).contains(source))
                        forbidden << lv->getPvNames(seg);
                }
            }
        }
    } else { // Whole pv move
        for (auto lv : m_vg->getLogicalVolumes())
            forbidden << getForbiddenTargets(lv, source);
    }
    
    forbidden.removeDuplicates();

    return forbidden;
} 


/* if there is only one possible source then we remove
   it from the target list completely */

QList<PhysVol *> PVMoveDialog::removeForbiddenTargets(QList<PhysVol *> targets, const QString source) 
{
    QStringList forbidden = QStringList() << source;

    for (int x = targets.size() - 1 ; x >= 0; --x) {
        for (auto remove : forbidden) {
            if (targets[x]->getName() == remove)
                targets.removeAt(x);
        }
    }

    return targets;
}

