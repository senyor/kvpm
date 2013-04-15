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

#ifndef LVMCONFIG_H
#define LVMCONFIG_H


#include <QStringList>


class LvmConfig
{
    static QString m_mirror_segtype_default;
    static bool m_mirror_logs_require_separate_pvs;
    static bool m_thin_pool_metadata_require_separate_pvs;
    static bool m_maximise_cling;

    QStringList getConfig();
    void setGlobal(QStringList &variables);
    void setAllocation(QStringList &variables);

public:
    LvmConfig();
    ~LvmConfig();
    void initialize();
    static QString getMirrorSegtypeDefault();
    static bool getMirrorLogsRequireSeparatePvs();
    static bool getThinPoolMetadataRequireSeparatePvs();
    static bool getMaximiseCling();
};

#endif
