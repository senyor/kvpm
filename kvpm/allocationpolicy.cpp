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


#include "allocationpolicy.h"

#include <KComboBox>
#include <KLocale>

#include <QDebug>
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
    case INHERITED_NORMAL:
        return QString(i18n("inherited (normal)"));
        break;
    case INHERITED_CONTIGUOUS:
        return QString(i18n("inherited (contiguous)"));
        break;
    case INHERITED_CLING:
        return QString(i18n("inherited (cling)"));
        break;
    case INHERITED_ANYWHERE:
        return QString(i18n("inherited (anywhere)"));
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
    case INHERITED_NORMAL:
    case INHERITED_CONTIGUOUS:
    case INHERITED_CLING:
    case INHERITED_ANYWHERE:
        return QString("inherited");
        break;
    default:
        return QString("normal");
    } 
}

PolicyComboBox::PolicyComboBox(AllocationPolicy policy, bool canInherit, QWidget *parent) 
    :  QWidget(parent)
{
    QLabel *const label = new QLabel(i18n("Allocation policy: "));

    m_combo = new KComboBox();
    m_combo->addItem(i18n("Normal"));
    m_combo->addItem(i18n("Contiguous"));
    m_combo->addItem(i18n("Cling"));
    m_combo->addItem(i18n("Anywhere"));

    if (policy == NORMAL) {
        m_combo->setCurrentIndex(0);
    } else if (policy == CONTIGUOUS) {
        m_combo->setCurrentIndex(1);
    } else if (policy == CLING) {
        m_combo->setCurrentIndex(2);
    } else if (policy == ANYWHERE) {
        m_combo->setCurrentIndex(3);
    } else if (policy > ANYWHERE && canInherit){
        m_combo->addItem(policyToLocalString(policy));
        m_combo->setCurrentIndex(4);
    } else {
        m_combo->setCurrentIndex(0);
    }

    QHBoxLayout *const layout = new QHBoxLayout;
    layout->addStretch();
    layout->addWidget(label);
    layout->addWidget(m_combo);
    layout->addStretch();

    connect(m_combo, SIGNAL(currentIndexChanged(int)),
            this, SLOT(emitNewPolicy(int)));

    setLayout(layout);
}

void PolicyComboBox::emitNewPolicy(const int index)
{
    emit policyChanged(getPolicy(index));
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
        return INHERITED_NORMAL;
        break;
    case 5:
        return INHERITED_CONTIGUOUS;
        break;
    case 6:
        return INHERITED_CLING;
        break;
    case 7:
        return INHERITED_ANYWHERE;
        break;
    default:
        return NORMAL;
    } 
}

AllocationPolicy PolicyComboBox::getEffectivePolicy()
{
    AllocationPolicy policy = getPolicy();

    if (policy == INHERITED_NORMAL)
        policy = NORMAL;
    else if (policy == INHERITED_CLING)
        policy = CLING;
    else if (policy == INHERITED_CONTIGUOUS)
        policy = CONTIGUOUS;
    else if (policy == INHERITED_ANYWHERE)
        policy = ANYWHERE;

    return policy;
}

AllocationPolicy PolicyComboBox::getPolicy(int index)
{
    switch(index) {
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
        return INHERITED_NORMAL;
        break;
    case 5:
        return INHERITED_CONTIGUOUS;
        break;
    case 6:
        return INHERITED_CLING;
        break;
    case 7:
        return INHERITED_ANYWHERE;
        break;
    default:
        return NORMAL;
    } 
}

