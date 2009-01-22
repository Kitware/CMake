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

#include "cmCPackDragNDropGenerator.h"
#include "cmCPackLog.h"
#include "cmSystemTools.h"

#include <cmsys/RegularExpression.hxx>

//----------------------------------------------------------------------
cmCPackDragNDropGenerator::cmCPackDragNDropGenerator()
{
}

//----------------------------------------------------------------------
cmCPackDragNDropGenerator::~cmCPackDragNDropGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackDragNDropGenerator::InitializeInternal()
{
  const std::string hdiutil_path = cmSystemTools::FindProgram("hdiutil",
    std::vector<std::string>(), false);
  if(hdiutil_path.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot locate hdiutil command"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_COMMAND_HDIUTIL", hdiutil_path.c_str());

  const std::string setfile_path = cmSystemTools::FindProgram("SetFile",
    std::vector<std::string>(1, "/Developer/Tools"), false);
  if(setfile_path.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Cannot locate SetFile command"
      << std::endl);
    return 0;
    }
  this->SetOptionIfNotSet("CPACK_COMMAND_SETFILE", setfile_path.c_str());

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
const char* cmCPackDragNDropGenerator::GetOutputExtension()
{
  return ".dmg";
}

//----------------------------------------------------------------------
int cmCPackDragNDropGenerator::CompressFiles(const char* outFileName,
  const char* toplevel, const std::vector<std::string>& files)
{
  (void) files;

  // Get optional arguments ...
  const std::string cpack_package_icon = this->GetOption("CPACK_PACKAGE_ICON") 
    ? this->GetOption("CPACK_PACKAGE_ICON") : "";

  // The staging directory contains everything that will end-up inside the
  // final disk image ...
  cmOStringStream staging;
  staging << toplevel;

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

//----------------------------------------------------------------------
bool cmCPackDragNDropGenerator::CopyFile(cmOStringStream& source,
  cmOStringStream& target)
{
  if(!cmSystemTools::CopyFileIfDifferent(
    source.str().c_str(),
    target.str().c_str()))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Error copying "
      << source.str()
      << " to "
      << target.str()
      << std::endl);

    return false;
    }

  return true;
}

//----------------------------------------------------------------------
bool cmCPackDragNDropGenerator::RunCommand(cmOStringStream& command,
  std::string* output)
{
  int exit_code = 1;

  bool result = cmSystemTools::RunSingleCommand(
    command.str().c_str(),
    output,
    &exit_code,
    0,
    this->GeneratorVerbose,
    0);

  if(!result || exit_code)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Error executing: "
      << command.str()
      << std::endl);

    return false;
    }

  return true;
}
