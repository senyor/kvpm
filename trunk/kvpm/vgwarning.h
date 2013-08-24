/*
 *
 *
 * Copyright (C) 2013 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef VGWARNING_H
#define VGWARNING_H

#include <QWidget>

class QVBoxLayout;

class VolGroup;


class VGWarning : public QWidget
{

    QVBoxLayout *m_layout = nullptr;

    QWidget *buildPartialWarning();
    QWidget *buildOpenFailedWarning(VolGroup *const group);
    QWidget *buildExportedNotice();

public:
    explicit VGWarning(QWidget *parent = nullptr);
    void loadMessage(VolGroup *const group);

};

#endif
