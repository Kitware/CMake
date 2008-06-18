/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "cmCPackBundleGenerator.h"
#include "cmCPackLog.h"
#include "cmSystemTools.h"

//----------------------------------------------------------------------
cmCPackBundleGenerator::cmCPackBundleGenerator()
{
}

//----------------------------------------------------------------------
cmCPackBundleGenerator::~cmCPackBundleGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackBundleGenerator::InitializeInternal()
{
  const std::string hdiutil_path = cmSystemTools::FindProgram("hdiutil",
    std::vector<std::string>(), false);
  if(hdiutil_path.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot locate hdiutil command"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_COMMAND_HDIUTIL", hdiutil_path.c_str());

  const std::string setfile_path = cmSystemTools::FindProgram("SetFile",
    std::vector<std::string>(1, "/Developer/Tools"), false);
  if(setfile_path.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot locate SetFile command"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_COMMAND_SETFILE", setfile_path.c_str());

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
const char* cmCPackBundleGenerator::GetOutputExtension()
{
  return ".dmg";
}

//----------------------------------------------------------------------
const char* cmCPackBundleGenerator::GetPackagingInstallPrefix()
{
  this->InstallPrefix = "/";
  this->InstallPrefix += this->GetOption("CPACK_BUNDLE_NAME");
  this->InstallPrefix += ".app/Contents/Resources";

  return this->InstallPrefix.c_str();
}

//----------------------------------------------------------------------
int cmCPackBundleGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  // The staging directory contains everything that will end-up inside the
  // final disk image ...
  cmOStringStream staging;
  staging << toplevel;

  cmOStringStream contents;
  contents << staging.str() << "/" << this->GetOption("CPACK_BUNDLE_NAME")
    << ".app/" << "Contents";

  cmOStringStream application;
  application << contents.str() << "/" << "MacOS";

  cmOStringStream resources;
  resources << contents.str() << "/" << "Resources";

  // Install a user-provided bundle metadata file ...
  if(this->GetOption("CPACK_BUNDLE_PLIST"))
    {
    cmOStringStream plist_source;
    plist_source << this->GetOption("CPACK_BUNDLE_PLIST");

    cmOStringStream plist_target;
    plist_target << contents.str() << "/" << "Info.plist";

    if(!this->CopyFile(plist_source, plist_target))
      {
      return 0;
      }
    }

  // Install a user-provided bundle icon ...
  if(this->GetOption("CPACK_BUNDLE_ICON"))
    {
    cmOStringStream icon_source;
    icon_source << this->GetOption("CPACK_BUNDLE_ICON");

    cmOStringStream icon_target;
    icon_target << resources.str() << "/"
      << this->GetOption("CPACK_BUNDLE_NAME") << ".icns";

    if(!this->CopyFile(icon_source, icon_target))
      {
      return 0;
      }
    }

  // Install a user-provided startup command (could be an executable or a
  // script) ...
  if(this->GetOption("CPACK_BUNDLE_STARTUP_COMMAND"))
    {
    cmOStringStream command_source;
    command_source << this->GetOption("CPACK_BUNDLE_STARTUP_COMMAND");

    cmOStringStream command_target;
    command_target << application.str() << "/"
      << this->GetOption("CPACK_BUNDLE_NAME");

    if(!this->CopyFile(command_source, command_target))
      {
      return 0;
      }

    cmSystemTools::SetPermissions(command_target.str().c_str(), 0777);
    }

  // Add a symlink to /Applications so users can drag-and-drop the bundle
  // into it
  cmOStringStream application_link;
  application_link << staging.str() << "/Applications";
  cmSystemTools::CreateSymlink("/Applications",
    application_link.str().c_str());

  // Optionally add a custom volume icon ...
  if(this->GetOption("CPACK_PACKAGE_ICON"))
    {
    cmOStringStream package_icon_source;
    package_icon_source << this->GetOption("CPACK_PACKAGE_ICON");

    cmOStringStream package_icon_destination;
    package_icon_destination << staging.str() << "/.VolumeIcon.icns";

    if(!this->CopyFile(package_icon_source, package_icon_destination))
      {
      return 0;
      }
    }

  // Create a temporary read-write disk image ...
  cmOStringStream temp_image;
  temp_image << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "/temp.dmg";

  cmOStringStream temp_image_command;
  temp_image_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
  temp_image_command << " create";
  temp_image_command << " -ov";
  temp_image_command << " -srcfolder \"" << staging.str() << "\"";
  temp_image_command << " -volname \""
    << this->GetOption("CPACK_PACKAGE_FILE_NAME") << "\"";
  temp_image_command << " -format UDRW";
  temp_image_command << " \"" << temp_image.str() << "\"";

  if(!this->RunCommand(temp_image_command))
    {
    return 0;
    }

  // Optionally set the custom icon flag for the image ...
  if(this->GetOption("CPACK_PACKAGE_ICON"))
    {
    cmOStringStream temp_mount;
    temp_mount << this->GetOption("CPACK_TOPLEVEL_DIRECTORY") << "/mnt";
    cmSystemTools::MakeDirectory(temp_mount.str().c_str());

    cmOStringStream attach_command;
    attach_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
    attach_command << " attach";
    attach_command << " -mountpoint \"" << temp_mount.str() << "\"";
    attach_command << " \"" << temp_image.str() << "\"";

    if(!this->RunCommand(attach_command))
      {
      return 0;
      }

    cmOStringStream setfile_command;
    setfile_command << this->GetOption("CPACK_COMMAND_SETFILE");
    setfile_command << " -a C";
    setfile_command << " \"" << temp_mount.str() << "\"";

    if(!this->RunCommand(setfile_command))
      {
      return 0;
      }

    cmOStringStream detach_command;
    detach_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
    detach_command << " detach";
    detach_command << " \"" << temp_mount.str() << "\""; 

    if(!this->RunCommand(detach_command))
      {
      return 0;
      }
    }

  // Create the final compressed read-only disk image ...
  cmOStringStream final_image_command;
  final_image_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
  final_image_command << " convert \"" << temp_image.str() << "\"";
  final_image_command << " -format UDZO";
  final_image_command << " -imagekey";
  final_image_command << " zlib-level=9";
  final_image_command << " -o \"" << outFileName << "\"";

  if(!this->RunCommand(final_image_command))
    {
    return 0;
    }

/*
  // Disk image directories
  std::string diskImageDirectory = toplevel;
  std::string diskImageBackgroundImageDir = diskImageDirectory
    + "/.background";

  // App bundle directories
  std::string packageDirFileName = toplevel;
  packageDirFileName += "/";
  packageDirFileName += this->GetOption("CPACK_PACKAGE_FILE_NAME");
  packageDirFileName += ".app";
  std::string contentsDirectory = packageDirFileName + "/Contents";
  std::string resourcesDirectory = contentsDirectory + "/Resources";
  std::string appDirectory = contentsDirectory + "/MacOS";

  const char* dir = resourcesDirectory.c_str();
  const char* appdir = appDirectory.c_str();
  const char* contDir = contentsDirectory.c_str();
  const char* iconFile = this->GetOption("CPACK_PACKAGE_ICON");
  if ( iconFile )
    {
    std::string iconFileName = cmsys::SystemTools::GetFilenameName(iconFile);
    if ( !cmSystemTools::FileExists(iconFile) )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find icon file: "
        << iconFile << ". Please check CPACK_PACKAGE_ICON setting."
        << std::endl);
      return 0;
      }
    std::string destFileName = resourcesDirectory + "/" + iconFileName;
    this->ConfigureFile(iconFile, destFileName.c_str(), true);
    this->SetOptionIfNotSet("CPACK_APPLE_GUI_ICON", iconFileName.c_str());
    }

  if (
    !this->CopyResourcePlistFile("VolumeIcon.icns", 
                                 diskImageDirectory.c_str(),
                                 ".VolumeIcon.icns", true ) ||
    !this->CopyResourcePlistFile("DS_Store", diskImageDirectory.c_str(),
      ".DS_Store", true ) ||
    !this->CopyResourcePlistFile("background.png",
      diskImageBackgroundImageDir.c_str(), "background.png", true ) ||
    !this->CopyResourcePlistFile("RuntimeScript", dir) ||
    !this->CopyResourcePlistFile("Bundle.Info.plist", contDir,
      "Info.plist" ) ||
    !this->CopyResourcePlistFile("OSXScriptLauncher", appdir, 
      this->GetOption("CPACK_PACKAGE_FILE_NAME"), true)
  )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem copying the resource files"
      << std::endl);
    return 0;
    }
*/

  return 1;
}

//----------------------------------------------------------------------
bool cmCPackBundleGenerator::CopyFile(cmOStringStream& source,
  cmOStringStream& target)
{
  return cmSystemTools::CopyFileIfDifferent(source.str().c_str(),
    target.str().c_str());
}

//----------------------------------------------------------------------
bool cmCPackBundleGenerator::RunCommand(cmOStringStream& command)
{
  std::string output;
  int exit_code = 1;

  bool result = cmSystemTools::RunSingleCommand(command.str().c_str(),
    &output, &exit_code, 0, this->GeneratorVerbose, 0);
  if(!result || exit_code)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running command: "
      << command.str().c_str() << std::endl);
    return false;
    }

  return true;
}
