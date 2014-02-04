/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2013 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackWIXGenerator.h"

#include <cmSystemTools.h>
#include <cmGeneratedFileStream.h>
#include <cmCryptoHash.h>
#include <CPack/cmCPackLog.h>
#include <CPack/cmCPackComponentGroup.h>

#include "cmWIXSourceWriter.h"
#include "cmWIXRichTextFormatWriter.h"

#include <cmsys/SystemTools.hxx>
#include <cmsys/Directory.hxx>
#include <cmsys/Encoding.hxx>
#include <cmsys/FStream.hxx>

#include <rpc.h> // for GUID generation

#include <sys/types.h>
#include <sys/stat.h>

cmCPackWIXGenerator::cmCPackWIXGenerator():
  HasDesktopShortcuts(false)
{

}

int cmCPackWIXGenerator::InitializeInternal()
{
  componentPackageMethod = ONE_PACKAGE;

  return this->Superclass::InitializeInternal();
}

bool cmCPackWIXGenerator::RunWiXCommand(const std::string& command)
{
  std::string cpackTopLevel;
  if(!RequireOption("CPACK_TOPLEVEL_DIRECTORY", cpackTopLevel))
    {
    return false;
    }

  std::string logFileName = cpackTopLevel + "/wix.log";

  cmCPackLogger(cmCPackLog::LOG_DEBUG,
    "Running WiX command: " << command << std::endl);

  std::string output;

  int returnValue = 0;
  bool status = cmSystemTools::RunSingleCommand(command.c_str(), &output,
    &returnValue, 0, cmSystemTools::OUTPUT_NONE);

  cmsys::ofstream logFile(logFileName.c_str(), std::ios::app);
  logFile << command << std::endl;
  logFile << output;
  logFile.close();

  if(!status || returnValue)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Problem running WiX candle. "
      "Please check '" << logFileName << "' for errors." << std::endl);

    return false;
    }

  return true;
}

bool cmCPackWIXGenerator::RunCandleCommand(
  const std::string& sourceFile, const std::string& objectFile)
{
  std::string executable;
  if(!RequireOption("CPACK_WIX_CANDLE_EXECUTABLE", executable))
    {
    return false;
    }

  std::stringstream command;
  command << QuotePath(executable);
  command << " -nologo";
  command << " -arch " << GetArchitecture();
  command << " -out " << QuotePath(objectFile);

  for(extension_set_t::const_iterator i = CandleExtensions.begin();
      i != CandleExtensions.end(); ++i)
    {
    command << " -ext " << QuotePath(*i);
    }

  AddCustomFlags("CPACK_WIX_CANDLE_EXTRA_FLAGS", command);

  command << " " << QuotePath(sourceFile);

  return RunWiXCommand(command.str());
}

bool cmCPackWIXGenerator::RunLightCommand(const std::string& objectFiles)
{
  std::string executable;
  if(!RequireOption("CPACK_WIX_LIGHT_EXECUTABLE", executable))
    {
    return false;
    }

  std::stringstream command;
  command << QuotePath(executable);
  command << " -nologo";
  command << " -out " << QuotePath(packageFileNames.at(0));

  for(extension_set_t::const_iterator i = LightExtensions.begin();
      i != LightExtensions.end(); ++i)
    {
    command << " -ext " << QuotePath(*i);
    }

  const char* const cultures = GetOption("CPACK_WIX_CULTURES");
  if(cultures)
    {
    command << " -cultures:" << cultures;
    }

  AddCustomFlags("CPACK_WIX_LIGHT_EXTRA_FLAGS", command);

  command << " " << objectFiles;

  return RunWiXCommand(command.str());
}

int cmCPackWIXGenerator::PackageFiles()
{
  if(!PackageFilesImpl() || cmSystemTools::GetErrorOccuredFlag())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Fatal WiX Generator Error" << std::endl);
    return false;
    }

  return true;
}

bool cmCPackWIXGenerator::InitializeWiXConfiguration()
{
  if(!ReadListFile("CPackWIX.cmake"))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Error while executing CPackWIX.cmake" << std::endl);
    return false;
    }

  if(GetOption("CPACK_WIX_PRODUCT_GUID") == 0)
    {
    std::string guid = GenerateGUID();
    SetOption("CPACK_WIX_PRODUCT_GUID", guid.c_str());

    cmCPackLogger(cmCPackLog::LOG_VERBOSE,
      "CPACK_WIX_PRODUCT_GUID implicitly set to " << guid << " . "
      << std::endl);
    }

  if(GetOption("CPACK_WIX_UPGRADE_GUID") == 0)
    {
    std::string guid = GenerateGUID();
    SetOption("CPACK_WIX_UPGRADE_GUID", guid.c_str());

    cmCPackLogger(cmCPackLog::LOG_WARNING,
      "CPACK_WIX_UPGRADE_GUID implicitly set to " << guid << " . "
      "Please refer to the documentation on how and why "
      "you might want to set this explicitly." << std::endl);
    }

  std::string cpackTopLevel;
  if(!RequireOption("CPACK_TOPLEVEL_DIRECTORY", cpackTopLevel))
    {
    return false;
    }

  if(GetOption("CPACK_WIX_LICENSE_RTF") == 0)
    {
    std::string licenseFilename = cpackTopLevel + "/License.rtf";
    SetOption("CPACK_WIX_LICENSE_RTF", licenseFilename.c_str());

    if(!CreateLicenseFile())
      {
      return false;
      }
    }

  if(GetOption("CPACK_PACKAGE_VENDOR") == 0)
    {
    std::string defaultVendor = "Humanity";
    SetOption("CPACK_PACKAGE_VENDOR", defaultVendor.c_str());

    cmCPackLogger(cmCPackLog::LOG_VERBOSE,
      "CPACK_PACKAGE_VENDOR implicitly set to " << defaultVendor << " . "
      << std::endl);
    }

  if(GetOption("CPACK_WIX_UI_REF") == 0)
    {
    std::string defaultRef = "WixUI_InstallDir";

    if(Components.size())
      {
      defaultRef = "WixUI_FeatureTree";
      }

    SetOption("CPACK_WIX_UI_REF", defaultRef.c_str());
    }

  CollectExtensions("CPACK_WIX_EXTENSIONS", CandleExtensions);
  CollectExtensions("CPACK_WIX_CANDLE_EXTENSIONS", CandleExtensions);

  LightExtensions.insert("WixUIExtension");
  CollectExtensions("CPACK_WIX_EXTENSIONS", LightExtensions);
  CollectExtensions("CPACK_WIX_LIGHT_EXTENSIONS", LightExtensions);

  const char* patchFilePath = GetOption("CPACK_WIX_PATCH_FILE");
  if(patchFilePath)
    {
    LoadPatchFragments(patchFilePath);
    }

  return true;
}

bool cmCPackWIXGenerator::PackageFilesImpl()
{
  if(!InitializeWiXConfiguration())
    {
    return false;
    }

  if(!CreateWiXVariablesIncludeFile())
    {
    return false;
    }

  if(!CreateWiXSourceFiles())
    {
    return false;
    }

  AppendUserSuppliedExtraSources();

  std::stringstream objectFiles;
  for(size_t i = 0; i < WixSources.size(); ++i)
    {
    const std::string& sourceFilename = WixSources[i];

    std::string objectFilename =
      cmSystemTools::GetFilenameWithoutExtension(sourceFilename) + ".wixobj";

    if(!RunCandleCommand(sourceFilename, objectFilename))
      {
      return false;
      }

    objectFiles << " " << QuotePath(objectFilename);
    }

  AppendUserSuppliedExtraObjects(objectFiles);

  return RunLightCommand(objectFiles.str());
}

void cmCPackWIXGenerator::AppendUserSuppliedExtraSources()
{
  const char *cpackWixExtraSources = GetOption("CPACK_WIX_EXTRA_SOURCES");
  if(!cpackWixExtraSources) return;

  cmSystemTools::ExpandListArgument(cpackWixExtraSources, WixSources);
}

void cmCPackWIXGenerator::AppendUserSuppliedExtraObjects(std::ostream& stream)
{
  const char *cpackWixExtraObjects = GetOption("CPACK_WIX_EXTRA_OBJECTS");
  if(!cpackWixExtraObjects) return;

  std::vector<std::string> expandedExtraObjects;

  cmSystemTools::ExpandListArgument(
    cpackWixExtraObjects, expandedExtraObjects);

  for(size_t i = 0; i < expandedExtraObjects.size(); ++i)
    {
      stream << " " << QuotePath(expandedExtraObjects[i]);
    }
}

bool cmCPackWIXGenerator::CreateWiXVariablesIncludeFile()
{
  std::string cpackTopLevel;
  if(!RequireOption("CPACK_TOPLEVEL_DIRECTORY", cpackTopLevel))
    {
    return false;
    }

  std::string includeFilename =
    cpackTopLevel + "/cpack_variables.wxi";

  cmWIXSourceWriter includeFile(Logger, includeFilename, true);
  CopyDefinition(includeFile, "CPACK_WIX_PRODUCT_GUID");
  CopyDefinition(includeFile, "CPACK_WIX_UPGRADE_GUID");
  CopyDefinition(includeFile, "CPACK_PACKAGE_VENDOR");
  CopyDefinition(includeFile, "CPACK_PACKAGE_NAME");
  CopyDefinition(includeFile, "CPACK_PACKAGE_VERSION");
  CopyDefinition(includeFile, "CPACK_WIX_LICENSE_RTF");
  CopyDefinition(includeFile, "CPACK_WIX_PRODUCT_ICON");
  CopyDefinition(includeFile, "CPACK_WIX_UI_BANNER");
  CopyDefinition(includeFile, "CPACK_WIX_UI_DIALOG");
  SetOptionIfNotSet("CPACK_WIX_PROGRAM_MENU_FOLDER",
    GetOption("CPACK_PACKAGE_NAME"));
  CopyDefinition(includeFile, "CPACK_WIX_PROGRAM_MENU_FOLDER");
  CopyDefinition(includeFile, "CPACK_WIX_UI_REF");

  return true;
}

void cmCPackWIXGenerator::CopyDefinition(
  cmWIXSourceWriter &source, const std::string &name)
{
  const char* value = GetOption(name.c_str());
  if(value)
    {
    AddDefinition(source, name, value);
    }
}

void cmCPackWIXGenerator::AddDefinition(cmWIXSourceWriter& source,
  const std::string& name, const std::string& value)
{
  std::stringstream tmp;
  tmp << name << "=\"" << value << '"';

  source.AddProcessingInstruction("define",
    cmWIXSourceWriter::WindowsCodepageToUtf8(tmp.str()));
}

bool cmCPackWIXGenerator::CreateWiXSourceFiles()
{
  std::string cpackTopLevel;
  if(!RequireOption("CPACK_TOPLEVEL_DIRECTORY", cpackTopLevel))
    {
    return false;
    }

  std::string directoryDefinitionsFilename =
    cpackTopLevel + "/directories.wxs";

  WixSources.push_back(directoryDefinitionsFilename);

  cmWIXSourceWriter directoryDefinitions(Logger, directoryDefinitionsFilename);
  directoryDefinitions.BeginElement("Fragment");

  directoryDefinitions.BeginElement("Directory");
  directoryDefinitions.AddAttribute("Id", "TARGETDIR");
  directoryDefinitions.AddAttribute("Name", "SourceDir");

  directoryDefinitions.BeginElement("Directory");
  if(GetArchitecture() == "x86")
    {
    directoryDefinitions.AddAttribute("Id", "ProgramFilesFolder");
    }
  else
    {
    directoryDefinitions.AddAttribute("Id", "ProgramFiles64Folder");
    }

  std::vector<std::string> install_root;

  std::string tmp;
  if(!RequireOption("CPACK_PACKAGE_INSTALL_DIRECTORY", tmp))
    {
    return false;
    }

  cmSystemTools::SplitPath(tmp.c_str(), install_root);

  if(!install_root.empty() && install_root.back().empty())
    {
    install_root.pop_back();
    }

  for(size_t i = 1; i < install_root.size(); ++i)
    {
    directoryDefinitions.BeginElement("Directory");

    if(i == install_root.size() - 1)
      {
      directoryDefinitions.AddAttribute("Id", "INSTALL_ROOT");
      }
    else
      {
      std::stringstream ss;
      ss << "INSTALL_PREFIX_" << i;
      directoryDefinitions.AddAttribute("Id", ss.str());
      }

    directoryDefinitions.AddAttribute("Name", install_root[i]);
  }

  std::string fileDefinitionsFilename =
    cpackTopLevel + "/files.wxs";

  WixSources.push_back(fileDefinitionsFilename);

  cmWIXSourceWriter fileDefinitions(Logger, fileDefinitionsFilename);
  fileDefinitions.BeginElement("Fragment");

  std::string featureDefinitionsFilename =
      cpackTopLevel +"/features.wxs";

  WixSources.push_back(featureDefinitionsFilename);

  cmWIXSourceWriter featureDefinitions(Logger, featureDefinitionsFilename);
  featureDefinitions.BeginElement("Fragment");

  featureDefinitions.BeginElement("Feature");
  featureDefinitions.AddAttribute("Id", "ProductFeature");
  featureDefinitions.AddAttribute("Display", "expand");
  featureDefinitions.AddAttribute("ConfigurableDirectory", "INSTALL_ROOT");

  std::string cpackPackageName;
  if(!RequireOption("CPACK_PACKAGE_NAME", cpackPackageName))
    {
    return false;
    }
  featureDefinitions.AddAttribute("Title", cpackPackageName);

  featureDefinitions.AddAttribute("Level", "1");

  if(!CreateCMakePackageRegistryEntry(featureDefinitions))
    {
    return false;
    }

  if(!CreateFeatureHierarchy(featureDefinitions))
    {
    return false;
    }

  featureDefinitions.EndElement("Feature");

  bool hasShortcuts = false;

  shortcut_map_t globalShortcuts;
  if(Components.empty())
    {
    AddComponentsToFeature(toplevel, "ProductFeature",
      directoryDefinitions, fileDefinitions, featureDefinitions,
      globalShortcuts);
    if(globalShortcuts.size())
      {
      hasShortcuts = true;
      }
    }
  else
    {
    for(std::map<std::string, cmCPackComponent>::const_iterator
      i = Components.begin(); i != Components.end(); ++i)
      {
      cmCPackComponent const& component = i->second;

      std::string componentPath = toplevel;
      componentPath += "/";
      componentPath += component.Name;

      std::string componentFeatureId = "CM_C_" + component.Name;

      shortcut_map_t featureShortcuts;
      AddComponentsToFeature(componentPath, componentFeatureId,
        directoryDefinitions, fileDefinitions,
        featureDefinitions, featureShortcuts);
      if(featureShortcuts.size())
        {
        hasShortcuts = true;
        }

      if(featureShortcuts.size())
        {
        if(!CreateStartMenuShortcuts(component.Name, componentFeatureId,
          featureShortcuts, fileDefinitions, featureDefinitions))
          {
          return false;
          }
        }
      }
    }

  if(hasShortcuts)
    {
    if(!CreateStartMenuShortcuts(std::string(), "ProductFeature",
        globalShortcuts, fileDefinitions, featureDefinitions))
      {
      return false;
      }
    }

  featureDefinitions.EndElement("Fragment");
  fileDefinitions.EndElement("Fragment");

  for(size_t i = 1; i < install_root.size(); ++i)
    {
    directoryDefinitions.EndElement("Directory");
    }

  directoryDefinitions.EndElement("Directory");

  if(hasShortcuts)
    {
    CreateStartMenuFolder(directoryDefinitions);
    }

  if(this->HasDesktopShortcuts)
    {
    CreateDesktopFolder(directoryDefinitions);
    }

  directoryDefinitions.EndElement("Directory");
  directoryDefinitions.EndElement("Fragment");

  std::string wixTemplate = FindTemplate("WIX.template.in");
  if(GetOption("CPACK_WIX_TEMPLATE") != 0)
    {
    wixTemplate = GetOption("CPACK_WIX_TEMPLATE");
    }
  if(wixTemplate.empty())
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Could not find CPack WiX template file WIX.template.in" << std::endl);
    return false;
    }

  std::string mainSourceFilePath = cpackTopLevel + "/main.wxs";

  if(!ConfigureFile(wixTemplate.c_str(), mainSourceFilePath .c_str()))
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Failed creating '" << mainSourceFilePath  <<
      "'' from template." << std::endl);

    return false;
    }

  WixSources.push_back(mainSourceFilePath);

  std::string fragmentList;
  for(cmWIXPatchParser::fragment_map_t::const_iterator
    i = Fragments.begin(); i != Fragments.end(); ++i)
    {
    if(!fragmentList.empty())
      {
      fragmentList += ", ";
      }

    fragmentList += "'";
    fragmentList += i->first;
    fragmentList += "'";
    }

  if(fragmentList.size())
    {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "Some XML patch fragments did not have matching IDs: " <<
        fragmentList << std::endl);
      return false;
    }

  return true;
}

bool cmCPackWIXGenerator::CreateCMakePackageRegistryEntry(
  cmWIXSourceWriter& featureDefinitions)
{
  const char* package = GetOption("CPACK_WIX_CMAKE_PACKAGE_REGISTRY");
  if(!package)
    {
    return true;
    }

  featureDefinitions.BeginElement("Component");
  featureDefinitions.AddAttribute("Id", "CM_PACKAGE_REGISTRY");
  featureDefinitions.AddAttribute("Directory", "TARGETDIR");
  featureDefinitions.AddAttribute("Guid", "*");

  std::string registryKey =
      std::string("Software\\Kitware\\CMake\\Packages\\") + package;

  std::string upgradeGuid = GetOption("CPACK_WIX_UPGRADE_GUID");

  featureDefinitions.BeginElement("RegistryValue");
  featureDefinitions.AddAttribute("Root", "HKLM");
  featureDefinitions.AddAttribute("Key", registryKey);
  featureDefinitions.AddAttribute("Name", upgradeGuid);
  featureDefinitions.AddAttribute("Type", "string");
  featureDefinitions.AddAttribute("Value", "[INSTALL_ROOT]");
  featureDefinitions.AddAttribute("KeyPath", "yes");
  featureDefinitions.EndElement("RegistryValue");

  featureDefinitions.EndElement("Component");

  return true;
}

bool cmCPackWIXGenerator::CreateFeatureHierarchy(
  cmWIXSourceWriter& featureDefinitions)
{
  for(std::map<std::string, cmCPackComponentGroup>::const_iterator
    i = ComponentGroups.begin(); i != ComponentGroups.end(); ++i)
    {
    cmCPackComponentGroup const& group = i->second;
    if(group.ParentGroup == 0)
      {
      if(!EmitFeatureForComponentGroup(featureDefinitions, group))
        {
        return false;
        }
      }
    }

  for(std::map<std::string, cmCPackComponent>::const_iterator
    i = Components.begin(); i != Components.end(); ++i)
    {
    cmCPackComponent const& component = i->second;

    if(!component.Group)
      {
      if(!EmitFeatureForComponent(featureDefinitions, component))
        {
        return false;
        }
      }
    }

  return true;
}

bool cmCPackWIXGenerator::EmitFeatureForComponentGroup(
  cmWIXSourceWriter& featureDefinitions,
  cmCPackComponentGroup const& group)
{
  featureDefinitions.BeginElement("Feature");
  featureDefinitions.AddAttribute("Id", "CM_G_" + group.Name);

  if(group.IsExpandedByDefault)
    {
    featureDefinitions.AddAttribute("Display", "expand");
    }

  featureDefinitions.AddAttributeUnlessEmpty(
    "Title", group.DisplayName);

  featureDefinitions.AddAttributeUnlessEmpty(
    "Description", group.Description);

  for(std::vector<cmCPackComponentGroup*>::const_iterator
    i = group.Subgroups.begin(); i != group.Subgroups.end(); ++i)
    {
    if(!EmitFeatureForComponentGroup(featureDefinitions, **i))
      {
      return false;
      }
    }

  for(std::vector<cmCPackComponent*>::const_iterator
    i = group.Components.begin(); i != group.Components.end(); ++i)
    {
    if(!EmitFeatureForComponent(featureDefinitions, **i))
      {
      return false;
      }
    }

  featureDefinitions.EndElement("Feature");

  return true;
}

bool cmCPackWIXGenerator::EmitFeatureForComponent(
  cmWIXSourceWriter& featureDefinitions,
  cmCPackComponent const& component)
{
  featureDefinitions.BeginElement("Feature");
  featureDefinitions.AddAttribute("Id", "CM_C_" + component.Name);

  featureDefinitions.AddAttributeUnlessEmpty(
    "Title", component.DisplayName);

  featureDefinitions.AddAttributeUnlessEmpty(
    "Description", component.Description);

  if(component.IsRequired)
    {
    featureDefinitions.AddAttribute("Absent", "disallow");
    }

  if(component.IsHidden)
    {
    featureDefinitions.AddAttribute("Display", "hidden");
    }

  featureDefinitions.EndElement("Feature");

  return true;
}

bool cmCPackWIXGenerator::AddComponentsToFeature(
  std::string const& rootPath,
  std::string const& featureId,
  cmWIXSourceWriter& directoryDefinitions,
  cmWIXSourceWriter& fileDefinitions,
  cmWIXSourceWriter& featureDefinitions,
  shortcut_map_t& shortcutMap)
{
  featureDefinitions.BeginElement("FeatureRef");
  featureDefinitions.AddAttribute("Id", featureId);

  std::vector<std::string> cpackPackageExecutablesList;
  const char *cpackPackageExecutables = GetOption("CPACK_PACKAGE_EXECUTABLES");
  if(cpackPackageExecutables)
    {
      cmSystemTools::ExpandListArgument(cpackPackageExecutables,
        cpackPackageExecutablesList);
      if(cpackPackageExecutablesList.size() % 2 != 0 )
        {
        cmCPackLogger(cmCPackLog::LOG_ERROR,
          "CPACK_PACKAGE_EXECUTABLES should contain pairs of <executable> and "
          "<text label>." << std::endl);
        return false;
        }
    }

  std::vector<std::string> cpackPackageDesktopLinksList;
  const char *cpackPackageDesktopLinks =
    GetOption("CPACK_CREATE_DESKTOP_LINKS");
  if(cpackPackageDesktopLinks)
    {
      cmSystemTools::ExpandListArgument(cpackPackageDesktopLinks,
        cpackPackageDesktopLinksList);
    }

  AddDirectoryAndFileDefinitons(
    rootPath, "INSTALL_ROOT",
    directoryDefinitions, fileDefinitions, featureDefinitions,
    cpackPackageExecutablesList, cpackPackageDesktopLinksList,
    shortcutMap);

  featureDefinitions.EndElement("FeatureRef");

  return true;
}

bool cmCPackWIXGenerator::CreateStartMenuShortcuts(
  std::string const& cpackComponentName,
  std::string const& featureId,
  shortcut_map_t& shortcutMap,
  cmWIXSourceWriter& fileDefinitions,
  cmWIXSourceWriter& featureDefinitions)
{
  bool thisHasDesktopShortcuts = false;

  featureDefinitions.BeginElement("FeatureRef");
  featureDefinitions.AddAttribute("Id", featureId);

  std::string cpackVendor;
  if(!RequireOption("CPACK_PACKAGE_VENDOR", cpackVendor))
    {
    return false;
    }

  std::string cpackPackageName;
  if(!RequireOption("CPACK_PACKAGE_NAME", cpackPackageName))
    {
    return false;
    }

  std::string idSuffix;
  if(!cpackComponentName.empty())
    {
      idSuffix += "_";
      idSuffix += cpackComponentName;
    }

  std::string componentId = "CM_SHORTCUT" + idSuffix;

  fileDefinitions.BeginElement("DirectoryRef");
  fileDefinitions.AddAttribute("Id", "PROGRAM_MENU_FOLDER");
  fileDefinitions.BeginElement("Component");
  fileDefinitions.AddAttribute("Id", componentId);
  fileDefinitions.AddAttribute("Guid", "*");

  for(shortcut_map_t::const_iterator
    i = shortcutMap.begin(); i != shortcutMap.end(); ++i)
    {
    std::string const& id = i->first;
    cmWIXShortcut const& shortcut = i->second;

    std::string shortcutId = std::string("CM_S") + id;
    std::string fileId = std::string("CM_F") + id;

    fileDefinitions.BeginElement("Shortcut");
    fileDefinitions.AddAttribute("Id", shortcutId);
    fileDefinitions.AddAttribute("Name", shortcut.textLabel);
    std::string target = "[#" + fileId + "]";
    fileDefinitions.AddAttribute("Target", target);
    fileDefinitions.AddAttribute("WorkingDirectory",
      shortcut.workingDirectoryId);
    fileDefinitions.EndElement("Shortcut");

    if (shortcut.desktop)
      {
        thisHasDesktopShortcuts = true;
      }
    }

  if(cpackComponentName.empty())
    {
    CreateUninstallShortcut(cpackPackageName, fileDefinitions);
    }

  fileDefinitions.BeginElement("RemoveFolder");
  fileDefinitions.AddAttribute("Id",
    "CM_REMOVE_PROGRAM_MENU_FOLDER" + idSuffix);
  fileDefinitions.AddAttribute("On", "uninstall");
  fileDefinitions.EndElement("RemoveFolder");

  std::string registryKey =
    std::string("Software\\") + cpackVendor + "\\" + cpackPackageName;

  fileDefinitions.BeginElement("RegistryValue");
  fileDefinitions.AddAttribute("Root", "HKCU");
  fileDefinitions.AddAttribute("Key", registryKey);

  std::string valueName;
  if(!cpackComponentName.empty())
    {
      valueName = cpackComponentName + "_";
    }
  valueName += "installed";

  fileDefinitions.AddAttribute("Name", valueName);
  fileDefinitions.AddAttribute("Type", "integer");
  fileDefinitions.AddAttribute("Value", "1");
  fileDefinitions.AddAttribute("KeyPath", "yes");
  fileDefinitions.EndElement("RegistryValue");

  fileDefinitions.EndElement("Component");
  fileDefinitions.EndElement("DirectoryRef");

  featureDefinitions.BeginElement("ComponentRef");
  featureDefinitions.AddAttribute("Id", componentId);
  featureDefinitions.EndElement("ComponentRef");

  if (thisHasDesktopShortcuts)
    {
    this->HasDesktopShortcuts = true;
    componentId = "CM_DESKTOP_SHORTCUT" + idSuffix;

    fileDefinitions.BeginElement("DirectoryRef");
    fileDefinitions.AddAttribute("Id", "DesktopFolder");
    fileDefinitions.BeginElement("Component");
    fileDefinitions.AddAttribute("Id", componentId);
    fileDefinitions.AddAttribute("Guid", "*");

    for (shortcut_map_t::const_iterator
      i = shortcutMap.begin(); i != shortcutMap.end(); ++i)
      {
      std::string const& id = i->first;
      cmWIXShortcut const& shortcut = i->second;

      if (!shortcut.desktop)
        continue;

      std::string shortcutId = std::string("CM_DS") + id;
      std::string fileId = std::string("CM_F") + id;

      fileDefinitions.BeginElement("Shortcut");
      fileDefinitions.AddAttribute("Id", shortcutId);
      fileDefinitions.AddAttribute("Name", shortcut.textLabel);
      std::string target = "[#" + fileId + "]";
      fileDefinitions.AddAttribute("Target", target);
      fileDefinitions.AddAttribute("WorkingDirectory",
        shortcut.workingDirectoryId);
      fileDefinitions.EndElement("Shortcut");
      }

    fileDefinitions.BeginElement("RegistryValue");
    fileDefinitions.AddAttribute("Root", "HKCU");
    fileDefinitions.AddAttribute("Key", registryKey);
    fileDefinitions.AddAttribute("Name", valueName + "_desktop");
    fileDefinitions.AddAttribute("Type", "integer");
    fileDefinitions.AddAttribute("Value", "1");
    fileDefinitions.AddAttribute("KeyPath", "yes");
    fileDefinitions.EndElement("RegistryValue");

    fileDefinitions.EndElement("Component");
    fileDefinitions.EndElement("DirectoryRef");

    featureDefinitions.BeginElement("ComponentRef");
    featureDefinitions.AddAttribute("Id", componentId);
    featureDefinitions.EndElement("ComponentRef");
    }

  featureDefinitions.EndElement("FeatureRef");

  return true;
}

void cmCPackWIXGenerator::CreateUninstallShortcut(
  std::string const& packageName,
  cmWIXSourceWriter& fileDefinitions)
{
  fileDefinitions.BeginElement("Shortcut");
  fileDefinitions.AddAttribute("Id", "UNINSTALL");
  fileDefinitions.AddAttribute("Name", "Uninstall " + packageName);
  fileDefinitions.AddAttribute("Description", "Uninstalls " + packageName);
  fileDefinitions.AddAttribute("Target", "[SystemFolder]msiexec.exe");
  fileDefinitions.AddAttribute("Arguments", "/x [ProductCode]");
  fileDefinitions.EndElement("Shortcut");
}

bool cmCPackWIXGenerator::CreateLicenseFile()
{
  std::string licenseSourceFilename;
  if(!RequireOption("CPACK_RESOURCE_FILE_LICENSE", licenseSourceFilename))
    {
    return false;
    }

  std::string licenseDestinationFilename;
  if(!RequireOption("CPACK_WIX_LICENSE_RTF", licenseDestinationFilename))
    {
    return false;
    }

  std::string extension = GetRightmostExtension(licenseSourceFilename);

  if(extension == ".rtf")
    {
    cmSystemTools::CopyAFile(
      licenseSourceFilename.c_str(),
      licenseDestinationFilename.c_str());
    }
  else if(extension == ".txt")
    {
    cmWIXRichTextFormatWriter rtfWriter(licenseDestinationFilename);

    cmsys::ifstream licenseSource(licenseSourceFilename.c_str());

    std::string line;
    while(std::getline(licenseSource, line))
      {
      rtfWriter.AddText(line);
      rtfWriter.AddText("\n");
      }
    }
  else
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "unsupported WiX License file extension '" <<
      extension << "'" << std::endl);

    return false;
    }

  return true;
}

void cmCPackWIXGenerator::AddDirectoryAndFileDefinitons(
  const std::string& topdir,
  const std::string& directoryId,
  cmWIXSourceWriter& directoryDefinitions,
  cmWIXSourceWriter& fileDefinitions,
  cmWIXSourceWriter& featureDefinitions,
  const std::vector<std::string>& packageExecutables,
  const std::vector<std::string>& desktopExecutables,
  shortcut_map_t& shortcutMap)
{
  cmsys::Directory dir;
  dir.Load(topdir.c_str());

  for(size_t i = 0; i < dir.GetNumberOfFiles(); ++i)
    {
    std::string fileName = dir.GetFile(static_cast<unsigned long>(i));

    if(fileName == "." || fileName == "..")
      {
      continue;
      }

    std::string fullPath = topdir + "/" + fileName;

    std::string relativePath = cmSystemTools::RelativePath(
      toplevel.c_str(), fullPath.c_str());

    std::string id = PathToId(relativePath);

    if(cmSystemTools::FileIsDirectory(fullPath.c_str()))
      {
      std::string subDirectoryId = std::string("CM_D") + id;

      directoryDefinitions.BeginElement("Directory");
      directoryDefinitions.AddAttribute("Id", subDirectoryId);
      directoryDefinitions.AddAttribute("Name", fileName);

      AddDirectoryAndFileDefinitons(
        fullPath, subDirectoryId,
        directoryDefinitions,
        fileDefinitions,
        featureDefinitions,
        packageExecutables,
        desktopExecutables,
        shortcutMap);

      ApplyPatchFragment(subDirectoryId, directoryDefinitions);
      directoryDefinitions.EndElement("Directory");
      }
    else
      {
      std::string componentId = std::string("CM_C") + id;
      std::string fileId = std::string("CM_F") + id;

      fileDefinitions.BeginElement("DirectoryRef");
      fileDefinitions.AddAttribute("Id", directoryId);

      fileDefinitions.BeginElement("Component");
      fileDefinitions.AddAttribute("Id", componentId);
      fileDefinitions.AddAttribute("Guid", "*");

      fileDefinitions.BeginElement("File");
      fileDefinitions.AddAttribute("Id", fileId);
      fileDefinitions.AddAttribute("Source", fullPath);
      fileDefinitions.AddAttribute("KeyPath", "yes");

      mode_t fileMode = 0;
      cmSystemTools::GetPermissions(fullPath.c_str(), fileMode);

      if(!(fileMode & S_IWRITE))
        {
        fileDefinitions.AddAttribute("ReadOnly", "yes");
        }

      ApplyPatchFragment(fileId, fileDefinitions);
      fileDefinitions.EndElement("File");

      ApplyPatchFragment(componentId, fileDefinitions);
      fileDefinitions.EndElement("Component");
      fileDefinitions.EndElement("DirectoryRef");

      featureDefinitions.BeginElement("ComponentRef");
      featureDefinitions.AddAttribute("Id", componentId);
      featureDefinitions.EndElement("ComponentRef");

      for(size_t j = 0; j < packageExecutables.size(); ++j)
        {
        std::string const& executableName = packageExecutables[j++];
        std::string const& textLabel = packageExecutables[j];

        if(cmSystemTools::LowerCase(fileName) ==
            cmSystemTools::LowerCase(executableName) + ".exe")
          {
          cmWIXShortcut &shortcut = shortcutMap[id];
          shortcut.textLabel= textLabel;
          shortcut.workingDirectoryId = directoryId;

          if(desktopExecutables.size() &&
             std::find(desktopExecutables.begin(),
                       desktopExecutables.end(),
                       executableName)
             != desktopExecutables.end())
            {
              shortcut.desktop = true;
            }
          }
        }
      }
    }
}

bool cmCPackWIXGenerator::RequireOption(
  const std::string& name, std::string &value) const
{
  const char* tmp = GetOption(name.c_str());
  if(tmp)
    {
    value = tmp;

    return true;
    }
  else
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Required variable " << name << " not set" << std::endl);

    return false;
    }
}

std::string cmCPackWIXGenerator::GetArchitecture() const
{
  std::string void_p_size;
  RequireOption("CPACK_WIX_SIZEOF_VOID_P", void_p_size);

  if(void_p_size == "8")
    {
    return "x64";
    }
  else
    {
    return "x86";
    }
}

std::string cmCPackWIXGenerator::GenerateGUID()
{
  UUID guid;
  UuidCreate(&guid);

  unsigned short *tmp = 0;
  UuidToStringW(&guid, &tmp);

  std::string result =
    cmsys::Encoding::ToNarrow(reinterpret_cast<wchar_t*>(tmp));
  RpcStringFreeW(&tmp);

  return cmSystemTools::UpperCase(result);
}

std::string cmCPackWIXGenerator::QuotePath(const std::string& path)
{
  return std::string("\"") + path + '"';
}

std::string cmCPackWIXGenerator::GetRightmostExtension(
  const std::string& filename)
{
  std::string extension;

  std::string::size_type i = filename.rfind(".");
  if(i != std::string::npos)
    {
    extension = filename.substr(i);
    }

  return cmSystemTools::LowerCase(extension);
}

std::string cmCPackWIXGenerator::PathToId(const std::string& path)
{
  id_map_t::const_iterator i = PathToIdMap.find(path);
  if(i != PathToIdMap.end()) return i->second;

  std::string id = CreateNewIdForPath(path);
  return id;
}

std::string cmCPackWIXGenerator::CreateNewIdForPath(const std::string& path)
{
  std::vector<std::string> components;
  cmSystemTools::SplitPath(path.c_str(), components, false);

  size_t replacementCount = 0;

  std::string identifier;
  std::string currentComponent;

  for(size_t i = 1; i < components.size(); ++i)
    {
    if(i != 1) identifier += '.';

    currentComponent = NormalizeComponentForId(
      components[i], replacementCount);

    identifier += currentComponent;
    }

  std::string idPrefix = "P";
  size_t replacementPercent = replacementCount * 100 / identifier.size();
  if(replacementPercent > 33 || identifier.size() > 60)
    {
    identifier = CreateHashedId(path, currentComponent);
    idPrefix = "H";
    }

  std::stringstream result;
  result << idPrefix << "_" << identifier;

  size_t ambiguityCount = ++IdAmbiguityCounter[identifier];

  if(ambiguityCount > 999)
    {
    cmCPackLogger(cmCPackLog::LOG_ERROR,
      "Error while trying to generate a unique Id for '" <<
      path << "'" << std::endl);

    return std::string();
    }
  else if(ambiguityCount > 1)
    {
    result << "_" << ambiguityCount;
    }

  std::string resultString = result.str();

  PathToIdMap[path] = resultString;

  return resultString;
}

std::string cmCPackWIXGenerator::CreateHashedId(
  const std::string& path, const std::string& normalizedFilename)
{
  cmsys::auto_ptr<cmCryptoHash> sha1 = cmCryptoHash::New("SHA1");
  std::string hash = sha1->HashString(path.c_str());

  std::string identifier;
  identifier += hash.substr(0, 7) + "_";

  const size_t maxFileNameLength = 52;
  if(normalizedFilename.length() > maxFileNameLength)
    {
    identifier += normalizedFilename.substr(0, maxFileNameLength - 3);
    identifier += "...";
    }
  else
    {
    identifier += normalizedFilename;
    }

  return identifier;
}

std::string cmCPackWIXGenerator::NormalizeComponentForId(
  const std::string& component, size_t& replacementCount)
{
  std::string result;
  result.resize(component.size());

  for(size_t i = 0; i < component.size(); ++i)
    {
    char c = component[i];
    if(IsLegalIdCharacter(c))
      {
      result[i] = c;
      }
    else
      {
      result[i] = '_';
      ++ replacementCount;
      }
    }

  return result;
}

bool cmCPackWIXGenerator::IsLegalIdCharacter(char c)
{
  return (c >= '0' && c <= '9') ||
      (c >= 'a' && c <= 'z') ||
      (c >= 'A' && c <= 'Z') ||
      c == '_' || c == '.';
}

void cmCPackWIXGenerator::CollectExtensions(
     const std::string& variableName, extension_set_t& extensions)
{
  const char *variableContent = GetOption(variableName.c_str());
  if(!variableContent) return;

  std::vector<std::string> list;
  cmSystemTools::ExpandListArgument(variableContent, list);

  for(std::vector<std::string>::const_iterator i = list.begin();
    i != list.end(); ++i)
    {
    extensions.insert(*i);
    }
}

void cmCPackWIXGenerator::AddCustomFlags(
  const std::string& variableName, std::ostream& stream)
{
  const char *variableContent = GetOption(variableName.c_str());
  if(!variableContent) return;

  std::vector<std::string> list;
  cmSystemTools::ExpandListArgument(variableContent, list);

  for(std::vector<std::string>::const_iterator i = list.begin();
    i != list.end(); ++i)
    {
      stream << " " << QuotePath(*i);
    }
}

void cmCPackWIXGenerator::CreateStartMenuFolder(
    cmWIXSourceWriter& directoryDefinitions)
{
  directoryDefinitions.BeginElement("Directory");
  directoryDefinitions.AddAttribute("Id", "ProgramMenuFolder");

  directoryDefinitions.BeginElement("Directory");
  directoryDefinitions.AddAttribute("Id", "PROGRAM_MENU_FOLDER");
  const char *startMenuFolder = GetOption("CPACK_WIX_PROGRAM_MENU_FOLDER");
  directoryDefinitions.AddAttribute("Name", startMenuFolder);
  directoryDefinitions.EndElement("Directory");

  directoryDefinitions.EndElement("Directory");
}

void cmCPackWIXGenerator::CreateDesktopFolder(
    cmWIXSourceWriter& directoryDefinitions)
{
    directoryDefinitions.BeginElement("Directory");
    directoryDefinitions.AddAttribute("Id", "DesktopFolder");
    directoryDefinitions.AddAttribute("Name", "Desktop");
    directoryDefinitions.EndElement("Directory");
}

void cmCPackWIXGenerator::LoadPatchFragments(const std::string& patchFilePath)
{
  cmWIXPatchParser parser(Fragments, Logger);
  parser.ParseFile(patchFilePath.c_str());
}

void cmCPackWIXGenerator::ApplyPatchFragment(
  const std::string& id, cmWIXSourceWriter& writer)
{
  cmWIXPatchParser::fragment_map_t::iterator i = Fragments.find(id);
  if(i == Fragments.end()) return;

  const cmWIXPatchElement& fragment = i->second;
  for(cmWIXPatchElement::child_list_t::const_iterator
    j = fragment.children.begin(); j != fragment.children.end(); ++j)
    {
    ApplyPatchElement(**j, writer);
    }

  Fragments.erase(i);
}

void cmCPackWIXGenerator::ApplyPatchElement(
  const cmWIXPatchElement& element, cmWIXSourceWriter& writer)
{
  writer.BeginElement(element.name);

  for(cmWIXPatchElement::attributes_t::const_iterator
    i = element.attributes.begin(); i != element.attributes.end(); ++i)
    {
    writer.AddAttribute(i->first, i->second);
    }

  for(cmWIXPatchElement::child_list_t::const_iterator
    i = element.children.begin(); i != element.children.end(); ++i)
    {
    ApplyPatchElement(**i, writer);
    }

  writer.EndElement(element.name);
}
