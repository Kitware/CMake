/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmSourceFile.h"

#include <utility>

#include <cm/string_view>
#include <cmext/string_view>

#include "cmGlobalGenerator.h"
#include "cmListFileCache.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmPolicies.h"
#include "cmProperty.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmValue.h"
#include "cmake.h"

cmSourceFile::cmSourceFile(cmMakefile* mf, const std::string& name,
                           bool generated, cmSourceFileLocationKind kind)
  : Location(mf, name, (!generated) ? kind : cmSourceFileLocationKind::Known)
{
  if (generated) {
    this->MarkAsGenerated();
  }
}

std::string const& cmSourceFile::GetExtension() const
{
  return this->Extension;
}

const std::string propTRUE = "1";
const std::string propFALSE = "0";
const std::string cmSourceFile::propLANGUAGE = "LANGUAGE";
const std::string cmSourceFile::propLOCATION = "LOCATION";
const std::string cmSourceFile::propGENERATED = "GENERATED";
const std::string cmSourceFile::propCOMPILE_DEFINITIONS =
  "COMPILE_DEFINITIONS";
const std::string cmSourceFile::propCOMPILE_OPTIONS = "COMPILE_OPTIONS";
const std::string cmSourceFile::propINCLUDE_DIRECTORIES =
  "INCLUDE_DIRECTORIES";

void cmSourceFile::SetObjectLibrary(std::string const& objlib)
{
  this->ObjectLibrary = objlib;
}

std::string cmSourceFile::GetObjectLibrary() const
{
  return this->ObjectLibrary;
}

std::string const& cmSourceFile::GetOrDetermineLanguage()
{
  // If the language was set explicitly by the user then use it.
  if (cmValue lang = this->GetProperty(propLANGUAGE)) {
    // Assign to member in order to return a reference.
    this->Language = *lang;
    return this->Language;
  }

  // Perform computation needed to get the language if necessary.
  if (this->Language.empty()) {
    // If a known extension is given or a known full path is given then trust
    // that the current extension is sufficient to determine the language. This
    // will fail only if the user specifies a full path to the source but
    // leaves off the extension, which is kind of weird.
    if (this->FullPath.empty() && this->Location.ExtensionIsAmbiguous() &&
        this->Location.DirectoryIsAmbiguous()) {
      // Finalize the file location to get the extension and set the language.
      this->ResolveFullPath();
    } else {
      // Use the known extension to get the language if possible.
      std::string ext =
        cmSystemTools::GetFilenameLastExtension(this->Location.GetName());
      this->CheckLanguage(ext);
    }
  }

  // Use the language determined from the file extension.
  return this->Language;
}

std::string cmSourceFile::GetLanguage() const
{
  // If the language was set explicitly by the user then use it.
  if (cmValue lang = this->GetProperty(propLANGUAGE)) {
    return *lang;
  }

  // Use the language determined from the file extension.
  return this->Language;
}

cmSourceFileLocation const& cmSourceFile::GetLocation() const
{
  return this->Location;
}

std::string const& cmSourceFile::ResolveFullPath(std::string* error,
                                                 std::string* cmp0115Warning)
{
  if (this->FullPath.empty()) {
    if (this->FindFullPath(error, cmp0115Warning)) {
      this->CheckExtension();
    }
  }
  return this->FullPath;
}

std::string const& cmSourceFile::GetFullPath() const
{
  return this->FullPath;
}

bool cmSourceFile::FindFullPath(std::string* error,
                                std::string* cmp0115Warning)
{
  // If the file is generated compute the location without checking on disk.
  // Note: We also check for a locally set GENERATED property, because
  //       it might have been set before policy CMP0118 was set to NEW.
  if (this->GetIsGenerated(CheckScope::GlobalAndLocal)) {
    // The file is either already a full path or is relative to the
    // build directory for the target.
    this->Location.DirectoryUseBinary();
    this->FullPath = this->Location.GetFullPath();
    this->FindFullPathFailed = false;
    return true;
  }

  // If this method has already failed once do not try again.
  if (this->FindFullPathFailed) {
    return false;
  }

  // The file is not generated.  It must exist on disk.
  cmMakefile const* makefile = this->Location.GetMakefile();
  // Location path
  std::string const& lPath = this->Location.GetFullPath();
  // List of extension lists
  std::vector<std::string> exts =
    makefile->GetCMakeInstance()->GetAllExtensions();
  auto cmp0115 = makefile->GetPolicyStatus(cmPolicies::CMP0115);
  auto cmp0118 = makefile->GetPolicyStatus(cmPolicies::CMP0118);
  bool const cmp0118new =
    cmp0118 != cmPolicies::OLD && cmp0118 != cmPolicies::WARN;

  // Tries to find the file in a given directory
  auto findInDir = [this, &exts, &lPath, cmp0115, cmp0115Warning, cmp0118new,
                    makefile](std::string const& dir) -> bool {
    // Compute full path
    std::string const fullPath = cmSystemTools::CollapseFullPath(lPath, dir);
    // Try full path
    if (cmp0118new &&
        makefile->GetGlobalGenerator()->IsGeneratedFile(fullPath)) {
      this->IsGenerated = true;
    }
    if (this->IsGenerated || cmSystemTools::FileExists(fullPath)) {
      this->FullPath = fullPath;
      return true;
    }
    // This has to be an if statement due to a bug in Oracle Developer Studio.
    // See https://community.oracle.com/tech/developers/discussion/4476246/
    // for details.
    if (cmp0115 == cmPolicies::OLD || cmp0115 == cmPolicies::WARN) {
      // Try full path with extension
      for (std::string const& ext : exts) {
        if (!ext.empty()) {
          std::string extPath = cmStrCat(fullPath, '.', ext);
          if (cmp0118new &&
              makefile->GetGlobalGenerator()->IsGeneratedFile(extPath)) {
            this->IsGenerated = true;
          }
          if (this->IsGenerated || cmSystemTools::FileExists(extPath)) {
            this->FullPath = extPath;
            if (cmp0115 == cmPolicies::WARN) {
              std::string warning =
                cmStrCat(cmPolicies::GetPolicyWarning(cmPolicies::CMP0115),
                         "\nFile:\n  ", extPath);
              if (cmp0115Warning) {
                *cmp0115Warning = std::move(warning);
              } else {
                makefile->GetCMakeInstance()->IssueMessage(
                  MessageType::AUTHOR_WARNING, warning);
              }
            }
            return true;
          }
        }
      }
    }
    // File not found
    return false;
  };

  // Try to find the file in various directories
  if (this->Location.DirectoryIsAmbiguous()) {
    if (findInDir(makefile->GetCurrentSourceDirectory()) ||
        findInDir(makefile->GetCurrentBinaryDirectory())) {
      return true;
    }
  } else {
    if (findInDir({})) {
      return true;
    }
  }

  // Compose error
  std::string err = cmStrCat("Cannot find source file:\n  ", lPath);
  switch (cmp0115) {
    case cmPolicies::OLD:
    case cmPolicies::WARN:
      err = cmStrCat(err, "\nTried extensions");
      for (auto const& ext : exts) {
        err = cmStrCat(err, " .", ext);
      }
      break;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
    case cmPolicies::NEW:
      break;
  }
  if (lPath == "FILE_SET"_s) {
    err += "\nHint: the FILE_SET keyword may only appear after a visibility "
           "specifier or another FILE_SET within the target_sources() "
           "command.";
  }
  if (error != nullptr) {
    *error = std::move(err);
  } else {
    makefile->IssueMessage(MessageType::FATAL_ERROR, err);
  }
  this->FindFullPathFailed = true;

  // File not found
  return false;
}

void cmSourceFile::CheckExtension()
{
  // Compute the extension.
  std::string realExt =
    cmSystemTools::GetFilenameLastExtension(this->FullPath);
  if (!realExt.empty()) {
    // Store the extension without the leading '.'.
    this->Extension = realExt.substr(1);
  }

  // Look for object files.
  if (this->Extension == "obj" || this->Extension == "o" ||
      this->Extension == "lo") {
    this->SetProperty("EXTERNAL_OBJECT", "1");
  }

  // Try to identify the source file language from the extension.
  if (this->Language.empty()) {
    this->CheckLanguage(this->Extension);
  }
}

void cmSourceFile::CheckLanguage(std::string const& ext)
{
  // Try to identify the source file language from the extension.
  cmMakefile const* mf = this->Location.GetMakefile();
  cmGlobalGenerator* gg = mf->GetGlobalGenerator();
  std::string l = gg->GetLanguageFromExtension(ext.c_str());
  if (!l.empty()) {
    this->Language = l;
  }
}

bool cmSourceFile::Matches(cmSourceFileLocation const& loc)
{
  return this->Location.Matches(loc);
}

void cmSourceFile::SetProperty(const std::string& prop, cmValue value)
{
  if (prop == propINCLUDE_DIRECTORIES) {
    this->IncludeDirectories.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->Location.GetMakefile()->GetBacktrace();
      this->IncludeDirectories.emplace_back(value, lfbt);
    }
  } else if (prop == propCOMPILE_OPTIONS) {
    this->CompileOptions.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->Location.GetMakefile()->GetBacktrace();
      this->CompileOptions.emplace_back(value, lfbt);
    }
  } else if (prop == propCOMPILE_DEFINITIONS) {
    this->CompileDefinitions.clear();
    if (value) {
      cmListFileBacktrace lfbt = this->Location.GetMakefile()->GetBacktrace();
      this->CompileDefinitions.emplace_back(value, lfbt);
    }
  } else {
    this->Properties.SetProperty(prop, value);
  }
}

void cmSourceFile::AppendProperty(const std::string& prop,
                                  const std::string& value, bool asString)
{
  if (prop == propINCLUDE_DIRECTORIES) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->Location.GetMakefile()->GetBacktrace();
      this->IncludeDirectories.emplace_back(value, lfbt);
    }
  } else if (prop == propCOMPILE_OPTIONS) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->Location.GetMakefile()->GetBacktrace();
      this->CompileOptions.emplace_back(value, lfbt);
    }
  } else if (prop == propCOMPILE_DEFINITIONS) {
    if (!value.empty()) {
      cmListFileBacktrace lfbt = this->Location.GetMakefile()->GetBacktrace();
      this->CompileDefinitions.emplace_back(value, lfbt);
    }
  } else {
    this->Properties.AppendProperty(prop, value, asString);
  }
}

cmValue cmSourceFile::GetPropertyForUser(const std::string& prop)
{
  // This method is a consequence of design history and backwards
  // compatibility.  GetProperty is (and should be) a const method.
  // Computed properties should not be stored back in the property map
  // but instead reference information already known.  If they need to
  // cache information in a mutable ivar to provide the return string
  // safely then so be it.
  //
  // The LOCATION property is particularly problematic.  The CMake
  // language has very loose restrictions on the names that will match
  // a given source file (for historical reasons).  Implementing
  // lookups correctly with such loose naming requires the
  // cmSourceFileLocation class to commit to a particular full path to
  // the source file as late as possible.  If the users requests the
  // LOCATION property we must commit now.
  if (prop == propLOCATION) {
    // Commit to a location.
    this->ResolveFullPath();
  }

  // Similarly, LANGUAGE can be determined by the file extension
  // if it is requested by the user.
  if (prop == propLANGUAGE) {
    // The pointer is valid until `this->Language` is modified.
    return cmValue(this->GetOrDetermineLanguage());
  }

  // Special handling for GENERATED property.
  if (prop == propGENERATED) {
    // We need to check policy CMP0118 in order to determine if we need to
    // possibly consider the value of a locally set GENERATED property, too.
    auto policyStatus =
      this->Location.GetMakefile()->GetPolicyStatus(cmPolicies::CMP0118);
    if (this->GetIsGenerated(
          (policyStatus == cmPolicies::WARN || policyStatus == cmPolicies::OLD)
            ? CheckScope::GlobalAndLocal
            : CheckScope::Global)) {
      return cmValue(propTRUE);
    }
    return cmValue(propFALSE);
  }

  // Perform the normal property lookup.
  return this->GetProperty(prop);
}

cmValue cmSourceFile::GetProperty(const std::string& prop) const
{
  // Check for computed properties.
  if (prop == propLOCATION) {
    if (this->FullPath.empty()) {
      return nullptr;
    }
    return cmValue(this->FullPath);
  }

  // Check for the properties with backtraces.
  if (prop == propINCLUDE_DIRECTORIES) {
    if (this->IncludeDirectories.empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmJoin(this->IncludeDirectories, ";");
    return cmValue(output);
  }

  if (prop == propCOMPILE_OPTIONS) {
    if (this->CompileOptions.empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmJoin(this->CompileOptions, ";");
    return cmValue(output);
  }

  if (prop == propCOMPILE_DEFINITIONS) {
    if (this->CompileDefinitions.empty()) {
      return nullptr;
    }

    static std::string output;
    output = cmJoin(this->CompileDefinitions, ";");
    return cmValue(output);
  }

  cmValue retVal = this->Properties.GetPropertyValue(prop);
  if (!retVal) {
    cmMakefile const* mf = this->Location.GetMakefile();
    const bool chain =
      mf->GetState()->IsPropertyChained(prop, cmProperty::SOURCE_FILE);
    if (chain) {
      return mf->GetProperty(prop, chain);
    }
    return nullptr;
  }

  return retVal;
}

const std::string& cmSourceFile::GetSafeProperty(const std::string& prop) const
{
  cmValue ret = this->GetProperty(prop);
  if (ret) {
    return *ret;
  }

  static std::string const s_empty;
  return s_empty;
}

bool cmSourceFile::GetPropertyAsBool(const std::string& prop) const
{
  return cmIsOn(this->GetProperty(prop));
}

void cmSourceFile::MarkAsGenerated()
{
  this->IsGenerated = true;
  const auto& mf = *this->Location.GetMakefile();
  mf.GetGlobalGenerator()->MarkAsGeneratedFile(this->ResolveFullPath());
}

bool cmSourceFile::GetIsGenerated(CheckScope checkScope) const
{
  if (this->IsGenerated) {
    // Globally marked as generated!
    return true;
  }
  if (checkScope == CheckScope::GlobalAndLocal) {
    // Check locally stored properties.
    return this->GetPropertyAsBool(propGENERATED);
  }
  return false;
}

void cmSourceFile::SetProperties(cmPropertyMap properties)
{
  this->Properties = std::move(properties);
}

cmCustomCommand* cmSourceFile::GetCustomCommand() const
{
  return this->CustomCommand.get();
}

void cmSourceFile::SetCustomCommand(std::unique_ptr<cmCustomCommand> cc)
{
  this->CustomCommand = std::move(cc);
}
