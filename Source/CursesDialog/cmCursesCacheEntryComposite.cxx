/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCursesCacheEntryComposite.h"
#include "cmCursesStringWidget.h"
#include "cmCursesLabelWidget.h"
#include "cmCursesBoolWidget.h"
#include "cmCursesPathWidget.h"
#include "cmCursesFilePathWidget.h"
#include "cmCursesDummyWidget.h"
#include "../cmSystemTools.h"

cmCursesCacheEntryComposite::cmCursesCacheEntryComposite(const char* key,
							 int labelwidth,
							 int entrywidth) :
  m_Key(key), m_LabelWidth(labelwidth), m_EntryWidth(entrywidth)
{
  m_Label = new cmCursesLabelWidget(m_LabelWidth, 1, 1, 1, key);
  m_IsNewLabel = new cmCursesLabelWidget(1, 1, 1, 1, " ");
  m_Entry = 0;
}

cmCursesCacheEntryComposite::cmCursesCacheEntryComposite(
  const char* key, const cmCacheManager::CacheEntry& value, bool isNew, 
  int labelwidth, int entrywidth) 
  : m_Key(key), m_LabelWidth(labelwidth), m_EntryWidth(entrywidth)
{
  m_Label = new cmCursesLabelWidget(m_LabelWidth, 1, 1, 1, key);
  if (isNew)
    {
    m_IsNewLabel = new cmCursesLabelWidget(1, 1, 1, 1, "*");
    }
  else
    {
    m_IsNewLabel = new cmCursesLabelWidget(1, 1, 1, 1, " ");
    }

  m_Entry = 0;
  switch ( value.m_Type )
    {
    case  cmCacheManager::BOOL:
      m_Entry = new cmCursesBoolWidget(m_EntryWidth, 1, 1, 1);
      if (cmSystemTools::IsOn(value.m_Value.c_str()))
	{
	static_cast<cmCursesBoolWidget*>(m_Entry)->SetValueAsBool(true);
	}
      else
	{
	static_cast<cmCursesBoolWidget*>(m_Entry)->SetValueAsBool(false);
	}
      break;
    case cmCacheManager::PATH:
      m_Entry = new cmCursesPathWidget(m_EntryWidth, 1, 1, 1);
      static_cast<cmCursesPathWidget*>(m_Entry)->SetString(
	value.m_Value.c_str());
      break;
    case cmCacheManager::FILEPATH:
      m_Entry = new cmCursesFilePathWidget(m_EntryWidth, 1, 1, 1);
      static_cast<cmCursesFilePathWidget*>(m_Entry)->SetString(
	value.m_Value.c_str());
      break;
    case cmCacheManager::STRING:
      m_Entry = new cmCursesStringWidget(m_EntryWidth, 1, 1, 1);
      static_cast<cmCursesStringWidget*>(m_Entry)->SetString(
	value.m_Value.c_str());
      break;
    default:
      // TODO : put warning message here
      break;
    }

}

cmCursesCacheEntryComposite::~cmCursesCacheEntryComposite()
{
  delete m_Label;
  delete m_IsNewLabel;
  delete m_Entry;
}

const char* cmCursesCacheEntryComposite::GetValue()
{
  if (m_Label)
    {
    return m_Label->GetValue();
    }
  else
    {
    return 0;
    }
}
