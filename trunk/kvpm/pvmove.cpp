/*
 *
 * 
 * Copyright (C) 2008, 2010, 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "pvmove.h"

#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>
#include <KMessageBox>
#include <KPushButton>

#include <QtGui>

#include "logvol.h"
#include "masterlist.h"
#include "misc.h"
#include "processprogress.h"
#include "physvol.h"
#include "pvcheckbox.h"
#include "volgroup.h"


struct NameAndRange
{
    QString   name;        // Physical volume name
    QString   name_range;  // name + range of extents  ie: /dev/sda1:10-100 and just name if no range specified 
    long long start;       // Starting extent
    long long end;         // Last extent
};


bool restart_pvmove()
{
    QStringList args;
    const QString message = i18n("Do you wish to restart all interrupted physical volume moves?");

    if(KMessageBox::questionYesNo(0, message) == KMessageBox::Yes){

        args << "pvmove";

        ProcessProgress resize(args);
        return true;
    }
    else
        return false;
}

bool stop_pvmove()
{
    QStringList args;
    const QString message = i18n("Do you wish to abort all physical volume moves currently in progress?");
    
    if(KMessageBox::questionYesNo(0, message) == KMessageBox::Yes){

        args << "pvmove" << "--abort";

        ProcessProgress resize(args);
        return true;
    }
    else
        return false;
}

PVMoveDialog::PVMoveDialog(PhysVol *physicalVolume, QWidget *parent) : KDialog(parent) 
{
    m_vg = physicalVolume->getVg();
    m_target_pvs = m_vg->getPhysicalVolumes();
    m_move_lv = false;
    m_move_segment = false;
    m_bailout = false;

    const QString name = physicalVolume->getName(); 
    const QList<LogVol *> lvs = m_vg->getLogicalVolumes();
    QStringList forbidden_targets;  // A whole pv can't be moved to a pv it is striped with along any segment
    QStringList striped_targets;

    NameAndRange *nar = new NameAndRange;
    nar->name = name;
    nar->name_range = name;
    m_sources.append(nar);

    forbidden_targets.append(name);

    for(int x = lvs.size() - 1; x >= 0; x--){
        for(int seg = lvs[x]->getSegmentCount() - 1; seg >= 0; seg--){
            if( lvs[x]->getSegmentStripes(seg) > 1 ){
                striped_targets = lvs[x]->getPvNames(seg);
                if( striped_targets.contains(name) )
                    forbidden_targets.append(striped_targets); 
            }
        }
    }

    forbidden_targets.removeDuplicates();

    for(int x = m_target_pvs.size() - 1 ; x >= 0; x--){
        for(int y = forbidden_targets.size() - 1; y >= 0; y--){
            if( m_target_pvs[x]->getName() == forbidden_targets[y] ){
                m_target_pvs.removeAt(x);
                forbidden_targets.removeAt(y);
                break;
            }
        }
    }

    removeFullTargets();

    if( !m_bailout )
        buildDialog();

    connect(this, SIGNAL(okClicked()), 
            this, SLOT(commitMove()));
}

PVMoveDialog::PVMoveDialog(LogVol *logicalVolume, int segment, QWidget *parent) : 
    KDialog(parent), 
    m_lv(logicalVolume)
{
    m_vg = m_lv->getVg();
    m_move_lv = true;
    m_target_pvs = m_vg->getPhysicalVolumes();
    m_bailout = false;

    if( segment >= 0 ){
        setupSegmentMove(segment);
        m_move_segment = true;
    }
    else{
        setupFullMove();
        m_move_segment = false;
    }

    /* if there is only one source physical volumes possible on this logical volume
       then we eliminate it from the possible target pv list completely. */

    if( m_sources.size() == 1 ){
	for(int x = m_target_pvs.size() - 1; x >= 0; x--){
	    if( m_target_pvs[x]->getName() == m_sources[0]->name )
		m_target_pvs.removeAt(x);
	}
    }

    /* If this is a segment move then all source pvs need to be 
       removed from the target list */

    if( m_move_segment ){
	for(int x = m_target_pvs.size() - 1; x >= 0; x--){
            for(int y = m_sources.size() - 1; y >= 0; y--){
                if( m_target_pvs[x]->getName() == m_sources[y]->name )
                    m_target_pvs.removeAt(x);
            }
        }
    }

    removeFullTargets();

    if( !m_bailout )
        buildDialog();

    connect(this, SIGNAL(okClicked()), 
            this, SLOT(commitMove()));
}

void PVMoveDialog::removeFullTargets(){

    for(int x = (m_target_pvs.size() - 1); x >= 0; x--){
        if( m_target_pvs[x]->getRemaining() <= 0 )
            m_target_pvs.removeAt(x);
    }

    /* If there is only one physical volume in the group or they are 
       all full then a pv move will have no place to go */

    if(m_target_pvs.size() < 1){
	KMessageBox::error(NULL, i18n("There are no available physical volumes with space to move to"));
        m_bailout = true;
    }
}

PVMoveDialog::~PVMoveDialog()
{
    for(int x = 0; x < m_sources.size(); x++)
        delete m_sources[x];
}

void PVMoveDialog::buildDialog()
{
    bool use_si_units;
    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", use_si_units, false);

    KLocale *const locale = KGlobal::locale();
    if(use_si_units)
        locale->setBinaryUnitDialect(KLocale::MetricBinaryDialect); 
    else
        locale->setBinaryUnitDialect(KLocale::IECBinaryDialect);

    QLabel *label;
    NoMungeRadioButton *radio_button;
    
    setWindowTitle( i18n("Move Physical Volume Extents") );
    QWidget *dialog_body = new QWidget(this);
    setMainWidget(dialog_body);
    QVBoxLayout *layout = new QVBoxLayout;
    dialog_body->setLayout(layout);

    if(m_move_lv){
        label = new QLabel( i18n("<b>Move only physical extents on:</b>") );
        label->setAlignment(Qt::AlignCenter);
	layout->addWidget(label);
	label = new QLabel("<b>" + m_lv->getFullName() + "</b>");
        label->setAlignment(Qt::AlignCenter);
	layout->addWidget(label);
    }

    QGroupBox *radio_group = new QGroupBox( i18n("Source Physical Volumes") );
    QGridLayout *radio_layout = new QGridLayout();
    radio_group->setLayout(radio_layout);
    layout->addWidget(radio_group);
    QHBoxLayout *lower_layout = new QHBoxLayout;
    layout->addLayout(lower_layout);

    m_pv_checkbox = new PVCheckBox(m_target_pvs);
    lower_layout->addWidget(m_pv_checkbox);

    const int radio_count = m_sources.size();

    if( radio_count > 1 ){
	for(int x = 0; x < radio_count; x++){
   
            if(m_move_segment){
                m_pv_used_space = (1 + m_sources[x]->end - m_sources[x]->start) * m_vg->getExtentSize();
                radio_button = new NoMungeRadioButton( QString("%1  %2").arg(m_sources[x]->name_range).arg(locale->formatByteSize(m_pv_used_space)));
                radio_button->setAlternateText( m_sources[x]->name );
            }
	    else if(m_move_lv){
	        m_pv_used_space = m_lv->getSpaceUsedOnPv(m_sources[x]->name);
                radio_button = new NoMungeRadioButton( QString("%1  %2").arg(m_sources[x]->name).arg(locale->formatByteSize(m_pv_used_space)));
                radio_button->setAlternateText( m_sources[x]->name );
            }
            else{
                m_pv_used_space = m_vg->getPvByName(m_sources[x]->name)->getSize() - m_vg->getPvByName(m_sources[x]->name )->getRemaining();
                radio_button = new NoMungeRadioButton( QString("%1  %2").arg(m_sources[x]->name).arg(locale->formatByteSize(m_pv_used_space)));
                radio_button->setAlternateText( m_sources[x]->name );
            }

            if(radio_count < 11 )
                radio_layout->addWidget(radio_button, x % 5, x / 5);
            else if (radio_count % 3 == 0)
                radio_layout->addWidget(radio_button, x % (radio_count / 3), x / (radio_count / 3));
            else
                radio_layout->addWidget(radio_button, x % ( (radio_count + 2) / 3), x / ( (radio_count + 2) / 3));

	    m_radio_buttons.append(radio_button);
	    
	    connect(radio_button, SIGNAL(toggled(bool)), 
		    this, SLOT(disableSource()));

	    if( !x )
		radio_button->setChecked(true);
	}
    }
    else{
        if(m_move_segment){
            m_pv_used_space = (1 + m_sources[0]->end - m_sources[0]->start) * m_vg->getExtentSize();
            radio_layout->addWidget( new QLabel( QString("%1  %2").arg(m_sources[0]->name_range).arg(locale->formatByteSize(m_pv_used_space)) ) );
        }
	else if(m_move_lv){
	    m_pv_used_space = m_lv->getSpaceUsedOnPv(m_sources[0]->name);
            radio_layout->addWidget( new QLabel( QString("%1  %2").arg(m_sources[0]->name).arg(locale->formatByteSize(m_pv_used_space)) ) );
        }
	else{
            m_pv_used_space = m_vg->getPvByName( m_sources[0]->name )->getSize() - m_vg->getPvByName( m_sources[0]->name )->getRemaining();
            radio_layout->addWidget( new QLabel( QString("%1  %2").arg(m_sources[0]->name).arg(locale->formatByteSize(m_pv_used_space)) ) );
        }
    }

    QGroupBox *alloc_box = new QGroupBox( i18n("Allocation Policy") );
    QVBoxLayout *alloc_box_layout = new QVBoxLayout;
    m_normal_button     = new QRadioButton( i18nc("The usual way", "Normal") );
    m_contiguous_button = new QRadioButton( i18n("Contiguous") );
    m_anywhere_button   = new QRadioButton( i18n("Anywhere") );
    m_inherited_button  = new QRadioButton( i18nc("Inherited from the group", "Inherited") );
    m_inherited_button->setChecked(true);
    m_cling_button      = new QRadioButton( i18n("Cling") );
    alloc_box_layout->addWidget(m_normal_button);
    alloc_box_layout->addWidget(m_contiguous_button);
    alloc_box_layout->addWidget(m_anywhere_button);
    alloc_box_layout->addWidget(m_inherited_button);
    alloc_box_layout->addWidget(m_cling_button);
    alloc_box_layout->addStretch();
    alloc_box->setLayout(alloc_box_layout);
    lower_layout->addWidget(alloc_box);

    connect(m_pv_checkbox, SIGNAL(stateChanged()), 
            this, SLOT(resetOkButton()));

    resetOkButton();
}

void PVMoveDialog::resetOkButton()
{
    const long long free_space_total = m_pv_checkbox->getRemainingSpace();
    long long needed_space_total = 0;
    QString pv_name;

    if(m_move_lv){
	if(m_radio_buttons.size() > 1){
	    for(int x = 0; x < m_radio_buttons.size(); x++){
		if(m_radio_buttons[x]->isChecked()){
		    pv_name = m_radio_buttons[x]->getAlternateText();
		    needed_space_total = m_lv->getSpaceUsedOnPv(pv_name);
		}
	    }
	}
	else{
	    pv_name = m_sources[0]->name;
	    needed_space_total = m_lv->getSpaceUsedOnPv(pv_name);
	}
    }
    else
        needed_space_total = m_pv_used_space; 
	    
    if(free_space_total < needed_space_total)
	enableButtonOk(false);
    else
	enableButtonOk(true);
}

void PVMoveDialog::disableSource()  // don't allow source and target to be the same pv
{
    PhysVol *source_pv = NULL;
    
    for(int x = m_radio_buttons.size() - 1; x>= 0; x--){
	if(m_radio_buttons[x]->isChecked())
	    source_pv = m_vg->getPvByName( m_sources[x]->name );
    }

    m_pv_checkbox->disableOrigin(source_pv);

    resetOkButton();
}

QStringList PVMoveDialog::arguments()
{
    QStringList args;
    QString source;

    args << "pvmove" << "--background";

    if(m_move_lv){
	args << "--name";
	args << m_lv->getFullName();
    }

    if ( !m_inherited_button->isChecked() ){        // "inherited" is what we get if 
        args << "--alloc";                          // we don't pass "--alloc" at all
        if ( m_contiguous_button->isChecked() )     // passing "--alloc" "inherited"  
            args << "contiguous" ;                  // doesn't work                        
        else if ( m_anywhere_button->isChecked() )
            args << "anywhere" ;
        else if ( m_cling_button->isChecked() )
            args << "cling" ;
        else
            args << "normal" ;
    }

    if( m_sources.size() > 1 ){
        for(int x = m_sources.size() - 1; x >= 0; x--){
            if(m_radio_buttons[x]->isChecked())
                source = m_sources[x]->name_range;
        }
    }
    else{
        source = m_sources[0]->name_range;
    }

    args << source;
    args << m_pv_checkbox->getNames(); // target(s)

    return args;
}

void PVMoveDialog::setupSegmentMove(int segment)
{
    const QStringList names = m_lv->getPvNames(segment);                      // source pv name
    const int stripes = m_lv->getSegmentStripes(segment);                     // source pv stripe count
    const long long extents = m_lv->getSegmentExtents(segment);               // extent count
    const QList<long long> starts = m_lv->getSegmentStartingExtent(segment);  // lv's first extent on pv 
    NameAndRange *nar;
    
    for(int x = 0; x < names.size(); x++){
        nar = new NameAndRange;
        nar->name  = names[x];
        nar->start = starts[x];
        nar->end   = starts[x] + (extents / stripes) - 1;
        nar->name_range = QString("%1:%2-%3").arg(nar->name).arg(nar->start).arg(nar->end);
        m_sources.append(nar);
    }
}

void PVMoveDialog::setupFullMove()
{
    const QStringList names = m_lv->getPvNamesAllFlat();
    NameAndRange *nar;

    for(int x = names.size() - 1; x >= 0; x--){
        nar = new NameAndRange();
        nar->name = names[x];
        nar->name_range = names[x];
        m_sources.append(nar);
    }
}

bool PVMoveDialog::bailout()
{
    return m_bailout;
}

void PVMoveDialog::commitMove()
{
    hide();
    ProcessProgress move( arguments() );
    return;
}

