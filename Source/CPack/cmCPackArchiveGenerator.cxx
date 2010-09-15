/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackArchiveGenerator.h"

#include "cmake.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"
#include <errno.h>

#include <cmsys/SystemTools.hxx>
#include <cmsys/Directory.hxx>
#include <cm_libarchive.h>

//----------------------------------------------------------------------
cmCPackArchiveGenerator::cmCPackArchiveGenerator(cmArchiveWrite::Compress t,
  cmArchiveWrite::Type at)
{
  this->Compress = t;
  this->Archive = at;
}

//----------------------------------------------------------------------
cmCPackArchiveGenerator::~cmCPackArchiveGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::InitializeInternal()
{
  this->SetOptionIfNotSet("CPACK_INCLUDE_TOPLEVEL_DIRECTORY", "1");
  return this->Superclass::InitializeInternal();
}
//----------------------------------------------------------------------
int cmCPackArchiveGenerator::addOneComponentToArchive(cmArchiveWrite& archive,
                             cmCPackComponent* component)
{
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "   - packaging component: "
      << component->Name
      << std::endl);
  // Add the files of this component to the archive
  std::string localToplevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));
  localToplevel += "/"+ component->Name;
  std::string dir = cmSystemTools::GetCurrentWorkingDirectory();
  // Change to local toplevel
  cmSystemTools::ChangeDirectory(localToplevel.c_str());
  std::vector<std::string>::const_iterator fileIt;
  for (fileIt = component->Files.begin(); fileIt != component->Files.end();
       ++fileIt )
    {
    cmCPackLogger(cmCPackLog::LOG_DEBUG,"Adding file: "
                  << (*fileIt) << std::endl);
    archive.Add(*fileIt);
    if (!archive)
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR, "ERROR while packaging files: "
            << archive.GetError()
            << std::endl);
      return 0;
      }
    }
  // Go back to previous dir
  cmSystemTools::ChangeDirectory(dir.c_str());
  return 1;
}

/*
 * The macro will open/create a file 'filename'
 * an declare and open the associated
 * cmArchiveWrite 'archive' object.
 */
#define DECLARE_AND_OPEN_ARCHIVE(filename,archive) \
cmGeneratedFileStream gf; \
gf.Open(filename.c_str(), false, true); \
if (!GenerateHeader(&gf)) \
  { \
   cmCPackLogger(cmCPackLog::LOG_ERROR, \
    "Problem to generate Header for archive < "     \
            << filename \
            << ">." << std::endl); \
    return 0; \
  } \
cmArchiveWrite archive(gf,this->Compress, this->Archive); \
if (!archive) \
  { \
  cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem to create archive < " \
     << filename \
     << ">. ERROR =" \
     << archive.GetError() \
     << std::endl); \
  return 0; \
  }

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::PackageComponents(bool ignoreComponentGroup)
{
  packageFileNames.clear();
  // The default behavior is to have one package by component group
  // unless CPACK_COMPONENTS_IGNORE_GROUP is specified.
  if (!ignoreComponentGroup)
    {
    std::map<std::string, cmCPackComponentGroup>::iterator compGIt;
    for (compGIt=this->ComponentGroups.begin();
        compGIt!=this->ComponentGroups.end(); ++compGIt)
      {
      cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Packaging component group: "
          << compGIt->first
          << std::endl);
      // Begin the archive for this group
      std::string packageFileName= std::string(toplevel);
      packageFileName += "/"
        +std::string(this->GetOption("CPACK_PACKAGE_FILE_NAME"))
        +"-"+compGIt->first + this->GetOutputExtension();
      // open a block in order to automatically close archive
      // at the end of the block
      {
        DECLARE_AND_OPEN_ARCHIVE(packageFileName,archive);
        // now iterate over the component of this group
        std::vector<cmCPackComponent*>::iterator compIt;
        for (compIt=(compGIt->second).Components.begin();
            compIt!=(compGIt->second).Components.end();
            ++compIt)
          {
          // Add the files of this component to the archive
          addOneComponentToArchive(archive,*compIt);
          }
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
      std::string localToplevel(this->GetOption("CPACK_TEMPORARY_DIRECTORY"));
      std::string packageFileName = std::string(toplevel);

      localToplevel += "/"+ compIt->first;
      packageFileName += "/"
        +std::string(this->GetOption("CPACK_PACKAGE_FILE_NAME"))
        +"-"+compIt->first + this->GetOutputExtension();
      {
        DECLARE_AND_OPEN_ARCHIVE(packageFileName,archive);
        // Add the files of this component to the archive
        addOneComponentToArchive(archive,&(compIt->second));
      }
      // add the generated package to package file names list
      packageFileNames.push_back(packageFileName);
      }
    }
  return 1;
}

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::PackageComponentsAllInOne(bool allComponentInOne)
{
  // reset the package file names
  packageFileNames.clear();
  packageFileNames.push_back(std::string(toplevel));
  packageFileNames[0] += "/"
    +std::string(this->GetOption("CPACK_PACKAGE_FILE_NAME"))
    +"-ALL" + this->GetOutputExtension();
  cmCPackLogger(cmCPackLog::LOG_VERBOSE,
                "Packaging all groups in one package..."
                "(CPACK_COMPONENTS_ALL_GROUPS_IN_ONE_PACKAGE is set)"
      << std::endl);
  DECLARE_AND_OPEN_ARCHIVE(packageFileNames[0],archive);

  // The ALL GROUP in ONE package case
  if (! allComponentInOne) {
    // iterate over the component groups
    std::map<std::string, cmCPackComponentGroup>::iterator compGIt;
    for (compGIt=this->ComponentGroups.begin();
        compGIt!=this->ComponentGroups.end(); ++compGIt)
      {
      cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Packaging component group: "
          << compGIt->first
          << std::endl);
      // now iterate over the component of this group
      std::vector<cmCPackComponent*>::iterator compIt;
      for (compIt=(compGIt->second).Components.begin();
          compIt!=(compGIt->second).Components.end();
          ++compIt)
        {
        // Add the files of this component to the archive
        addOneComponentToArchive(archive,*compIt);
        }
      }
  }
  // The ALL COMPONENT in ONE package case
  else
    {
    std::map<std::string, cmCPackComponent>::iterator compIt;
    for (compIt=this->Components.begin();compIt!=this->Components.end();
         ++compIt )
      {
      // Add the files of this component to the archive
      addOneComponentToArchive(archive,&(compIt->second));
      }
    }
  // archive goes out of scope so it will finalized and closed.
  return 1;
}

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::PackageFiles()
{
  cmCPackLogger(cmCPackLog::LOG_DEBUG, "Toplevel: "
                << toplevel << std::endl);

  // The default behavior is to create 1 package by component group
  // unless the user asked to put all COMPONENTS in a single package
  bool allGroupInOne = (NULL !=
                        (this->GetOption(
                          "CPACK_COMPONENTS_ALL_GROUPS_IN_ONE_PACKAGE")));
  bool allComponentInOne = (NULL !=
                            (this->GetOption(
                              "CPACK_COMPONENTS_ALL_IN_ONE_PACKAGE")));
  bool ignoreComponentGroup = ( NULL !=
                                (this->GetOption(
                                  "CPACK_COMPONENTS_IGNORE_GROUPS")));

  std::string groupingType;

  // Second way to specify grouping
  if (NULL != this->GetOption("CPACK_COMPONENTS_GROUPING")) {
     groupingType = this->GetOption("CPACK_COMPONENTS_GROUPING");
  }

  if (groupingType.length()>0)
    {
    cmCPackLogger(cmCPackLog::LOG_VERBOSE,  "["
        << this->Name << "]"
        << " requested component grouping = "<< groupingType <<std::endl);
    if (groupingType == "ALL_GROUP_IN_ONE")
      {
      allGroupInOne = true;
      }
    else if (groupingType == "ALL_COMPONENT_IN_ONE")
      {
      allComponentInOne = true;
      }
    else if (groupingType == "IGNORE")
      {
      ignoreComponentGroup = true;
      }
    else
      {
      cmCPackLogger(cmCPackLog::LOG_WARNING, "["
              << this->Name << "]"
              << " requested component grouping type <"<< groupingType
              << "> UNKNOWN not in (ALL_GROUP_IN_ONE,"
                    "ALL_COMPONENT_IN_ONE,IGNORE)" <<std::endl);
      }
    }

  // Some components were defined but NO group
  // force ignoreGroups
  if (this->ComponentGroups.empty() && (!this->Components.empty())
      && (!ignoreComponentGroup)) {
    cmCPackLogger(cmCPackLog::LOG_WARNING, "["
              << this->Name << "]"
              << " Some Components defined but NO component group:"
              << " Ignoring component group."
              << std::endl);
    ignoreComponentGroup = true;
  }
  // CASE 1 : COMPONENT ALL-IN-ONE package
  // If ALL GROUPS or ALL COMPONENTS in ONE package has been requested
  // then the package file is unique and should be open here.
  if (allComponentInOne || (allGroupInOne && (!this->ComponentGroups.empty())))
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

  // CASE 3 : NON COMPONENT package.
  DECLARE_AND_OPEN_ARCHIVE(packageFileNames[0],archive);
  std::vector<std::string>::const_iterator fileIt;
  std::string dir = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(toplevel.c_str());
  for ( fileIt = files.begin(); fileIt != files.end(); ++ fileIt )
    {
    // Get the relative path to the file
    std::string rp = cmSystemTools::RelativePath(toplevel.c_str(),
                                                 fileIt->c_str());
    archive.Add(rp);
    if(!archive)
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem while adding file< "
          << *fileIt
          << "> to archive <"
          << packageFileNames[0] << "> .ERROR ="
          << archive.GetError()
          << std::endl);
      return 0;
      }
    }
  cmSystemTools::ChangeDirectory(dir.c_str());
  // The destructor of cmArchiveWrite will close and finish the write
  return 1;
}

//----------------------------------------------------------------------
int cmCPackArchiveGenerator::GenerateHeader(std::ostream*)
{
  return 1;
}

bool cmCPackArchiveGenerator::SupportsComponentInstallation() const {
  return true;
}
