/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2012 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "cmCPackWIXGenerator.h"

#include <cmSystemTools.h>
#include <cmGeneratedFileStream.h>
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
  if(nullptr != cultures)
	  command << " -cultures:" << cultures;
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

  size_t directoryCounter = 0;
  size_t fileCounter = 0;

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

  AddDirectoryAndFileDefinitons(
    toplevel, "INSTALL_ROOT",
    directoryDefinitions, fileDefinitions, featureDefinitions,
    directoryCounter, fileCounter);

  featureDefinitions.EndElement();
  featureDefinitions.EndElement();
  fileDefinitions.EndElement();

  for(size_t i = 1; i < install_root.size(); ++i)
    {
    directoryDefinitions.EndElement();
    }

  directoryDefinitions.EndElement();
  directoryDefinitions.EndElement();
  directoryDefinitions.EndElement();

  std::string wixTemplate = FindTemplate("WIX.template.in");
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
  size_t& directoryCounter,
  size_t& fileCounter)
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

    if(cmSystemTools::FileIsDirectory(fullPath.c_str()))
      {
      std::stringstream tmp;
      tmp << "DIR_ID_" << ++directoryCounter;
      std::string subDirectoryId = tmp.str();

      directoryDefinitions.BeginElement("Directory");
      directoryDefinitions.AddAttribute("Id", subDirectoryId);
      directoryDefinitions.AddAttribute("Name", fileName);

      AddDirectoryAndFileDefinitons(
        fullPath, subDirectoryId,
        directoryDefinitions,
        fileDefinitions,
        featureDefinitions,
        directoryCounter,
        fileCounter);

      directoryDefinitions.EndElement();
      }
    else
      {
      std::stringstream tmp;
      tmp << "_ID_" << ++fileCounter;
      std::string idSuffix = tmp.str();

      std::string componentId = std::string("CMP") + idSuffix;
      std::string fileId = std::string("FILE") + idSuffix;

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
