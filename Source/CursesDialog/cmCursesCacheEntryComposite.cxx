/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
  Key(key), LabelWidth(labelwidth), EntryWidth(entrywidth)
{
  this->Label = new cmCursesLabelWidget(this->LabelWidth, 1, 1, 1, key);
  this->IsNewLabel = new cmCursesLabelWidget(1, 1, 1, 1, " ");
  this->Entry = 0; 
  this->Entry = new cmCursesStringWidget(this->EntryWidth, 1, 1, 1);
}

cmCursesCacheEntryComposite::cmCursesCacheEntryComposite(
  const char* key, const cmCacheManager::CacheIterator& it, bool isNew, 
  int labelwidth, int entrywidth) 
  : Key(key), LabelWidth(labelwidth), EntryWidth(entrywidth)
{
  this->Label = new cmCursesLabelWidget(this->LabelWidth, 1, 1, 1, key);
  if (isNew)
    {
    this->IsNewLabel = new cmCursesLabelWidget(1, 1, 1, 1, "*");
    }
  else
    {
    this->IsNewLabel = new cmCursesLabelWidget(1, 1, 1, 1, " ");
    }

  this->Entry = 0;
  switch ( it.GetType() )
    {
    case  cmCacheManager::BOOL:
      this->Entry = new cmCursesBoolWidget(this->EntryWidth, 1, 1, 1);
      if (cmSystemTools::IsOn(it.GetValue()))
        {
        static_cast<cmCursesBoolWidget*>(this->Entry)->SetValueAsBool(true);
        }
      else
        {
        static_cast<cmCursesBoolWidget*>(this->Entry)->SetValueAsBool(false);
        }
      break;
    case cmCacheManager::PATH:
      this->Entry = new cmCursesPathWidget(this->EntryWidth, 1, 1, 1);
      static_cast<cmCursesPathWidget*>(this->Entry)->SetString(
        it.GetValue());
      break;
    case cmCacheManager::FILEPATH:
      this->Entry = new cmCursesFilePathWidget(this->EntryWidth, 1, 1, 1);
      static_cast<cmCursesFilePathWidget*>(this->Entry)->SetString(
        it.GetValue());
      break;
    case cmCacheManager::STRING:
      this->Entry = new cmCursesStringWidget(this->EntryWidth, 1, 1, 1);
      static_cast<cmCursesStringWidget*>(this->Entry)->SetString(
        it.GetValue());
      break;
    case cmCacheManager::UNINITIALIZED:
      cmSystemTools::Error("Found an undefined variable: ", it.GetName());      
      break;
    default:
      // TODO : put warning message here
      break;
    }

}

cmCursesCacheEntryComposite::~cmCursesCacheEntryComposite()
{
  delete this->Label;
  delete this->IsNewLabel;
  delete this->Entry;
}

const char* cmCursesCacheEntryComposite::GetValue()
{
  if (this->Label)
    {
    return this->Label->GetValue();
    }
  else
    {
    return 0;
    }
}
