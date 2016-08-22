/*
 *
 *
 * Copyright (C) 2013, 2016 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as
 * published by the Free Software Foundation.
 *
 * See the file "COPYING" for the exact licensing terms.
 */


#include "vgwarning.h"

#include <KFormat>
#include <KLocalizedString>

#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>
#include <QVBoxLayout>

#include "volgroup.h"



VGWarning::VGWarning(QWidget *parent) : 
    QWidget(parent)
{
    m_layout = new QVBoxLayout();
    setLayout(m_layout);
    hide();
}

void VGWarning::loadMessage(VolGroup *const group)
{
    for (auto child : this->findChildren<QWidget *>()) {
        m_layout->removeWidget(child);
        child->deleteLater();
    }

    if (group->isExported() || group->isPartial() || group->openFailed()) {
        show();

        if (group->isExported())
            m_layout->addWidget(buildExportedNotice());

        if (group->isPartial())
            m_layout->addWidget(buildPartialWarning());

        if (group->openFailed())
            m_layout->addWidget(buildOpenFailedWarning(group));
    } else {
        hide();
    }
}

QWidget *VGWarning::buildExportedNotice()
{
    QWidget *const notice = new QWidget();
    QHBoxLayout *const notice_layout = new QHBoxLayout();
    notice->setLayout(notice_layout);

    notice_layout->addStretch();
    notice_layout->addWidget(new QLabel(i18n("<b>Exported Volume Group</b>")));
    notice_layout->addStretch();

    return notice;
}

QWidget *VGWarning::buildOpenFailedWarning(VolGroup *const group)
{
    QWidget *const warning = new QWidget();
    QHBoxLayout *const warning_layout = new QHBoxLayout();
    warning->setLayout(warning_layout);

    warning_layout->addStretch();
    QLabel *const icon_label = new QLabel();
    icon_label->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-warning")).pixmap(32, 32));
    warning_layout->addWidget(icon_label);
    warning_layout->addSpacing(10);
    QLabel *warning_label = new QLabel();

    if (group->isClustered())
        warning_label->setText(i18n("<b>Warning: clustered volume group could not be opened</b>"));
    else
        warning_label->setText(i18n("<b>Warning: volume group could not be opened</b>"));

    warning_layout->addWidget(warning_label);
    warning_layout->addStretch();

    return warning;
}

QWidget *VGWarning::buildPartialWarning()
{
    QWidget *const warning = new QWidget();
    QHBoxLayout *const warning_layout = new QHBoxLayout();
    warning->setLayout(warning_layout);

    warning_layout->addStretch();
    QLabel *const icon_label = new QLabel();
    icon_label->setPixmap(QIcon::fromTheme(QStringLiteral("dialog-warning")).pixmap(32, 32));
    warning_layout->addWidget(icon_label);
    warning_layout->addSpacing(10);
    QLabel *const warning_label = new QLabel(i18n("<b>Warning: Partial volume group, some physical volumes are missing</b>"));
    warning_layout->addWidget(warning_label);
    warning_layout->addStretch();

    return warning;
}


