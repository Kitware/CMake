/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackIFWGenerator.h"

#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmSystemTools.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmCPackLog.h"
#include "cmCPackComponentGroup.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Glob.hxx>
#include <cmsys/Directory.hxx>
#include <cmsys/RegularExpression.hxx>
#include <cmXMLSafe.h>

//----------------------------------------------------------------------
cmCPackIFWGenerator::cmCPackIFWGenerator()
{
}

//----------------------------------------------------------------------
cmCPackIFWGenerator::~cmCPackIFWGenerator()
{
}

//----------------------------------------------------------------------
int cmCPackIFWGenerator::PackageFiles()
{
  cmCPackLogger(cmCPackLog::LOG_OUTPUT, "- Configuration" << std::endl);

  if (!IfwCreateConfigFile())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "CPack error: Could not create IFW \"config.xml\" file."
                  << std::endl);
    return false;
    }

  if (Components.empty() && !IfwCreatePackageFile())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "CPack error: Could not create IFW "
                  "\"root/meta/package.xml\" file."
                  << std::endl);
    return false;
    }

  std::string ifwTLD = this->GetOption("CPACK_TOPLEVEL_DIRECTORY");
  std::string ifwTmpFile = ifwTLD;
  ifwTmpFile += "/IFWOutput.log";

  std::set<std::string> ifwDependsComponents;
  std::string ifwBinaryComponents;
  std::string ifwDownloadedComponents;

  // Create groups meta information
  std::map<std::string, cmCPackComponentGroup>::iterator groupIt;
  for(groupIt = this->ComponentGroups.begin();
      groupIt != this->ComponentGroups.end();
      ++groupIt
    )
    {
    std::string macroPrefix = "CPACK_IFW_COMPONENT_GROUP_"
      + cmsys::SystemTools::UpperCase(groupIt->second.Name);

    std::string groupId = IfwGetGroupId(&groupIt->second);

    if(!ifwBinaryComponents.empty()) ifwBinaryComponents += ",";
    ifwBinaryComponents += groupId;

    std::string pkgMetaDir = this->toplevel + "/packages/"
      + groupId
      + "/meta";

    std::string pkgXmlFileName = pkgMetaDir
      + "/package.xml";

    cmGeneratedFileStream pkgXml(pkgXmlFileName.data());
    pkgXml << "<?xml version=\"1.0\"?>" << std::endl;
    pkgXml << "<Package>" << std::endl;
    pkgXml << "    <DisplayName>" << groupIt->second.DisplayName
           << "</DisplayName>" << std::endl;
    pkgXml << "    <Description>" << groupIt->second.Description
           << "</Description>" << std::endl;
    pkgXml << "    <Name>" << groupId << "</Name>" << std::endl;

    // Version
    const char* ifwPackageVersion = this->GetOption("CPACK_PACKAGE_VERSION");
    const char* ifwGroupVersion = this->GetOption(macroPrefix + "_VERSION");
    pkgXml << "    <Version>"
           << (ifwGroupVersion ? ifwGroupVersion : ifwPackageVersion)
           << "</Version>" << std::endl;
    pkgXml << "    <ReleaseDate>" << IfwCreateCurrentDate()
           << "</ReleaseDate>" << std::endl;

    // Licenses
    std::vector<std::string> licenses;
    if(IfwParseLicenses(licenses, macroPrefix + "_LICENSES", pkgMetaDir))
      {
      pkgXml << "    <Licenses>" << std::endl;
      for(size_t i = 0; i < licenses.size(); i += 2)
        {
        pkgXml << "        <License "
               << "name=\"" << licenses[i] << "\" "
               << "file=\"" << licenses[i + 1] << "\" "
               << "/>" <<std::endl;
        }
      pkgXml << "    </Licenses>" << std::endl;
      }

    // Priority
    if(const char* ifwGroupPriority =
       this->GetOption(macroPrefix + "_PRIORITY"))
      {
      pkgXml << "    <SortingPriority>" << ifwGroupPriority
             << "</SortingPriority>" << std::endl;
      }
    pkgXml << "</Package>" << std::endl;
    }

  // Create components meta information
  std::map<std::string, cmCPackComponent>::iterator compIt;
  for (compIt = this->Components.begin();
       compIt != this->Components.end();
       ++compIt)
    {
    // Component id
    std::string ifwCompId = IfwGetComponentId(&compIt->second);

    std::string pkgMetaDir = this->toplevel + "/"
      + GetComponentInstallDirNamePrefix(compIt->second.Name)
      + ifwCompId + "/meta";
    std::string pkgXmlFileName = pkgMetaDir + "/package.xml";
    cmGeneratedFileStream pkgXml(pkgXmlFileName.data());

    // Check IFW version for component
    std::string macroPrefix = "CPACK_IFW_COMPONENT_"
      + cmsys::SystemTools::UpperCase(compIt->second.Name);

    pkgXml << "<?xml version=\"1.0\"?>" << std::endl;
    pkgXml << "<Package>" << std::endl;
    pkgXml << "    <DisplayName>" << compIt->second.DisplayName
           << "</DisplayName>" << std::endl;
    pkgXml << "    <Description>" << compIt->second.Description
           << "</Description>" << std::endl;
    pkgXml << "    <Name>" << ifwCompId << "</Name>" << std::endl;

    // Version
    const char* ifwPackageVersion = this->GetOption("CPACK_PACKAGE_VERSION");
    const char* ifwCompVersion =
      this->GetOption(macroPrefix + "_VERSION");
    pkgXml << "    <Version>"
           <<  (ifwCompVersion ? ifwCompVersion : ifwPackageVersion)
           << "</Version>" << std::endl;

    pkgXml << "    <ReleaseDate>" << IfwCreateCurrentDate()
           << "</ReleaseDate>" << std::endl;

    // Script
    const char* ifwCompScript =
      this->GetOption(macroPrefix + "_SCRIPT");
    if (ifwCompScript)
      {
      // Copy file
      std::string ifwCompScriptFile = pkgMetaDir + "/operations.qs";
      cmsys::SystemTools::CopyFileIfDifferent(ifwCompScript,
                                              ifwCompScriptFile.data());
      pkgXml << "    <Script>" << "operations.qs" << "</Script>" << std::endl;
      }

    // Check dependencies
    std::set<std::string> compDepSet;
    // CMake dependencies
    if (!compIt->second.Dependencies.empty())
      {
      std::vector<cmCPackComponent *>::iterator depCompIt;
      for(depCompIt = compIt->second.Dependencies.begin();
          depCompIt != compIt->second.Dependencies.end();
          ++depCompIt)
        {
        compDepSet.insert(IfwGetComponentId(*depCompIt));
        }
      }
    // QtIFW dependencies
    if(const char *ifwCompDepsStr = this->GetOption(macroPrefix + "_DEPENDS"))
      {
      std::vector<std::string> ifwCompDepsVector;
      cmSystemTools::ExpandListArgument(ifwCompDepsStr,
                                        ifwCompDepsVector);
      for(std::vector<std::string>::iterator
            depCompIt = ifwCompDepsVector.begin();
          depCompIt != ifwCompDepsVector.end(); ++depCompIt)
        {
        compDepSet.insert(*depCompIt);
        ifwDependsComponents.insert(*depCompIt);
        }
      }

    // Write dependencies
    if  (!compDepSet.empty())
      {
      pkgXml << "    <Dependencies>";
      std::set<std::string>::iterator it = compDepSet.begin();
      pkgXml << *it;
      ++it;
      while(it != compDepSet.end())
        {
        pkgXml << "," << *it;
        ++it;
        }
      pkgXml << "</Dependencies>" << std::endl;
      }

    // Licenses
    std::vector<std::string> licenses;
    if(IfwParseLicenses(licenses, macroPrefix + "_LICENSES", pkgMetaDir))
      {
      pkgXml << "    <Licenses>" << std::endl;
      for(size_t i = 0; i < licenses.size(); i += 2)
        {
        pkgXml << "        <License "
               << "name=\"" << licenses[i] << "\" "
               << "file=\"" << licenses[i + 1] << "\" "
               << "/>" <<std::endl;
        }
      pkgXml << "    </Licenses>" << std::endl;
      }

    // TODO: Check how enable virtual component (now it's allways disabled)
    if (compIt->second.IsRequired) {
    pkgXml << "    <ForcedInstallation>true</ForcedInstallation>"
           << std::endl;
    } else if (compIt->second.IsDisabledByDefault) {
    pkgXml << "    <Default>false</Default>" << std::endl;
    } else if (compIt->second.IsHidden) {
    pkgXml << "    <Virtual>true</Virtual>" << std::endl;
    } else {
    pkgXml << "    <Default>true</Default>" << std::endl;
    }

    // Priority
    if(const char* ifwCompPriority =
       this->GetOption(macroPrefix + "_PRIORITY"))
      {
      pkgXml << "    <SortingPriority>" << ifwCompPriority
             << "</SortingPriority>" << std::endl;
      }

    pkgXml << "</Package>" << std::endl;

    // Downloaded
    if (compIt->second.IsDownloaded)
      {
      if (!ifwDownloadedComponents.empty()) ifwDownloadedComponents += ",";
      ifwDownloadedComponents += ifwCompId;
      }
    else
      {
      if (!ifwBinaryComponents.empty()) ifwBinaryComponents += ",";
      ifwBinaryComponents += ifwCompId;
      }
    }

  // Run repogen
  if (!ifwDownloadSite.empty())
    {
    std::string ifwCmd = ifwRepoGen;
    ifwCmd += " -c " + this->toplevel + "/config/config.xml";
    ifwCmd += " -p " + this->toplevel + "/packages";

    if(!ifwPkgsDirsVector.empty())
      {
      for(std::vector<std::string>::iterator it = ifwPkgsDirsVector.begin();
          it != ifwPkgsDirsVector.end(); ++it)
        {
        ifwCmd += " -p " + *it;
        }
      }

    if (!ifwOnlineOnly && !ifwDownloadedComponents.empty()) {
    ifwCmd += " -i " + ifwDownloadedComponents;
    }
    ifwCmd += " " + this->toplevel + "/repository";
    cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Execute: " << ifwCmd
                  << std::endl);
    std::string output;
    int retVal = 1;
    cmCPackLogger(cmCPackLog::LOG_OUTPUT,
                  "- Generate repository" << std::endl);
    bool res = cmSystemTools::RunSingleCommand(
      ifwCmd.c_str(), &output, &retVal, 0, this->GeneratorVerbose, 0);
    if ( !res || retVal )
      {
      cmGeneratedFileStream ofs(ifwTmpFile.c_str());
      ofs << "# Run command: " << ifwCmd << std::endl
          << "# Output:" << std::endl
          << output << std::endl;
      cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running IFW command: "
                    << ifwCmd << std::endl
                    << "Please check " << ifwTmpFile << " for errors"
                    << std::endl);
      return 0;
      }
    }

  // Run binary creator
  {
  std::string ifwCmd = ifwBinCreator;
  ifwCmd += " -c " + this->toplevel + "/config/config.xml";
  ifwCmd += " -p " + this->toplevel + "/packages";

  if(!ifwPkgsDirsVector.empty())
    {
    for(std::vector<std::string>::iterator it = ifwPkgsDirsVector.begin();
        it != ifwPkgsDirsVector.end(); ++it)
      {
      ifwCmd += " -p " + *it;
      }
    }

  if (ifwOnlineOnly)
    {
    ifwCmd += " --online-only";
    }
  else if (!ifwDownloadedComponents.empty() && !ifwDownloadSite.empty())
    {
    ifwCmd += " -e " + ifwDownloadedComponents;
    }
  else if (!ifwDependsComponents.empty())
    {
    ifwCmd += " -i ";
    std::set<std::string>::iterator it = ifwDependsComponents.begin();
    ifwCmd += *it;
    ++it;
    while(it != ifwDependsComponents.end())
      {
      ifwCmd += "," + (*it);
      ++it;
      }

    ifwCmd += "," + ifwBinaryComponents;
    }
  // TODO: set correct name for multipackages
  if (this->packageFileNames.size() > 0)
    {
    ifwCmd += " " + packageFileNames[0];
    }
  else
    {
    ifwCmd += " installer";
    }
  cmCPackLogger(cmCPackLog::LOG_VERBOSE, "Execute: " << ifwCmd
                << std::endl);
  std::string output;
  int retVal = 1;
  cmCPackLogger(cmCPackLog::LOG_OUTPUT, "- Generate package" << std::endl);
  bool res = cmSystemTools::RunSingleCommand(
    ifwCmd.c_str(), &output, &retVal, 0, this->GeneratorVerbose, 0);
  if ( !res || retVal )
    {
    cmGeneratedFileStream ofs(ifwTmpFile.c_str());
    ofs << "# Run command: " << ifwCmd << std::endl
        << "# Output:" << std::endl
        << output << std::endl;
    cmCPackLogger(cmCPackLog::LOG_ERROR, "Problem running IFW command: "
                  << ifwCmd << std::endl
                  << "Please check " << ifwTmpFile << " for errors"
                  << std::endl);
    return 0;
    }
  }

  return 1;
}

//----------------------------------------------------------------------
const char *cmCPackIFWGenerator::GetPackagingInstallPrefix()
{
  const char *defPrefix = cmCPackGenerator::GetPackagingInstallPrefix();

  std::string tmpPref = defPrefix ? defPrefix : "";

  if(this->Components.empty())
    {
    tmpPref += "packages/root/data";
    }

  this->SetOption("CPACK_IFW_PACKAGING_INSTALL_PREFIX", tmpPref.c_str());

  return this->GetOption("CPACK_IFW_PACKAGING_INSTALL_PREFIX");
}

//----------------------------------------------------------------------
const char *cmCPackIFWGenerator::GetOutputExtension()
{
  const char *suffix = this->GetOption("CMAKE_EXECUTABLE_SUFFIX");
  return suffix ? suffix : "";
}

//----------------------------------------------------------------------
std::string cmCPackIFWGenerator::IfwGetGroupId(cmCPackComponentGroup *group)
{
  std::string ifwGroupId;
  std::string ifwGroupName;
  std::list<cmCPackComponentGroup*> groups;
  while(group)
    {
    groups.push_front(group);
    group = group->ParentGroup;
    }
  std::list<cmCPackComponentGroup*>::iterator it = groups.begin();
  if(it != groups.end())
    {
    ifwGroupId = IfwGetGroupName(*it);
    ++it;
    }
  while(it != groups.end())
    {
    ifwGroupName = IfwGetGroupName(*it);

    if(ifwResolveDuplicateNames)
      {
      if(ifwGroupName.substr(0, ifwGroupId.size()) == ifwGroupId)
        {
        ifwGroupId = ifwGroupName;
        ++it;
        continue;
        }
      }

    ifwGroupId += "." + ifwGroupName;

    ++it;
    }

  return ifwGroupId;
}

//----------------------------------------------------------------------
std::string cmCPackIFWGenerator::IfwGetComponentId(cmCPackComponent *component)
{
  std::string ifwCompId;
  if(component) {
  ifwCompId = IfwGetGroupId(component->Group);
  if(!ifwCompId.empty()) ifwCompId += ".";
  std::string ifwCompName = IfwGetComponentName(component);
  if(ifwResolveDuplicateNames &&
     (ifwCompName.substr(0, ifwCompId.size()) == ifwCompId))
    {
    ifwCompId = ifwCompName;
    }
  else
    {
    ifwCompId += ifwCompName;
    }
  }
  return ifwCompId;
}

//----------------------------------------------------------------------
std::string cmCPackIFWGenerator::IfwGetGroupName(cmCPackComponentGroup *group)
{
  std::string ifwGroupName = group->Name;
  if(const char* name =
     this->GetOption("CPACK_IFW_COMPONENT_GROUP_"
                     + cmsys::SystemTools::UpperCase(group->Name) + "_NAME"))
    {
    ifwGroupName = name;
    }
  return ifwGroupName;
}

//----------------------------------------------------------------------
std::string
cmCPackIFWGenerator::IfwGetComponentName(cmCPackComponent *component)
{
  return IfwGetComponentName(component->Name);
}

//----------------------------------------------------------------------
std::string
cmCPackIFWGenerator::IfwGetComponentName(const std::string &componentName)
{
  std::string ifwCompName = componentName;
  if(const char* name =
     this->GetOption("CPACK_IFW_COMPONENT_"
                     + cmsys::SystemTools::UpperCase(componentName) + "_NAME"))
    {
    ifwCompName = name;
    }
  return ifwCompName;
}

//----------------------------------------------------------------------
int cmCPackIFWGenerator::InitializeInternal()
{
  // Search Qt Installer Framework tools

  if(!this->IsOn("CPACK_IFW_BINARYCREATOR_EXECUTABLE_FOUND") ||
     !this->IsOn("CPACK_IFW_REPOGEN_EXECUTABLE_FOUND"))
    {
    this->ReadListFile("CPackIFW.cmake");
    }

  // Look 'binarycreator' executable (needs)

  if(this->IsOn("CPACK_IFW_BINARYCREATOR_EXECUTABLE_FOUND"))
    {
    const char *ifwBinCreatorStr =
      this->GetOption("CPACK_IFW_BINARYCREATOR_EXECUTABLE");
    ifwBinCreator = ifwBinCreatorStr ? ifwBinCreatorStr : "";
    }
  else
    {
    ifwBinCreator = "";
    }

  if (ifwBinCreator.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
                  "Cannot find QtIFW compiler \"binarycreator\": "
                  "likely it is not installed, or not in your PATH"
                  << std::endl);
    return 0;
    }

  // Look 'repogen' executable (optional)

  if(this->IsOn("CPACK_IFW_REPOGEN_EXECUTABLE_FOUND"))
    {
    const char *ifwRepoGenStr =
      this->GetOption("CPACK_IFW_REPOGEN_EXECUTABLE");
    ifwRepoGen = ifwRepoGenStr ? ifwRepoGenStr : "";
    }
  else
    {
    ifwRepoGen = "";
    }

  // // Variables that Change Behavior

  // Resolve duplicate names
  ifwResolveDuplicateNames = this->IsOn("CPACK_IFW_RESOLVE_DUPLICATE_NAMES");

  // Additional packages dirs
  ifwPkgsDirsVector.clear();
  if(const char* dirs = this->GetOption("CPACK_IFW_PACKAGES_DIRECTORIES"))
    {
    cmSystemTools::ExpandListArgument(dirs,
                                      ifwPkgsDirsVector);
    }

  // Remote repository

  if (const char *site = this->GetOption("CPACK_DOWNLOAD_SITE"))
    {
    ifwDownloadSite = site;
    }

  ifwOnlineOnly = this->IsOn("CPACK_DOWNLOAD_ALL") ? true : false;

  if (!ifwDownloadSite.empty() && ifwRepoGen.empty()) {
  cmCPackLogger(cmCPackLog::LOG_ERROR,
                "Cannot find QtIFW repository generator \"repogen\": "
                "likely it is not installed, or not in your PATH"
                << std::endl);
  return 0;
  }

  return this->Superclass::InitializeInternal();
}

//----------------------------------------------------------------------
std::string
cmCPackIFWGenerator::GetComponentInstallDirNamePrefix(
  const std::string& /*componentName*/)
{
  return "packages/";
}

//----------------------------------------------------------------------
std::string
cmCPackIFWGenerator::GetComponentInstallDirNameSuffix(
  const std::string& componentName)
{
  std::map<std::string, cmCPackComponent>::iterator
    compIt = this->Components.find(componentName);

  cmCPackComponent *comp =
    compIt != this->Components.end() ? &compIt->second : 0;

  const std::string prefix = GetComponentInstallDirNamePrefix(componentName);
  const std::string suffix = "/data";

  if (componentPackageMethod == ONE_PACKAGE_PER_COMPONENT) {
  return prefix + IfwGetComponentId(comp) + suffix;
  }

  if (componentPackageMethod == ONE_PACKAGE) {
  return std::string(prefix + "ALL_COMPONENTS_IN_ONE" + suffix);
  }

  return prefix + IfwGetComponentId(comp) + suffix;
}

//----------------------------------------------------------------------
bool cmCPackIFWGenerator::GetListOfSubdirectories(
  const char* topdir, std::vector<std::string>& dirs)
{
  cmsys::Directory dir;
  dir.Load(topdir);
  size_t fileNum;
  for (fileNum = 0; fileNum <  dir.GetNumberOfFiles(); ++fileNum)
    {
    if (strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".") &&
        strcmp(dir.GetFile(static_cast<unsigned long>(fileNum)),".."))
      {
      cmsys_stl::string fullPath = topdir;
      fullPath += "/";
      fullPath += dir.GetFile(static_cast<unsigned long>(fileNum));
      if(cmsys::SystemTools::FileIsDirectory(fullPath.c_str()) &&
         !cmsys::SystemTools::FileIsSymlink(fullPath.c_str()))
        {
        if (!this->GetListOfSubdirectories(fullPath.c_str(), dirs))
          {
          return false;
          }
        }
      }
    }
  dirs.push_back(topdir);
  return true;
}

//----------------------------------------------------------------------
enum cmCPackGenerator::CPackSetDestdirSupport
cmCPackIFWGenerator::SupportsSetDestdir() const
{
  return cmCPackGenerator::SETDESTDIR_SHOULD_NOT_BE_USED;
}

//----------------------------------------------------------------------
bool cmCPackIFWGenerator::SupportsAbsoluteDestination() const
{
  return false;
}

//----------------------------------------------------------------------
bool cmCPackIFWGenerator::SupportsComponentInstallation() const
{
  return true;
}

//----------------------------------------------------------------------
int cmCPackIFWGenerator::IfwCreateConfigFile()
{
  cmGeneratedFileStream cfg((this->toplevel + "/config/config.xml").data());

  std::string ifwPkgName;
  if (const char *name = this->GetOption("CPACK_PACKAGE_NAME"))
    {
    ifwPkgName = name;
    }
  else
    {
    ifwPkgName = "Your package";
    }

  std::string ifwPkgDescription;
  if (const char *name = this->GetOption("CPACK_PACKAGE_DESCRIPTION_SUMMARY"))
    {
    ifwPkgDescription = name;
    }
  else
    {
    ifwPkgDescription = "Your package description";
    }

  std::string ifwPkgVersion;
  if (const char *version = this->GetOption("CPACK_PACKAGE_VERSION"))
    {
    ifwPkgVersion = version;
    }
  else
    {
    ifwPkgVersion = "1.0.0";
    }

  const char *ifwPkgInstDir =
    this->GetOption("CPACK_PACKAGE_INSTALL_DIRECTORY");
  const char *ifwTargetDir =
    this->GetOption("CPACK_IFW_TARGET_DIRECTORY");
  const char *ifwAdminTargetDir =
    this->GetOption("CPACK_IFW_ADMIN_TARGET_DIRECTORY");

  cfg << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << std::endl;
  cfg << "<Installer>" << std::endl;
  cfg << "    <Name>" << cmXMLSafe(ifwPkgName).str() << "</Name>" << std::endl;
  cfg << "    <Version>" << ifwPkgVersion << "</Version>" << std::endl;
  cfg << "    <Title>" << cmXMLSafe(ifwPkgDescription).str() << "</Title>"
      << std::endl;

  // Default target directory for installation
  if (ifwTargetDir)
    {
    cfg << "    <TargetDir>" << ifwTargetDir << "</TargetDir>" << std::endl;
    }
  else if (ifwPkgInstDir)
    {
    cfg << "    <TargetDir>@ApplicationsDir@/" << ifwPkgInstDir
        << "</TargetDir>" << std::endl;
    }
  else
    {
    cfg << "    <TargetDir>@RootDir@/usr/local</TargetDir>" << std::endl;
    }

  // Default target directory for installation with administrator rights
  if (ifwAdminTargetDir)
    {
    cfg << "    <AdminTargetDir>" << ifwAdminTargetDir
        << "</AdminTargetDir>" << std::endl;
    }

  if (!ifwDownloadSite.empty())
    {
    cfg << "    <RemoteRepositories>" << std::endl;
    cfg << "        <Repository>" << std::endl;
    cfg << "            <Url>" << ifwDownloadSite << "</Url>" << std::endl;
    // These properties can now be set from "cpack_configure_downloads"
    //                 <Enabled>1</Enabled>
    //                 <Username>user</Username>
    //                 <Password>password</Password>
    //                 <DisplayName>Example repository</DisplayName>
    cfg << "        </Repository>" << std::endl;
    cfg << "    </RemoteRepositories>" << std::endl;
    }

  // CPack IFW default policy
  cfg << "    <!-- CPack IFW default policy -->" << std::endl;
  cfg << "    <AllowNonAsciiCharacters>true</AllowNonAsciiCharacters>"
      << std::endl;
  cfg << "    <AllowSpaceInPath>true</AllowSpaceInPath>" << std::endl;

  cfg << "</Installer>" << std::endl;

  return 1;
}

//----------------------------------------------------------------------
// Create default package file
int cmCPackIFWGenerator::IfwCreatePackageFile()
{
  std::string ifwPkgName;
  if (const char *name = this->GetOption("CPACK_PACKAGE_NAME"))
    {
    ifwPkgName = name;
    }
  else
    {
    ifwPkgName = "Your package";
    }

  std::string ifwPkgDescription;
  if (const char *name = this->GetOption("CPACK_PACKAGE_DESCRIPTION_SUMMARY"))
    {
    ifwPkgDescription = name;
    }
  else
    {
    ifwPkgDescription = "Your package description";
    }

  cmGeneratedFileStream
    pkgXml((this->toplevel + "/packages/root/meta/package.xml").data());
  pkgXml << "<?xml version=\"1.0\"?>" << std::endl;
  pkgXml << "<Package>" << std::endl;

  pkgXml << "    <DisplayName>" << ifwPkgName << "</DisplayName>" << std::endl;
  pkgXml << "    <Description>" << ifwPkgDescription
         << "</Description>" << std::endl;
  pkgXml << "    <Name>" << "root" << "</Name>" << std::endl;
  pkgXml << "    <Version>" << this->GetOption("CPACK_PACKAGE_VERSION")
         << "</Version>" << std::endl;
  pkgXml << "    <ReleaseDate>" << IfwCreateCurrentDate() << "</ReleaseDate>"
         << std::endl;

  pkgXml << "    <ForcedInstallation>true</ForcedInstallation>" << std::endl;
  pkgXml << "    <Default>true</Default>" << std::endl;

  pkgXml << "</Package>" << std::endl;

  return 1;
}

//----------------------------------------------------------------------
std::string cmCPackIFWGenerator::IfwCreateCurrentDate()
{
  time_t rawtime;
  struct tm * timeinfo;
  char buffer[80];

  time (&rawtime);
  timeinfo = localtime(&rawtime);

  strftime(buffer, 80, "%Y-%m-%d", timeinfo);

  return buffer;
}

//----------------------------------------------------------------------
bool cmCPackIFWGenerator::IfwParseLicenses(std::vector<std::string> &licenses,
                                           const std::string &variable,
                                           const std::string &metaDir)
{
  if (const char *option = this->GetOption(variable))
    {
    if(!licenses.empty()) licenses.clear();
    cmSystemTools::ExpandListArgument( option, licenses );
    }
  else
    {
    return false;
    }

  if ( licenses.size() % 2 != 0 )
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR, variable
                  << " should contain pairs of <display_name> and <file_path>."
                  << std::endl);
    return false;
    }

  for(size_t i = 1; i < licenses.size(); i += 2)
    {
    std::string name = cmSystemTools::GetFilenameName(licenses[i]);
    std::string path = metaDir + "/" + name;
    cmsys::SystemTools::CopyFileIfDifferent(licenses[i].data(), path.data());
    licenses[i] = name;
    }

  return licenses.size() > 1;
}
