#include "cmCursesCacheEntryComposite.h"
#include "cmCursesStringWidget.h"
#include "cmCursesLabelWidget.h"
#include "cmCursesBoolWidget.h"
#include "cmCursesPathWidget.h"
#include "cmCursesFilePathWidget.h"
#include "cmCursesDummyWidget.h"
#include "cmSystemTools.h"

cmCursesCacheEntryComposite::cmCursesCacheEntryComposite(const char* key) :
  m_Key(key)
{
  m_Label = new cmCursesLabelWidget(30, 1, 1, 1, key);
  m_IsNewLabel = new cmCursesLabelWidget(1, 1, 1, 1, " ");
  m_Entry = 0;
}

cmCursesCacheEntryComposite::cmCursesCacheEntryComposite(
  const char* key, const cmCacheManager::CacheEntry& value, bool isNew) :
  m_Key(key)
{
  m_Label = new cmCursesLabelWidget(30, 1, 1, 1, key);
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
      m_Entry = new cmCursesBoolWidget(30, 1, 1, 1);
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
      m_Entry = new cmCursesPathWidget(30, 1, 1, 1);
      static_cast<cmCursesPathWidget*>(m_Entry)->SetString(
	value.m_Value.c_str());
      break;
    case cmCacheManager::FILEPATH:
      m_Entry = new cmCursesFilePathWidget(30, 1, 1, 1);
      static_cast<cmCursesFilePathWidget*>(m_Entry)->SetString(
	value.m_Value.c_str());
      break;
    case cmCacheManager::STRING:
      m_Entry = new cmCursesStringWidget(30, 1, 1, 1);
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
