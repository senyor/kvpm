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

#ifndef PEDTHREAD_H
#define PEDTHREAD_H


#include <parted/parted.h>

#include <QDebug>
#include <QSemaphore>
#include <QThread>



class PedThread : public QThread
{
private:
    QSemaphore *m_semaphore;

public:
    PedThread(QSemaphore *semaphore) : m_semaphore(semaphore) { m_semaphore->acquire(); }

protected:
    void run() {     
        ped_device_free_all();
        ped_device_probe_all();
        m_semaphore->release();
    }
};

#endif
