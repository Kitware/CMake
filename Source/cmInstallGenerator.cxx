/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmInstallGenerator.h"

#include <sstream>
#include <utility>

#include <cm/string_view>

#include "cmDiagnostics.h"
#include "cmListFileCache.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmInstallGenerator::cmInstallGenerator(
  std::string destination, std::vector<std::string> const& configurations,
  std::string component, MessageLevel message, bool excludeFromAll,
  bool allComponents, cmListFileBacktrace backtrace)
  : cmScriptGenerator("CMAKE_INSTALL_CONFIG_NAME", configurations)
  , Destination(std::move(destination))
  , Component(std::move(component))
  , Message(message)
  , ExcludeFromAll(excludeFromAll)
  , AllComponents(allComponents)
  , Backtrace(std::move(backtrace))
{
}

cmInstallGenerator::~cmInstallGenerator() = default;

bool cmInstallGenerator::HaveInstall()
{
  return true;
}

void cmInstallGenerator::CheckCMP0082(bool& haveSubdirectoryInstall,
                                      bool& haveInstallAfterSubdirectory)
{
  if (haveSubdirectoryInstall) {
    haveInstallAfterSubdirectory = true;
  }
}

void cmInstallGenerator::AddInstallRule(
  std::ostream& os, std::string const& dest, cmInstallType type,
  std::vector<std::string> const& files, bool optional /* = false */,
  char const* permissionsFile /* = nullptr */,
  char const* permissionsDir /* = nullptr */,
  char const* rename /* = nullptr */, char const* literalArgs /* = nullptr */,
  Indent indent, char const* filesVar /* = nullptr */)
{
  // Use the FILE command to install the file.
  std::string stype;
  switch (type) {
    case cmInstallType_DIRECTORY:
      stype = "DIRECTORY";
      break;
    case cmInstallType_PROGRAMS:
      stype = "PROGRAM";
      break;
    case cmInstallType_EXECUTABLE:
      stype = "EXECUTABLE";
      break;
    case cmInstallType_STATIC_LIBRARY:
      stype = "STATIC_LIBRARY";
      break;
    case cmInstallType_SHARED_LIBRARY:
      stype = "SHARED_LIBRARY";
      break;
    case cmInstallType_MODULE_LIBRARY:
      stype = "MODULE";
      break;
    case cmInstallType_FILES:
      stype = "FILE";
      break;
  }
  if (cmSystemTools::FileIsFullPath(dest)) {
    if (!files.empty()) {
      os << indent << "list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES\n";
      os << indent << " \"";
      bool firstIteration = true;
      for (std::string const& file : files) {
        if (!firstIteration) {
          os << ";";
        }
        os << dest << "/";
        if (rename && *rename) {
          os << rename;
        } else {
          os << cmSystemTools::GetFilenameNameView(file);
        }
        firstIteration = false;
      }
      os << "\")\n";
    }
    if (filesVar) {
      os << indent << "foreach(_cmake_abs_file IN LISTS " << filesVar << ")\n";
      os << indent.Next()
         << "get_filename_component(_cmake_abs_file_name "
            "\"${_cmake_abs_file}\" NAME)\n";
      os << indent.Next() << "list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES \""
         << dest << "/${_cmake_abs_file_name}\")\n";
      os << indent << "endforeach()\n";
      os << indent << "unset(_cmake_abs_file_name)\n";
      os << indent << "unset(_cmake_abs_file)\n";
    }
    os << indent << "if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)\n";
    os << indent.Next()
       << "message(WARNING \"ABSOLUTE path INSTALL "
          "DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}\")\n";
    os << indent << "endif()\n";

    os << indent << "if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)\n";
    os << indent.Next()
       << "message(FATAL_ERROR \"ABSOLUTE path INSTALL "
          "DESTINATION forbidden (by caller): "
          "${CMAKE_ABSOLUTE_DESTINATION_FILES}\")\n";
    os << indent << "endif()\n";
  }
  std::string absDest = ConvertToAbsoluteDestination(dest);
  os << indent << "file(INSTALL DESTINATION \"" << absDest << "\" TYPE "
     << stype;
  if (optional) {
    os << " OPTIONAL";
  }
  switch (this->Message) {
    case MessageDefault:
      break;
    case MessageAlways:
      os << " MESSAGE_ALWAYS";
      break;
    case MessageLazy:
      os << " MESSAGE_LAZY";
      break;
    case MessageNever:
      os << " MESSAGE_NEVER";
      break;
  }
  if (permissionsFile && *permissionsFile) {
    os << " PERMISSIONS" << permissionsFile;
  }
  if (permissionsDir && *permissionsDir) {
    os << " DIR_PERMISSIONS" << permissionsDir;
  }
  if (rename && *rename) {
    os << " RENAME \"" << rename << "\"";
  }
  os << " FILES";
  if (files.size() == 1) {
    os << " \"" << files[0] << "\"";
  } else {
    for (std::string const& f : files) {
      os << "\n" << indent << "  \"" << f << "\"";
    }
    if (filesVar) {
      os << " ${" << filesVar << "}";
    }
    os << "\n" << indent << " ";
    if (!(literalArgs && *literalArgs)) {
      os << " ";
    }
  }
  if (literalArgs && *literalArgs) {
    os << literalArgs;
  }
  os << ")\n";
}

std::string cmInstallGenerator::CreateComponentTest(
  std::string const& component, bool excludeFromAll, bool allComponents)
{
  if (allComponents) {
    if (excludeFromAll) {
      return "CMAKE_INSTALL_COMPONENT";
    }
    return {};
  }

  std::string result = "CMAKE_INSTALL_COMPONENT STREQUAL \"";
  result += component;
  result += "\"";
  if (!excludeFromAll) {
    result += " OR NOT CMAKE_INSTALL_COMPONENT";
  }

  return result;
}

void cmInstallGenerator::GenerateScript(std::ostream& os)
{
  // Track indentation.
  Indent indent;

  std::string componentTest = this->CreateComponentTest(
    this->Component, this->ExcludeFromAll, this->AllComponents);

  // Begin this block of installation.
  if (!componentTest.empty()) {
    os << indent << "if(" << componentTest << ")\n";
  }

  // Generate the script possibly with per-configuration code.
  this->GenerateScriptConfigs(os,
                              this->AllComponents ? indent : indent.Next());

  // End this block of installation.
  if (!componentTest.empty()) {
    os << indent << "endif()\n\n";
  }
}

bool cmInstallGenerator::InstallsForConfig(std::string const& config)
{
  return this->GeneratesForConfig(config);
}

std::string cmInstallGenerator::ConvertToAbsoluteDestination(
  std::string const& dest)
{
  if (dest == ".") {
    return "${CMAKE_INSTALL_PREFIX}";
  }

  std::string result;
  if (!dest.empty() && !cmSystemTools::FileIsFullPath(dest)) {
    result = "${CMAKE_INSTALL_PREFIX}/";
  }
  result += dest;
  return result;
}

void cmInstallGenerator::CheckAbsoluteDestination(
  std::string const& dest, cmLocalGenerator* lg, cmListFileBacktrace const& bt)
{
  if (!cmSystemTools::FileIsFullPath(dest)) {
    return;
  }
  lg->IssueDiagnostic(
    cmDiagnostics::CMD_INSTALL_ABSOLUTE_DESTINATION,
    cmStrCat("INSTALL command given absolute DESTINATION path (", dest,
             ").\n"),
    bt);
}

cmInstallGenerator::MessageLevel cmInstallGenerator::SelectMessageLevel(
  cmMakefile* mf, bool never)
{
  if (never) {
    return MessageNever;
  }
  std::string m = mf->GetSafeDefinition("CMAKE_INSTALL_MESSAGE");
  if (m == "ALWAYS") {
    return MessageAlways;
  }
  if (m == "LAZY") {
    return MessageLazy;
  }
  if (m == "NEVER") {
    return MessageNever;
  }
  return MessageDefault;
}

std::string cmInstallGenerator::GetDestDirPath(std::string const& file)
{
  // Construct the path of the file on disk after installation on
  // which tweaks may be performed.
  std::string toDestDirPath = "$ENV{DESTDIR}";
  if (file[0] != '/' && file[0] != '$') {
    toDestDirPath += "/";
  }
  toDestDirPath += file;
  return toDestDirPath;
}

void cmInstallGenerator::AddTweak(std::ostream& os, Indent indent,
                                  std::string const& config,
                                  std::string const& file,
                                  TweakMethod const& tweak)
{
  std::ostringstream tw;
  tweak(tw, indent.Next(), config, file);
  std::string tws = tw.str();
  if (!tws.empty()) {
    os << indent << "if(EXISTS \"" << file << "\" AND\n"
       << indent << "   NOT IS_SYMLINK \"" << file << "\")\n";
    os << tws;
    os << indent << "endif()\n";
  }
}

void cmInstallGenerator::AddTweak(std::ostream& os, Indent indent,
                                  std::string const& config,
                                  std::string const& dir,
                                  std::vector<std::string> const& files,
                                  TweakMethod const& tweak)
{
  if (files.size() == 1) {
    // Tweak a single file.
    AddTweak(os, indent, config, GetDestDirPath(cmStrCat(dir, files[0])),
             tweak);
  } else {
    // Generate a foreach loop to tweak multiple files.
    std::ostringstream tw;
    AddTweak(tw, indent.Next(), config, "${file}", tweak);
    std::string tws = tw.str();
    if (!tws.empty()) {
      Indent indent2 = indent.Next().Next();
      os << indent << "foreach(file\n";
      for (std::string const& f : files) {
        os << indent2 << "\"" << GetDestDirPath(cmStrCat(dir, f)) << "\"\n";
      }
      os << indent2 << ")\n";
      os << tws;
      os << indent << "endforeach()\n";
    }
  }
}
