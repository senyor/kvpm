/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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
};


bool restart_pvmove()
{
    const QStringList args = QStringList() << "pvmove";
    const QString message = i18n("Do you wish to restart all interrupted physical volume moves?");

    if (KMessageBox::questionYesNo(0, message) == KMessageBox::Yes) {
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

    if (KMessageBox::questionYesNo(0, message) == KMessageBox::Yes) {
        ProcessProgress resize(args);
        return true;
    } else {
        return false;
    }
}

PVMoveDialog::PVMoveDialog(PhysVol *const physicalVolume, QWidget *parent) : KDialog(parent)
{
    m_vg = physicalVolume->getVg();
    m_target_pvs = m_vg->getPhysicalVolumes();
    m_move_lv = false;
    m_move_segment = false;
    m_bailout = false;

    const QString name = physicalVolume->getName();
    const QList<LogVol *> lvs = m_vg->getLogicalVolumes();
    QStringList forbidden_targets;  // A whole pv can't be moved to a pv it is striped with along any segment
    QStringList striped_targets;

    NameAndRange *nar = new NameAndRange;
    nar->name = name;
    nar->name_range = name;
    m_sources.append(nar);

    forbidden_targets.append(name);

    for (int x = lvs.size() - 1; x >= 0; x--) {
        for (int seg = lvs[x]->getSegmentCount() - 1; seg >= 0; seg--) {
            if (lvs[x]->getSegmentStripes(seg) > 1) {
                striped_targets = lvs[x]->getPvNames(seg);
                if (striped_targets.contains(name))
                    forbidden_targets.append(striped_targets);
            }
        }
    }

    forbidden_targets.removeDuplicates();

    for (int x = m_target_pvs.size() - 1 ; x >= 0; x--) {
        for (int y = forbidden_targets.size() - 1; y >= 0; y--) {
            if (m_target_pvs[x]->getName() == forbidden_targets[y]) {
                m_target_pvs.removeAt(x);
                forbidden_targets.removeAt(y);
                break;
            }
        }
    }

    removeFullTargets();

    if (!hasMovableExtents()){
        m_bailout = true;
        KMessageBox::error(NULL, i18n("None of the extents on this volume can be moved"));
    }

    if (!m_bailout)
        buildDialog();

    connect(this, SIGNAL(okClicked()),
            this, SLOT(commitMove()));
}

PVMoveDialog::PVMoveDialog(LogVol *const logicalVolume, int const segment, QWidget *parent) :
    KDialog(parent),
    m_lv(logicalVolume)
{
    m_vg = m_lv->getVg();
    m_move_lv = true;
    m_target_pvs = m_vg->getPhysicalVolumes();
    m_bailout = false;

    if (m_lv->isThinVolume()){
        m_bailout = true;
        KMessageBox::error(NULL, i18n("Moving physical volumes is not supported on thin volumes"));
    } else {
        if (segment >= 0) {
            setupSegmentMove(segment);
            m_move_segment = true;
        } else {
            setupFullMove();
            m_move_segment = false;
        }

        /* if there is only one source physical volumes possible on this logical volume
           then we eliminate it from the possible target pv list completely. */
        
        if (m_sources.size() == 1) {
            for (int x = m_target_pvs.size() - 1; x >= 0; x--) {
                if (m_target_pvs[x]->getName() == m_sources[0]->name)
                    m_target_pvs.removeAt(x);
            }
        }
        
        /* If this is a segment move then all source pvs need to be
           removed from the target list */
        
        if (m_move_segment) {
            for (int x = m_target_pvs.size() - 1; x >= 0; x--) {
                for (int y = m_sources.size() - 1; y >= 0; y--) {
                    if (m_target_pvs[x]->getName() == m_sources[y]->name)
                        m_target_pvs.removeAt(x);
                }
            }
        }
        
        removeFullTargets();
        buildDialog();

        connect(this, SIGNAL(okClicked()),
                this, SLOT(commitMove()));
    }
}

void PVMoveDialog::removeFullTargets()
{
    for (int x = m_target_pvs.size() - 1; x >= 0; x--) {
        if (m_target_pvs[x]->getRemaining() <= 0 || !m_target_pvs[x]->isAllocatable())
            m_target_pvs.removeAt(x);
    }

    /* If there is only one physical volume in the group or they are
       all full then a pv move will have no place to go */

    if (m_target_pvs.size() < 1) {
        KMessageBox::error(NULL, i18n("There are no allocatable physical volumes with space to move to"));
        m_bailout = true;
    }
}

PVMoveDialog::~PVMoveDialog()
{
    for (int x = 0; x < m_sources.size(); x++)
        delete m_sources[x];
}

void PVMoveDialog::buildDialog()
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

    QLabel *label;
    NoMungeRadioButton *radio_button;

    setWindowTitle(i18n("Move Physical Extents"));
    QWidget *const dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout;

    label = new QLabel(i18n("<b>Move physical extents</b>"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label);
    dialog_body->setLayout(layout);

    if (m_move_lv) {
        label->setText(i18n("<b>Move only physical extents on:</b>"));
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
        label = new QLabel("<b>" + m_lv->getFullName() + "</b>");
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }
    layout->addSpacing(5);

    QGroupBox *const source_group = new QGroupBox(i18n("Source Physical Volumes"));
    QVBoxLayout *const source_layout = new QVBoxLayout;
    QGridLayout *const radio_layout = new QGridLayout();
    source_layout->addLayout(radio_layout);
    source_group->setLayout(source_layout);
    layout->addWidget(source_group);
    QHBoxLayout *const lower_layout = new QHBoxLayout;
    layout->addLayout(lower_layout);

    QList<long long> normal;
    QList<long long> contiguous;

    for (int x = 0; x < m_target_pvs.size(); x++) {
        normal.append(m_target_pvs[x]->getRemaining());
        contiguous.append(m_target_pvs[x]->getContiguous());
    }

    m_pv_box = new PvGroupBox(m_target_pvs, normal, contiguous, m_vg->getPolicy(), NO_POLICY, true);
    lower_layout->addWidget(m_pv_box);

    const int radio_count = m_sources.size();

    if (radio_count > 1) {
        for (int x = 0; x < radio_count; x++) {

            if (m_move_segment) {
                m_pv_used_space = (1 + m_sources[x]->end - m_sources[x]->start) * m_vg->getExtentSize();
                radio_button = new NoMungeRadioButton(QString("%1  %2").arg(m_sources[x]->name_range).arg(locale->formatByteSize(m_pv_used_space, 1, dialect)));
                radio_button->setAlternateText(m_sources[x]->name);
            } else if (m_move_lv) {
                m_pv_used_space = m_lv->getSpaceUsedOnPv(m_sources[x]->name);
                radio_button = new NoMungeRadioButton(QString("%1  %2").arg(m_sources[x]->name).arg(locale->formatByteSize(m_pv_used_space, 1, dialect)));
                radio_button->setAlternateText(m_sources[x]->name);
            } else {
                m_pv_used_space = m_vg->getPvByName(m_sources[x]->name)->getSize() - m_vg->getPvByName(m_sources[x]->name)->getRemaining();
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
                    this, SLOT(disableSource()));

            if (!x)
                radio_button->setChecked(true);
        }
    } else {

        source_group->setTitle(i18n("Source Physical Volume"));

        if (m_move_segment) {
            m_pv_used_space = (1 + m_sources[0]->end - m_sources[0]->start) * m_vg->getExtentSize();
            radio_layout->addWidget(new QLabel(QString("%1  %2").arg(m_sources[0]->name_range).arg(locale->formatByteSize(m_pv_used_space, 1, dialect))));
        } else if (m_move_lv) {
            m_pv_used_space = m_lv->getSpaceUsedOnPv(m_sources[0]->name);
            radio_layout->addWidget(new QLabel(QString("%1  %2").arg(m_sources[0]->name).arg(locale->formatByteSize(m_pv_used_space, 1, dialect))));
        } else {
            m_pv_used_space = m_vg->getPvByName(m_sources[0]->name)->getSize() - m_vg->getPvByName(m_sources[0]->name)->getRemaining();
            radio_layout->addWidget(new QLabel(QString("%1  %2").arg(m_sources[0]->name).arg(locale->formatByteSize(m_pv_used_space, 1, dialect))));
            source_layout->addWidget(extentWidget());
        }
    }

    connect(m_pv_box, SIGNAL(stateChanged()),
            this, SLOT(resetOkButton()));

    disableSource();

    resetOkButton();
}

void PVMoveDialog::resetOkButton()
{
    const long long free_space_total = m_pv_box->getRemainingSpace();
    long long needed_space_total = 0;
    QString pv_name;

    if (m_move_lv) {
        if (m_radio_buttons.size() > 1) {
            for (int x = 0; x < m_radio_buttons.size(); x++) {
                if (m_radio_buttons[x]->isChecked()) {
                    pv_name = m_radio_buttons[x]->getAlternateText();
                    needed_space_total = m_lv->getSpaceUsedOnPv(pv_name);
                }
            }
        } else {
            pv_name = m_sources[0]->name;
            needed_space_total = m_lv->getSpaceUsedOnPv(pv_name);
        }
    } else
        needed_space_total = m_pv_used_space;

    if (free_space_total < needed_space_total)
        enableButtonOk(false);
    else
        enableButtonOk(true);
}

void PVMoveDialog::disableSource()  // don't allow source and target to be the same pv
{
    PhysVol *source_pv = NULL;

    for (int x = m_radio_buttons.size() - 1; x >= 0; x--) {
        if (m_radio_buttons[x]->isChecked())
            source_pv = m_vg->getPvByName(m_sources[x]->name);
    }

    m_pv_box->disableOrigin(source_pv);
    resetOkButton();
}

QStringList PVMoveDialog::arguments()
{
    QStringList args = QStringList() << "pvmove" << "--background";
    QString source;

    if (m_move_lv) {
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
    NameAndRange *nar = NULL;

    for (int x = 0; x < names.size(); x++) {
        nar = new NameAndRange;
        nar->name  = names[x];
        nar->start = starts[x];
        nar->end   = starts[x] + (extents / stripes) - 1;
        nar->name_range = QString("%1:%2-%3").arg(nar->name).arg(nar->start).arg(nar->end);
        m_sources.append(nar);
    }
}

void PVMoveDialog::setupFullMove()
{
    const QStringList names = m_lv->getPvNamesAllFlat();
    NameAndRange *nar = NULL;

    for (int x = names.size() - 1; x >= 0; x--) {
        nar = new NameAndRange();
        nar->name = names[x];
        nar->name_range = names[x];
        m_sources.append(nar);
    }
}

bool PVMoveDialog::bailout()
{
    return m_bailout;
}

void PVMoveDialog::commitMove()
{
    hide();
    ProcessProgress move(arguments());
    return;
}

QWidget* PVMoveDialog::extentWidget()
{
    bool use_si_units;
    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", use_si_units, false);
    QLabel *label = NULL;

    KLocale::BinaryUnitDialect dialect;
    KLocale *const locale = KGlobal::locale();

    if (use_si_units)
        dialect = KLocale::MetricBinaryDialect;
    else
        dialect = KLocale::IECBinaryDialect;

    QWidget *const widget = new QWidget();
    QGridLayout *const layout = new QGridLayout();
    widget->setLayout(layout);

    label = new QLabel(i18n("Logical Volumes"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label, 0, 0);
    label = new QLabel(i18n("Space Used"));
    label->setAlignment(Qt::AlignCenter);
    layout->addWidget(label, 0, 1);

    QList<LVSegmentExtent *> segs;
    segs = m_vg->getPvByName(m_sources[0]->name)->sortByExtent();
    QStringList lv_names = getLvNames();

    for (int x = 0; x < lv_names.size(); x++) {
        LogVol *const lv = m_vg->getLvByName(lv_names[x]); 

        if (lv != NULL){
            label = new QLabel(lv_names[x]);
            layout->addWidget(label, x + 1, 0);

            label = new QLabel(QString("%1").arg(locale->formatByteSize(lv->getSpaceUsedOnPv(m_sources[0]->name), 1, dialect)));
            label->setAlignment(Qt::AlignRight);
            layout->addWidget(label, x + 1, 1); 

            if (lv->isLvmMirror() || lv->isLvmMirrorLeg() || lv->isLvmMirrorLog() || lv->isCowSnap() || lv->isCowOrigin()){
                label = new QLabel(i18n("<Not movable>"));
                layout->addWidget(label, x + 1, 2);
            }
        }
    }

    return widget;
}

bool PVMoveDialog::hasMovableExtents()
{
    bool movable = false;
    QStringList lv_names = getLvNames();

    for (int x = 0; x < lv_names.size(); x++) {
        LogVol *const lv = m_vg->getLvByName(lv_names[x]); 
        if (lv != NULL){
            if (!lv->isLvmMirror() && !lv->isLvmMirrorLeg() && !lv->isLvmMirrorLog() && !lv->isCowSnap() && !lv->isCowOrigin()){
                movable = true;
            }
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

    for (int x = 0; x < segs.size(); x++) {
        lv_names.append(segs[x]->lv_name);
        delete segs[x];
    }

    lv_names.removeDuplicates();
    lv_names.sort();

    return lv_names;
}
