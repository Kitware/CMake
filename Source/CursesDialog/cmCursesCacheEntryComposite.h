#ifndef __cmCursesCacheEntryComposite_h
#define __cmCursesCacheEntryComposite_h

#include "cmCursesLabelWidget.h"
#include "cmCacheManager.h"

class cmCursesCacheEntryComposite
{
public:
  cmCursesCacheEntryComposite(const char* key);
  cmCursesCacheEntryComposite(const char* key,
			      const cmCacheManager::CacheEntry& value, 
			      bool isNew);
  ~cmCursesCacheEntryComposite();

  friend class cmCursesMainForm;

protected:
  cmCursesCacheEntryComposite(const cmCursesCacheEntryComposite& from);
  void operator=(const cmCursesCacheEntryComposite&);

  cmCursesLabelWidget* m_Label;
  cmCursesLabelWidget* m_IsNewLabel;
  cmCursesWidget* m_Entry;
  std::string m_Key;
};

#endif // __cmCursesCacheEntryComposite_h
