/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2013, 2014, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "pvgroupbox.h"

#include "misc.h"
#include "physvol.h"
#include "storagebase.h"
#include "volgroup.h"

#include <KConfigSkeleton>
#include <KGlobal>

#include <QPushButton>




namespace {
    bool isLessThan(const StorageBase *const dev1 ,const StorageBase *const dev2)
    {
        return dev1->getName() < dev2->getName();
    }
}



PvGroupBox::PvGroupBox(QList<QSharedPointer<PvSpace>> spaceList, 
                       AllocationPolicy policy, AllocationPolicy vgpolicy, 
                       bool target, QWidget *parent) : 
    QGroupBox(parent), 
    m_target(target)
{
    for (auto space : spaceList) {
        m_pvs.append(space->pv);
        m_normal.append(space->normal);
        m_contiguous.append(space->contiguous);
    }
    
    if (m_target)
        setTitle(i18n("Target Physical Volumes"));
    else
        setTitle(i18n("Available Physical Volumes"));

    QGridLayout *const layout = new QGridLayout();
    setLayout(layout);

    NoMungeCheck *check;
    const int pv_check_count = m_pvs.size();
    m_space_label   = new QLabel;
    m_extents_label = new QLabel;

    m_dialect = getDialect();

    if (pv_check_count < 1) {
        m_extent_size = 1;
        QLabel *pv_label = new QLabel(i18n("<b>No suitable volumes found</b>"));
        layout->addWidget(pv_label);
    } else if (pv_check_count < 2) {
        m_extent_size = m_pvs[0]->getVg()->getExtentSize();
        QLabel *pv_label = new QLabel(m_pvs[0]->getMapperName() + "  " + KFormat().formatByteSize(m_pvs[0]->getRemaining(), 1, m_dialect));
        layout->addWidget(pv_label, 0, 0, 1, -1);
        
        addLabelsAndButtons(layout, pv_check_count, policy, vgpolicy);
        calculateSpace();
    } else {
        m_extent_size = m_pvs[0]->getVg()->getExtentSize();
        for (int x = 0; x < pv_check_count; x++) {
            
            check = new NoMungeCheck(m_pvs[x]->getMapperName() + "  " + KFormat().formatByteSize(m_pvs[x]->getRemaining(), 1, m_dialect));
            check->setAlternateText(m_pvs[x]->getMapperName());
            m_pv_checks.append(check);

            if (pv_check_count < 11)
                layout->addWidget(m_pv_checks[x], x % 5, x / 5);
            else if (pv_check_count % 3 == 0)
                layout->addWidget(m_pv_checks[x], x % (pv_check_count / 3), x / (pv_check_count / 3));
            else
                layout->addWidget(m_pv_checks[x], x % ((pv_check_count + 2) / 3), x / ((pv_check_count + 2) / 3));

            connect(check, SIGNAL(clicked(bool)),
                    this, SLOT(calculateSpace()));
        }

        addLabelsAndButtons(layout, pv_check_count, policy, vgpolicy);
    }

    setChecksToPolicy();
}

PvGroupBox::PvGroupBox(const QList <const StorageBase *> devices, const long long extentSize, QWidget *parent) : 
    QGroupBox(parent),
    m_devices(devices),
    m_extent_size(extentSize)
{
    if (devices.size() > 1)
        setTitle(i18n("Available Physical Volumes"));
    else
        setTitle(i18n("Physical Volume"));

    QGridLayout *const layout = new QGridLayout();
    setLayout(layout);
    QString name;
    long long size;
    int dev_count = 0;

    NoMungeCheck *check;
    qSort(m_devices.begin(), m_devices.end(), isLessThan);
    const int pv_check_count = m_devices.size();
    m_space_label   = new QLabel;
    m_extents_label = new QLabel;

    m_dialect = getDialect();

    if (pv_check_count < 1) {
        QLabel *pv_label = new QLabel(i18n("none found"));
        layout->addWidget(pv_label);
    } else if (pv_check_count < 2) {
        name = m_devices[0]->getName();
        size = m_devices[0]->getSize();

        QLabel *pv_label = new QLabel(name + "  " + KFormat().formatByteSize(size, 1, m_dialect));
        layout->addWidget(pv_label, 0, 0, 1, -1);
        addLabelsAndButtons(layout, pv_check_count, NO_POLICY, NO_POLICY);

        calculateSpace();
    } else {
        for (int x = 0; x < m_devices.size(); x++) {
            dev_count++;
            name = m_devices[x]->getName();
            size = m_devices[x]->getSize();
            check = new NoMungeCheck(name + "  " + KFormat().formatByteSize(size, 1, m_dialect));
            check->setAlternateText(name);
            check->setData(QVariant(size));
            m_pv_checks.append(check);

            if (pv_check_count < 11)
                layout->addWidget(m_pv_checks[x], x % 5, x / 5);
            else if (pv_check_count % 3 == 0)
                layout->addWidget(m_pv_checks[x], x % (pv_check_count / 3), x / (pv_check_count / 3));
            else
                layout->addWidget(m_pv_checks[x], x % ((pv_check_count + 2) / 3), x / ((pv_check_count + 2) / 3));

            connect(check, SIGNAL(clicked(bool)),
                    this, SLOT(calculateSpace()));
        }

        addLabelsAndButtons(layout, pv_check_count, NO_POLICY, NO_POLICY);
        setExtentSize(extentSize);
    }
}

QStringList PvGroupBox::getNames()
{
    QStringList names;

    if (m_pv_checks.size()) {
        for (auto check : m_pv_checks) {
            if (check->isChecked())
                names << check->getAlternateText();
        }
    } else if (m_pvs.size()) {
        names << m_pvs[0]->getMapperName();
    } else if (m_devices.size()) {
        names << m_devices[0]->getName();
    }

    return names;
}

QStringList PvGroupBox::getAllNames()
{
    QStringList names;

    if (m_pv_checks.size()) {
        for (auto check : m_pv_checks) {
            names << check->getAlternateText();
        }
    } else if (m_pvs.size()) {
        names << m_pvs[0]->getMapperName();
    } else if (m_devices.size()) {
        names << m_devices[0]->getName();
    }

    return names;
}

long long PvGroupBox::getRemainingSpace()
{
    long long space = 0;

    if (m_pv_checks.size()) {
        for (auto check : m_pv_checks) {
            if (check->isChecked())
                space += (check->getData()).toLongLong();
        }
    } else if (m_pvs.size()) {
        space = m_pvs[0]->getRemaining();
    } else if (m_devices.size()) {
        space = m_devices[0]->getSize();
    } else {
        space = 0;
    }

    return space;
}

long long PvGroupBox::getLargestSelectedSpace()
{
    long long largest = 0;
    long long space = 0;

    if (!m_pvs.isEmpty()) {
        if (m_pv_checks.size()) {
            for (auto check : m_pv_checks) {
                space = check->getData().toLongLong();
                
                if (check->isChecked() && (space > largest))
                    largest = space;
            }
        } else {
            largest = m_pvs[0]->getContiguous();
        }
    }

    return largest;
}

QList<long long> PvGroupBox::getRemainingSpaceList()
{
    QList<long long> space;

    if (m_pv_checks.size()) {
        for (auto check : m_pv_checks) {
            if (check->isChecked())
                space.append((check->getData()).toLongLong());
        }
    } else if (m_pvs.size()) {
        space.append(m_pvs[0]->getRemaining());
    } else if (m_devices.size()) {
        space.append(m_devices[0]->getSize());
    }

    return space;
}

void PvGroupBox::selectAll()
{
    for (auto check : m_pv_checks) {
        if (check->isEnabled())
            check->setChecked(true);
    }

    calculateSpace();

    return;
}

void PvGroupBox::selectNone()
{
    for (auto check : m_pv_checks)
        check->setChecked(false);

    calculateSpace();

    return;
}

void PvGroupBox::calculateSpace()
{
    if ((getEffectivePolicy() == CONTIGUOUS) && m_target) {
        m_space_label->setText(i18n("Contiguous space: %1", KFormat().formatByteSize(getLargestSelectedSpace(), 1, m_dialect)));
        m_extents_label->setText(i18n("Contiguous extents: %1", getLargestSelectedSpace() / m_extent_size));
    } else {
        m_space_label->setText(i18n("Space: %1", KFormat().formatByteSize(getRemainingSpace(), 1, m_dialect)));
        m_extents_label->setText(i18n("Extents: %1", getRemainingSpace() / m_extent_size));
    }

    emit stateChanged();

    return;
}

void PvGroupBox::setExtentSize(long long extentSize)
{
    m_extent_size = (extentSize >= 1024) ? extentSize : 1024;

    if (m_pv_checks.size()) {
        for (int x = 0; x < m_pv_checks.size(); ++x) {
            if ((m_pv_checks[x]->getData()).toLongLong() > (m_extent_size + 0xfffffLL))    // 1 MiB for MDA, fix this
                m_pv_checks[x]->setEnabled(true);                                          // when MDA size is put in
            else {                                                                         // liblvm2app
                m_pv_checks[x]->setChecked(false); 
                m_pv_checks[x]->setEnabled(false);
            }
        }
    }

    calculateSpace();
}


// Disable checkboxes that cant be used under the current settings
// such as the allocation policy that is selected.

void PvGroupBox::disableChecks(QStringList pvs)
{
    for(auto check : m_pv_checks)
        check->setEnabled(true);

    if (!pvs.isEmpty() && !m_pv_checks.isEmpty()) {
        for (auto disable_name : pvs) {
            for (int x = 0; x < m_pvs.size(); ++x) {
                if (m_pvs[x]->getMapperName() == disable_name) {
                    m_pv_checks[x]->setChecked(false);
                    m_pv_checks[x]->setEnabled(false);
                }
            }
        }
    }
}

QHBoxLayout *PvGroupBox::getButtons()
{
    QHBoxLayout *const layout = new QHBoxLayout();
    QPushButton *const all    = new QPushButton(i18n("Select all"));
    QPushButton *const none   = new QPushButton(i18n("Clear all"));

    layout->addStretch();
    layout->addWidget(all);
    layout->addStretch();
    layout->addWidget(none);
    layout->addStretch();

    connect(all,  SIGNAL(clicked(bool)), this, SLOT(selectAll()));
    connect(none, SIGNAL(clicked(bool)), this, SLOT(selectNone()));

    return layout;
}

void PvGroupBox::addLabelsAndButtons(QGridLayout *layout, int pvCount, AllocationPolicy policy, AllocationPolicy vgpolicy)
{
    QVBoxLayout *const spacer1 = new QVBoxLayout;
    QVBoxLayout *const spacer2 = new QVBoxLayout;
    spacer1->addSpacing(10);
    spacer2->addSpacing(10);
    const int count = layout->rowCount();

    layout->addLayout(spacer1,         count,     0, 1, -1);
    layout->addWidget(m_space_label,   count + 1, 0, 1, -1);
    layout->addWidget(m_extents_label, count + 2, 0, 1, -1);
    layout->addLayout(spacer2,         count + 3, 0, 1, -1);

    if (pvCount > 1)
        layout->addLayout(getButtons(), count + 4, 0, 1, -1);

    m_policy_combo = new PolicyComboBox(policy, vgpolicy);
    connect(m_policy_combo,  SIGNAL(policyChanged(AllocationPolicy)), this, SLOT(setChecksToPolicy()));
    QHBoxLayout *const policy_layout = new QHBoxLayout;
    policy_layout->addStretch();
    policy_layout->addWidget(m_policy_combo);
    policy_layout->addStretch();

    layout->addLayout(policy_layout, count + 5, 0, 1, -1);

    if(policy == NO_POLICY)
        m_policy_combo->hide();

    selectAll();
}

AllocationPolicy PvGroupBox::getPolicy()
{
    if (m_policy_combo == nullptr)
        return NORMAL;
    else
        return m_policy_combo->getPolicy();
}

AllocationPolicy PvGroupBox::getEffectivePolicy()
{
    if (m_policy_combo == nullptr)
        return NORMAL;
    else
        return m_policy_combo->getEffectivePolicy();
}

void PvGroupBox::setChecksToPolicy()
{
    if (!m_pvs.isEmpty() && m_policy_combo != nullptr) {
        const AllocationPolicy policy = getEffectivePolicy();

        for (int x = 0; x < m_pv_checks.size(); ++x) {
            if (policy == CONTIGUOUS) {
                m_pv_checks[x]->setText(m_pvs[x]->getMapperName() + "  " + KFormat().formatByteSize(m_contiguous[x], 1, m_dialect));
                m_pv_checks[x]->setData(QVariant(m_contiguous[x]));
                
            } else {
                m_pv_checks[x]->setText(m_pvs[x]->getMapperName() + "  " + KFormat().formatByteSize(m_normal[x], 1, m_dialect));
                m_pv_checks[x]->setData(QVariant(m_normal[x]));
            }
        }
    }

    calculateSpace();
}

KFormat::BinaryUnitDialect PvGroupBox::getDialect()
{
    bool use_si_units;
    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", use_si_units, false);

    KFormat::BinaryUnitDialect dialect;

    if (use_si_units)
        dialect = KFormat::MetricBinaryDialect;
    else
        dialect = KFormat::IECBinaryDialect;

    return dialect;
}
