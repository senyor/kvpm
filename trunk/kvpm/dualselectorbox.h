/*
 *
 * 
 * Copyright (C) 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef DUALSELECTORBOX_H
#define DUALSELECTORBOX_H

#include <QWidget>

class SizeSelectorBox;

class DualSelectorBox : public QWidget
{
Q_OBJECT

    SizeSelectorBox *m_size_selector;
    SizeSelectorBox *m_offset_selector;
    long long m_max_size;

signals:
    void changed();

public:
    DualSelectorBox(long long unitSize, long long minSize, long long maxSize, long long initialSize, 
                    long long minOffset, long long maxOffset, long long initialOffset, QWidget *parent = 0);

    long long getCurrentSize();
    long long getCurrentOffset();
    bool isValid();

private slots:
    void resetSelectors();
    void offsetChanged();
    void sizeChanged();

};

#endif
