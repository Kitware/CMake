/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <memory>
#include <string>

class cmCursesLabelWidget;
class cmCursesWidget;
class cmState;

class cmCursesCacheEntryComposite
{
public:
  cmCursesCacheEntryComposite(std::string const& key, int labelwidth,
                              int entrywidth);
  cmCursesCacheEntryComposite(std::string const& key, cmState* state,
                              bool isNew, int labelwidth, int entrywidth);
  ~cmCursesCacheEntryComposite();

  cmCursesCacheEntryComposite(cmCursesCacheEntryComposite const&) = delete;
  cmCursesCacheEntryComposite& operator=(cmCursesCacheEntryComposite const&) =
    delete;

  cmCursesCacheEntryComposite(cmCursesCacheEntryComposite&&) = default;
  cmCursesCacheEntryComposite& operator=(cmCursesCacheEntryComposite&&) =
    default;

  char const* GetValue();

  friend class cmCursesMainForm;

protected:
  std::unique_ptr<cmCursesLabelWidget> Label;
  std::unique_ptr<cmCursesLabelWidget> IsNewLabel;
  std::unique_ptr<cmCursesWidget> Entry;
  std::string Key;
  int LabelWidth;
  int EntryWidth;
};
