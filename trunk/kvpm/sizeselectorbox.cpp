/*
 *
 * 
 * Copyright (C) 2011 Benjamin Scott   <benscott@nwlink.com>
 *
 * This file is part of the kvpm project.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License,  version 3, as 
 * published by the Free Software Foundation.
 * 
 * See the file "COPYING" for the exact licensing terms.
 */


#include "sizeselectorbox.h"

#include <KConfigSkeleton>
#include <KGlobal>
#include <KLocale>

#include <QtGui>


SizeSelectorBox::SizeSelectorBox(long long unitSize, long long minSize, long long maxSize, long long initialSize,
                                 bool isVolume, bool isOffset, bool isNew, QWidget *parent) : 
    QGroupBox(parent),
    m_max_size(maxSize),
    m_min_size(minSize),
    m_unit_size(unitSize),
    m_is_volume(isVolume),
    m_is_offset(isOffset),
    m_is_new(isNew)
{
    m_initial_size = initialSize;
    m_current_size = initialSize;
    m_is_valid = true;
    m_constrained_max = m_max_size;
    m_constrained_min = m_min_size;

    KConfigSkeleton skeleton;
    skeleton.setCurrentGroup("General");
    skeleton.addItemBool("use_si_units", m_use_si_units, false);

    QVBoxLayout *const layout = new QVBoxLayout();
    QHBoxLayout *const upper_layout = new QHBoxLayout();

    m_size_slider = new QSlider(Qt::Horizontal);
    m_size_slider->setRange(0, 100);
    m_size_edit   = new KLineEdit();
    m_size_edit->setAlignment(Qt::AlignRight);

    m_suffix_combo = new KComboBox();
    if(m_use_si_units){
        m_suffix_combo->insertItem(0,"MB");
        m_suffix_combo->insertItem(1,"GB");
        m_suffix_combo->insertItem(2,"TB");
    }
    else{
        m_suffix_combo->insertItem(0,"MiB");
        m_suffix_combo->insertItem(1,"GiB");
        m_suffix_combo->insertItem(2,"TiB");
    }

    if(m_is_volume)
        m_suffix_combo->insertItem(0, i18n("Extents"));
    m_suffix_combo->setInsertPolicy(KComboBox::NoInsert);
    m_suffix_combo->setCurrentIndex(1);

    if(m_is_new){
        if( !m_is_offset )
            m_current_size = m_constrained_max;
        else
            m_current_size = 0;
    }

    m_size_validator = new QDoubleValidator(m_size_edit);
    m_size_edit->setValidator(m_size_validator);
    m_size_validator->setBottom(0);

    if(m_is_volume){
        setTitle( i18n("Volume Size") );
        if( !m_is_new ){
            m_size_box = new QCheckBox( i18n("Lock selected size") );
            m_size_box->setChecked(false);
            layout->addWidget(m_size_box);
            setConstraints(false);

            connect(m_size_box, SIGNAL(toggled(bool)),
                    this, SLOT(lock(bool)));
        }
    }
    else if(m_is_offset){
        setTitle( i18n("Partition Start") );
        m_offset_box = new QCheckBox( i18n("Lock partition start") );
        m_offset_box->setChecked(false);
        setConstraints(false);
        layout->addWidget(m_offset_box);

        connect(m_offset_box, SIGNAL(toggled(bool)),
                this, SLOT(lock(bool)));
    }
    else{
        setTitle( i18n("Partition Size") );
        m_size_box = new QCheckBox( i18n("Lock selected size") );
        m_size_box->setChecked(false);
        layout->addWidget(m_size_box);

        connect(m_size_box, SIGNAL(toggled(bool)),
                this, SLOT(lock(bool)));

        if( !m_is_new ){
            m_shrink_box = new QCheckBox( i18n("Prevent shrinking") );
            m_shrink_box->setChecked(false);
            setConstraints(false);
            layout->addWidget(m_shrink_box);

            connect(m_shrink_box, SIGNAL(toggled(bool)),
                    this, SLOT(lockShrink(bool)));

            connect(m_size_box, SIGNAL(toggled(bool)),
                    this, SLOT(disableLockShrink(bool)));
        }
    }

    QLabel *const edit_label = new QLabel();

    if(m_is_offset)
        edit_label->setText( i18n("New start:") );
    else
        edit_label->setText( i18n("New size:") );

    edit_label->setBuddy(m_size_edit);
    upper_layout->addWidget(edit_label);
    upper_layout->addWidget(m_size_edit);
    upper_layout->addWidget(m_suffix_combo);
    layout->addLayout(upper_layout);
    layout->addWidget(m_size_slider);

    setLayout(layout);
    updateSlider();
    updateEdit();
    updateValidator();

    connect(m_size_edit, SIGNAL(textEdited(QString)),
	    this, SLOT(setToEdit(QString)));

    connect(m_size_slider, SIGNAL(sliderMoved(int)),
	    this, SLOT(setToSlider(int)));

    connect(m_suffix_combo, SIGNAL(currentIndexChanged(int)),
	    this, SLOT(updateEdit()));

    if( m_min_size == m_max_size ){
        setCurrentSize(m_min_size);
        m_size_box->setChecked(true);
        setEnabled(false);
    }
}

void SizeSelectorBox::resetToInitial()
{
    if( !isEnabled() )
        return;

    m_current_size = m_initial_size;

    if(m_is_volume){
        if( !m_is_new ){
            m_shrink_box->setChecked(false);
        }
    }
    else if(m_is_offset){
        m_offset_box->setChecked(false);
    }
    else{
        m_size_box->setChecked(false);

        if( !m_is_new ){
            m_shrink_box->setChecked(false);
        }
    }

    updateEdit();
    updateSlider();
}

void SizeSelectorBox::setToSlider(int value)
{
    long long new_size = ((m_constrained_max - m_constrained_min) * ( (double)value / 100 )) + m_constrained_min;
    m_size_slider->setValue(value);

    if( (value >= 100 ) || ( new_size > m_constrained_max ) )
        m_current_size = m_constrained_max;
    else if( ( value <= 0 ) || ( new_size < m_constrained_min ) )
        m_current_size = m_constrained_min;
    else 
        m_current_size = new_size; 
  
    updateEdit();
} 

bool SizeSelectorBox::setCurrentSize(long long size)
{
    if( isLocked() ){
        return false;
    }
    else if( size > m_constrained_max ){
        return false;
    }
    else if( size < m_constrained_min ){
        return false;
    }
    else{
        m_current_size = size;
        updateEdit();
        updateSlider();
        return true;
    }
}

void SizeSelectorBox::updateSlider()
{
    int percent = qRound(100.0 * ((double)(m_current_size - m_constrained_min)/(m_constrained_max - m_constrained_min))); 
    m_size_slider->setValue(percent);
}

void SizeSelectorBox::setConstrainedMax(long long max)
{
    if( isLocked() || !m_size_edit->isEnabled() )
        return;

    if( ( max < 0 ) || ( max > m_max_size ) )
        max = m_max_size;

    m_constrained_max = max;

    if( m_constrained_min > m_constrained_max )
        m_constrained_min = m_constrained_max;

    if( m_current_size > m_constrained_max )
        setCurrentSize(m_constrained_max);

    updateValidator();
    updateSlider();
}

long long  SizeSelectorBox::getCurrentSize()
{
    return m_current_size;
}

long long  SizeSelectorBox::getMaximumSize()
{
    return m_constrained_max;
}

long long  SizeSelectorBox::getMinimumSize()
{
    return m_constrained_min;
}

void SizeSelectorBox::lock(bool lock)
{
    setConstraints(lock);
    emit stateChanged();
}

void SizeSelectorBox::disableLockShrink(bool disable)
{
    m_shrink_box->setEnabled( !disable );
}

void SizeSelectorBox::setConstraints(bool lock)
{
    if(lock){
        if( !m_is_valid ){
            if(m_current_size > m_constrained_max)
                m_current_size = m_constrained_max;
            if(m_current_size < m_constrained_min)
                m_current_size = m_constrained_min;
            
            updateEdit();
        }
        
        m_constrained_max = m_current_size;
        m_constrained_min = m_current_size;
        m_size_edit->setEnabled(false);
        m_size_slider->setEnabled(false);
        m_suffix_combo->setEnabled(false);
    }
    else{
        if( !m_is_valid ){
            if(m_current_size > m_constrained_max)
                m_current_size = m_constrained_max;
            if(m_current_size < m_constrained_min)
                m_current_size = m_constrained_min;

            updateEdit();
        }

        m_constrained_min = m_min_size;
        m_size_edit->setEnabled(true);
        m_size_slider->setEnabled(true);
        m_suffix_combo->setEnabled(true);
        m_constrained_max = m_max_size;

        updateValidator();
        updateSlider();
    }
}

void SizeSelectorBox::lockShrink(bool lock)
{
    if(lock){
        if( !m_is_valid ){
            if(m_current_size > m_constrained_max)
                m_current_size = m_constrained_max;
            if(m_current_size < m_constrained_min)
                m_current_size = m_constrained_min;
            
            updateEdit();
        }
        
        m_constrained_min = m_initial_size;
        if(m_current_size < m_constrained_min){
            m_current_size = m_constrained_min;
            updateValidator();
            updateSlider();
        }
    }
    else{
        if( !m_is_valid ){
            if(m_current_size > m_constrained_max)
                m_current_size = m_constrained_max;
            if(m_current_size < m_constrained_min)
                m_current_size = m_constrained_min;

            updateEdit();
        }

        m_constrained_min = m_min_size;
        updateValidator();
        updateSlider();
    }

    emit stateChanged();
}

void SizeSelectorBox::setToEdit(QString size)
{
    long long proposed_size;
    int x = 0;

    if(m_size_validator->validate(size, x) == QValidator::Acceptable){

        if(m_suffix_combo->currentIndex() == 0 && m_is_volume)
            proposed_size = size.toLongLong();
        else
            proposed_size = convertSizeToUnits(m_suffix_combo->currentIndex(), size.toDouble());

        if( (proposed_size >= m_constrained_min) && (proposed_size <= m_constrained_max)){
            m_current_size = proposed_size;
            m_is_valid = true;

            emit stateChanged();
            return;     
        }
    }

    m_is_valid = false;
    emit stateChanged();
}

long long SizeSelectorBox::convertSizeToUnits(int index, double size)
{
    if(m_is_volume)
        index -= 1;

    long double partition_size = size;

    if(m_use_si_units){
        if(index == 0)
            partition_size *= (long double)1.0E6;
        else if(index == 1)
            partition_size *= (long double)1.0E9;
        else{
            partition_size *= (long double)1.0E6;
            partition_size *= (long double)1.0E6;
        }
    }
    else{
        if(index == 0)
            partition_size *= (long double)0x100000;
        else if(index == 1)
            partition_size *= (long double)0x40000000;
        else{
            partition_size *= (long double)0x100000;
            partition_size *= (long double)0x100000;
        }
    }

    partition_size /= m_unit_size;

    if (partition_size < 0)
        partition_size = 0;

    return qRound64(partition_size);
}

void SizeSelectorBox::updateEdit()
{
    long double sized =  (long double)m_current_size * m_unit_size;
    int index = m_suffix_combo->currentIndex();

    updateValidator();

    if(m_is_volume)
        index -= 1;

    if(m_use_si_units){
        if(index == 0)
            sized /= (long double)1.0E6;
        else if(index == 1)
            sized /= (long double)1.0E9;
        else{
            sized /= (long double)1.0E6;
            sized /= (long double)1.0E6;
        }
    }
    else{
        if(index == 0)
            sized /= (long double)0x100000;
        else if(index == 1)
            sized /= (long double)0x40000000;
        else{
            sized /= (long double)0x100000;
            sized /= (long double)0x100000;
        }
    }

    if( index == -1 )
        m_size_edit->setText( QString("%1").arg( (double)m_current_size, 0, 'g', 5) );
    else
        m_size_edit->setText( QString("%1").arg( (double)sized, 0, 'g', 5) );
    
    if( (m_current_size >= m_constrained_min) && (m_current_size <= m_constrained_max))
        m_is_valid = true;
    else
        m_is_valid = false;

    emit stateChanged();
}

void SizeSelectorBox::updateValidator()
{
    long double valid_topd = (long double)m_constrained_max * m_unit_size;
    long double valid_bottomd = (long double)m_constrained_min * m_unit_size;
    int index = m_suffix_combo->currentIndex();

    if(m_is_volume)
        index -= 1;

    if(m_use_si_units){
        if(index == 0){
            valid_topd /= (long double)1.0E6;
            valid_bottomd /= (long double)1.0E6;
        }
        else if(index == 1){
            valid_topd /= (long double)1.0E9;
            valid_bottomd /= (long double)1.0E9;
        }
        else{
            valid_topd /= (long double)1.0E6;
            valid_topd /= (long double)1.0E6;
            valid_bottomd /= (long double)1.0E6;
            valid_bottomd /= (long double)1.0E6;
        }
    }
    else{
        if(index == 0){
            valid_topd /= (long double)0x100000;
            valid_bottomd /= (long double)0x100000;
        }
        else if(index == 1){
            valid_topd /= (long double)0x40000000;
            valid_bottomd /= (long double)0x40000000;
        }
        else{
            valid_topd /= (long double)0x100000;
            valid_topd /= (long double)0x100000;
            valid_bottomd /= (long double)0x100000;
            valid_bottomd /= (long double)0x100000;
        }
        
    }

    if( valid_bottomd < 0 )
        valid_bottomd = 0;
        
    if( index != -1 ){    
        m_size_validator->setTop( (double)valid_topd );
        m_size_validator->setBottom( (double)valid_bottomd );
    }
    else{
        m_size_validator->setTop( (double)m_constrained_max );
        m_size_validator->setBottom( (double)m_constrained_min );
    }
}

bool SizeSelectorBox::isValid()
{
    return m_is_valid;
}

bool SizeSelectorBox::isLocked() // this should include the lock button too
{
    return !isEnabled();
}

