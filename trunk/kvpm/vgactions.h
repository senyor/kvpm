/*
 *
 *
 * Copyright (C) 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */

#ifndef VGACTIONS_H
#define VGACTIONS_H


#include <KActionCollection>

class QAction;

class VolGroup;


class VGActions : public KActionCollection
{
    Q_OBJECT

    VolGroup *m_vg = nullptr;

public:
    VGActions(QWidget *parent = nullptr);
    void setVg(VolGroup *vg);

private slots:
    void callDialog(QAction *);

};

#endif
