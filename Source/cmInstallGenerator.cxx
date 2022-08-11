/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmInstallGenerator.h"

#include <sstream>
#include <utility>

#include "cmMakefile.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

cmInstallGenerator::cmInstallGenerator(
  std::string destination, std::vector<std::string> const& configurations,
  std::string component, MessageLevel message, bool exclude_from_all,
  bool all_components, cmListFileBacktrace backtrace)
  : cmScriptGenerator("CMAKE_INSTALL_CONFIG_NAME", configurations)
  , Destination(std::move(destination))
  , Component(std::move(component))
  , Message(message)
  , ExcludeFromAll(exclude_from_all)
  , AllComponents(all_components)
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
  const char* permissions_file /* = nullptr */,
  const char* permissions_dir /* = nullptr */,
  const char* rename /* = nullptr */, const char* literal_args /* = nullptr */,
  Indent indent, const char* files_var /* = nullptr */)
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
          os << cmSystemTools::GetFilenameName(file);
        }
        firstIteration = false;
      }
      os << "\")\n";
    }
    if (files_var) {
      os << indent << "foreach(_cmake_abs_file IN LISTS " << files_var
         << ")\n";
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
  if (permissions_file && *permissions_file) {
    os << " PERMISSIONS" << permissions_file;
  }
  if (permissions_dir && *permissions_dir) {
    os << " DIR_PERMISSIONS" << permissions_dir;
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
    if (files_var) {
      os << " ${" << files_var << "}";
    }
    os << "\n" << indent << " ";
    if (!(literal_args && *literal_args)) {
      os << " ";
    }
  }
  if (literal_args && *literal_args) {
    os << literal_args;
  }
  os << ")\n";
}

std::string cmInstallGenerator::CreateComponentTest(
  const std::string& component, bool exclude_from_all, bool all_components)
{
  if (all_components) {
    if (exclude_from_all) {
      return "CMAKE_INSTALL_COMPONENT";
    }
    return {};
  }

  std::string result = "CMAKE_INSTALL_COMPONENT STREQUAL \"";
  result += component;
  result += "\"";
  if (!exclude_from_all) {
    result += " OR NOT CMAKE_INSTALL_COMPONENT";
  }

  return result;
}

void cmInstallGenerator::GenerateScript(std::ostream& os)
{
  // Track indentation.
  Indent indent;

  std::string component_test = this->CreateComponentTest(
    this->Component, this->ExcludeFromAll, this->AllComponents);

  // Begin this block of installation.
  if (!component_test.empty()) {
    os << indent << "if(" << component_test << ")\n";
  }

  // Generate the script possibly with per-configuration code.
  this->GenerateScriptConfigs(os,
                              this->AllComponents ? indent : indent.Next());

  // End this block of installation.
  if (!component_test.empty()) {
    os << indent << "endif()\n\n";
  }
}

bool cmInstallGenerator::InstallsForConfig(const std::string& config)
{
  return this->GeneratesForConfig(config);
}

std::string cmInstallGenerator::ConvertToAbsoluteDestination(
  std::string const& dest)
{
  std::string result;
  if (!dest.empty() && !cmSystemTools::FileIsFullPath(dest)) {
    result = "${CMAKE_INSTALL_PREFIX}/";
  }
  result += dest;
  return result;
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
                                  const std::string& config,
                                  std::string const& file,
                                  const TweakMethod& tweak)
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
                                  const std::string& config,
                                  std::string const& dir,
                                  std::vector<std::string> const& files,
                                  const TweakMethod& tweak)
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
