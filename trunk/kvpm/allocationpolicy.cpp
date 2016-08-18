/*
 *
 *
 * Copyright (C) 2008, 2010, 2011, 2012, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "allocationpolicy.h"

#include <KLocalizedString>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>



QString policyToLocalString(AllocationPolicy policy)
{
    switch(policy) {
    case NO_POLICY:
        return QString("");
        break;
    case NORMAL:
        return QString(i18n("normal"));
        break;
    case CONTIGUOUS:
        return QString(i18n("contiguous"));
        break;
    case CLING:
        return QString(i18n("cling"));
        break;
    case ANYWHERE:
        return QString(i18n("anywhere"));
        break;
    case INHERIT_NORMAL:
        return QString(i18n("inherit (normal)"));
        break;
    case INHERIT_CONTIGUOUS:
        return QString(i18n("inherit (contiguous)"));
        break;
    case INHERIT_CLING:
        return QString(i18n("inherit (cling)"));
        break;
    case INHERIT_ANYWHERE:
        return QString(i18n("inherit (anywhere)"));
        break;
    default:
        return QString(i18n("normal"));
    } 
}

QString policyToString(AllocationPolicy policy)
{
    switch(policy) {
    case NO_POLICY:
        return QString("");
        break;
    case NORMAL:
        return QString("normal");
        break;
    case CONTIGUOUS:
        return QString("contiguous");
        break;
    case CLING:
        return QString("cling");
        break;
    case ANYWHERE:
        return QString("anywhere");
        break;
    case INHERIT_NORMAL:
    case INHERIT_CONTIGUOUS:
    case INHERIT_CLING:
    case INHERIT_ANYWHERE:
        return QString("inherit");
        break;
    default:
        return QString("normal");
    } 
}

PolicyComboBox::PolicyComboBox(AllocationPolicy policy, AllocationPolicy vgpolicy, QWidget *parent) 
    :  QWidget(parent), m_vg_policy(vgpolicy)
{
    QLabel *const label = new QLabel(i18n("Allocation policy: "));

    m_combo = new QComboBox();
    m_combo->addItem(i18n("Normal"));
    m_combo->addItem(i18n("Contiguous"));
    m_combo->addItem(i18n("Cling"));
    m_combo->addItem(i18n("Anywhere"));

    if (m_vg_policy != NO_POLICY)
        m_combo->addItem(i18n("Inherit (%1)", policyToString(vgpolicy)));

    if (policy == NORMAL)
        m_combo->setCurrentIndex(0);
    else if (policy == CONTIGUOUS)
        m_combo->setCurrentIndex(1);
    else if (policy == CLING)
        m_combo->setCurrentIndex(2);
    else if (policy == ANYWHERE)
        m_combo->setCurrentIndex(3);
    else if ((policy > ANYWHERE) && (m_combo->count() > 4))
        m_combo->setCurrentIndex(4);
    else
        m_combo->setCurrentIndex(0);

    QHBoxLayout *const layout = new QHBoxLayout;
    layout->addWidget(label);
    layout->addWidget(m_combo);

    connect(m_combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(emitNewPolicy()));

    setLayout(layout);
}

void PolicyComboBox::emitNewPolicy()
{
    emit policyChanged(getPolicy());
}

AllocationPolicy PolicyComboBox::getPolicy()
{
    switch(m_combo->currentIndex()) {
    case 0:
        return NORMAL;
        break;
    case 1:
        return CONTIGUOUS;
        break;
    case 2:
        return CLING;
        break;
    case 3:
        return ANYWHERE;
        break;
    case 4:
        switch(m_vg_policy) {
        case NORMAL:
            return INHERIT_NORMAL;
            break;
        case CONTIGUOUS:
            return INHERIT_CONTIGUOUS;
            break;
        case CLING:
            return INHERIT_CLING;
            break;
        case ANYWHERE:
            return INHERIT_ANYWHERE;
            break;
        default:
            return INHERIT_NORMAL;
            break;
        }
    default:
        return NORMAL;
    } 
}

AllocationPolicy PolicyComboBox::getEffectivePolicy()
{
    switch(m_combo->currentIndex()) {
    case 0:
        return NORMAL;
        break;
    case 1:
        return CONTIGUOUS;
        break;
    case 2:
        return CLING;
        break;
    case 3:
        return ANYWHERE;
        break;
    case 4:
        return m_vg_policy;
        break;
    default:
        return NORMAL;
    } 
}

