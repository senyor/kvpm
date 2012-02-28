/*
 *
 * 
 * Copyright (C) 2011, 2012 Benjamin Scott   <benscott@nwlink.com>
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



DualSelectorBox::DualSelectorBox(const long long sectorSize,  const long long totalSpace, QWidget *parent) 
    : QWidget(parent), m_space(totalSpace)
{

    QVBoxLayout *const layout = new QVBoxLayout();
    layout->setMargin(0);

    const long ONE_MIB = 0x100000 / sectorSize;

    m_size_selector   = new SizeSelectorBox(sectorSize, 2 * ONE_MIB, m_space, m_space, false, false, true);
    m_offset_selector = new SizeSelectorBox(sectorSize, 0, m_space - ONE_MIB, 0, false, true);

    layout->addWidget(m_size_selector);
    layout->addWidget(m_offset_selector);
    setLayout(layout);

    connect(m_size_selector, SIGNAL(stateChanged()),
	    this , SLOT(sizeChanged()));

    connect(m_offset_selector, SIGNAL(stateChanged()),
	    this , SLOT(offsetChanged()));
}

DualSelectorBox::DualSelectorBox(const long long sectorSize, const long long totalSpace, 
                                 const long long minSize,    const long long maxSize,   const long long initialSize, 
                                 const long long minOffset,  const long long maxOffset, const long long initialOffset, 
                                 QWidget *parent) 
    : QWidget(parent), m_space(totalSpace)
{

    QVBoxLayout *const layout = new QVBoxLayout();
    layout->setMargin(0);

    m_size_selector   = new SizeSelectorBox(sectorSize, minSize,   maxSize,   initialSize,   false, false, true);

    if(minSize == maxSize)
        m_offset_selector = new SizeSelectorBox(sectorSize, minOffset, maxOffset, initialOffset, false, true, false, false);
    else
        m_offset_selector = new SizeSelectorBox(sectorSize, minOffset, maxOffset, initialOffset, false, true, false, true);

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
    const long long max = m_space;
    const long long current_size   = m_size_selector->getCurrentSize();
    const long long current_offset = m_offset_selector->getCurrentSize();

    if( m_size_selector->isValid() ){
        if( !m_offset_selector->isValid() )
            m_offset_selector->setCurrentSize( m_offset_selector->getCurrentSize() ); // reset to last valid value 

        if( !m_offset_selector->isLocked() ){
                
            if( m_size_selector->isLocked() )
                m_offset_selector->setConstrainedMax(max - current_size);
            else
                m_offset_selector->setConstrainedMax( max - m_size_selector->getMinimumSize() );
            
            if( m_offset_selector->getCurrentSize() > (max - current_size) )
                if( !m_offset_selector->setCurrentSize(max - current_size) )
                    m_size_selector->setCurrentSize(max - current_offset);
        }
        else{
            m_size_selector->setConstrainedMax(max - current_offset);
        }        
    }

    emit changed();
} 

void DualSelectorBox::offsetChanged()
{
    const long long max = m_space;
    const long long current_size   = m_size_selector->getCurrentSize();
    const long long current_offset = m_offset_selector->getCurrentSize();

    if( m_offset_selector->isValid() ){

        if( !m_size_selector->isValid() )
            m_size_selector->setCurrentSize( m_size_selector->getCurrentSize() ); // poke it to make it valid 

        if( !m_size_selector->isLocked() ){
            m_size_selector->setConstrainedMax(max);

            if( m_offset_selector->isLocked() )
                m_size_selector->setConstrainedMax(max - current_offset);
            else
                m_size_selector->setConstrainedMax( max - m_offset_selector->getMinimumSize() );

            if( m_size_selector->getCurrentSize() > (max - current_offset) )
                if( !m_size_selector->setCurrentSize(max - current_offset) )
                    m_offset_selector->setCurrentSize(max - current_size);
        }
        else{
            m_offset_selector->setConstrainedMax(max - current_size);
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

