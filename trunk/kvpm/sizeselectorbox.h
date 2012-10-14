/*
 *
 *
 * Copyright (C) 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef SIZESELECTORBOX_H
#define SIZESELECTORBOX_H

#include <KComboBox>
#include <KLineEdit>

#include <QCheckBox>
#include <QDoubleValidator>
#include <QGroupBox>
#include <QSlider>
#include <QLabel>

class SizeSelectorBox : public QGroupBox
{
    Q_OBJECT

    long long  m_max_size;        // max size possible in sectors or extents, not bytes etc.
    long long  m_min_size;
    long long  m_constrained_max; // working max less than or equal to m_max_size
    long long  m_constrained_min;
    long long  m_current_size;    // current size in sectors or extents, not bytes etc.
    long long  m_initial_size;    // Starting size in sectors or extents, not bytes etc.
    long long  m_unit_size;       // the size of the extents or sectors in bytes
    QSlider   *m_size_slider;
    KComboBox *m_suffix_combo;
    KLineEdit *m_size_edit;
    QDoubleValidator *m_size_validator;
    QCheckBox *m_shrink_box;    // Allow partition/volume to shrink.
    QCheckBox *m_size_box;      // Lock size to change
    QCheckBox *m_offset_box;    // Lock offset to change.

    bool m_is_volume;    // Is it a volume or a partition?
    bool m_is_offset;    // Are we selecting the starting point offset of a partition?
    bool m_is_new;       // New volume/partition, not resizing old one?
    bool m_start_locked; // Start out with the size check box checked
    bool m_is_valid;     // Valid data?
    bool m_use_si_units;

    long long convertSizeToUnits(int index, double size); // ie: convert MiBs to sectors or extents
    void updateValidator();
    void updateSlider();
    void setConstraints(bool unlock);

signals:
    void stateChanged();

public:
    SizeSelectorBox(long long unitSize, long long minSize, long long maxSize, long long initialSize,  // size in extents
                    bool isVolume, bool isOffset, bool isNew = false, bool startLocked = false, QWidget *parent = 0);

    long long getCurrentSize();
    long long getMinimumSize();
    long long getMaximumSize();
    void resetToInitial();
    void setConstrainedMax(long long max);
    bool isLocked();
    bool setCurrentSize(long long size);  // return false if size is outside min or max
    bool isValid();

private slots:
    void setToSlider(int value);
    void setToEdit(QString size);
    void updateEdit();
    void lock(bool lock);
    void lockShrink(bool lock);
    void disableLockShrink(bool disable);
};

#endif
