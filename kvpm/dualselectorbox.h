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

#ifndef DUALSELECTORBOX_H
#define DUALSELECTORBOX_H

#include <QWidget>

class SizeSelectorBox;

class DualSelectorBox : public QWidget
{
    Q_OBJECT

    SizeSelectorBox *m_size_selector;
    SizeSelectorBox *m_offset_selector;
    long long m_space;


signals:
    void changed();

public:
    DualSelectorBox(const long long sectorSize,  const long long totalSpace,
                    const long long minSize,   const long long maxSize,   const long long initialSize,
                    const long long minOffset, const long long maxOffset, const long long initialOffset,
                    QWidget *parent = 0);

    DualSelectorBox(const long long sectorSize, const long long totalSpace, QWidget *parent = 0);

    long long getNewSize();
    long long getNewOffset();
    bool isValid();

public slots:
    void resetSelectors();

private slots:
    void offsetChanged();
    void sizeChanged();

};

#endif
