/*
 *
 *
 * Copyright (C) 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef ALLOCATIONPOLICY_H
#define ALLOCATIONPOLICY_H


#include <QWidget>


class QString;
class KComboBox;


typedef enum {
    NO_POLICY  = 0,
    NORMAL     = 1,
    CONTIGUOUS = 2,
    CLING      = 3,
    ANYWHERE   = 4,
    INHERIT_NORMAL     = 5,  // Don't change order, INHERIT_* must be after other policies 
    INHERIT_CONTIGUOUS = 6,
    INHERIT_CLING      = 7,
    INHERIT_ANYWHERE   = 8,
} AllocationPolicy;


QString policyToString(const AllocationPolicy policy);
QString policyToLocalString(const AllocationPolicy policy);


class PolicyComboBox: public QWidget
{
    Q_OBJECT

private:
    KComboBox *m_combo;
    AllocationPolicy m_vg_policy;

public:
    explicit PolicyComboBox(const AllocationPolicy policy, const AllocationPolicy vgpolicy = NO_POLICY, QWidget *parent = NULL);
    AllocationPolicy getEffectivePolicy();
    AllocationPolicy getPolicy();

private slots:
    void emitNewPolicy();

signals:
    void policyChanged(AllocationPolicy policy);
};

#endif















