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
#include <QList>
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

        // the ped_device_free_all() sometimes causes ped to freeze up completely
        // during ped_device_probe_all() if a device is removed, even if the device come back.

        //        ped_device_free_all();

        PedDevice *dev = NULL;
        QList<PedDevice *> dev_list;

        // removing them one by one seems to work ok.

        while ((dev = ped_device_get_next(dev))) {
            dev_list << dev;            
        }
        
        for (auto device : dev_list) {
            ped_device_cache_remove(device);
            ped_device_destroy(device);
        }
        
        ped_device_probe_all();
        m_semaphore->release();
    }
};

#endif
