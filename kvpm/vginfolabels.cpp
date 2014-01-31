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


#include "vginfolabels.h"

#include <KConfigSkeleton>
#include <KGlobal>
#include <KIcon>
#include <KLocale>

#include <QDebug>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "misc.h"
#include "volgroup.h"



VGInfoLabels::VGInfoLabels(VolGroup *const group, QWidget *parent) : 
    QFrame(parent)
{
    QLabel *extent_size_label, *size_label, *used_label,
           *free_label, *lvm_fmt_label, *resizable_label,
           *clustered_label, *allocatable_label,
           *max_lv_label, *max_pv_label, *policy_label, *mda_label,
           *uuid_label;

    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setLineWidth(2);

    QString clustered, resizable;
    QVBoxLayout *upper_layout = new QVBoxLayout();
    QHBoxLayout *hlayout1 = new QHBoxLayout();
    QHBoxLayout *hlayout2 = new QHBoxLayout();
    upper_layout->setMargin(0);
    hlayout1->setMargin(0);
    hlayout2->setMargin(0);
    hlayout1->setSpacing(0);
    hlayout2->setSpacing(0);
    upper_layout->addLayout(hlayout1);
    upper_layout->addLayout(hlayout2);

    QVBoxLayout *vlayout1 = new QVBoxLayout();
    QVBoxLayout *vlayout2 = new QVBoxLayout();
    QVBoxLayout *vlayout3 = new QVBoxLayout();
    QVBoxLayout *vlayout4 = new QVBoxLayout();
    QVBoxLayout *vlayout5 = new QVBoxLayout();
    QVBoxLayout *vlayout6 = new QVBoxLayout();
    QVBoxLayout *vlayout7 = new QVBoxLayout();

    QWidget *label_widget1 = new QWidget();
    QWidget *label_widget2 = new QWidget();
    QWidget *label_widget3 = new QWidget();
    QWidget *label_widget4 = new QWidget();
    QWidget *label_widget5 = new QWidget();
    QWidget *label_widget6 = new QWidget();
    QWidget *label_widget7 = new QWidget();

    label_widget1->setBackgroundRole(QPalette::Base);
    label_widget1->setAutoFillBackground(true);
    label_widget2->setBackgroundRole(QPalette::AlternateBase);
    label_widget2->setAutoFillBackground(true);
    label_widget3->setBackgroundRole(QPalette::Base);
    label_widget3->setAutoFillBackground(true);
    label_widget4->setBackgroundRole(QPalette::AlternateBase);
    label_widget4->setAutoFillBackground(true);
    label_widget5->setBackgroundRole(QPalette::Base);
    label_widget5->setAutoFillBackground(true);
    label_widget6->setBackgroundRole(QPalette::AlternateBase);
    label_widget6->setAutoFillBackground(true);
    label_widget7->setBackgroundRole(QPalette::Base);
    label_widget7->setAutoFillBackground(true);

    label_widget1->setLayout(vlayout1);
    label_widget2->setLayout(vlayout2);
    label_widget3->setLayout(vlayout3);
    label_widget4->setLayout(vlayout4);
    label_widget5->setLayout(vlayout5);
    label_widget6->setLayout(vlayout6);
    label_widget7->setLayout(vlayout7);

    if (group->isResizable())
        resizable = "Yes";
    else
        resizable = "No";

    if (group->isClustered())
        clustered = "Yes";
    else
        clustered = "No";

    KConfigSkeleton skeleton;
    bool use_si_units;

    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", use_si_units, false);

    KLocale::BinaryUnitDialect dialect;
    KLocale *const locale = KGlobal::locale();

    if (use_si_units)
        dialect = KLocale::MetricBinaryDialect;
    else
        dialect = KLocale::IECBinaryDialect;

    used_label = new QLabel(i18nc("Space used up",  "Used: %1", locale->formatByteSize(group->getUsedSpace(), 1, dialect)));
    free_label = new QLabel(i18nc("Space not used", "Free: %1", locale->formatByteSize(group->getFreeSpace(), 1, dialect)));
    size_label = new QLabel(i18nc("Total space on device", "Total: %1", locale->formatByteSize(group->getSize(), 1, dialect)));
    lvm_fmt_label   = new QLabel(i18n("Format: %1", group->getFormat()));
    policy_label    = new QLabel(i18n("Policy: %1", policyToLocalString(group->getPolicy())));
    resizable_label = new QLabel(i18n("Resizable: %1", resizable));
    clustered_label = new QLabel(i18n("Clustered: %1", clustered));
    allocatable_label = new QLabel(i18n("Allocatable: %1", locale->formatByteSize(group->getAllocatableSpace(), 1, dialect)));
    extent_size_label = new QLabel(i18n("Extent size: %1", locale->formatByteSize(group->getExtentSize(), 1, dialect)));
    mda_label         = new QLabel(i18n("MDA: %1 Used: %2", group->getMdaCount(), group->getMdaUsed()));
    uuid_label        = new QLabel(i18n("UUID: %1", group->getUuid()));
    uuid_label->setWordWrap(true);

    vlayout1->addWidget(size_label);
    vlayout1->addWidget(allocatable_label);
    vlayout2->addWidget(used_label);
    vlayout2->addWidget(free_label);
    vlayout3->addWidget(clustered_label);
    vlayout3->addWidget(lvm_fmt_label);
    vlayout4->addWidget(extent_size_label);
    vlayout4->addWidget(policy_label);
    vlayout5->addWidget(resizable_label);
    vlayout5->addWidget(mda_label);
    vlayout6->addWidget(uuid_label);

    hlayout1->addWidget(label_widget1);
    hlayout1->addWidget(label_widget2);
    hlayout1->addWidget(label_widget3);
    hlayout1->addWidget(label_widget4);
    hlayout1->addWidget(label_widget5);
    hlayout1->addWidget(label_widget6);

    if (group->getPvMax())
        max_pv_label = new QLabel(i18n("Max pvs: %1", group->getPvMax()));
    else
        max_pv_label = new QLabel(i18n("Max pvs: Unlimited"));

    if (group->getLvMax())
        max_lv_label = new QLabel(i18n("Max lvs: %1", group->getLvMax()));
    else
        max_lv_label = new QLabel(i18n("Max lvs: Unlimited"));

    vlayout7->addWidget(max_pv_label);
    vlayout7->addWidget(max_lv_label);
    hlayout1->addWidget(label_widget7);

    if (!group->getLvMax() && !group->getPvMax())
        label_widget7->hide();

    setLayout(upper_layout);
}

