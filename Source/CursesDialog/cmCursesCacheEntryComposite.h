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
#ifndef __cmCursesCacheEntryComposite_h
#define __cmCursesCacheEntryComposite_h

#include "../cmCacheManager.h"
#include "cmCursesLabelWidget.h"

class cmCursesCacheEntryComposite
{
public:
  cmCursesCacheEntryComposite(const char* key, int labelwidth, int entrywidth);
  cmCursesCacheEntryComposite(const char* key,
			      const cmCacheManager::CacheIterator& it, 
			      bool isNew, int labelwidth, int entrywidth);
  ~cmCursesCacheEntryComposite();
  const char* GetValue();

  friend class cmCursesMainForm;

protected:
  cmCursesCacheEntryComposite(const cmCursesCacheEntryComposite& from);
  void operator=(const cmCursesCacheEntryComposite&);

  cmCursesLabelWidget* m_Label;
  cmCursesLabelWidget* m_IsNewLabel;
  cmCursesWidget* m_Entry;
  std::string m_Key;
  int m_LabelWidth;
  int m_EntryWidth;
};

#endif // __cmCursesCacheEntryComposite_h
