/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCPackRPMGenerator.h"
#include "cmCPackLog.h"
#include "cmSystemTools.h"

//----------------------------------------------------------------------
cmCPackRPMGenerator::cmCPackRPMGenerator()
{
}

//----------------------------------------------------------------------
cmCPackRPMGenerator::~cmCPackRPMGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackRPMGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_PACKAGING_INSTALL_PREFIX", "/usr");
  if (cmSystemTools::IsOff(this->GetOption("CPACK_SET_DESTDIR")))
    {
    this->SetOption("CPACK_SET_DESTDIR", "I_ON");
    }
  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
int cmCPackRPMGenerator::PackageComponents(bool ignoreGroup)
{
  int retval = 1;
  /* Reset package file name list it will be populated during the
   * component packaging run*/
  packageFileNames.clear();
  std::string initialTopLevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));

  // The default behavior is to have one package by component group
  // unless CPACK_COMPONENTS_IGNORE_GROUP is specified.
  if (!ignoreGroup)
    {
    std::map<std::string, cmCPackComponentGroup>::iterator compGIt;
    for (compGIt=this->ComponentGroups.begin();
        compGIt!=this->ComponentGroups.end(); ++compGIt)
      {
      cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Packaging component group: "
          << compGIt->first
          << std::endl);
      // Begin the archive for this group
      std::string localToplevel(initialTopLevel);
      std::string packageFileName(
          cmSystemTools::GetParentDirectory(toplevel.c_str())
                                 );
      std::string outputFileName(
          std::string(this->GetOption("CPACK_PACKAGE_FILE_NAME"))
          +"-"+compGIt->first + this->GetOutputExtension()
                                );

      localToplevel += "/"+ compGIt->first;
      /* replace the TEMP DIRECTORY with the component one */
      this->SetOption("CPACK_TEMPORARY_DIRECTORY",localToplevel.c_str());
      packageFileName += "/"+ outputFileName;
      /* replace proposed CPACK_OUTPUT_FILE_NAME */
      this->SetOption("CPACK_OUTPUT_FILE_NAME",outputFileName.c_str());
      /* replace the TEMPORARY package file name */
      this->SetOption("CPACK_TEMPORARY_PACKAGE_FILE_NAME",
                      packageFileName.c_str());
      // Tell CPackRPM.cmake the name of the component GROUP.
      this->SetOption("CPACK_RPM_PACKAGE_COMPONENT",compGIt->first.c_str());
      if (!this->ReadListFile("CPackRPM.cmake"))
        {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
            "Error while execution CPackRPM.cmake" << std::endl);
        retval = 0;
        }
      // add the generated package to package file names list
      packageFileNames.push_back(packageFileName);
      }
    }
  // CPACK_COMPONENTS_IGNORE_GROUPS is set
  // We build 1 package per component
  else
    {
    std::map<std::string, cmCPackComponent>::iterator compIt;
    for (compIt=this->Components.begin();
         compIt!=this->Components.end(); ++compIt )
      {
      std::string localToplevel(initialTopLevel);
      std::string packageFileName(
          cmSystemTools::GetParentDirectory(toplevel.c_str())
                                 );
      std::string outputFileName(
          std::string(this->GetOption("CPACK_PACKAGE_FILE_NAME")
                                )
        +"-"+compIt->first + this->GetOutputExtension());

      localToplevel += "/"+ compIt->first;
      /* replace the TEMP DIRECTORY with the component one */
      this->SetOption("CPACK_TEMPORARY_DIRECTORY",localToplevel.c_str());
      packageFileName += "/"+ outputFileName;
      /* replace proposed CPACK_OUTPUT_FILE_NAME */
      this->SetOption("CPACK_OUTPUT_FILE_NAME",outputFileName.c_str());
      /* replace the TEMPORARY package file name */
      this->SetOption("CPACK_TEMPORARY_PACKAGE_FILE_NAME",
                      packageFileName.c_str());

      this->SetOption("CPACK_RPM_PACKAGE_COMPONENT",compIt->first.c_str());
      if (!this->ReadListFile("CPackRPM.cmake"))
        {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
                      "Error while execution CPackRPM.cmake" << std::endl);
        retval = 0;
        }
      // add the generated package to package file names list
      packageFileNames.push_back(packageFileName);
      }
    }
  return retval;
}

//----------------------------------------------------------------------
int cmCPackRPMGenerator::PackageComponentsAllInOne(bool allComponent)
{
  int retval = 1;
  std::string compInstDirName;
  /* Reset package file name list it will be populated during the
   * component packaging run*/
  packageFileNames.clear();
  std::string initialTopLevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));

  // all GROUP in one vs all COMPONENT in one
  if (allComponent)
    {
    compInstDirName = "ALL_COMPONENTS_IN_ONE";
    }
  else
    {
    compInstDirName = "ALL_GROUPS_IN_ONE";
    }

  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Packaging all groups in one package..."
                "(CPACK_COMPONENTS_ALL_[GROUPS_]IN_ONE_PACKAGE is set)"
      << std::endl);

  // The ALL GROUPS in ONE package case
  std::string localToplevel(initialTopLevel);
  std::string packageFileName(
      cmSystemTools::GetParentDirectory(toplevel.c_str())
                             );
  std::string outputFileName(
            std::string(this->GetOption("CPACK_PACKAGE_FILE_NAME"))
            +"-ALL"+ this->GetOutputExtension()
                            );
  // all GROUP in one vs all COMPONENT in one
  localToplevel += "/"+compInstDirName;

  /* replace the TEMP DIRECTORY with the component one */
  this->SetOption("CPACK_TEMPORARY_DIRECTORY",localToplevel.c_str());
  packageFileName += "/"+ outputFileName;
  /* replace proposed CPACK_OUTPUT_FILE_NAME */
  this->SetOption("CPACK_OUTPUT_FILE_NAME",outputFileName.c_str());
  /* replace the TEMPORARY package file name */
  this->SetOption("CPACK_TEMPORARY_PACKAGE_FILE_NAME",
      packageFileName.c_str());
  // Tell CPackRPM.cmake the name of the component GROUP.
  this->SetOption("CPACK_RPM_PACKAGE_COMPONENT",compInstDirName.c_str());
  if (!this->ReadListFile("CPackRPM.cmake"))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Error while execution CPackRPM.cmake" << std::endl);
    retval = 0;
    }
  // add the generated package to package file names list
  packageFileNames.push_back(packageFileName);

  return 1;
}

//----------------------------------------------------------------------
int cmCPackRPMGenerator::PackageFiles()
{
  int retval = 1;

  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Toplevel: "
                  << toplevel << std::endl);

  /* Are we in the component packaging case */
  if (SupportsComponentInstallation()) {
    // CASE 1 : COMPONENT ALL-IN-ONE package
    // If ALL GROUPS or ALL COMPONENTS in ONE package has been requested
    // then the package file is unique and should be open here.
    if (allComponentInOne ||
        (allGroupInOne && (!this->ComponentGroups.empty()))
       )
      {
      return PackageComponentsAllInOne(allComponentInOne);
      }
    // CASE 2 : COMPONENT CLASSICAL package(s) (i.e. not all-in-one)
    // There will be 1 package for each component group
    // however one may require to ignore component group and
    // in this case you'll get 1 package for each component.
    else if ((!this->ComponentGroups.empty()) || (ignoreComponentGroup))
      {
      return PackageComponents(ignoreComponentGroup);
      }
  }
  // CASE 3 : NON COMPONENT package.
  else
    {
    if (!this->ReadListFile("CPackRPM.cmake"))
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
                    "Error while execution CPackRPM.cmake" << std::endl);
      retval = 0;
      }
    }

  if (!this->IsSet("RPMBUILD_EXECUTABLE")) 
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Cannot find rpmbuild" << std::endl);
    retval = 0;
    }
  return retval;
}

bool cmCPackRPMGenerator::SupportsComponentInstallation() const
  {
  if (IsOn("CPACK_RPM_COMPONENT_INSTALL"))
    {
      return true;
    }
  else
    {
      return false;
    }
  }

std::string cmCPackRPMGenerator::GetComponentInstallDirNameSuffix(
    const std::string& componentName)
  {
  if (ignoreComponentGroup) {
    return componentName;
  }

  if (allComponentInOne) {
    return std::string("ALL_COMPONENTS_IN_ONE");
  }
  // We have to find the name of the COMPONENT GROUP
  // the current COMPONENT belongs to.
  std::string groupVar = "CPACK_COMPONENT_" +
        cmSystemTools::UpperCase(componentName) + "_GROUP";
    if (NULL != GetOption(groupVar.c_str()))
      {
      if (allGroupInOne)
        {
        return std::string("ALL_GROUPS_IN_ONE");
        }
      else
        {
        return std::string(GetOption(groupVar.c_str()));
        }
      }
    else
      {
      return componentName;
      }
  }
