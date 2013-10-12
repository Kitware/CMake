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

#include <rpc.h> // for GUID generation

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

  std::ofstream logFile(logFileName.c_str(), std::ios::app);
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
  command << " -ext WixUIExtension";
  const char* const cultures = GetOption("CPACK_WIX_CULTURES");
  if(cultures)
    {
    command << " -cultures:" << cultures;
    }
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

  std::stringstream objectFiles;
  for(size_t i = 0; i < wixSources.size(); ++i)
    {
    const std::string& sourceFilename = wixSources[i];

    std::string objectFilename =
      cmSystemTools::GetFilenameWithoutExtension(sourceFilename) + ".wixobj";

    if(!RunCandleCommand(sourceFilename, objectFilename))
      {
      return false;
      }

    objectFiles << " " << QuotePath(objectFilename);
    }

  return RunLightCommand(objectFiles.str());
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

  wixSources.push_back(directoryDefinitionsFilename);

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

  wixSources.push_back(fileDefinitionsFilename);

  cmWIXSourceWriter fileDefinitions(Logger, fileDefinitionsFilename);
  fileDefinitions.BeginElement("Fragment");

  std::string featureDefinitionsFilename =
      cpackTopLevel +"/features.wxs";

  wixSources.push_back(featureDefinitionsFilename);

  cmWIXSourceWriter featureDefinitions(Logger, featureDefinitionsFilename);
  featureDefinitions.BeginElement("Fragment");

  featureDefinitions.BeginElement("Feature");
  featureDefinitions.AddAttribute("Id", "ProductFeature");
  featureDefinitions.AddAttribute("Title", Name);
  featureDefinitions.AddAttribute("Level", "1");
  featureDefinitions.EndElement();

  featureDefinitions.BeginElement("FeatureRef");
  featureDefinitions.AddAttribute("Id", "ProductFeature");

  const char *cpackPackageExecutables = GetOption("CPACK_PACKAGE_EXECUTABLES");
  std::vector<std::string> cpackPkgExecutables;
  std::string regKey;
  if ( cpackPackageExecutables )
    {
    cmSystemTools::ExpandListArgument(cpackPackageExecutables,
      cpackPkgExecutables);
    if ( cpackPkgExecutables.size() % 2 != 0 )
      {
      cmCPackLogger(cmCPackLog::LOG_ERROR,
        "CPACK_PACKAGE_EXECUTABLES should contain pairs of <executable> and "
        "<icon name>." << std::endl);
      cpackPkgExecutables.clear();
      }

    const char *cpackVendor = GetOption("CPACK_PACKAGE_VENDOR");
    const char *cpackPkgName = GetOption("CPACK_PACKAGE_NAME");
    if (!cpackVendor || !cpackPkgName)
      {
      cmCPackLogger(cmCPackLog::LOG_WARNING, "CPACK_PACKAGE_VENDOR and "
        "CPACK_PACKAGE_NAME must be defined for shortcut creation"
        << std::endl);
      cpackPkgExecutables.clear();
      }
    else
      {
        regKey = std::string("Software/") + cpackVendor + "/" + cpackPkgName;
      }
    }

  std::vector<std::string> dirIdExecutables;
  AddDirectoryAndFileDefinitons(
    toplevel, "INSTALL_ROOT",
    directoryDefinitions, fileDefinitions, featureDefinitions,
    cpackPkgExecutables, dirIdExecutables);

  directoryDefinitions.EndElement();
  directoryDefinitions.EndElement();

  if (dirIdExecutables.size() > 0 && dirIdExecutables.size() % 3 == 0)
    {
    fileDefinitions.BeginElement("DirectoryRef");
    fileDefinitions.AddAttribute("Id", "PROGRAM_MENU_FOLDER");
    fileDefinitions.BeginElement("Component");
    fileDefinitions.AddAttribute("Id", "SHORTCUT");
    fileDefinitions.AddAttribute("Guid", "*");

    std::vector<std::string>::iterator it;
    for ( it = dirIdExecutables.begin() ;
          it != dirIdExecutables.end();
          ++it)
      {
      std::string fileName = *it++;
      std::string iconName = *it++;
      std::string directoryId = *it;

      fileDefinitions.BeginElement("Shortcut");

      // the iconName is more likely to contain blanks early on
      std::string shortcutName = fileName;

      std::string::size_type const dotPos = shortcutName.find('.');
      if(std::string::npos == dotPos)
        { shortcutName = shortcutName.substr(0, dotPos); }
      fileDefinitions.AddAttribute("Id", "SHORTCUT_" + shortcutName);
      fileDefinitions.AddAttribute("Name", iconName);
      std::string target = "[" + directoryId + "]" + fileName;
      fileDefinitions.AddAttribute("Target", target);
      fileDefinitions.AddAttribute("WorkingDirectory", directoryId);
      fileDefinitions.EndElement();
      }
    fileDefinitions.BeginElement("Shortcut");
    fileDefinitions.AddAttribute("Id", "UNINSTALL");
    std::string pkgName = GetOption("CPACK_PACKAGE_NAME");
    fileDefinitions.AddAttribute("Name", "Uninstall " + pkgName);
    fileDefinitions.AddAttribute("Description", "Uninstalls " + pkgName);
    fileDefinitions.AddAttribute("Target", "[SystemFolder]msiexec.exe");
    fileDefinitions.AddAttribute("Arguments", "/x [ProductCode]");
    fileDefinitions.EndElement();
    fileDefinitions.BeginElement("RemoveFolder");
    fileDefinitions.AddAttribute("Id", "PROGRAM_MENU_FOLDER");
    fileDefinitions.AddAttribute("On", "uninstall");
    fileDefinitions.EndElement();
    fileDefinitions.BeginElement("RegistryValue");
    fileDefinitions.AddAttribute("Root", "HKCU");
    fileDefinitions.AddAttribute("Key", regKey);
    fileDefinitions.AddAttribute("Name", "installed");
    fileDefinitions.AddAttribute("Type", "integer");
    fileDefinitions.AddAttribute("Value", "1");
    fileDefinitions.AddAttribute("KeyPath", "yes");

    featureDefinitions.BeginElement("ComponentRef");
    featureDefinitions.AddAttribute("Id", "SHORTCUT");
    featureDefinitions.EndElement();
    directoryDefinitions.BeginElement("Directory");
    directoryDefinitions.AddAttribute("Id", "ProgramMenuFolder");
    directoryDefinitions.BeginElement("Directory");
    directoryDefinitions.AddAttribute("Id", "PROGRAM_MENU_FOLDER");
    const char *startMenuFolder = GetOption("CPACK_WIX_PROGRAM_MENU_FOLDER");
    directoryDefinitions.AddAttribute("Name", startMenuFolder);
  }

  featureDefinitions.EndElement();
  featureDefinitions.EndElement();
  fileDefinitions.EndElement();
  directoryDefinitions.EndElement();

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

  wixSources.push_back(mainSourceFilePath);

  return true;
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

    std::ifstream licenseSource(licenseSourceFilename.c_str());

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
  const std::vector<std::string>& pkgExecutables,
  std::vector<std::string>& dirIdExecutables)
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
        pkgExecutables,
        dirIdExecutables);
      directoryDefinitions.EndElement();
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

      fileDefinitions.EndElement();
      fileDefinitions.EndElement();
      fileDefinitions.EndElement();

      featureDefinitions.BeginElement("ComponentRef");
      featureDefinitions.AddAttribute("Id", componentId);
      featureDefinitions.EndElement();

      std::vector<std::string>::const_iterator it;
      for (it = pkgExecutables.begin() ;
           it != pkgExecutables.end() ;
           ++it)
        {
        std::string execName = *it++;
        std::string iconName = *it;

        if (cmSystemTools::LowerCase(fileName) ==
            cmSystemTools::LowerCase(execName) + ".exe")
          {
            dirIdExecutables.push_back(fileName);
            dirIdExecutables.push_back(iconName);
            dirIdExecutables.push_back(directoryId);
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

  unsigned char *tmp = 0;
  UuidToString(&guid, &tmp);

  std::string result(reinterpret_cast<char*>(tmp));
  RpcStringFree(&tmp);

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
  id_map_t::const_iterator i = pathToIdMap.find(path);
  if(i != pathToIdMap.end()) return i->second;

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

  size_t ambiguityCount = ++idAmbiguityCounter[identifier];

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

  pathToIdMap[path] = resultString;

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
