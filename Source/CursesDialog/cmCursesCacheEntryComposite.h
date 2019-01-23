/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#ifndef cmCursesCacheEntryComposite_h
#define cmCursesCacheEntryComposite_h

#include "cmConfigure.h" // IWYU pragma: keep

#include <string>

class cmCursesLabelWidget;
class cmCursesWidget;
class cmake;

class cmCursesCacheEntryComposite
{
public:
  cmCursesCacheEntryComposite(const std::string& key, int labelwidth,
                              int entrywidth);
  cmCursesCacheEntryComposite(const std::string& key, cmake* cm, bool isNew,
                              int labelwidth, int entrywidth);
  ~cmCursesCacheEntryComposite();

  cmCursesCacheEntryComposite(cmCursesCacheEntryComposite const&) = delete;
  cmCursesCacheEntryComposite& operator=(cmCursesCacheEntryComposite const&) =
    delete;

  const char* GetValue();

  friend class cmCursesMainForm;

protected:
  cmCursesLabelWidget* Label;
  cmCursesLabelWidget* IsNewLabel;
  cmCursesWidget* Entry;
  std::string Key;
  int LabelWidth;
  int EntryWidth;
};

#endif // cmCursesCacheEntryComposite_h
