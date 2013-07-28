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


#ifndef KVPMDIALOG_H
#define KVPMDIALOG_H

#include <KDialog>


/* Sometimes the constuctor of a dialog is called with arguments
   that make executing the dialog pointless. The run() function
   is a substitute for the exec() function. If the subclass calls
   preventExec() in its constructor then any subsequent call to
   run will exit without calling exec(). Otherwise exec() is called
   and the dialog behaves normally. */

class KvpmDialog : public KDialog
{
    Q_OBJECT

    bool m_allow_run = true;

protected:
    void preventExec(); // call this if the dialog should not be exectued
    bool willExec();    // True if preventRun was never called

protected slots:
    virtual void commit() = 0;

public:
    explicit KvpmDialog(QWidget *parent = nullptr);
    virtual ~KvpmDialog();
    int run();

};

#endif

