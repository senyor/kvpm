/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the Kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "lvchange.h"

#include <KLocale>
#include <KSeparator>

#include <QtGui>

#include "logvol.h"
#include "processprogress.h"


LVChangeDialog::LVChangeDialog(LogVol *const volume, QWidget *parent) : 
    KDialog(parent),
    m_lv(volume)
{
    setWindowTitle( i18n("Change Logical Volume Attributes") );

    QWidget *const dialog_body = new QWidget();
    setMainWidget(dialog_body);
    QVBoxLayout *const layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    QLabel *const name = new QLabel( i18n("Change Volume: <b>%1</b>", m_lv->getName()) );
    name->setAlignment(Qt::AlignCenter);
    layout->addSpacing(10);
    layout->addWidget(name);
    layout->addStretch();

    KTabWidget *const tab_widget = new KTabWidget();
    tab_widget->setAutomaticResizeTabs(true); 
    layout->addWidget(tab_widget);
    
    tab_widget->addTab(buildGeneralTab(),  i18nc("The standard or basic options", "General") );
    tab_widget->addTab(buildAdvancedTab(), i18nc("Less used or complex options", "Advanced") );

    if( m_lv->isSnap() || m_lv->isMirror() ){

        if(  m_lv->isMirror() )
            tab_widget->insertTab(1, buildMirrorTab(), i18n("Mirror") );
        else
            tab_widget->insertTab(1, buildMirrorTab(), i18n("Snapshot") );

        connect(m_dmeventd_box,     SIGNAL(clicked()), this, SLOT(resetOkButton()));
        connect(m_monitor_button,   SIGNAL(clicked()), this, SLOT(resetOkButton()));
        connect(m_nomonitor_button, SIGNAL(clicked()), this, SLOT(resetOkButton()));
        connect(m_ignore_button,    SIGNAL(clicked()), this, SLOT(resetOkButton()));
        connect(m_resync_check,     SIGNAL(clicked()), this, SLOT(resetOkButton()));
    }

    connect(m_alloc_box,         SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_normal_button,     SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_contiguous_button, SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_anywhere_button,   SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_cling_button,      SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_inherit_button,    SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_available_check,   SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_ro_check,          SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_refresh_check,     SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_udevsync_check,    SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_persistant_check,  SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_tag_group,         SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_deltag_combo, SIGNAL(currentIndexChanged(int)), this, SLOT(resetOkButton()));
    connect(m_tag_edit, SIGNAL(userTextChanged(QString)), this, SLOT(resetOkButton()));

    connect(m_polling_box,       SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_poll_button,       SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_nopoll_button,     SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_devnum_box,        SIGNAL(clicked()), this, SLOT(resetOkButton()));
    connect(m_udevsync_box,      SIGNAL(clicked()), this, SLOT(resetOkButton()));

    connect(this, SIGNAL(okClicked()), this, SLOT(commitChanges()));

    resetOkButton();
}

QWidget *LVChangeDialog::buildGeneralTab()
{
    QWidget *const tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    tab->setLayout(layout);

    QGroupBox *general_group = new QGroupBox();
    QVBoxLayout *general_layout = new QVBoxLayout();
    general_group->setLayout(general_layout);
    layout->addWidget(general_group);

    m_available_check  = new QCheckBox( i18n("Make volume available for use") );
    m_ro_check         = new QCheckBox( i18n("Make volume read only") );
    m_refresh_check    = new QCheckBox( i18n("Refresh volume metadata") );
    general_layout->addWidget(m_available_check);
    general_layout->addWidget(m_ro_check);
    general_layout->addWidget(m_refresh_check);

    if( m_lv->isActive() )
	m_available_check->setChecked(true);

    if( m_lv->isMounted() || m_lv->isSnap() )    
        m_available_check->setEnabled(false);

    if( !m_lv->isWritable() )
	m_ro_check->setChecked(true);

    m_tag_group = new QGroupBox( i18n("Change Volume Tags"));
    m_tag_group->setCheckable(true);
    m_tag_group->setChecked(false);
    layout->addWidget(m_tag_group);
    QHBoxLayout *add_tag_layout = new QHBoxLayout();
    QHBoxLayout *del_tag_layout = new QHBoxLayout();
    QVBoxLayout *tag_group_layout = new QVBoxLayout();
    tag_group_layout->addLayout(add_tag_layout);
    tag_group_layout->addLayout(del_tag_layout);
    m_tag_group->setLayout(tag_group_layout);
    add_tag_layout->addWidget( new QLabel( i18n("Add new tag:")) );
    m_tag_edit = new KLineEdit();
    QRegExp rx("[0-9a-zA-Z_\\.+-]*");
    QRegExpValidator *tag_validator = new QRegExpValidator( rx, m_tag_edit );
    m_tag_edit->setValidator(tag_validator);
    add_tag_layout->addWidget(m_tag_edit);
    del_tag_layout->addWidget( new QLabel( i18n("Remove tag:")) );
    m_deltag_combo = new KComboBox();
    m_deltag_combo->setEditable(false);
    QStringList tags = m_lv->getTags();
    for(int x = 0; x < tags.size(); x++)
        m_deltag_combo->addItem( tags[x] );
    m_deltag_combo->insertItem(0, QString(""));
    m_deltag_combo->setCurrentIndex(0);
    del_tag_layout->addWidget(m_deltag_combo);

    m_alloc_box = new QGroupBox( i18n("Change Allocation Policy") );
    m_alloc_box->setCheckable(true);
    m_alloc_box->setChecked(false);
    QVBoxLayout *alloc_box_layout = new QVBoxLayout;
    m_normal_button     = new QRadioButton( i18nc("The usual way", "Normal") );
    m_contiguous_button = new QRadioButton( i18n("Contiguous") );
    m_anywhere_button   = new QRadioButton( i18n("Anywhere") );
    m_cling_button      = new QRadioButton( i18n("Cling") );
    m_inherit_button    = new QRadioButton( i18nc("Inherited from the parent group", "Inherited") );

    QString policy = m_lv->getPolicy(); 

    if( policy == "Contiguous" ){
        m_contiguous_button->setEnabled(false);
        m_contiguous_button->setText("Contiguous (current)");
        m_normal_button->setChecked(true);
    }
    else if( policy == "Inherited"){
        m_inherit_button->setEnabled(false);
        m_inherit_button->setText("Inherited (current)");
        m_normal_button->setChecked(true);
    }
    else if( policy == "Anywhere" ){
        m_anywhere_button->setEnabled(false);
        m_anywhere_button->setText("Anywhere (current)");
        m_normal_button->setChecked(true);
    }
    else if( policy == "Cling" ){
        m_cling_button->setEnabled(false);
        m_cling_button->setText("Cling (current)");
        m_normal_button->setChecked(true);
    }
    else{
        m_normal_button->setEnabled(false);
        m_normal_button->setText("Normal (current)");
        m_cling_button->setChecked(true);
    }

    alloc_box_layout->addWidget(m_normal_button);
    alloc_box_layout->addWidget(m_contiguous_button);
    alloc_box_layout->addWidget(m_anywhere_button);
    alloc_box_layout->addWidget(m_cling_button);
    alloc_box_layout->addWidget(m_inherit_button);
    m_alloc_box->setLayout(alloc_box_layout);
    layout->addWidget(m_alloc_box);

    return tab;
}

QWidget *LVChangeDialog::buildMirrorTab()
{
    QWidget *const tab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout();
    tab->setLayout(layout);
    layout->addStretch();

    QGroupBox *const resync_box = new QGroupBox( i18n("Mirror Sync") );
    if( !m_lv->isMirror() ){
        resync_box->setEnabled(false);
        resync_box->hide();
    }

    layout->addWidget(resync_box);
    QVBoxLayout *const resync_layout = new QVBoxLayout();
    resync_box->setLayout(resync_layout);
    m_resync_check = new QCheckBox( i18n("Re-synchronize mirrors") );
    m_resync_check->setEnabled( !m_lv->isMounted() );
    resync_layout->addWidget(m_resync_check);

    m_dmeventd_box = new QGroupBox( i18n("Monitoring With dmeventd") );
    m_dmeventd_box->setCheckable(true);
    m_dmeventd_box->setChecked(false);

    QVBoxLayout *const mirror_layout = new QVBoxLayout();
    m_dmeventd_box->setLayout(mirror_layout);
    m_monitor_button = new QRadioButton( i18n("Monitor with dmeventd") );
    m_nomonitor_button = new QRadioButton( i18n("Do not monitor") );
    m_ignore_button  = new QRadioButton( i18n("Ignore dmeventd") );
    m_monitor_button->setChecked(true);
    mirror_layout->addWidget(m_monitor_button);
    mirror_layout->addWidget(m_nomonitor_button);
    mirror_layout->addWidget(m_ignore_button);
    layout->addWidget(m_dmeventd_box);
    layout->addStretch();

    return tab;
}

QWidget *LVChangeDialog::buildAdvancedTab()
{
    QWidget *const tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout();
    tab->setLayout(layout);

    m_polling_box = new QGroupBox( i18n("Chnage Volume Polling") );
    m_polling_box->setCheckable(true);
    m_polling_box->setChecked(false);
    layout->addWidget(m_polling_box);
    QVBoxLayout *poll_layout = new QVBoxLayout();
    m_polling_box->setLayout(poll_layout);
    m_poll_button = new QRadioButton( i18n("Start polling") );
    m_poll_button->setChecked(true);
    poll_layout->addWidget(m_poll_button);
    m_nopoll_button = new QRadioButton( i18n("Stop polling") );
    poll_layout->addWidget(m_nopoll_button);

    m_udevsync_box = new QGroupBox( i18n("Udev Synchronizing") );
    layout->addWidget(m_udevsync_box);
    QVBoxLayout *sync_layout = new QVBoxLayout();
    m_udevsync_box->setLayout(sync_layout);
    m_udevsync_check = new QCheckBox( i18n("Synchronize with udev") );
    m_udevsync_check->setChecked(true);
    sync_layout->addWidget(m_udevsync_check);
    
    m_devnum_box = new QGroupBox( i18n("Set Kernel Device Numbers") );
    m_devnum_box->setCheckable(true);
    QVBoxLayout *devnum_layout = new QVBoxLayout();
    m_devnum_box->setLayout(devnum_layout);
    QHBoxLayout *major_layout = new QHBoxLayout();
    QHBoxLayout *minor_layout = new QHBoxLayout();
    m_persistant_check = new QCheckBox( i18n("Use persistant device numbers") );
    devnum_layout->addWidget(m_persistant_check); 
    devnum_layout->addLayout(major_layout);
    devnum_layout->addLayout(minor_layout);

    m_major_edit = new KLineEdit(QString("%1").arg(m_lv->getMajorDevice()));
    major_layout->addWidget( new QLabel( i18n("Major number: ") ) );
    major_layout->addWidget(m_major_edit);
    m_minor_edit = new KLineEdit(QString("%1").arg(m_lv->getMinorDevice()));
    minor_layout->addWidget( new QLabel( i18n("Minor number: ") ) );
    minor_layout->addWidget(m_minor_edit);
    layout->addWidget(m_devnum_box);
    m_persistant_check->setChecked( m_lv->isPersistant() );
    m_devnum_box->setChecked(false);

    return tab;
}

QStringList LVChangeDialog::arguments()
{
    QStringList args, temp;

    args << "lvchange" << "--yes"; // answer yes to any question

    if( !m_lv->isSnap() ){    
        if( m_available_check->isChecked() && ( !m_lv->isActive() ))
	    args << "--available" << "y";
	else if( ( ! m_available_check->isChecked() ) && ( m_lv->isActive() ))
	    args << "--available" << "n";
    }

    if( m_ro_check->isChecked() && m_lv->isWritable() )
	args << "--permission" << "r";
    else if( ( ! m_ro_check->isChecked() ) && ( ! m_lv->isWritable() ) )
	args << "--permission" << "rw";

    if( m_refresh_check->isChecked() )
        args << "--refresh";

    if( m_lv->isSnap() || m_lv->isMirror() ){ // These buttons are undefined if the mirrortab isn't built!
        if( m_resync_check->isChecked() )
            args << "--resync";
    
        if( m_dmeventd_box->isChecked() ){
            if( m_monitor_button->isChecked() )
                args << "--monitor" << "y";
            else if( m_nomonitor_button->isChecked() )
                args << "--monitor" << "n";
            else
                args << "--ignoremonitoring";
        }
    }

    if( m_devnum_box->isChecked() ){
        if( m_persistant_check->isChecked() ){
            args << "--force" << "--persistent" << "y";
            args << "--major" << m_major_edit->text();
            args << "--minor" << m_minor_edit->text();
        }
        else{
            args << "--force" << "--persistent" << "n";
            args << "--major" << m_major_edit->text();
            args << "--minor" << m_minor_edit->text();
        }
    }

    if(m_polling_box->isChecked()){
        if(m_poll_button->isChecked())
            args << "--poll" << "y";
        else
            args << "--poll" << "y";
    }
    
    if(!m_udevsync_check->isChecked())
        args << "--noudevsync";

    if( m_tag_group->isChecked() ){
        if( m_deltag_combo->currentIndex() )
            args << "--deltag" << m_deltag_combo->currentText();
        if( ! (m_tag_edit->text()).isEmpty() )
            args << "--addtag" << m_tag_edit->text();
    }

    if( m_alloc_box->isChecked() ){
        args << "--alloc";
        if( m_contiguous_button->isChecked() )
            args << "contiguous";
        else if( m_anywhere_button->isChecked() )
            args << "anywhere";
        else if( m_cling_button->isChecked() )
            args << "cling";
        else if( m_inherit_button->isChecked() )
            args << "inherit";
        else
            args << "normal";
    }

    args << m_lv->getFullName();

    return args;
}

void LVChangeDialog::commitChanges()
{
    hide();
    ProcessProgress change_lv( arguments() );
}

void LVChangeDialog::resetOkButton(){

    if( m_available_check->isChecked() )
        m_polling_box->setEnabled(true);
    else{
        m_polling_box->setEnabled(false);
        m_polling_box->setChecked(false);
    }         

    QStringList args = arguments();

    args.removeAt( args.indexOf( QString("--ignoremonitoring") ) );

    if( args.size() > 3 )
        enableButtonOk(true);
    else
        enableButtonOk(false);
}

