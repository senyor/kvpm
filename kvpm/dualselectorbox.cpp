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


#include <KLocale>

#include <QtGui>

#include "dualselectorbox.h"
#include "sizeselectorbox.h"



DualSelectorBox::DualSelectorBox(long long unitSize, 
                                 long long minSize,   long long maxSize,   long long initialSize, 
                                 long long minOffset, long long maxOffset, long long initialOffset, 
                                 QWidget *parent) : QWidget(parent)
{

    m_max_size = maxSize;
    QVBoxLayout *layout = new QVBoxLayout();

    m_size_selector   = new SizeSelectorBox(unitSize, minSize,   maxSize,   initialSize,   false, false, true);
    m_offset_selector = new SizeSelectorBox(unitSize, minOffset, maxOffset, initialOffset, false, true);

    layout->addWidget(m_size_selector);
    layout->addWidget(m_offset_selector);
    setLayout(layout);

    connect(m_size_selector, SIGNAL(stateChanged()),
	    this , SLOT(sizeChanged()));

    connect(m_offset_selector, SIGNAL(stateChanged()),
	    this , SLOT(offsetChanged()));

}

void DualSelectorBox::sizeChanged()
{
    const long long max = m_max_size;
    const long long current_size   = m_size_selector->getCurrentSize();
    const long long current_offset = m_offset_selector->getCurrentSize();

    if( m_size_selector->isValid() ){
        if( ! m_offset_selector->isValid() )
            m_offset_selector->setCurrentSize( m_offset_selector->getCurrentSize() ); // reset to last valid value 

        if( ! m_offset_selector->isLocked() ){
                
            if( m_size_selector->isLocked() )
                m_offset_selector->setConstrainedMax( max - current_size );
            else
                m_offset_selector->setConstrainedMax( max - m_size_selector->getMinimumSize() );
            
            if( m_offset_selector->getCurrentSize() > ( max - current_size ) )
                if( ! m_offset_selector->setCurrentSize( max - current_size ) )
                    m_size_selector->setCurrentSize( max - current_offset );
        }
        else{
            m_size_selector->setConstrainedMax( max - current_offset);
        }        
    }

    emit changed();
} 

void DualSelectorBox::offsetChanged()
{
    const long long max = m_max_size;
    const long long current_size   = m_size_selector->getCurrentSize();
    const long long current_offset = m_offset_selector->getCurrentSize();

    if( m_offset_selector->isValid() ){
        if( ! m_size_selector->isValid() )
            m_size_selector->setCurrentSize( m_size_selector->getCurrentSize() ); // poke it to make it valid 

        if( ! m_size_selector->isLocked() ){
            m_size_selector->setConstrainedMax( max );

            if( m_offset_selector->isLocked() )
                m_size_selector->setConstrainedMax( max - current_offset );
            else
                m_size_selector->setConstrainedMax( max - m_offset_selector->getMinimumSize() );

            if( m_size_selector->getCurrentSize() > ( max - current_offset ) )
                if( ! m_size_selector->setCurrentSize( max - current_offset ) )
                    m_offset_selector->setCurrentSize( max - current_size );
        }
        else{
            m_offset_selector->setConstrainedMax( max - current_size );
        }
    }

    emit changed();
}

void DualSelectorBox::resetSelectors()
{
    m_size_selector->resetToInitial();
    m_offset_selector->resetToInitial();
}

long long DualSelectorBox::getCurrentSize()
{
    return m_size_selector->getCurrentSize();
}

long long DualSelectorBox::getCurrentOffset()
{
    return m_offset_selector->getCurrentSize();
}

bool DualSelectorBox::isValid()
{
    return ( m_size_selector->isValid() && m_offset_selector->isValid() ); 
}

