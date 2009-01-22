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

#include <cmsys/RegularExpression.hxx>

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
  const char* name = this->GetOption("CPACK_BUNDLE_NAME");
  if(0 == name)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "CPACK_BUNDLE_NAME must be set to use the Bundle generator."
      << std::endl);

    return 0;
    }

  return this->Superclass::InitializeInternal();
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
  (void) files;

  // Get required arguments ...
  const std::string cpack_bundle_name = this->GetOption("CPACK_BUNDLE_NAME")
    ? this->GetOption("CPACK_BUNDLE_NAME") : "";
  if(cpack_bundle_name.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "CPACK_BUNDLE_NAME must be set."
      << std::endl);

    return 0;
    }

  const std::string cpack_bundle_plist = this->GetOption("CPACK_BUNDLE_PLIST")
    ? this->GetOption("CPACK_BUNDLE_PLIST") : "";
  if(cpack_bundle_plist.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "CPACK_BUNDLE_PLIST must be set."
      << std::endl);

    return 0;
    }

  const std::string cpack_bundle_icon = this->GetOption("CPACK_BUNDLE_ICON")
    ? this->GetOption("CPACK_BUNDLE_ICON") : "";
  if(cpack_bundle_icon.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "CPACK_BUNDLE_ICON must be set."
      << std::endl);

    return 0;
    }

  // Get optional arguments ...
  const std::string cpack_package_icon = this->GetOption("CPACK_PACKAGE_ICON") 
    ? this->GetOption("CPACK_PACKAGE_ICON") : "";

  const std::string cpack_bundle_startup_command = 
    this->GetOption("CPACK_BUNDLE_STARTUP_COMMAND") 
    ? this->GetOption("CPACK_BUNDLE_STARTUP_COMMAND") : "";

  // The staging directory contains everything that will end-up inside the
  // final disk image ...
  cmOStringStream staging;
  staging << toplevel;

  cmOStringStream contents;
  contents << staging.str() << "/" << cpack_bundle_name
    << ".app/" << "Contents";

  cmOStringStream application;
  application << contents.str() << "/" << "MacOS";

  cmOStringStream resources;
  resources << contents.str() << "/" << "Resources";

  // Install a required, user-provided bundle metadata file ...
  cmOStringStream plist_source;
  plist_source << cpack_bundle_plist;

  cmOStringStream plist_target;
  plist_target << contents.str() << "/" << "Info.plist";

  if(!this->CopyFile(plist_source, plist_target))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Error copying plist.  Check the value of CPACK_BUNDLE_PLIST."
      << std::endl);

    return 0;
    }

  // Install a user-provided bundle icon ...
  cmOStringStream icon_source;
  icon_source << cpack_bundle_icon;

  cmOStringStream icon_target;
  icon_target << resources.str() << "/" << cpack_bundle_name << ".icns";

  if(!this->CopyFile(icon_source, icon_target))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Error copying bundle icon.  Check the value of CPACK_BUNDLE_ICON."
      << std::endl);

    return 0;
    }

  // Optionally a user-provided startup command (could be an
  // executable or a script) ...
  if(!cpack_bundle_startup_command.empty())
    {
    cmOStringStream command_source;
    command_source << cpack_bundle_startup_command;

    cmOStringStream command_target;
    command_target << application.str() << "/" << cpack_bundle_name;

    if(!this->CopyFile(command_source, command_target))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error copying startup command. "
                    " Check the value of CPACK_BUNDLE_STARTUP_COMMAND."
        << std::endl);

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
  if(!cpack_package_icon.empty())
    {
    cmOStringStream package_icon_source;
    package_icon_source << cpack_package_icon;

    cmOStringStream package_icon_destination;
    package_icon_destination << staging.str() << "/.VolumeIcon.icns";

    if(!this->CopyFile(package_icon_source, package_icon_destination))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error copying disk volume icon.  "
                    "Check the value of CPACK_PACKAGE_ICON."
        << std::endl);

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
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error generating temporary disk image."
        << std::endl);

    return 0;
    }

  // Optionally set the custom icon flag for the image ...
  if(!cpack_package_icon.empty())
    {
    cmOStringStream temp_mount;

    cmOStringStream attach_command;
    attach_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
    attach_command << " attach";
    attach_command << " \"" << temp_image.str() << "\"";

    std::string attach_output;
    if(!this->RunCommand(attach_command, &attach_output))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error attaching temporary disk image."
        << std::endl);

      return 0;
      }

    cmsys::RegularExpression mountpoint_regex(".*(/Volumes/[^\n]+)\n.*");
    mountpoint_regex.find(attach_output.c_str());
    temp_mount << mountpoint_regex.match(1);

    cmOStringStream setfile_command;
    setfile_command << this->GetOption("CPACK_COMMAND_SETFILE");
    setfile_command << " -a C";
    setfile_command << " \"" << temp_mount.str() << "\"";

    if(!this->RunCommand(setfile_command))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error assigning custom icon to temporary disk image."
        << std::endl);

      return 0;
      }

    cmOStringStream detach_command;
    detach_command << this->GetOption("CPACK_COMMAND_HDIUTIL");
    detach_command << " detach";
    detach_command << " \"" << temp_mount.str() << "\""; 

    if(!this->RunCommand(detach_command))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error detaching temporary disk image."
        << std::endl);

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
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Error compressing disk image."
      << std::endl);

    return 0;
    }

  return 1;
}
