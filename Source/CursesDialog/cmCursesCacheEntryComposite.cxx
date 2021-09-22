/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCursesCacheEntryComposite.h"

#include <cassert>
#include <utility>
#include <vector>

#include <cm/memory>

#include "cmCursesBoolWidget.h"
#include "cmCursesFilePathWidget.h"
#include "cmCursesLabelWidget.h"
#include "cmCursesOptionsWidget.h"
#include "cmCursesPathWidget.h"
#include "cmCursesStringWidget.h"
#include "cmCursesWidget.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"

cmCursesCacheEntryComposite::cmCursesCacheEntryComposite(
  const std::string& key, int labelwidth, int entrywidth)
  : Key(key)
  , LabelWidth(labelwidth)
  , EntryWidth(entrywidth)
{
  this->Label =
    cm::make_unique<cmCursesLabelWidget>(this->LabelWidth, 1, 1, 1, key);
  this->IsNewLabel = cm::make_unique<cmCursesLabelWidget>(1, 1, 1, 1, " ");
  this->Entry =
    cm::make_unique<cmCursesStringWidget>(this->EntryWidth, 1, 1, 1);
}

cmCursesCacheEntryComposite::cmCursesCacheEntryComposite(
  const std::string& key, cmState* state, bool isNew, int labelwidth,
  int entrywidth)
  : Key(key)
  , LabelWidth(labelwidth)
  , EntryWidth(entrywidth)
{
  this->Label =
    cm::make_unique<cmCursesLabelWidget>(this->LabelWidth, 1, 1, 1, key);
  if (isNew) {
    this->IsNewLabel = cm::make_unique<cmCursesLabelWidget>(1, 1, 1, 1, "*");
  } else {
    this->IsNewLabel = cm::make_unique<cmCursesLabelWidget>(1, 1, 1, 1, " ");
  }

  cmValue value = state->GetCacheEntryValue(key);
  assert(value);
  switch (state->GetCacheEntryType(key)) {
    case cmStateEnums::BOOL: {
      auto bw = cm::make_unique<cmCursesBoolWidget>(this->EntryWidth, 1, 1, 1);
      bw->SetValueAsBool(cmIsOn(*value));
      this->Entry = std::move(bw);
      break;
    }
    case cmStateEnums::PATH: {
      auto pw = cm::make_unique<cmCursesPathWidget>(this->EntryWidth, 1, 1, 1);
      pw->SetString(*value);
      this->Entry = std::move(pw);
      break;
    }
    case cmStateEnums::FILEPATH: {
      auto fpw =
        cm::make_unique<cmCursesFilePathWidget>(this->EntryWidth, 1, 1, 1);
      fpw->SetString(*value);
      this->Entry = std::move(fpw);
      break;
    }
    case cmStateEnums::STRING: {
      cmValue stringsProp = state->GetCacheEntryProperty(key, "STRINGS");
      if (stringsProp) {
        auto ow =
          cm::make_unique<cmCursesOptionsWidget>(this->EntryWidth, 1, 1, 1);
        for (std::string const& opt : cmExpandedList(*stringsProp)) {
          ow->AddOption(opt);
        }
        ow->SetOption(*value);
        this->Entry = std::move(ow);
      } else {
        auto sw =
          cm::make_unique<cmCursesStringWidget>(this->EntryWidth, 1, 1, 1);
        sw->SetString(*value);
        this->Entry = std::move(sw);
      }
      break;
    }
    case cmStateEnums::UNINITIALIZED:
      cmSystemTools::Error("Found an undefined variable: " + key);
      break;
    default:
      // TODO : put warning message here
      break;
  }
}

cmCursesCacheEntryComposite::~cmCursesCacheEntryComposite() = default;

const char* cmCursesCacheEntryComposite::GetValue()
{
  if (this->Label) {
    return this->Label->GetValue();
  }
  return nullptr;
}
