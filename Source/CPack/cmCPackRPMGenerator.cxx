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
int cmCPackRPMGenerator::PackageFiles()
{
  int retval = 1;
  /* Digest Component grouping specification */
  retval = PrepareGroupingKind();
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Toplevel: "
                  << toplevel << std::endl);

  /* Are we in the component packaging case */
  if (SupportsComponentInstallation() & (!this->ComponentGroups.empty()))
    {
    /* Reset package file name list it will be populated during the
     * component packaging run*/
    packageFileNames.clear();
    std::string initialTopLevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));
    /* One Package per component CASE */
    /* Iterate over components */
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
  /* This is the non component case */
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

