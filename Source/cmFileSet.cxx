/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmFileSet.h"

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include <cm/optional>
#include <cmext/algorithm>
#include <cmext/string_view>

#include "cmList.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmRange.h"
#include "cmStringAlgorithms.h"
#include "cmTarget.h"

namespace Metadata = cm::FileSetMetadata;

cmFileSet::cmFileSet(cmMakefile* makefile, cmTarget* target, std::string name,
                     std::string type, Metadata::Visibility visibility)
  : Makefile(makefile)
  , Target(target)
  , Name(std::move(name))
  , Type(std::move(type))
  , Visibility(visibility)
{
}

void cmFileSet::CopyEntries(cmFileSet const* fs)
{
  cm::append(this->DirectoryEntries, fs->DirectoryEntries);
  cm::append(this->FileEntries, fs->FileEntries);
}

void cmFileSet::ClearDirectoryEntries()
{
  this->DirectoryEntries.clear();
}

void cmFileSet::AddDirectoryEntry(BT<std::string> directories)
{
  this->DirectoryEntries.push_back(std::move(directories));
}

void cmFileSet::ClearFileEntries()
{
  this->FileEntries.clear();
}

void cmFileSet::AddFileEntry(BT<std::string> files)
{
  this->FileEntries.push_back(std::move(files));
}

cmBTStringRange cmFileSet::GetIncludeDirectories() const
{
  return cmMakeRange(this->IncludeDirectories);
}
cmBTStringRange cmFileSet::GetInterfaceIncludeDirectories() const
{
  return cmMakeRange(this->InterfaceIncludeDirectories);
}

cmBTStringRange cmFileSet::GetCompileOptions() const
{
  return cmMakeRange(this->CompileOptions);
}
cmBTStringRange cmFileSet::GetInterfaceCompileOptions() const
{
  return cmMakeRange(this->InterfaceCompileOptions);
}

cmBTStringRange cmFileSet::GetCompileDefinitions() const
{
  return cmMakeRange(this->CompileDefinitions);
}
cmBTStringRange cmFileSet::GetInterfaceCompileDefinitions() const
{
  return cmMakeRange(this->InterfaceCompileDefinitions);
}

namespace {
enum class ReadOnlyCondition
{
  All,
  Imported,
  NonImported,
};

struct ReadOnlyProperty
{
  ReadOnlyProperty(ReadOnlyCondition cond)
    : Condition{ cond }
  {
  }
  // ReadOnlyProperty(ReadOnlyCondition cond, cmPolicies::PolicyID id)
  //   : Condition{ cond }
  //   , Policy{ id }
  // {
  // }

  ReadOnlyCondition Condition;
  cm::optional<cmPolicies::PolicyID> Policy;

  std::string message(std::string const& prop, cmTarget* target,
                      cmFileSet* fileSet) const
  {
    std::string msg;
    if (this->Condition == ReadOnlyCondition::All) {
      msg = cmStrCat(" property is read-only for the file set \"",
                     fileSet->GetName(), " of the target \"");
    } else if (this->Condition == ReadOnlyCondition::Imported) {
      msg = " property can't be set on a file set attached to the imported "
            "target \"";
    } else if (this->Condition == ReadOnlyCondition::NonImported) {
      msg =
        " property can't be set on a file set attached to the non-imported "
        "target \"";
    }
    return cmStrCat(prop, msg, target->GetName(), "\"\n");
  }

  bool isReadOnly(std::string const& prop, cmMakefile* context,
                  cmTarget* target, cmFileSet* fileSet) const
  {
    auto importedTarget = target->IsImported();
    bool matchingCondition = true;
    if ((!importedTarget && this->Condition == ReadOnlyCondition::Imported) ||
        (importedTarget &&
         this->Condition == ReadOnlyCondition::NonImported)) {
      matchingCondition = false;
    }
    if (!matchingCondition) {
      // Not read-only in this scenario
      return false;
    }

    bool readOnly = true;
    if (!this->Policy) {
      // No policy associated, so is always read-only
      context->IssueMessage(MessageType::FATAL_ERROR,
                            this->message(prop, target, fileSet));
    }
    return readOnly;
  }
};

bool IsSettableProperty(cmMakefile* context, cmTarget* target,
                        cmFileSet* fileSet, std::string const& prop)
{
  using ROC = ReadOnlyCondition;
  static std::unordered_map<std::string, ReadOnlyProperty> const readOnlyProps{
    { "TYPE", { ROC::All } }, { "SCOPE", { ROC::All } }
  };

  auto it = readOnlyProps.find(prop);

  if (it != readOnlyProps.end()) {
    return !(it->second.isReadOnly(prop, context, target, fileSet));
  }
  return true;
}

cm::string_view const BASE_DIRS = "BASE_DIRS"_s;
cm::string_view const SOURCES = "SOURCES"_s;
cm::string_view const INTERFACE_SOURCES = "INTERFACE_SOURCES"_s;
cm::string_view const INCLUDE_DIRECTORIES = "INCLUDE_DIRECTORIES"_s;
cm::string_view const INTERFACE_INCLUDE_DIRECTORIES =
  "INTERFACE_INCLUDE_DIRECTORIES"_s;
cm::string_view const COMPILE_DEFINITIONS = "COMPILE_DEFINITIONS"_s;
cm::string_view const INTERFACE_COMPILE_DEFINITIONS =
  "INTERFACE_COMPILE_DEFINITIONS"_s;
cm::string_view const COMPILE_OPTIONS = "COMPILE_OPTIONS"_s;
cm::string_view const INTERFACE_COMPILE_OPTIONS =
  "INTERFACE_COMPILE_OPTIONS"_s;
}

void cmFileSet::SetProperty(std::string const& prop, cmValue value)
{
  if (!IsSettableProperty(this->Makefile, this->Target, this, prop)) {
    return;
  }

  if (prop == BASE_DIRS) {
    this->ClearDirectoryEntries();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->AddDirectoryEntry(BT<std::string>{ value, lfbt });
    }
  } else if (prop == SOURCES) {
    if (!this->IsForSelf()) {
      return;
    }
    this->ClearFileEntries();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->AddFileEntry(BT<std::string>{ value, lfbt });
    }
  } else if (prop == INTERFACE_SOURCES) {
    if (!this->IsForInterface()) {
      return;
    }
    this->ClearFileEntries();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->AddFileEntry(BT<std::string>{ value, lfbt });
    }
  } else if (prop == INCLUDE_DIRECTORIES) {
    if (!this->IsForSelf()) {
      return;
    }
    this->IncludeDirectories.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->IncludeDirectories.emplace_back(value, lfbt);
    }
  } else if (prop == INTERFACE_INCLUDE_DIRECTORIES) {
    if (!this->IsForInterface()) {
      return;
    }
    this->InterfaceIncludeDirectories.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->InterfaceIncludeDirectories.emplace_back(value, lfbt);
    }
  } else if (prop == COMPILE_OPTIONS) {
    if (!this->IsForSelf()) {
      return;
    }
    this->CompileOptions.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->CompileOptions.emplace_back(value, lfbt);
    }
  } else if (prop == INTERFACE_COMPILE_OPTIONS) {
    if (!this->IsForInterface()) {
      return;
    }
    this->InterfaceCompileOptions.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->InterfaceCompileOptions.emplace_back(value, lfbt);
    }
  } else if (prop == COMPILE_DEFINITIONS) {
    if (!this->IsForSelf()) {
      return;
    }
    this->CompileDefinitions.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->CompileDefinitions.emplace_back(value, lfbt);
    }
  } else if (prop == INTERFACE_COMPILE_DEFINITIONS) {
    if (!this->IsForInterface()) {
      return;
    }
    this->InterfaceCompileDefinitions.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->InterfaceCompileDefinitions.emplace_back(value, lfbt);
    }
  } else {
    this->Properties.SetProperty(prop, value);
  }
}

void cmFileSet::AppendProperty(std::string const& prop,
                               std::string const& value, bool asString)
{
  if (!IsSettableProperty(this->Makefile, this->Target, this, prop)) {
    return;
  }

  if (prop == BASE_DIRS) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->AddDirectoryEntry(BT<std::string>{ value, lfbt });
    }
  } else if (prop == SOURCES) {
    if (!this->IsForSelf()) {
      return;
    }
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->AddFileEntry(BT<std::string>{ value, lfbt });
    }
  } else if (prop == INTERFACE_SOURCES) {
    if (!this->IsForInterface()) {
      return;
    }
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->AddFileEntry(BT<std::string>{ value, lfbt });
    }
  } else if (prop == INCLUDE_DIRECTORIES) {
    if (!this->IsForSelf()) {
      return;
    }
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->IncludeDirectories.emplace_back(value, lfbt);
    }
  } else if (prop == INTERFACE_INCLUDE_DIRECTORIES) {
    if (!this->IsForInterface()) {
      return;
    }
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->InterfaceIncludeDirectories.emplace_back(value, lfbt);
    }
  } else if (prop == COMPILE_OPTIONS) {
    if (!this->IsForSelf()) {
      return;
    }
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->CompileOptions.emplace_back(value, lfbt);
    }
  } else if (prop == INTERFACE_COMPILE_OPTIONS) {
    if (!this->IsForInterface()) {
      return;
    }
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->InterfaceCompileOptions.emplace_back(value, lfbt);
    }
  } else if (prop == COMPILE_DEFINITIONS) {
    if (!this->IsForSelf()) {
      return;
    }
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->CompileDefinitions.emplace_back(value, lfbt);
    }
  } else if (prop == INTERFACE_COMPILE_DEFINITIONS) {
    if (!this->IsForInterface()) {
      return;
    }
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->GetMakefile()->GetBacktrace();
      this->InterfaceCompileDefinitions.emplace_back(value, lfbt);
    }
  } else {
    this->Properties.AppendProperty(prop, value, asString);
  }
}

cmValue cmFileSet::GetProperty(std::string const& prop) const
{
  // Check for the properties with backtraces.
  if (prop == BASE_DIRS) {

    static std::string output;
    output = cmList::to_string(this->GetDirectoryEntries());
    return cmValue(output);
  }
  if (prop == SOURCES) {
    if (!this->IsForSelf() || this->GetFileEntries().empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmList::to_string(this->GetFileEntries());
    return cmValue(output);
  }
  if (prop == INTERFACE_SOURCES) {
    if (!this->IsForInterface() || this->GetFileEntries().empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmList::to_string(this->GetFileEntries());
    return cmValue(output);
  }

  if (prop == INCLUDE_DIRECTORIES) {
    if (this->IncludeDirectories.empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmList::to_string(this->IncludeDirectories);
    return cmValue(output);
  }

  if (prop == INTERFACE_INCLUDE_DIRECTORIES) {
    if (this->InterfaceIncludeDirectories.empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmList::to_string(this->InterfaceIncludeDirectories);
    return cmValue(output);
  }

  if (prop == COMPILE_OPTIONS) {
    if (this->CompileOptions.empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmList::to_string(this->CompileOptions);
    return cmValue(output);
  }

  if (prop == INTERFACE_COMPILE_OPTIONS) {
    if (this->InterfaceCompileOptions.empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmList::to_string(this->InterfaceCompileOptions);
    return cmValue(output);
  }

  if (prop == COMPILE_DEFINITIONS) {
    if (this->CompileDefinitions.empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmList::to_string(this->CompileDefinitions);
    return cmValue(output);
  }

  if (prop == INTERFACE_COMPILE_DEFINITIONS) {
    if (this->InterfaceCompileDefinitions.empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmList::to_string(this->InterfaceCompileDefinitions);
    return cmValue(output);
  }

  if (prop == "TYPE"_s) {
    return cmValue{ this->GetType() };
  }
  if (prop == "SCOPE"_s) {
    static std::string scope =
      std::string{ Metadata::VisibilityToName(this->GetVisibility()) };
    return cmValue{ scope };
  }

  return this->Properties.GetPropertyValue(prop);
}
