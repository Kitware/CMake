/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmVisualStudio10TargetGenerator.h"

#include "cmComputeLinkInformation.h"
#include "cmCustomCommandGenerator.h"
#include "cmGeneratedFileStream.h"
#include "cmGeneratorTarget.h"
#include "cmGlobalVisualStudio10Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmVS10CLFlagTable.h"
#include "cmVS10LibFlagTable.h"
#include "cmVS10LinkFlagTable.h"
#include "cmVS10MASMFlagTable.h"
#include "cmVS10RCFlagTable.h"
#include "cmVS11CLFlagTable.h"
#include "cmVS11LibFlagTable.h"
#include "cmVS11LinkFlagTable.h"
#include "cmVS11MASMFlagTable.h"
#include "cmVS11RCFlagTable.h"
#include "cmVS12CLFlagTable.h"
#include "cmVS12LibFlagTable.h"
#include "cmVS12LinkFlagTable.h"
#include "cmVS12MASMFlagTable.h"
#include "cmVS12RCFlagTable.h"
#include "cmVS14CLFlagTable.h"
#include "cmVS14LibFlagTable.h"
#include "cmVS14LinkFlagTable.h"
#include "cmVS14MASMFlagTable.h"
#include "cmVS14RCFlagTable.h"
#include "cmVisualStudioGeneratorOptions.h"
#include "windows.h"

#include <cmsys/auto_ptr.hxx>

cmIDEFlagTable const* cmVisualStudio10TargetGenerator::GetClFlagTable() const
{
  if (this->MSTools) {
    cmGlobalVisualStudioGenerator::VSVersion v =
      this->LocalGenerator->GetVersion();
    if (v >= cmGlobalVisualStudioGenerator::VS14) {
      return cmVS14CLFlagTable;
    } else if (v >= cmGlobalVisualStudioGenerator::VS12) {
      return cmVS12CLFlagTable;
    } else if (v == cmGlobalVisualStudioGenerator::VS11) {
      return cmVS11CLFlagTable;
    } else {
      return cmVS10CLFlagTable;
    }
  }
  return 0;
}

cmIDEFlagTable const* cmVisualStudio10TargetGenerator::GetRcFlagTable() const
{
  if (this->MSTools) {
    cmGlobalVisualStudioGenerator::VSVersion v =
      this->LocalGenerator->GetVersion();
    if (v >= cmGlobalVisualStudioGenerator::VS14) {
      return cmVS14RCFlagTable;
    } else if (v >= cmGlobalVisualStudioGenerator::VS12) {
      return cmVS12RCFlagTable;
    } else if (v == cmGlobalVisualStudioGenerator::VS11) {
      return cmVS11RCFlagTable;
    } else {
      return cmVS10RCFlagTable;
    }
  }
  return 0;
}

cmIDEFlagTable const* cmVisualStudio10TargetGenerator::GetLibFlagTable() const
{
  if (this->MSTools) {
    cmGlobalVisualStudioGenerator::VSVersion v =
      this->LocalGenerator->GetVersion();
    if (v >= cmGlobalVisualStudioGenerator::VS14) {
      return cmVS14LibFlagTable;
    } else if (v >= cmGlobalVisualStudioGenerator::VS12) {
      return cmVS12LibFlagTable;
    } else if (v == cmGlobalVisualStudioGenerator::VS11) {
      return cmVS11LibFlagTable;
    } else {
      return cmVS10LibFlagTable;
    }
  }
  return 0;
}

cmIDEFlagTable const* cmVisualStudio10TargetGenerator::GetLinkFlagTable() const
{
  if (this->MSTools) {
    cmGlobalVisualStudioGenerator::VSVersion v =
      this->LocalGenerator->GetVersion();
    if (v >= cmGlobalVisualStudioGenerator::VS14) {
      return cmVS14LinkFlagTable;
    } else if (v >= cmGlobalVisualStudioGenerator::VS12) {
      return cmVS12LinkFlagTable;
    } else if (v == cmGlobalVisualStudioGenerator::VS11) {
      return cmVS11LinkFlagTable;
    } else {
      return cmVS10LinkFlagTable;
    }
  }
  return 0;
}

cmIDEFlagTable const* cmVisualStudio10TargetGenerator::GetMasmFlagTable() const
{
  if (this->MSTools) {
    cmGlobalVisualStudioGenerator::VSVersion v =
      this->LocalGenerator->GetVersion();
    if (v >= cmGlobalVisualStudioGenerator::VS14) {
      return cmVS14MASMFlagTable;
    } else if (v >= cmGlobalVisualStudioGenerator::VS12) {
      return cmVS12MASMFlagTable;
    } else if (v == cmGlobalVisualStudioGenerator::VS11) {
      return cmVS11MASMFlagTable;
    } else {
      return cmVS10MASMFlagTable;
    }
  }
  return 0;
}

static std::string cmVS10EscapeXML(std::string arg)
{
  cmSystemTools::ReplaceString(arg, "&", "&amp;");
  cmSystemTools::ReplaceString(arg, "<", "&lt;");
  cmSystemTools::ReplaceString(arg, ">", "&gt;");
  return arg;
}

static std::string cmVS10EscapeComment(std::string comment)
{
  // MSBuild takes the CDATA of a <Message></Message> element and just
  // does "echo $CDATA" with no escapes.  We must encode the string.
  // http://technet.microsoft.com/en-us/library/cc772462%28WS.10%29.aspx
  std::string echoable;
  for (std::string::iterator c = comment.begin(); c != comment.end(); ++c) {
    switch (*c) {
      case '\r':
        break;
      case '\n':
        echoable += '\t';
        break;
      case '"': /* no break */
      case '|': /* no break */
      case '&': /* no break */
      case '<': /* no break */
      case '>': /* no break */
      case '^':
        echoable += '^'; /* no break */
      default:
        echoable += *c;
        break;
    }
  }
  return echoable;
}

cmVisualStudio10TargetGenerator::cmVisualStudio10TargetGenerator(
  cmGeneratorTarget* target, cmGlobalVisualStudio10Generator* gg)
{
  this->GlobalGenerator = gg;
  this->GeneratorTarget = target;
  this->Makefile = target->Target->GetMakefile();
  this->Makefile->GetConfigurations(this->Configurations);
  this->LocalGenerator =
    (cmLocalVisualStudio7Generator*)this->GeneratorTarget->GetLocalGenerator();
  this->Name = this->GeneratorTarget->GetName();
  this->GUID = this->GlobalGenerator->GetGUID(this->Name.c_str());
  this->Platform = gg->GetPlatformName();
  this->NsightTegra = gg->IsNsightTegra();
  for (int i =
         sscanf(gg->GetNsightTegraVersion().c_str(), "%u.%u.%u.%u",
                &this->NsightTegraVersion[0], &this->NsightTegraVersion[1],
                &this->NsightTegraVersion[2], &this->NsightTegraVersion[3]);
       i < 4; ++i) {
    this->NsightTegraVersion[i] = 0;
  }
  this->MSTools = !this->NsightTegra;
  this->TargetCompileAsWinRT = false;
  this->BuildFileStream = 0;
  this->IsMissingFiles = false;
  this->DefaultArtifactDir =
    this->LocalGenerator->GetCurrentBinaryDirectory() + std::string("/") +
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
}

cmVisualStudio10TargetGenerator::~cmVisualStudio10TargetGenerator()
{
  for (OptionsMap::iterator i = this->ClOptions.begin();
       i != this->ClOptions.end(); ++i) {
    delete i->second;
  }
  for (OptionsMap::iterator i = this->LinkOptions.begin();
       i != this->LinkOptions.end(); ++i) {
    delete i->second;
  }
  if (!this->BuildFileStream) {
    return;
  }
  if (this->BuildFileStream->Close()) {
    this->GlobalGenerator->FileReplacedDuringGenerate(this->PathToVcxproj);
  }
  delete this->BuildFileStream;
}

void cmVisualStudio10TargetGenerator::WritePlatformConfigTag(
  const char* tag, const std::string& config, int indentLevel,
  const char* attribute, const char* end, std::ostream* stream)

{
  if (!stream) {
    stream = this->BuildFileStream;
  }
  stream->fill(' ');
  stream->width(indentLevel * 2);
  (*stream) << "";
  (*stream) << "<" << tag << " Condition=\"'$(Configuration)|$(Platform)'=='";
  (*stream) << config << "|" << this->Platform << "'\"";
  if (attribute) {
    (*stream) << attribute;
  }
  // close the tag
  (*stream) << ">";
  if (end) {
    (*stream) << end;
  }
}

void cmVisualStudio10TargetGenerator::WriteString(const char* line,
                                                  int indentLevel)
{
  this->BuildFileStream->fill(' ');
  this->BuildFileStream->width(indentLevel * 2);
  // write an empty string to get the fill level indent to print
  (*this->BuildFileStream) << "";
  (*this->BuildFileStream) << line;
}

#define VS10_USER_PROPS "$(UserRootDir)\\Microsoft.Cpp.$(Platform).user.props"

void cmVisualStudio10TargetGenerator::Generate()
{
  // do not generate external ms projects
  if (this->GeneratorTarget->GetType() == cmState::INTERFACE_LIBRARY ||
      this->GeneratorTarget->GetProperty("EXTERNAL_MSPROJECT")) {
    return;
  }
  // Tell the global generator the name of the project file
  this->GeneratorTarget->Target->SetProperty("GENERATOR_FILE_NAME",
                                             this->Name.c_str());
  this->GeneratorTarget->Target->SetProperty("GENERATOR_FILE_NAME_EXT",
                                             ".vcxproj");
  if (this->GeneratorTarget->GetType() <= cmState::OBJECT_LIBRARY) {
    if (!this->ComputeClOptions()) {
      return;
    }
    if (!this->ComputeRcOptions()) {
      return;
    }
    if (!this->ComputeMasmOptions()) {
      return;
    }
    if (!this->ComputeLinkOptions()) {
      return;
    }
  }
  std::string path = this->LocalGenerator->GetCurrentBinaryDirectory();
  path += "/";
  path += this->Name;
  path += ".vcxproj";
  this->BuildFileStream = new cmGeneratedFileStream(path.c_str());
  this->PathToVcxproj = path;
  this->BuildFileStream->SetCopyIfDifferent(true);

  // Write the encoding header into the file
  char magic[] = { char(0xEF), char(0xBB), char(0xBF) };
  this->BuildFileStream->write(magic, 3);

  // get the tools version to use
  const std::string toolsVer(this->GlobalGenerator->GetToolsVersion());
  std::string project_defaults = "<?xml version=\"1.0\" encoding=\"" +
    this->GlobalGenerator->Encoding() + "\"?>\n";
  project_defaults.append("<Project DefaultTargets=\"Build\" ToolsVersion=\"");
  project_defaults.append(toolsVer + "\" ");
  project_defaults.append(
    "xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n");
  this->WriteString(project_defaults.c_str(), 0);

  if (this->NsightTegra) {
    this->WriteString("<PropertyGroup Label=\"NsightTegraProject\">\n", 1);
    const int nsightTegraMajorVersion = this->NsightTegraVersion[0];
    const int nsightTegraMinorVersion = this->NsightTegraVersion[1];
    if (nsightTegraMajorVersion >= 2) {
      this->WriteString("<NsightTegraProjectRevisionNumber>", 2);
      if (nsightTegraMajorVersion > 3 ||
          (nsightTegraMajorVersion == 3 && nsightTegraMinorVersion >= 1)) {
        (*this->BuildFileStream) << "11";
      } else {
        // Nsight Tegra 2.0 uses project revision 9.
        (*this->BuildFileStream) << "9";
      }
      (*this->BuildFileStream) << "</NsightTegraProjectRevisionNumber>\n";
      // Tell newer versions to upgrade silently when loading.
      this->WriteString("<NsightTegraUpgradeOnceWithoutPrompt>"
                        "true"
                        "</NsightTegraUpgradeOnceWithoutPrompt>\n",
                        2);
    } else {
      // Require Nsight Tegra 1.6 for JCompile support.
      this->WriteString("<NsightTegraProjectRevisionNumber>"
                        "7"
                        "</NsightTegraProjectRevisionNumber>\n",
                        2);
    }
    this->WriteString("</PropertyGroup>\n", 1);
  }

  this->WriteProjectConfigurations();
  this->WriteString("<PropertyGroup Label=\"Globals\">\n", 1);
  this->WriteString("<ProjectGUID>", 2);
  (*this->BuildFileStream) << "{" << this->GUID << "}</ProjectGUID>\n";

  if (this->MSTools &&
      this->GeneratorTarget->GetType() <= cmState::GLOBAL_TARGET) {
    this->WriteApplicationTypeSettings();
    this->VerifyNecessaryFiles();
  }

  const char* vsProjectTypes =
    this->GeneratorTarget->GetProperty("VS_GLOBAL_PROJECT_TYPES");
  if (vsProjectTypes) {
    this->WriteString("<ProjectTypes>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(vsProjectTypes)
                             << "</ProjectTypes>\n";
  }

  const char* vsProjectName =
    this->GeneratorTarget->GetProperty("VS_SCC_PROJECTNAME");
  const char* vsLocalPath =
    this->GeneratorTarget->GetProperty("VS_SCC_LOCALPATH");
  const char* vsProvider =
    this->GeneratorTarget->GetProperty("VS_SCC_PROVIDER");

  if (vsProjectName && vsLocalPath && vsProvider) {
    this->WriteString("<SccProjectName>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(vsProjectName)
                             << "</SccProjectName>\n";
    this->WriteString("<SccLocalPath>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(vsLocalPath)
                             << "</SccLocalPath>\n";
    this->WriteString("<SccProvider>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(vsProvider)
                             << "</SccProvider>\n";

    const char* vsAuxPath =
      this->GeneratorTarget->GetProperty("VS_SCC_AUXPATH");
    if (vsAuxPath) {
      this->WriteString("<SccAuxPath>", 2);
      (*this->BuildFileStream) << cmVS10EscapeXML(vsAuxPath)
                               << "</SccAuxPath>\n";
    }
  }

  if (this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_COMPONENT")) {
    this->WriteString("<WinMDAssembly>true</WinMDAssembly>\n", 2);
  }

  const char* vsGlobalKeyword =
    this->GeneratorTarget->GetProperty("VS_GLOBAL_KEYWORD");
  if (!vsGlobalKeyword) {
    this->WriteString("<Keyword>Win32Proj</Keyword>\n", 2);
  } else {
    this->WriteString("<Keyword>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(vsGlobalKeyword)
                             << "</Keyword>\n";
  }

  const char* vsGlobalRootNamespace =
    this->GeneratorTarget->GetProperty("VS_GLOBAL_ROOTNAMESPACE");
  if (vsGlobalRootNamespace) {
    this->WriteString("<RootNamespace>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(vsGlobalRootNamespace)
                             << "</RootNamespace>\n";
  }

  this->WriteString("<Platform>", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(this->Platform)
                           << "</Platform>\n";
  const char* projLabel = this->GeneratorTarget->GetProperty("PROJECT_LABEL");
  if (!projLabel) {
    projLabel = this->Name.c_str();
  }
  this->WriteString("<ProjectName>", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(projLabel) << "</ProjectName>\n";
  if (const char* targetFrameworkVersion = this->GeneratorTarget->GetProperty(
        "VS_DOTNET_TARGET_FRAMEWORK_VERSION")) {
    this->WriteString("<TargetFrameworkVersion>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(targetFrameworkVersion)
                             << "</TargetFrameworkVersion>\n";
  }

  std::vector<std::string> keys = this->GeneratorTarget->GetPropertyKeys();
  for (std::vector<std::string>::const_iterator keyIt = keys.begin();
       keyIt != keys.end(); ++keyIt) {
    static const char* prefix = "VS_GLOBAL_";
    if (keyIt->find(prefix) != 0)
      continue;
    std::string globalKey = keyIt->substr(strlen(prefix));
    // Skip invalid or separately-handled properties.
    if (globalKey == "" || globalKey == "PROJECT_TYPES" ||
        globalKey == "ROOTNAMESPACE" || globalKey == "KEYWORD") {
      continue;
    }
    const char* value = this->GeneratorTarget->GetProperty(keyIt->c_str());
    if (!value)
      continue;
    this->WriteString("<", 2);
    (*this->BuildFileStream) << globalKey << ">" << cmVS10EscapeXML(value)
                             << "</" << globalKey << ">\n";
  }

  this->WriteString("</PropertyGroup>\n", 1);
  this->WriteString("<Import Project="
                    "\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n",
                    1);
  this->WriteProjectConfigurationValues();
  this->WriteString(
    "<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n", 1);
  this->WriteString("<ImportGroup Label=\"ExtensionSettings\">\n", 1);
  if (this->GlobalGenerator->IsMasmEnabled()) {
    this->WriteString("<Import Project=\"$(VCTargetsPath)\\"
                      "BuildCustomizations\\masm.props\" />\n",
                      2);
  }
  this->WriteString("</ImportGroup>\n", 1);
  this->WriteString("<ImportGroup Label=\"PropertySheets\">\n", 1);
  this->WriteString("<Import Project=\"" VS10_USER_PROPS "\""
                    " Condition=\"exists('" VS10_USER_PROPS "')\""
                    " Label=\"LocalAppDataPlatform\" />\n",
                    2);
  this->WritePlatformExtensions();
  this->WriteString("</ImportGroup>\n", 1);
  this->WriteString("<PropertyGroup Label=\"UserMacros\" />\n", 1);
  this->WriteWinRTPackageCertificateKeyFile();
  this->WritePathAndIncrementalLinkOptions();
  this->WriteItemDefinitionGroups();
  this->WriteCustomCommands();
  this->WriteAllSources();
  this->WriteDotNetReferences();
  this->WriteEmbeddedResourceGroup();
  this->WriteXamlFilesGroup();
  this->WriteWinRTReferences();
  this->WriteProjectReferences();
  this->WriteSDKReferences();
  this->WriteString(
    "<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\""
    " />\n",
    1);
  this->WriteTargetSpecificReferences();
  this->WriteString("<ImportGroup Label=\"ExtensionTargets\">\n", 1);
  if (this->GlobalGenerator->IsMasmEnabled()) {
    this->WriteString("<Import Project=\"$(VCTargetsPath)\\"
                      "BuildCustomizations\\masm.targets\" />\n",
                      2);
  }
  this->WriteString("</ImportGroup>\n", 1);
  this->WriteString("</Project>", 0);
  // The groups are stored in a separate file for VS 10
  this->WriteGroups();
}

void cmVisualStudio10TargetGenerator::WriteDotNetReferences()
{
  std::vector<std::string> references;
  if (const char* vsDotNetReferences =
        this->GeneratorTarget->GetProperty("VS_DOTNET_REFERENCES")) {
    cmSystemTools::ExpandListArgument(vsDotNetReferences, references);
  }
  if (!references.empty()) {
    this->WriteString("<ItemGroup>\n", 1);
    for (std::vector<std::string>::iterator ri = references.begin();
         ri != references.end(); ++ri) {
      this->WriteString("<Reference Include=\"", 2);
      (*this->BuildFileStream) << cmVS10EscapeXML(*ri) << "\">\n";
      this->WriteString("<CopyLocalSatelliteAssemblies>true"
                        "</CopyLocalSatelliteAssemblies>\n",
                        3);
      this->WriteString("<ReferenceOutputAssembly>true"
                        "</ReferenceOutputAssembly>\n",
                        3);
      this->WriteString("</Reference>\n", 2);
    }
    this->WriteString("</ItemGroup>\n", 1);
  }
}

void cmVisualStudio10TargetGenerator::WriteEmbeddedResourceGroup()
{
  std::vector<cmSourceFile const*> resxObjs;
  this->GeneratorTarget->GetResxSources(resxObjs, "");
  if (!resxObjs.empty()) {
    this->WriteString("<ItemGroup>\n", 1);
    for (std::vector<cmSourceFile const*>::const_iterator oi =
           resxObjs.begin();
         oi != resxObjs.end(); ++oi) {
      std::string obj = (*oi)->GetFullPath();
      this->WriteString("<EmbeddedResource Include=\"", 2);
      this->ConvertToWindowsSlash(obj);
      (*this->BuildFileStream) << obj << "\">\n";

      this->WriteString("<DependentUpon>", 3);
      std::string hFileName = obj.substr(0, obj.find_last_of(".")) + ".h";
      (*this->BuildFileStream) << hFileName << "</DependentUpon>\n";

      for (std::vector<std::string>::const_iterator i =
             this->Configurations.begin();
           i != this->Configurations.end(); ++i) {
        this->WritePlatformConfigTag("LogicalName", i->c_str(), 3);
        if (this->GeneratorTarget->GetProperty("VS_GLOBAL_ROOTNAMESPACE")) {
          (*this->BuildFileStream) << "$(RootNamespace).";
        }
        (*this->BuildFileStream) << "%(Filename)";
        (*this->BuildFileStream) << ".resources";
        (*this->BuildFileStream) << "</LogicalName>\n";
      }

      this->WriteString("</EmbeddedResource>\n", 2);
    }
    this->WriteString("</ItemGroup>\n", 1);
  }
}

void cmVisualStudio10TargetGenerator::WriteXamlFilesGroup()
{
  std::vector<cmSourceFile const*> xamlObjs;
  this->GeneratorTarget->GetXamlSources(xamlObjs, "");
  if (!xamlObjs.empty()) {
    this->WriteString("<ItemGroup>\n", 1);
    for (std::vector<cmSourceFile const*>::const_iterator oi =
           xamlObjs.begin();
         oi != xamlObjs.end(); ++oi) {
      std::string obj = (*oi)->GetFullPath();
      std::string xamlType;
      const char* xamlTypeProperty = (*oi)->GetProperty("VS_XAML_TYPE");
      if (xamlTypeProperty) {
        xamlType = xamlTypeProperty;
      } else {
        xamlType = "Page";
      }

      this->WriteSource(xamlType, *oi, ">\n");
      this->WriteString("<SubType>Designer</SubType>\n", 3);
      this->WriteString("</", 2);
      (*this->BuildFileStream) << xamlType << ">\n";
    }
    this->WriteString("</ItemGroup>\n", 1);
  }
}

void cmVisualStudio10TargetGenerator::WriteTargetSpecificReferences()
{
  if (this->MSTools) {
    if (this->GlobalGenerator->TargetsWindowsPhone() &&
        this->GlobalGenerator->GetSystemVersion() == "8.0") {
      this->WriteString("<Import Project=\""
                        "$(MSBuildExtensionsPath)\\Microsoft\\WindowsPhone\\v"
                        "$(TargetPlatformVersion)\\Microsoft.Cpp.WindowsPhone."
                        "$(TargetPlatformVersion).targets\" />\n",
                        1);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteWinRTReferences()
{
  std::vector<std::string> references;
  if (const char* vsWinRTReferences =
        this->GeneratorTarget->GetProperty("VS_WINRT_REFERENCES")) {
    cmSystemTools::ExpandListArgument(vsWinRTReferences, references);
  }

  if (this->GlobalGenerator->TargetsWindowsPhone() &&
      this->GlobalGenerator->GetSystemVersion() == "8.0" &&
      references.empty()) {
    references.push_back("platform.winmd");
  }
  if (!references.empty()) {
    this->WriteString("<ItemGroup>\n", 1);
    for (std::vector<std::string>::iterator ri = references.begin();
         ri != references.end(); ++ri) {
      this->WriteString("<Reference Include=\"", 2);
      (*this->BuildFileStream) << cmVS10EscapeXML(*ri) << "\">\n";
      this->WriteString("<IsWinMDFile>true</IsWinMDFile>\n", 3);
      this->WriteString("</Reference>\n", 2);
    }
    this->WriteString("</ItemGroup>\n", 1);
  }
}

// ConfigurationType Application, Utility StaticLibrary DynamicLibrary

void cmVisualStudio10TargetGenerator::WriteProjectConfigurations()
{
  this->WriteString("<ItemGroup Label=\"ProjectConfigurations\">\n", 1);
  for (std::vector<std::string>::const_iterator i =
         this->Configurations.begin();
       i != this->Configurations.end(); ++i) {
    this->WriteString("<ProjectConfiguration Include=\"", 2);
    (*this->BuildFileStream) << *i << "|" << this->Platform << "\">\n";
    this->WriteString("<Configuration>", 3);
    (*this->BuildFileStream) << *i << "</Configuration>\n";
    this->WriteString("<Platform>", 3);
    (*this->BuildFileStream) << cmVS10EscapeXML(this->Platform)
                             << "</Platform>\n";
    this->WriteString("</ProjectConfiguration>\n", 2);
  }
  this->WriteString("</ItemGroup>\n", 1);
}

void cmVisualStudio10TargetGenerator::WriteProjectConfigurationValues()
{
  for (std::vector<std::string>::const_iterator i =
         this->Configurations.begin();
       i != this->Configurations.end(); ++i) {
    this->WritePlatformConfigTag("PropertyGroup", i->c_str(), 1,
                                 " Label=\"Configuration\"", "\n");
    std::string configType = "<ConfigurationType>";
    if (const char* vsConfigurationType =
          this->GeneratorTarget->GetProperty("VS_CONFIGURATION_TYPE")) {
      configType += cmVS10EscapeXML(vsConfigurationType);
    } else {
      switch (this->GeneratorTarget->GetType()) {
        case cmState::SHARED_LIBRARY:
        case cmState::MODULE_LIBRARY:
          configType += "DynamicLibrary";
          break;
        case cmState::OBJECT_LIBRARY:
        case cmState::STATIC_LIBRARY:
          configType += "StaticLibrary";
          break;
        case cmState::EXECUTABLE:
          if (this->NsightTegra &&
              !this->GeneratorTarget->GetPropertyAsBool("ANDROID_GUI")) {
            // Android executables are .so too.
            configType += "DynamicLibrary";
          } else {
            configType += "Application";
          }
          break;
        case cmState::UTILITY:
        case cmState::GLOBAL_TARGET:
          if (this->NsightTegra) {
            // Tegra-Android platform does not understand "Utility".
            configType += "StaticLibrary";
          } else {
            configType += "Utility";
          }
          break;
        case cmState::UNKNOWN_LIBRARY:
        case cmState::INTERFACE_LIBRARY:
          break;
      }
    }
    configType += "</ConfigurationType>\n";
    this->WriteString(configType.c_str(), 2);

    if (this->MSTools) {
      this->WriteMSToolConfigurationValues(*i);
    } else if (this->NsightTegra) {
      this->WriteNsightTegraConfigurationValues(*i);
    }

    this->WriteString("</PropertyGroup>\n", 1);
  }
}

void cmVisualStudio10TargetGenerator::WriteMSToolConfigurationValues(
  std::string const& config)
{
  cmGlobalVisualStudio10Generator* gg =
    static_cast<cmGlobalVisualStudio10Generator*>(this->GlobalGenerator);
  const char* mfcFlag =
    this->GeneratorTarget->Target->GetMakefile()->GetDefinition(
      "CMAKE_MFC_FLAG");
  std::string mfcFlagValue = mfcFlag ? mfcFlag : "0";

  std::string useOfMfcValue = "false";
  if (this->GeneratorTarget->GetType() <= cmState::OBJECT_LIBRARY) {
    if (mfcFlagValue == "1") {
      useOfMfcValue = "Static";
    } else if (mfcFlagValue == "2") {
      useOfMfcValue = "Dynamic";
    }
  }
  std::string mfcLine = "<UseOfMfc>";
  mfcLine += useOfMfcValue + "</UseOfMfc>\n";
  this->WriteString(mfcLine.c_str(), 2);

  if ((this->GeneratorTarget->GetType() <= cmState::OBJECT_LIBRARY &&
       this->ClOptions[config]->UsingUnicode()) ||
      this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_COMPONENT") ||
      this->GlobalGenerator->TargetsWindowsPhone() ||
      this->GlobalGenerator->TargetsWindowsStore() ||
      this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_EXTENSIONS")) {
    this->WriteString("<CharacterSet>Unicode</CharacterSet>\n", 2);
  } else if (this->GeneratorTarget->GetType() <= cmState::MODULE_LIBRARY &&
             this->ClOptions[config]->UsingSBCS()) {
    this->WriteString("<CharacterSet>NotSet</CharacterSet>\n", 2);
  } else {
    this->WriteString("<CharacterSet>MultiByte</CharacterSet>\n", 2);
  }
  if (const char* toolset = gg->GetPlatformToolset()) {
    std::string pts = "<PlatformToolset>";
    pts += toolset;
    pts += "</PlatformToolset>\n";
    this->WriteString(pts.c_str(), 2);
  }
  if (this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_COMPONENT") ||
      this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_EXTENSIONS")) {
    this->WriteString("<WindowsAppContainer>true"
                      "</WindowsAppContainer>\n",
                      2);
  }
}

void cmVisualStudio10TargetGenerator::WriteNsightTegraConfigurationValues(
  std::string const&)
{
  cmGlobalVisualStudio10Generator* gg =
    static_cast<cmGlobalVisualStudio10Generator*>(this->GlobalGenerator);
  const char* toolset = gg->GetPlatformToolset();
  std::string ntv = "<NdkToolchainVersion>";
  ntv += toolset ? toolset : "Default";
  ntv += "</NdkToolchainVersion>\n";
  this->WriteString(ntv.c_str(), 2);
  if (const char* minApi =
        this->GeneratorTarget->GetProperty("ANDROID_API_MIN")) {
    this->WriteString("<AndroidMinAPI>", 2);
    (*this->BuildFileStream) << "android-" << cmVS10EscapeXML(minApi)
                             << "</AndroidMinAPI>\n";
  }
  if (const char* api = this->GeneratorTarget->GetProperty("ANDROID_API")) {
    this->WriteString("<AndroidTargetAPI>", 2);
    (*this->BuildFileStream) << "android-" << cmVS10EscapeXML(api)
                             << "</AndroidTargetAPI>\n";
  }

  if (const char* cpuArch =
        this->GeneratorTarget->GetProperty("ANDROID_ARCH")) {
    this->WriteString("<AndroidArch>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(cpuArch) << "</AndroidArch>\n";
  }

  if (const char* stlType =
        this->GeneratorTarget->GetProperty("ANDROID_STL_TYPE")) {
    this->WriteString("<AndroidStlType>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(stlType)
                             << "</AndroidStlType>\n";
  }
}

void cmVisualStudio10TargetGenerator::WriteCustomCommands()
{
  this->SourcesVisited.clear();
  std::vector<cmSourceFile const*> customCommands;
  this->GeneratorTarget->GetCustomCommands(customCommands, "");
  for (std::vector<cmSourceFile const*>::const_iterator si =
         customCommands.begin();
       si != customCommands.end(); ++si) {
    this->WriteCustomCommand(*si);
  }
}

void cmVisualStudio10TargetGenerator::WriteCustomCommand(
  cmSourceFile const* sf)
{
  if (this->SourcesVisited.insert(sf).second) {
    if (std::vector<cmSourceFile*> const* depends =
          this->GeneratorTarget->GetSourceDepends(sf)) {
      for (std::vector<cmSourceFile*>::const_iterator di = depends->begin();
           di != depends->end(); ++di) {
        this->WriteCustomCommand(*di);
      }
    }
    if (cmCustomCommand const* command = sf->GetCustomCommand()) {
      this->WriteString("<ItemGroup>\n", 1);
      this->WriteCustomRule(sf, *command);
      this->WriteString("</ItemGroup>\n", 1);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteCustomRule(
  cmSourceFile const* source, cmCustomCommand const& command)
{
  std::string sourcePath = source->GetFullPath();
  // VS 10 will always rebuild a custom command attached to a .rule
  // file that doesn't exist so create the file explicitly.
  if (source->GetPropertyAsBool("__CMAKE_RULE")) {
    if (!cmSystemTools::FileExists(sourcePath.c_str())) {
      // Make sure the path exists for the file
      std::string path = cmSystemTools::GetFilenamePath(sourcePath);
      cmSystemTools::MakeDirectory(path.c_str());
      cmsys::ofstream fout(sourcePath.c_str());
      if (fout) {
        fout << "# generated from CMake\n";
        fout.flush();
        fout.close();
        // Force given file to have a very old timestamp, thus
        // preventing dependent rebuilds.
        this->ForceOld(sourcePath);
      } else {
        std::string error = "Could not create file: [";
        error += sourcePath;
        error += "]  ";
        cmSystemTools::Error(error.c_str(),
                             cmSystemTools::GetLastSystemError().c_str());
      }
    }
  }
  cmLocalVisualStudio7Generator* lg = this->LocalGenerator;

  this->WriteSource("CustomBuild", source, ">\n");

  for (std::vector<std::string>::const_iterator i =
         this->Configurations.begin();
       i != this->Configurations.end(); ++i) {
    cmCustomCommandGenerator ccg(command, *i, this->LocalGenerator);
    std::string comment = lg->ConstructComment(ccg);
    comment = cmVS10EscapeComment(comment);
    std::string script = cmVS10EscapeXML(lg->ConstructScript(ccg));
    this->WritePlatformConfigTag("Message", i->c_str(), 3);
    (*this->BuildFileStream) << cmVS10EscapeXML(comment) << "</Message>\n";
    this->WritePlatformConfigTag("Command", i->c_str(), 3);
    (*this->BuildFileStream) << script << "</Command>\n";
    this->WritePlatformConfigTag("AdditionalInputs", i->c_str(), 3);

    (*this->BuildFileStream) << cmVS10EscapeXML(source->GetFullPath());
    for (std::vector<std::string>::const_iterator d = ccg.GetDepends().begin();
         d != ccg.GetDepends().end(); ++d) {
      std::string dep;
      if (this->LocalGenerator->GetRealDependency(d->c_str(), i->c_str(),
                                                  dep)) {
        this->ConvertToWindowsSlash(dep);
        (*this->BuildFileStream) << ";" << cmVS10EscapeXML(dep);
      }
    }
    (*this->BuildFileStream) << ";%(AdditionalInputs)</AdditionalInputs>\n";
    this->WritePlatformConfigTag("Outputs", i->c_str(), 3);
    const char* sep = "";
    for (std::vector<std::string>::const_iterator o = ccg.GetOutputs().begin();
         o != ccg.GetOutputs().end(); ++o) {
      std::string out = *o;
      this->ConvertToWindowsSlash(out);
      (*this->BuildFileStream) << sep << cmVS10EscapeXML(out);
      sep = ";";
    }
    (*this->BuildFileStream) << "</Outputs>\n";
    if (this->LocalGenerator->GetVersion() >
        cmGlobalVisualStudioGenerator::VS10) {
      // VS >= 11 let us turn off linking of custom command outputs.
      this->WritePlatformConfigTag("LinkObjects", i->c_str(), 3);
      (*this->BuildFileStream) << "false</LinkObjects>\n";
    }
  }
  this->WriteString("</CustomBuild>\n", 2);
}

std::string cmVisualStudio10TargetGenerator::ConvertPath(
  std::string const& path, bool forceRelative)
{
  return forceRelative
    ? cmSystemTools::RelativePath(
        this->LocalGenerator->GetCurrentBinaryDirectory(), path.c_str())
    : path.c_str();
}

void cmVisualStudio10TargetGenerator::ConvertToWindowsSlash(std::string& s)
{
  // first convert all of the slashes
  std::string::size_type pos = 0;
  while ((pos = s.find('/', pos)) != std::string::npos) {
    s[pos] = '\\';
    pos++;
  }
}
void cmVisualStudio10TargetGenerator::WriteGroups()
{
  // collect up group information
  std::vector<cmSourceGroup> sourceGroups = this->Makefile->GetSourceGroups();
  std::vector<cmSourceFile*> classes;
  if (!this->GeneratorTarget->GetConfigCommonSourceFiles(classes)) {
    return;
  }

  std::set<cmSourceGroup*> groupsUsed;
  for (std::vector<cmSourceFile*>::const_iterator s = classes.begin();
       s != classes.end(); s++) {
    cmSourceFile* sf = *s;
    std::string const& source = sf->GetFullPath();
    cmSourceGroup* sourceGroup =
      this->Makefile->FindSourceGroup(source.c_str(), sourceGroups);
    groupsUsed.insert(sourceGroup);
  }

  this->AddMissingSourceGroups(groupsUsed, sourceGroups);

  // Write out group file
  std::string path = this->LocalGenerator->GetCurrentBinaryDirectory();
  path += "/";
  path += this->Name;
  path += ".vcxproj.filters";
  cmGeneratedFileStream fout(path.c_str());
  fout.SetCopyIfDifferent(true);
  char magic[] = { char(0xEF), char(0xBB), char(0xBF) };
  fout.write(magic, 3);
  cmGeneratedFileStream* save = this->BuildFileStream;
  this->BuildFileStream = &fout;

  // get the tools version to use
  const std::string toolsVer(this->GlobalGenerator->GetToolsVersion());
  std::string project_defaults = "<?xml version=\"1.0\" encoding=\"" +
    this->GlobalGenerator->Encoding() + "\"?>\n";
  project_defaults.append("<Project ToolsVersion=\"");
  project_defaults.append(toolsVer + "\" ");
  project_defaults.append(
    "xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">\n");
  this->WriteString(project_defaults.c_str(), 0);

  for (ToolSourceMap::const_iterator ti = this->Tools.begin();
       ti != this->Tools.end(); ++ti) {
    this->WriteGroupSources(ti->first.c_str(), ti->second, sourceGroups);
  }

  // Added files are images and the manifest.
  if (!this->AddedFiles.empty()) {
    this->WriteString("<ItemGroup>\n", 1);
    for (std::vector<std::string>::const_iterator oi =
           this->AddedFiles.begin();
         oi != this->AddedFiles.end(); ++oi) {
      std::string fileName =
        cmSystemTools::LowerCase(cmSystemTools::GetFilenameName(*oi));
      if (fileName == "wmappmanifest.xml") {
        this->WriteString("<XML Include=\"", 2);
        (*this->BuildFileStream) << *oi << "\">\n";
        this->WriteString("<Filter>Resource Files</Filter>\n", 3);
        this->WriteString("</XML>\n", 2);
      } else if (cmSystemTools::GetFilenameExtension(fileName) ==
                 ".appxmanifest") {
        this->WriteString("<AppxManifest Include=\"", 2);
        (*this->BuildFileStream) << *oi << "\">\n";
        this->WriteString("<Filter>Resource Files</Filter>\n", 3);
        this->WriteString("</AppxManifest>\n", 2);
      } else if (cmSystemTools::GetFilenameExtension(fileName) == ".pfx") {
        this->WriteString("<None Include=\"", 2);
        (*this->BuildFileStream) << *oi << "\">\n";
        this->WriteString("<Filter>Resource Files</Filter>\n", 3);
        this->WriteString("</None>\n", 2);
      } else {
        this->WriteString("<Image Include=\"", 2);
        (*this->BuildFileStream) << *oi << "\">\n";
        this->WriteString("<Filter>Resource Files</Filter>\n", 3);
        this->WriteString("</Image>\n", 2);
      }
    }
    this->WriteString("</ItemGroup>\n", 1);
  }

  std::vector<cmSourceFile const*> resxObjs;
  this->GeneratorTarget->GetResxSources(resxObjs, "");
  if (!resxObjs.empty()) {
    this->WriteString("<ItemGroup>\n", 1);
    for (std::vector<cmSourceFile const*>::const_iterator oi =
           resxObjs.begin();
         oi != resxObjs.end(); ++oi) {
      std::string obj = (*oi)->GetFullPath();
      this->WriteString("<EmbeddedResource Include=\"", 2);
      this->ConvertToWindowsSlash(obj);
      (*this->BuildFileStream) << cmVS10EscapeXML(obj) << "\">\n";
      this->WriteString("<Filter>Resource Files</Filter>\n", 3);
      this->WriteString("</EmbeddedResource>\n", 2);
    }
    this->WriteString("</ItemGroup>\n", 1);
  }

  // Add object library contents as external objects.
  std::vector<std::string> objs;
  this->GeneratorTarget->UseObjectLibraries(objs, "");
  if (!objs.empty()) {
    this->WriteString("<ItemGroup>\n", 1);
    for (std::vector<std::string>::const_iterator oi = objs.begin();
         oi != objs.end(); ++oi) {
      std::string obj = *oi;
      this->WriteString("<Object Include=\"", 2);
      this->ConvertToWindowsSlash(obj);
      (*this->BuildFileStream) << cmVS10EscapeXML(obj) << "\">\n";
      this->WriteString("<Filter>Object Libraries</Filter>\n", 3);
      this->WriteString("</Object>\n", 2);
    }
    this->WriteString("</ItemGroup>\n", 1);
  }

  this->WriteString("<ItemGroup>\n", 1);
  for (std::set<cmSourceGroup*>::iterator g = groupsUsed.begin();
       g != groupsUsed.end(); ++g) {
    cmSourceGroup* sg = *g;
    const char* name = sg->GetFullName();
    if (strlen(name) != 0) {
      this->WriteString("<Filter Include=\"", 2);
      (*this->BuildFileStream) << name << "\">\n";
      std::string guidName = "SG_Filter_";
      guidName += name;
      this->WriteString("<UniqueIdentifier>", 3);
      std::string guid = this->GlobalGenerator->GetGUID(guidName.c_str());
      (*this->BuildFileStream) << "{" << guid << "}"
                               << "</UniqueIdentifier>\n";
      this->WriteString("</Filter>\n", 2);
    }
  }
  if (!objs.empty()) {
    this->WriteString("<Filter Include=\"Object Libraries\">\n", 2);
    std::string guidName = "SG_Filter_Object Libraries";
    this->WriteString("<UniqueIdentifier>", 3);
    std::string guid = this->GlobalGenerator->GetGUID(guidName.c_str());
    (*this->BuildFileStream) << "{" << guid << "}"
                             << "</UniqueIdentifier>\n";
    this->WriteString("</Filter>\n", 2);
  }

  if (!resxObjs.empty() || !this->AddedFiles.empty()) {
    this->WriteString("<Filter Include=\"Resource Files\">\n", 2);
    std::string guidName = "SG_Filter_Resource Files";
    this->WriteString("<UniqueIdentifier>", 3);
    std::string guid = this->GlobalGenerator->GetGUID(guidName.c_str());
    (*this->BuildFileStream) << "{" << guid << "}"
                             << "</UniqueIdentifier>\n";
    this->WriteString("<Extensions>rc;ico;cur;bmp;dlg;rc2;rct;bin;rgs;", 3);
    (*this->BuildFileStream) << "gif;jpg;jpeg;jpe;resx;tiff;tif;png;wav;";
    (*this->BuildFileStream) << "mfcribbon-ms</Extensions>\n";
    this->WriteString("</Filter>\n", 2);
  }

  this->WriteString("</ItemGroup>\n", 1);
  this->WriteString("</Project>\n", 0);
  // restore stream pointer
  this->BuildFileStream = save;

  if (fout.Close()) {
    this->GlobalGenerator->FileReplacedDuringGenerate(path);
  }
}

// Add to groupsUsed empty source groups that have non-empty children.
void cmVisualStudio10TargetGenerator::AddMissingSourceGroups(
  std::set<cmSourceGroup*>& groupsUsed,
  const std::vector<cmSourceGroup>& allGroups)
{
  for (std::vector<cmSourceGroup>::const_iterator current = allGroups.begin();
       current != allGroups.end(); ++current) {
    std::vector<cmSourceGroup> const& children = current->GetGroupChildren();
    if (children.empty()) {
      continue; // the group is really empty
    }

    this->AddMissingSourceGroups(groupsUsed, children);

    cmSourceGroup* current_ptr = const_cast<cmSourceGroup*>(&(*current));
    if (groupsUsed.find(current_ptr) != groupsUsed.end()) {
      continue; // group has already been added to set
    }

    // check if it least one of the group's descendants is not empty
    // (at least one child must already have been added)
    std::vector<cmSourceGroup>::const_iterator child_it = children.begin();
    while (child_it != children.end()) {
      cmSourceGroup* child_ptr = const_cast<cmSourceGroup*>(&(*child_it));
      if (groupsUsed.find(child_ptr) != groupsUsed.end()) {
        break; // found a child that was already added => add current group too
      }
      child_it++;
    }

    if (child_it == children.end()) {
      continue; // no descendants have source files => ignore this group
    }

    groupsUsed.insert(current_ptr);
  }
}

void cmVisualStudio10TargetGenerator::WriteGroupSources(
  const char* name, ToolSources const& sources,
  std::vector<cmSourceGroup>& sourceGroups)
{
  this->WriteString("<ItemGroup>\n", 1);
  for (ToolSources::const_iterator s = sources.begin(); s != sources.end();
       ++s) {
    cmSourceFile const* sf = s->SourceFile;
    std::string const& source = sf->GetFullPath();
    cmSourceGroup* sourceGroup =
      this->Makefile->FindSourceGroup(source.c_str(), sourceGroups);
    const char* filter = sourceGroup->GetFullName();
    this->WriteString("<", 2);
    std::string path = this->ConvertPath(source, s->RelativePath);
    this->ConvertToWindowsSlash(path);
    (*this->BuildFileStream) << name << " Include=\"" << cmVS10EscapeXML(path);
    if (strlen(filter)) {
      (*this->BuildFileStream) << "\">\n";
      this->WriteString("<Filter>", 3);
      (*this->BuildFileStream) << filter << "</Filter>\n";
      this->WriteString("</", 2);
      (*this->BuildFileStream) << name << ">\n";
    } else {
      (*this->BuildFileStream) << "\" />\n";
    }
  }
  this->WriteString("</ItemGroup>\n", 1);
}

void cmVisualStudio10TargetGenerator::WriteHeaderSource(cmSourceFile const* sf)
{
  std::string const& fileName = sf->GetFullPath();
  if (this->IsResxHeader(fileName)) {
    this->WriteSource("ClInclude", sf, ">\n");
    this->WriteString("<FileType>CppForm</FileType>\n", 3);
    this->WriteString("</ClInclude>\n", 2);
  } else if (this->IsXamlHeader(fileName)) {
    this->WriteSource("ClInclude", sf, ">\n");
    this->WriteString("<DependentUpon>", 3);
    std::string xamlFileName = fileName.substr(0, fileName.find_last_of("."));
    (*this->BuildFileStream) << xamlFileName << "</DependentUpon>\n";
    this->WriteString("</ClInclude>\n", 2);
  } else {
    this->WriteSource("ClInclude", sf);
  }
}

void cmVisualStudio10TargetGenerator::WriteExtraSource(cmSourceFile const* sf)
{
  bool toolHasSettings = false;
  std::string tool = "None";
  std::string shaderType;
  std::string shaderEntryPoint;
  std::string shaderModel;
  std::string shaderAdditionalFlags;
  std::string ext = cmSystemTools::LowerCase(sf->GetExtension());
  if (ext == "hlsl") {
    tool = "FXCompile";
    // Figure out the type of shader compiler to use.
    if (const char* st = sf->GetProperty("VS_SHADER_TYPE")) {
      shaderType = st;
      toolHasSettings = true;
    }
    // Figure out which entry point to use if any
    if (const char* se = sf->GetProperty("VS_SHADER_ENTRYPOINT")) {
      shaderEntryPoint = se;
      toolHasSettings = true;
    }
    // Figure out which shader model to use if any
    if (const char* sm = sf->GetProperty("VS_SHADER_MODEL")) {
      shaderModel = sm;
      toolHasSettings = true;
    }
    // Figure out if there's any additional flags to use
    if (const char* saf = sf->GetProperty("VS_SHADER_FLAGS")) {
      shaderAdditionalFlags = saf;
      toolHasSettings = true;
    }
  } else if (ext == "jpg" || ext == "png") {
    tool = "Image";
  } else if (ext == "resw") {
    tool = "PRIResource";
  } else if (ext == "xml") {
    tool = "XML";
  }

  if (this->NsightTegra) {
    // Nsight Tegra needs specific file types to check up-to-dateness.
    std::string name = cmSystemTools::LowerCase(sf->GetLocation().GetName());
    if (name == "androidmanifest.xml" || name == "build.xml" ||
        name == "proguard.cfg" || name == "proguard-project.txt" ||
        ext == "properties") {
      tool = "AndroidBuild";
    } else if (ext == "java") {
      tool = "JCompile";
    } else if (ext == "asm" || ext == "s") {
      tool = "ClCompile";
    }
  }

  std::string deployContent;
  std::string deployLocation;
  if (this->GlobalGenerator->TargetsWindowsPhone() ||
      this->GlobalGenerator->TargetsWindowsStore()) {
    const char* content = sf->GetProperty("VS_DEPLOYMENT_CONTENT");
    if (content && *content) {
      toolHasSettings = true;
      deployContent = content;

      const char* location = sf->GetProperty("VS_DEPLOYMENT_LOCATION");
      if (location && *location) {
        deployLocation = location;
      }
    }
  }

  if (toolHasSettings) {
    this->WriteSource(tool, sf, ">\n");

    if (!deployContent.empty()) {
      cmGeneratorExpression ge;
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
        ge.Parse(deployContent);
      // Deployment location cannot be set on a configuration basis
      if (!deployLocation.empty()) {
        this->WriteString("<Link>", 3);
        (*this->BuildFileStream) << deployLocation
                                 << "\\%(FileName)%(Extension)";
        this->WriteString("</Link>\n", 0);
      }
      for (size_t i = 0; i != this->Configurations.size(); ++i) {
        if (0 == strcmp(cge->Evaluate(this->LocalGenerator,
                                      this->Configurations[i]),
                        "1")) {
          this->WriteString("<DeploymentContent Condition=\""
                            "'$(Configuration)|$(Platform)'=='",
                            3);
          (*this->BuildFileStream) << this->Configurations[i] << "|"
                                   << this->Platform << "'\">true";
          this->WriteString("</DeploymentContent>\n", 0);
        } else {
          this->WriteString("<ExcludedFromBuild Condition=\""
                            "'$(Configuration)|$(Platform)'=='",
                            3);
          (*this->BuildFileStream) << this->Configurations[i] << "|"
                                   << this->Platform << "'\">true";
          this->WriteString("</ExcludedFromBuild>\n", 0);
        }
      }
    }
    if (!shaderType.empty()) {
      this->WriteString("<ShaderType>", 3);
      (*this->BuildFileStream) << cmVS10EscapeXML(shaderType)
                               << "</ShaderType>\n";
    }
    if (!shaderEntryPoint.empty()) {
      this->WriteString("<EntryPointName>", 3);
      (*this->BuildFileStream) << cmVS10EscapeXML(shaderEntryPoint)
                               << "</EntryPointName>\n";
    }
    if (!shaderModel.empty()) {
      this->WriteString("<ShaderModel>", 3);
      (*this->BuildFileStream) << cmVS10EscapeXML(shaderModel)
                               << "</ShaderModel>\n";
    }
    if (!shaderAdditionalFlags.empty()) {
      this->WriteString("<AdditionalOptions>", 3);
      (*this->BuildFileStream) << cmVS10EscapeXML(shaderAdditionalFlags)
                               << "</AdditionalOptions>\n";
    }
    this->WriteString("</", 2);
    (*this->BuildFileStream) << tool << ">\n";
  } else {
    this->WriteSource(tool, sf);
  }
}

void cmVisualStudio10TargetGenerator::WriteSource(std::string const& tool,
                                                  cmSourceFile const* sf,
                                                  const char* end)
{
  // Visual Studio tools append relative paths to the current dir, as in:
  //
  //  c:\path\to\current\dir\..\..\..\relative\path\to\source.c
  //
  // and fail if this exceeds the maximum allowed path length.  Our path
  // conversion uses full paths when possible to allow deeper trees.
  bool forceRelative = false;
  std::string sourceFile = this->ConvertPath(sf->GetFullPath(), false);
  if (this->LocalGenerator->GetVersion() ==
        cmGlobalVisualStudioGenerator::VS10 &&
      cmSystemTools::FileIsFullPath(sourceFile.c_str())) {
    // Normal path conversion resulted in a full path.  VS 10 (but not 11)
    // refuses to show the property page in the IDE for a source file with a
    // full path (not starting in a '.' or '/' AFAICT).  CMake <= 2.8.4 used a
    // relative path but to allow deeper build trees CMake 2.8.[5678] used a
    // full path except for custom commands.  Custom commands do not work
    // without a relative path, but they do not seem to be involved in tools
    // with the above behavior.  For other sources we now use a relative path
    // when the combined path will not be too long so property pages appear.
    std::string sourceRel = this->ConvertPath(sf->GetFullPath(), true);
    size_t const maxLen = 250;
    if (sf->GetCustomCommand() ||
        ((strlen(this->LocalGenerator->GetCurrentBinaryDirectory()) + 1 +
          sourceRel.length()) <= maxLen)) {
      forceRelative = true;
      sourceFile = sourceRel;
    } else {
      this->GlobalGenerator->PathTooLong(this->GeneratorTarget, sf, sourceRel);
    }
  }
  this->ConvertToWindowsSlash(sourceFile);
  this->WriteString("<", 2);
  (*this->BuildFileStream) << tool << " Include=\""
                           << cmVS10EscapeXML(sourceFile) << "\""
                           << (end ? end : " />\n");

  ToolSource toolSource = { sf, forceRelative };
  this->Tools[tool].push_back(toolSource);
}

void cmVisualStudio10TargetGenerator::WriteSources(
  std::string const& tool, std::vector<cmSourceFile const*> const& sources)
{
  for (std::vector<cmSourceFile const*>::const_iterator si = sources.begin();
       si != sources.end(); ++si) {
    this->WriteSource(tool, *si);
  }
}

void cmVisualStudio10TargetGenerator::WriteAllSources()
{
  if (this->GeneratorTarget->GetType() > cmState::UTILITY) {
    return;
  }
  this->WriteString("<ItemGroup>\n", 1);

  std::vector<cmSourceFile const*> headerSources;
  this->GeneratorTarget->GetHeaderSources(headerSources, "");
  for (std::vector<cmSourceFile const*>::const_iterator si =
         headerSources.begin();
       si != headerSources.end(); ++si) {
    this->WriteHeaderSource(*si);
  }
  std::vector<cmSourceFile const*> idlSources;
  this->GeneratorTarget->GetIDLSources(idlSources, "");
  this->WriteSources("Midl", idlSources);

  std::vector<cmSourceFile const*> objectSources;
  this->GeneratorTarget->GetObjectSources(objectSources, "");
  for (std::vector<cmSourceFile const*>::const_iterator si =
         objectSources.begin();
       si != objectSources.end(); ++si) {
    const std::string& lang = (*si)->GetLanguage();
    std::string tool;
    if (lang == "C" || lang == "CXX") {
      tool = "ClCompile";
    } else if (lang == "ASM_MASM" && this->GlobalGenerator->IsMasmEnabled()) {
      tool = "MASM";
    } else if (lang == "RC") {
      tool = "ResourceCompile";
    }

    if (!tool.empty()) {
      this->WriteSource(tool, *si, " ");
      if (this->OutputSourceSpecificFlags(*si)) {
        this->WriteString("</", 2);
        (*this->BuildFileStream) << tool << ">\n";
      } else {
        (*this->BuildFileStream) << " />\n";
      }
    } else {
      this->WriteSource("None", *si);
    }
  }

  std::vector<cmSourceFile const*> manifestSources;
  this->GeneratorTarget->GetAppManifest(manifestSources, "");
  this->WriteSources("AppxManifest", manifestSources);

  std::vector<cmSourceFile const*> certificateSources;
  this->GeneratorTarget->GetCertificates(certificateSources, "");
  this->WriteSources("None", certificateSources);

  std::vector<cmSourceFile const*> externalObjects;
  this->GeneratorTarget->GetExternalObjects(externalObjects, "");
  for (std::vector<cmSourceFile const*>::iterator si = externalObjects.begin();
       si != externalObjects.end();) {
    if (!(*si)->GetObjectLibrary().empty()) {
      si = externalObjects.erase(si);
    } else {
      ++si;
    }
  }
  if (this->LocalGenerator->GetVersion() >
      cmGlobalVisualStudioGenerator::VS10) {
    // For VS >= 11 we use LinkObjects to avoid linking custom command
    // outputs.  Use Object for all external objects, generated or not.
    this->WriteSources("Object", externalObjects);
  } else {
    // If an object file is generated in this target, then vs10 will use
    // it in the build, and we have to list it as None instead of Object.
    for (std::vector<cmSourceFile const*>::const_iterator si =
           externalObjects.begin();
         si != externalObjects.end(); ++si) {
      std::vector<cmSourceFile*> const* d =
        this->GeneratorTarget->GetSourceDepends(*si);
      this->WriteSource((d && !d->empty()) ? "None" : "Object", *si);
    }
  }

  std::vector<cmSourceFile const*> extraSources;
  this->GeneratorTarget->GetExtraSources(extraSources, "");
  for (std::vector<cmSourceFile const*>::const_iterator si =
         extraSources.begin();
       si != extraSources.end(); ++si) {
    this->WriteExtraSource(*si);
  }

  // Add object library contents as external objects.
  std::vector<std::string> objs;
  this->GeneratorTarget->UseObjectLibraries(objs, "");
  for (std::vector<std::string>::const_iterator oi = objs.begin();
       oi != objs.end(); ++oi) {
    std::string obj = *oi;
    this->WriteString("<Object Include=\"", 2);
    this->ConvertToWindowsSlash(obj);
    (*this->BuildFileStream) << cmVS10EscapeXML(obj) << "\" />\n";
  }

  if (cmSourceFile const* defsrc =
        this->GeneratorTarget->GetModuleDefinitionFile("")) {
    this->WriteSource("None", defsrc);
  }

  if (this->IsMissingFiles) {
    this->WriteMissingFiles();
  }

  this->WriteString("</ItemGroup>\n", 1);
}

bool cmVisualStudio10TargetGenerator::OutputSourceSpecificFlags(
  cmSourceFile const* source)
{
  cmSourceFile const& sf = *source;

  std::string objectName;
  if (this->GeneratorTarget->HasExplicitObjectName(&sf)) {
    objectName = this->GeneratorTarget->GetObjectName(&sf);
  }
  std::string flags;
  std::string defines;
  if (const char* cflags = sf.GetProperty("COMPILE_FLAGS")) {
    flags += cflags;
  }
  if (const char* cdefs = sf.GetProperty("COMPILE_DEFINITIONS")) {
    defines += cdefs;
  }
  std::string lang =
    this->GlobalGenerator->GetLanguageFromExtension(sf.GetExtension().c_str());
  std::string sourceLang = this->LocalGenerator->GetSourceFileLanguage(sf);
  const std::string& linkLanguage = this->GeneratorTarget->GetLinkerLanguage();
  bool needForceLang = false;
  // source file does not match its extension language
  if (lang != sourceLang) {
    needForceLang = true;
    lang = sourceLang;
  }
  // if the source file does not match the linker language
  // then force c or c++
  const char* compileAs = 0;
  if (needForceLang || (linkLanguage != lang)) {
    if (lang == "CXX") {
      // force a C++ file type
      compileAs = "CompileAsCpp";
    } else if (lang == "C") {
      // force to c
      compileAs = "CompileAsC";
    }
  }
  bool noWinRT = this->TargetCompileAsWinRT && lang == "C";
  bool hasFlags = false;
  // for the first time we need a new line if there is something
  // produced here.
  const char* firstString = ">\n";
  if (!objectName.empty()) {
    (*this->BuildFileStream) << firstString;
    firstString = "";
    hasFlags = true;
    this->WriteString("<ObjectFileName>", 3);
    (*this->BuildFileStream) << "$(IntDir)/" << objectName
                             << "</ObjectFileName>\n";
  }
  for (std::vector<std::string>::const_iterator config =
         this->Configurations.begin();
       config != this->Configurations.end(); ++config) {
    std::string configUpper = cmSystemTools::UpperCase(*config);
    std::string configDefines = defines;
    std::string defPropName = "COMPILE_DEFINITIONS_";
    defPropName += configUpper;
    if (const char* ccdefs = sf.GetProperty(defPropName.c_str())) {
      if (!configDefines.empty()) {
        configDefines += ";";
      }
      configDefines += ccdefs;
    }
    // if we have flags or defines for this config then
    // use them
    if (!flags.empty() || !configDefines.empty() || compileAs || noWinRT) {
      (*this->BuildFileStream) << firstString;
      firstString = ""; // only do firstString once
      hasFlags = true;
      cmVisualStudioGeneratorOptions clOptions(
        this->LocalGenerator, cmVisualStudioGeneratorOptions::Compiler,
        this->GetClFlagTable(), 0, this);
      if (compileAs) {
        clOptions.AddFlag("CompileAs", compileAs);
      }
      if (noWinRT) {
        clOptions.AddFlag("CompileAsWinRT", "false");
      }
      clOptions.Parse(flags.c_str());
      if (clOptions.HasFlag("AdditionalIncludeDirectories")) {
        clOptions.AppendFlag("AdditionalIncludeDirectories",
                             "%(AdditionalIncludeDirectories)");
      }
      if (clOptions.HasFlag("DisableSpecificWarnings")) {
        clOptions.AppendFlag("DisableSpecificWarnings",
                             "%(DisableSpecificWarnings)");
      }
      clOptions.AddDefines(configDefines.c_str());
      clOptions.SetConfiguration((*config).c_str());
      clOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");
      clOptions.OutputFlagMap(*this->BuildFileStream, "      ");
      clOptions.OutputPreprocessorDefinitions(*this->BuildFileStream, "      ",
                                              "\n", lang);
    }
  }
  if (this->IsXamlSource(source->GetFullPath())) {
    (*this->BuildFileStream) << firstString;
    firstString = ""; // only do firstString once
    hasFlags = true;
    this->WriteString("<DependentUpon>", 3);
    const std::string& fileName = source->GetFullPath();
    std::string xamlFileName = fileName.substr(0, fileName.find_last_of("."));
    (*this->BuildFileStream) << xamlFileName << "</DependentUpon>\n";
  }

  return hasFlags;
}

void cmVisualStudio10TargetGenerator::WritePathAndIncrementalLinkOptions()
{
  cmState::TargetType ttype = this->GeneratorTarget->GetType();
  if (ttype > cmState::GLOBAL_TARGET) {
    return;
  }

  this->WriteString("<PropertyGroup>\n", 2);
  this->WriteString("<_ProjectFileVersion>10.0.20506.1"
                    "</_ProjectFileVersion>\n",
                    3);
  for (std::vector<std::string>::const_iterator config =
         this->Configurations.begin();
       config != this->Configurations.end(); ++config) {
    if (ttype >= cmState::UTILITY) {
      this->WritePlatformConfigTag("IntDir", config->c_str(), 3);
      *this->BuildFileStream
        << "$(Platform)\\$(Configuration)\\$(ProjectName)\\"
        << "</IntDir>\n";
    } else {
      std::string intermediateDir =
        this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
      intermediateDir += "/";
      intermediateDir += *config;
      intermediateDir += "/";
      std::string outDir;
      std::string targetNameFull;
      if (ttype == cmState::OBJECT_LIBRARY) {
        outDir = intermediateDir;
        targetNameFull = this->GeneratorTarget->GetName();
        targetNameFull += ".lib";
      } else {
        outDir = this->GeneratorTarget->GetDirectory(config->c_str()) + "/";
        targetNameFull = this->GeneratorTarget->GetFullName(config->c_str());
      }
      this->ConvertToWindowsSlash(intermediateDir);
      this->ConvertToWindowsSlash(outDir);

      this->WritePlatformConfigTag("OutDir", config->c_str(), 3);
      *this->BuildFileStream << cmVS10EscapeXML(outDir) << "</OutDir>\n";

      this->WritePlatformConfigTag("IntDir", config->c_str(), 3);
      *this->BuildFileStream << cmVS10EscapeXML(intermediateDir)
                             << "</IntDir>\n";

      std::string name =
        cmSystemTools::GetFilenameWithoutLastExtension(targetNameFull);
      this->WritePlatformConfigTag("TargetName", config->c_str(), 3);
      *this->BuildFileStream << cmVS10EscapeXML(name) << "</TargetName>\n";

      std::string ext =
        cmSystemTools::GetFilenameLastExtension(targetNameFull);
      if (ext.empty()) {
        // An empty TargetExt causes a default extension to be used.
        // A single "." appears to be treated as an empty extension.
        ext = ".";
      }
      this->WritePlatformConfigTag("TargetExt", config->c_str(), 3);
      *this->BuildFileStream << cmVS10EscapeXML(ext) << "</TargetExt>\n";

      this->OutputLinkIncremental(*config);
    }
  }
  this->WriteString("</PropertyGroup>\n", 2);
}

void cmVisualStudio10TargetGenerator::OutputLinkIncremental(
  std::string const& configName)
{
  if (!this->MSTools) {
    return;
  }
  // static libraries and things greater than modules do not need
  // to set this option
  if (this->GeneratorTarget->GetType() == cmState::STATIC_LIBRARY ||
      this->GeneratorTarget->GetType() > cmState::MODULE_LIBRARY) {
    return;
  }
  Options& linkOptions = *(this->LinkOptions[configName]);

  const char* incremental = linkOptions.GetFlag("LinkIncremental");
  this->WritePlatformConfigTag("LinkIncremental", configName.c_str(), 3);
  *this->BuildFileStream << (incremental ? incremental : "true")
                         << "</LinkIncremental>\n";
  linkOptions.RemoveFlag("LinkIncremental");

  const char* manifest = linkOptions.GetFlag("GenerateManifest");
  this->WritePlatformConfigTag("GenerateManifest", configName.c_str(), 3);
  *this->BuildFileStream << (manifest ? manifest : "true")
                         << "</GenerateManifest>\n";
  linkOptions.RemoveFlag("GenerateManifest");

  // Some link options belong here.  Use them now and remove them so that
  // WriteLinkOptions does not use them.
  const char* flags[] = { "LinkDelaySign", "LinkKeyFile", 0 };
  for (const char** f = flags; *f; ++f) {
    const char* flag = *f;
    if (const char* value = linkOptions.GetFlag(flag)) {
      this->WritePlatformConfigTag(flag, configName.c_str(), 3);
      *this->BuildFileStream << value << "</" << flag << ">\n";
      linkOptions.RemoveFlag(flag);
    }
  }
}

bool cmVisualStudio10TargetGenerator::ComputeClOptions()
{
  for (std::vector<std::string>::const_iterator i =
         this->Configurations.begin();
       i != this->Configurations.end(); ++i) {
    if (!this->ComputeClOptions(*i)) {
      return false;
    }
  }
  return true;
}

bool cmVisualStudio10TargetGenerator::ComputeClOptions(
  std::string const& configName)
{
  // much of this was copied from here:
  // copied from cmLocalVisualStudio7Generator.cxx 805
  // TODO: Integrate code below with cmLocalVisualStudio7Generator.

  cmsys::auto_ptr<Options> pOptions(new Options(
    this->LocalGenerator, Options::Compiler, this->GetClFlagTable()));
  Options& clOptions = *pOptions;

  std::string flags;
  const std::string& linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(configName.c_str());
  if (linkLanguage.empty()) {
    cmSystemTools::Error(
      "CMake can not determine linker language for target: ",
      this->Name.c_str());
    return false;
  }
  if (linkLanguage == "C" || linkLanguage == "CXX" ||
      linkLanguage == "Fortran") {
    std::string baseFlagVar = "CMAKE_";
    baseFlagVar += linkLanguage;
    baseFlagVar += "_FLAGS";
    flags =
      this->GeneratorTarget->Target->GetMakefile()->GetRequiredDefinition(
        baseFlagVar.c_str());
    std::string flagVar =
      baseFlagVar + std::string("_") + cmSystemTools::UpperCase(configName);
    flags += " ";
    flags +=
      this->GeneratorTarget->Target->GetMakefile()->GetRequiredDefinition(
        flagVar.c_str());
  }
  // set the correct language
  if (linkLanguage == "C") {
    clOptions.AddFlag("CompileAs", "CompileAsC");
  }
  if (linkLanguage == "CXX") {
    clOptions.AddFlag("CompileAs", "CompileAsCpp");
  }
  this->LocalGenerator->AddCompileOptions(flags, this->GeneratorTarget,
                                          linkLanguage, configName.c_str());

  // Get preprocessor definitions for this directory.
  std::string defineFlags =
    this->GeneratorTarget->Target->GetMakefile()->GetDefineFlags();
  if (this->MSTools) {
    clOptions.FixExceptionHandlingDefault();
    clOptions.AddFlag("PrecompiledHeader", "NotUsing");
    std::string asmLocation = configName + "/";
    clOptions.AddFlag("AssemblerListingLocation", asmLocation.c_str());
  }
  clOptions.Parse(flags.c_str());
  clOptions.Parse(defineFlags.c_str());
  std::vector<std::string> targetDefines;
  this->GeneratorTarget->GetCompileDefinitions(targetDefines,
                                               configName.c_str(), "CXX");
  clOptions.AddDefines(targetDefines);
  if (this->MSTools) {
    clOptions.SetVerboseMakefile(
      this->Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"));
  }

  // Add a definition for the configuration name.
  std::string configDefine = "CMAKE_INTDIR=\"";
  configDefine += configName;
  configDefine += "\"";
  clOptions.AddDefine(configDefine);
  if (const char* exportMacro = this->GeneratorTarget->GetExportMacro()) {
    clOptions.AddDefine(exportMacro);
  }

  if (this->MSTools) {
    // If we have the VS_WINRT_COMPONENT set then force Compile as WinRT.
    if (this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_COMPONENT")) {
      clOptions.AddFlag("CompileAsWinRT", "true");
      // For WinRT components, add the _WINRT_DLL define to produce a lib
      if (this->GeneratorTarget->GetType() == cmState::SHARED_LIBRARY ||
          this->GeneratorTarget->GetType() == cmState::MODULE_LIBRARY) {
        clOptions.AddDefine("_WINRT_DLL");
      }
    } else if (this->GlobalGenerator->TargetsWindowsStore() ||
               this->GlobalGenerator->TargetsWindowsPhone()) {
      if (!clOptions.IsWinRt()) {
        clOptions.AddFlag("CompileAsWinRT", "false");
      }
    }
    if (const char* winRT = clOptions.GetFlag("CompileAsWinRT")) {
      if (cmSystemTools::IsOn(winRT)) {
        this->TargetCompileAsWinRT = true;
      }
    }
  }

  this->ClOptions[configName] = pOptions.release();
  return true;
}

void cmVisualStudio10TargetGenerator::WriteClOptions(
  std::string const& configName, std::vector<std::string> const& includes)
{
  Options& clOptions = *(this->ClOptions[configName]);
  this->WriteString("<ClCompile>\n", 2);
  clOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");
  clOptions.AppendFlag("AdditionalIncludeDirectories", includes);
  clOptions.AppendFlag("AdditionalIncludeDirectories",
                       "%(AdditionalIncludeDirectories)");
  clOptions.OutputFlagMap(*this->BuildFileStream, "      ");
  clOptions.OutputPreprocessorDefinitions(*this->BuildFileStream, "      ",
                                          "\n", "CXX");

  if (this->NsightTegra) {
    if (const char* processMax =
          this->GeneratorTarget->GetProperty("ANDROID_PROCESS_MAX")) {
      this->WriteString("<ProcessMax>", 3);
      *this->BuildFileStream << cmVS10EscapeXML(processMax)
                             << "</ProcessMax>\n";
    }
  }

  if (this->MSTools) {
    cmsys::RegularExpression clangToolset("v[0-9]+_clang_.*");
    const char* toolset = this->GlobalGenerator->GetPlatformToolset();
    if (toolset && clangToolset.find(toolset)) {
      this->WriteString("<ObjectFileName>"
                        "$(IntDir)%(filename).obj"
                        "</ObjectFileName>\n",
                        3);
    } else {
      this->WriteString("<ObjectFileName>$(IntDir)</ObjectFileName>\n", 3);
    }

    // If not in debug mode, write the DebugInformationFormat field
    // without value so PDBs don't get generated uselessly.
    if (!clOptions.IsDebug()) {
      this->WriteString("<DebugInformationFormat>"
                        "</DebugInformationFormat>\n",
                        3);
    }

    // Specify the compiler program database file if configured.
    std::string pdb =
      this->GeneratorTarget->GetCompilePDBPath(configName.c_str());
    if (!pdb.empty()) {
      this->ConvertToWindowsSlash(pdb);
      this->WriteString("<ProgramDataBaseFileName>", 3);
      *this->BuildFileStream << cmVS10EscapeXML(pdb)
                             << "</ProgramDataBaseFileName>\n";
    }
  }

  this->WriteString("</ClCompile>\n", 2);
}

bool cmVisualStudio10TargetGenerator::ComputeRcOptions()
{
  for (std::vector<std::string>::const_iterator i =
         this->Configurations.begin();
       i != this->Configurations.end(); ++i) {
    if (!this->ComputeRcOptions(*i)) {
      return false;
    }
  }
  return true;
}

bool cmVisualStudio10TargetGenerator::ComputeRcOptions(
  std::string const& configName)
{
  cmsys::auto_ptr<Options> pOptions(new Options(
    this->LocalGenerator, Options::ResourceCompiler, this->GetRcFlagTable()));
  Options& rcOptions = *pOptions;

  std::string CONFIG = cmSystemTools::UpperCase(configName);
  std::string rcConfigFlagsVar = std::string("CMAKE_RC_FLAGS_") + CONFIG;
  std::string flags =
    std::string(this->Makefile->GetSafeDefinition("CMAKE_RC_FLAGS")) +
    std::string(" ") +
    std::string(this->Makefile->GetSafeDefinition(rcConfigFlagsVar));

  rcOptions.Parse(flags.c_str());
  this->RcOptions[configName] = pOptions.release();
  return true;
}

void cmVisualStudio10TargetGenerator::WriteRCOptions(
  std::string const& configName, std::vector<std::string> const& includes)
{
  if (!this->MSTools) {
    return;
  }
  this->WriteString("<ResourceCompile>\n", 2);

  // Preprocessor definitions and includes are shared with clOptions.
  Options& clOptions = *(this->ClOptions[configName]);
  clOptions.OutputPreprocessorDefinitions(*this->BuildFileStream, "      ",
                                          "\n", "RC");

  Options& rcOptions = *(this->RcOptions[configName]);
  rcOptions.AppendFlag("AdditionalIncludeDirectories", includes);
  rcOptions.AppendFlag("AdditionalIncludeDirectories",
                       "%(AdditionalIncludeDirectories)");
  rcOptions.OutputFlagMap(*this->BuildFileStream, "      ");
  rcOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");

  this->WriteString("</ResourceCompile>\n", 2);
}

bool cmVisualStudio10TargetGenerator::ComputeMasmOptions()
{
  if (!this->GlobalGenerator->IsMasmEnabled()) {
    return true;
  }
  for (std::vector<std::string>::const_iterator i =
         this->Configurations.begin();
       i != this->Configurations.end(); ++i) {
    if (!this->ComputeMasmOptions(*i)) {
      return false;
    }
  }
  return true;
}

bool cmVisualStudio10TargetGenerator::ComputeMasmOptions(
  std::string const& configName)
{
  cmsys::auto_ptr<Options> pOptions(new Options(
    this->LocalGenerator, Options::MasmCompiler, this->GetMasmFlagTable()));
  Options& masmOptions = *pOptions;

  std::string CONFIG = cmSystemTools::UpperCase(configName);
  std::string configFlagsVar = std::string("CMAKE_ASM_MASM_FLAGS_") + CONFIG;
  std::string flags =
    std::string(this->Makefile->GetSafeDefinition("CMAKE_ASM_MASM_FLAGS")) +
    std::string(" ") +
    std::string(this->Makefile->GetSafeDefinition(configFlagsVar));

  masmOptions.Parse(flags.c_str());
  this->MasmOptions[configName] = pOptions.release();
  return true;
}

void cmVisualStudio10TargetGenerator::WriteMasmOptions(
  std::string const& configName, std::vector<std::string> const& includes)
{
  if (!this->MSTools || !this->GlobalGenerator->IsMasmEnabled()) {
    return;
  }
  this->WriteString("<MASM>\n", 2);

  // Preprocessor definitions and includes are shared with clOptions.
  Options& clOptions = *(this->ClOptions[configName]);
  clOptions.OutputPreprocessorDefinitions(*this->BuildFileStream, "      ",
                                          "\n", "ASM_MASM");

  Options& masmOptions = *(this->MasmOptions[configName]);
  masmOptions.AppendFlag("IncludePaths", includes);
  masmOptions.AppendFlag("IncludePaths", "%(IncludePaths)");
  masmOptions.OutputFlagMap(*this->BuildFileStream, "      ");
  masmOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");

  this->WriteString("</MASM>\n", 2);
}

void cmVisualStudio10TargetGenerator::WriteLibOptions(
  std::string const& config)
{
  if (this->GeneratorTarget->GetType() != cmState::STATIC_LIBRARY &&
      this->GeneratorTarget->GetType() != cmState::OBJECT_LIBRARY) {
    return;
  }
  std::string libflags;
  this->LocalGenerator->GetStaticLibraryFlags(
    libflags, cmSystemTools::UpperCase(config), this->GeneratorTarget);
  if (!libflags.empty()) {
    this->WriteString("<Lib>\n", 2);
    cmVisualStudioGeneratorOptions libOptions(
      this->LocalGenerator, cmVisualStudioGeneratorOptions::Linker,
      this->GetLibFlagTable(), 0, this);
    libOptions.Parse(libflags.c_str());
    libOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");
    libOptions.OutputFlagMap(*this->BuildFileStream, "      ");
    this->WriteString("</Lib>\n", 2);
  }

  // We cannot generate metadata for static libraries.  WindowsPhone
  // and WindowsStore tools look at GenerateWindowsMetadata in the
  // Link tool options even for static libraries.
  if (this->GlobalGenerator->TargetsWindowsPhone() ||
      this->GlobalGenerator->TargetsWindowsStore()) {
    this->WriteString("<Link>\n", 2);
    this->WriteString("<GenerateWindowsMetadata>false"
                      "</GenerateWindowsMetadata>\n",
                      3);
    this->WriteString("</Link>\n", 2);
  }
}

void cmVisualStudio10TargetGenerator::WriteManifestOptions(
  std::string const& config)
{
  if (this->GeneratorTarget->GetType() != cmState::EXECUTABLE &&
      this->GeneratorTarget->GetType() != cmState::SHARED_LIBRARY &&
      this->GeneratorTarget->GetType() != cmState::MODULE_LIBRARY) {
    return;
  }

  std::vector<cmSourceFile const*> manifest_srcs;
  this->GeneratorTarget->GetManifests(manifest_srcs, config);
  if (!manifest_srcs.empty()) {
    this->WriteString("<Manifest>\n", 2);
    this->WriteString("<AdditionalManifestFiles>", 3);
    for (std::vector<cmSourceFile const*>::const_iterator mi =
           manifest_srcs.begin();
         mi != manifest_srcs.end(); ++mi) {
      std::string m = this->ConvertPath((*mi)->GetFullPath(), false);
      this->ConvertToWindowsSlash(m);
      (*this->BuildFileStream) << m << ";";
    }
    (*this->BuildFileStream) << "</AdditionalManifestFiles>\n";
    this->WriteString("</Manifest>\n", 2);
  }
}

void cmVisualStudio10TargetGenerator::WriteAntBuildOptions(
  std::string const& configName)
{
  // Look through the sources for AndroidManifest.xml and use
  // its location as the root source directory.
  std::string rootDir = this->LocalGenerator->GetCurrentSourceDirectory();
  {
    std::vector<cmSourceFile const*> extraSources;
    this->GeneratorTarget->GetExtraSources(extraSources, "");
    for (std::vector<cmSourceFile const*>::const_iterator si =
           extraSources.begin();
         si != extraSources.end(); ++si) {
      if ("androidmanifest.xml" ==
          cmSystemTools::LowerCase((*si)->GetLocation().GetName())) {
        rootDir = (*si)->GetLocation().GetDirectory();
        break;
      }
    }
  }

  // Tell MSBuild to launch Ant.
  {
    std::string antBuildPath = rootDir;
    this->WriteString("<AntBuild>\n", 2);
    this->WriteString("<AntBuildPath>", 3);
    this->ConvertToWindowsSlash(antBuildPath);
    (*this->BuildFileStream) << cmVS10EscapeXML(antBuildPath)
                             << "</AntBuildPath>\n";
  }

  if (this->GeneratorTarget->GetPropertyAsBool("ANDROID_SKIP_ANT_STEP")) {
    this->WriteString("<SkipAntStep>true</SkipAntStep>\n", 3);
  }

  if (this->GeneratorTarget->GetPropertyAsBool("ANDROID_PROGUARD")) {
    this->WriteString("<EnableProGuard>true</EnableProGuard>\n", 3);
  }

  if (const char* proGuardConfigLocation =
        this->GeneratorTarget->GetProperty("ANDROID_PROGUARD_CONFIG_PATH")) {
    this->WriteString("<ProGuardConfigLocation>", 3);
    (*this->BuildFileStream) << cmVS10EscapeXML(proGuardConfigLocation)
                             << "</ProGuardConfigLocation>\n";
  }

  if (const char* securePropertiesLocation =
        this->GeneratorTarget->GetProperty("ANDROID_SECURE_PROPS_PATH")) {
    this->WriteString("<SecurePropertiesLocation>", 3);
    (*this->BuildFileStream) << cmVS10EscapeXML(securePropertiesLocation)
                             << "</SecurePropertiesLocation>\n";
  }

  if (const char* nativeLibDirectoriesExpression =
        this->GeneratorTarget->GetProperty("ANDROID_NATIVE_LIB_DIRECTORIES")) {
    cmGeneratorExpression ge;
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
      ge.Parse(nativeLibDirectoriesExpression);
    std::string nativeLibDirs =
      cge->Evaluate(this->LocalGenerator, configName);
    this->WriteString("<NativeLibDirectories>", 3);
    (*this->BuildFileStream) << cmVS10EscapeXML(nativeLibDirs)
                             << "</NativeLibDirectories>\n";
  }

  if (const char* nativeLibDependenciesExpression =
        this->GeneratorTarget->GetProperty(
          "ANDROID_NATIVE_LIB_DEPENDENCIES")) {
    cmGeneratorExpression ge;
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
      ge.Parse(nativeLibDependenciesExpression);
    std::string nativeLibDeps =
      cge->Evaluate(this->LocalGenerator, configName);
    this->WriteString("<NativeLibDependencies>", 3);
    (*this->BuildFileStream) << cmVS10EscapeXML(nativeLibDeps)
                             << "</NativeLibDependencies>\n";
  }

  if (const char* javaSourceDir =
        this->GeneratorTarget->GetProperty("ANDROID_JAVA_SOURCE_DIR")) {
    this->WriteString("<JavaSourceDir>", 3);
    (*this->BuildFileStream) << cmVS10EscapeXML(javaSourceDir)
                             << "</JavaSourceDir>\n";
  }

  if (const char* jarDirectoriesExpression =
        this->GeneratorTarget->GetProperty("ANDROID_JAR_DIRECTORIES")) {
    cmGeneratorExpression ge;
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
      ge.Parse(jarDirectoriesExpression);
    std::string jarDirectories =
      cge->Evaluate(this->LocalGenerator, configName);
    this->WriteString("<JarDirectories>", 3);
    (*this->BuildFileStream) << cmVS10EscapeXML(jarDirectories)
                             << "</JarDirectories>\n";
  }

  if (const char* jarDeps =
        this->GeneratorTarget->GetProperty("ANDROID_JAR_DEPENDENCIES")) {
    this->WriteString("<JarDependencies>", 3);
    (*this->BuildFileStream) << cmVS10EscapeXML(jarDeps)
                             << "</JarDependencies>\n";
  }

  if (const char* assetsDirectories =
        this->GeneratorTarget->GetProperty("ANDROID_ASSETS_DIRECTORIES")) {
    this->WriteString("<AssetsDirectories>", 3);
    (*this->BuildFileStream) << cmVS10EscapeXML(assetsDirectories)
                             << "</AssetsDirectories>\n";
  }

  {
    std::string manifest_xml = rootDir + "/AndroidManifest.xml";
    this->ConvertToWindowsSlash(manifest_xml);
    this->WriteString("<AndroidManifestLocation>", 3);
    (*this->BuildFileStream) << cmVS10EscapeXML(manifest_xml)
                             << "</AndroidManifestLocation>\n";
  }

  if (const char* antAdditionalOptions =
        this->GeneratorTarget->GetProperty("ANDROID_ANT_ADDITIONAL_OPTIONS")) {
    this->WriteString("<AdditionalOptions>", 3);
    (*this->BuildFileStream) << cmVS10EscapeXML(antAdditionalOptions)
                             << " %(AdditionalOptions)</AdditionalOptions>\n";
  }

  this->WriteString("</AntBuild>\n", 2);
}

bool cmVisualStudio10TargetGenerator::ComputeLinkOptions()
{
  if (this->GeneratorTarget->GetType() == cmState::EXECUTABLE ||
      this->GeneratorTarget->GetType() == cmState::SHARED_LIBRARY ||
      this->GeneratorTarget->GetType() == cmState::MODULE_LIBRARY) {
    for (std::vector<std::string>::const_iterator i =
           this->Configurations.begin();
         i != this->Configurations.end(); ++i) {
      if (!this->ComputeLinkOptions(*i)) {
        return false;
      }
    }
  }
  return true;
}

bool cmVisualStudio10TargetGenerator::ComputeLinkOptions(
  std::string const& config)
{
  cmsys::auto_ptr<Options> pOptions(new Options(
    this->LocalGenerator, Options::Linker, this->GetLinkFlagTable(), 0, this));
  Options& linkOptions = *pOptions;

  const std::string& linkLanguage =
    this->GeneratorTarget->GetLinkerLanguage(config.c_str());
  if (linkLanguage.empty()) {
    cmSystemTools::Error(
      "CMake can not determine linker language for target: ",
      this->Name.c_str());
    return false;
  }

  std::string CONFIG = cmSystemTools::UpperCase(config);

  const char* linkType = "SHARED";
  if (this->GeneratorTarget->GetType() == cmState::MODULE_LIBRARY) {
    linkType = "MODULE";
  }
  if (this->GeneratorTarget->GetType() == cmState::EXECUTABLE) {
    linkType = "EXE";
  }
  std::string flags;
  std::string linkFlagVarBase = "CMAKE_";
  linkFlagVarBase += linkType;
  linkFlagVarBase += "_LINKER_FLAGS";
  flags += " ";
  flags += this->GeneratorTarget->Target->GetMakefile()->GetRequiredDefinition(
    linkFlagVarBase.c_str());
  std::string linkFlagVar = linkFlagVarBase + "_" + CONFIG;
  flags += " ";
  flags += this->GeneratorTarget->Target->GetMakefile()->GetRequiredDefinition(
    linkFlagVar.c_str());
  const char* targetLinkFlags =
    this->GeneratorTarget->GetProperty("LINK_FLAGS");
  if (targetLinkFlags) {
    flags += " ";
    flags += targetLinkFlags;
  }
  std::string flagsProp = "LINK_FLAGS_";
  flagsProp += CONFIG;
  if (const char* flagsConfig =
        this->GeneratorTarget->GetProperty(flagsProp.c_str())) {
    flags += " ";
    flags += flagsConfig;
  }
  std::string standardLibsVar = "CMAKE_";
  standardLibsVar += linkLanguage;
  standardLibsVar += "_STANDARD_LIBRARIES";
  std::string libs =
    this->Makefile->GetSafeDefinition(standardLibsVar.c_str());
  // Remove trailing spaces from libs
  std::string::size_type pos = libs.size() - 1;
  if (!libs.empty()) {
    while (libs[pos] == ' ') {
      pos--;
    }
  }
  if (pos != libs.size() - 1) {
    libs = libs.substr(0, pos + 1);
  }
  // Replace spaces in libs with ;
  cmSystemTools::ReplaceString(libs, " ", ";");
  std::vector<std::string> libVec;
  cmSystemTools::ExpandListArgument(libs, libVec);

  cmComputeLinkInformation* pcli =
    this->GeneratorTarget->GetLinkInformation(config.c_str());
  if (!pcli) {
    cmSystemTools::Error(
      "CMake can not compute cmComputeLinkInformation for target: ",
      this->Name.c_str());
    return false;
  }
  // add the libraries for the target to libs string
  cmComputeLinkInformation& cli = *pcli;
  this->AddLibraries(cli, libVec);
  linkOptions.AddFlag("AdditionalDependencies", libVec);

  std::vector<std::string> const& ldirs = cli.GetDirectories();
  std::vector<std::string> linkDirs;
  for (std::vector<std::string>::const_iterator d = ldirs.begin();
       d != ldirs.end(); ++d) {
    // first just full path
    linkDirs.push_back(*d);
    // next path with configuration type Debug, Release, etc
    linkDirs.push_back(*d + "/$(Configuration)");
  }
  linkDirs.push_back("%(AdditionalLibraryDirectories)");
  linkOptions.AddFlag("AdditionalLibraryDirectories", linkDirs);

  std::string targetName;
  std::string targetNameSO;
  std::string targetNameFull;
  std::string targetNameImport;
  std::string targetNamePDB;
  if (this->GeneratorTarget->GetType() == cmState::EXECUTABLE) {
    this->GeneratorTarget->GetExecutableNames(targetName, targetNameFull,
                                              targetNameImport, targetNamePDB,
                                              config.c_str());
  } else {
    this->GeneratorTarget->GetLibraryNames(targetName, targetNameSO,
                                           targetNameFull, targetNameImport,
                                           targetNamePDB, config.c_str());
  }

  if (this->MSTools) {
    linkOptions.AddFlag("Version", "");

    if (this->GeneratorTarget->GetPropertyAsBool("WIN32_EXECUTABLE")) {
      if (this->GlobalGenerator->TargetsWindowsCE()) {
        linkOptions.AddFlag("SubSystem", "WindowsCE");
        if (this->GeneratorTarget->GetType() == cmState::EXECUTABLE) {
          if (this->ClOptions[config]->UsingUnicode()) {
            linkOptions.AddFlag("EntryPointSymbol", "wWinMainCRTStartup");
          } else {
            linkOptions.AddFlag("EntryPointSymbol", "WinMainCRTStartup");
          }
        }
      } else {
        linkOptions.AddFlag("SubSystem", "Windows");
      }
    } else {
      if (this->GlobalGenerator->TargetsWindowsCE()) {
        linkOptions.AddFlag("SubSystem", "WindowsCE");
        if (this->GeneratorTarget->GetType() == cmState::EXECUTABLE) {
          if (this->ClOptions[config]->UsingUnicode()) {
            linkOptions.AddFlag("EntryPointSymbol", "mainWCRTStartup");
          } else {
            linkOptions.AddFlag("EntryPointSymbol", "mainACRTStartup");
          }
        }
      } else {
        linkOptions.AddFlag("SubSystem", "Console");
      };
    }

    if (const char* stackVal = this->Makefile->GetDefinition(
          "CMAKE_" + linkLanguage + "_STACK_SIZE")) {
      linkOptions.AddFlag("StackReserveSize", stackVal);
    }

    if (this->LocalGenerator->GetVersion() >=
        cmGlobalVisualStudioGenerator::VS14) {
      linkOptions.AddFlag("GenerateDebugInformation", "No");
    } else {
      linkOptions.AddFlag("GenerateDebugInformation", "false");
    }

    std::string pdb = this->GeneratorTarget->GetPDBDirectory(config.c_str());
    pdb += "/";
    pdb += targetNamePDB;
    std::string imLib =
      this->GeneratorTarget->GetDirectory(config.c_str(), true);
    imLib += "/";
    imLib += targetNameImport;

    linkOptions.AddFlag("ImportLibrary", imLib.c_str());
    linkOptions.AddFlag("ProgramDataBaseFile", pdb.c_str());

    // A Windows Runtime component uses internal .NET metadata,
    // so does not have an import library.
    if (this->GeneratorTarget->GetPropertyAsBool("VS_WINRT_COMPONENT") &&
        this->GeneratorTarget->GetType() != cmState::EXECUTABLE) {
      linkOptions.AddFlag("GenerateWindowsMetadata", "true");
    } else if (this->GlobalGenerator->TargetsWindowsPhone() ||
               this->GlobalGenerator->TargetsWindowsStore()) {
      // WindowsPhone and WindowsStore components are in an app container
      // and produce WindowsMetadata.  If we are not producing a WINRT
      // component, then do not generate the metadata here.
      linkOptions.AddFlag("GenerateWindowsMetadata", "false");
    }

    if (this->GlobalGenerator->TargetsWindowsPhone() &&
        this->GlobalGenerator->GetSystemVersion() == "8.0") {
      // WindowsPhone 8.0 does not have ole32.
      linkOptions.AppendFlag("IgnoreSpecificDefaultLibraries", "ole32.lib");
    }
  } else if (this->NsightTegra) {
    linkOptions.AddFlag("SoName", targetNameSO.c_str());
  }

  linkOptions.Parse(flags.c_str());

  if (this->MSTools) {
    if (cmSourceFile const* defsrc =
          this->GeneratorTarget->GetModuleDefinitionFile("")) {
      linkOptions.AddFlag("ModuleDefinitionFile",
                          defsrc->GetFullPath().c_str());
    }
    linkOptions.AppendFlag("IgnoreSpecificDefaultLibraries",
                           "%(IgnoreSpecificDefaultLibraries)");
  }

  if (this->GeneratorTarget->GetType() == cmState::SHARED_LIBRARY &&
      this->Makefile->IsOn("CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS")) {
    if (this->GeneratorTarget->GetPropertyAsBool(
          "WINDOWS_EXPORT_ALL_SYMBOLS")) {
      linkOptions.AddFlag("ModuleDefinitionFile", "$(IntDir)exportall.def");
    }
  }

  // Hack to fix flag version selection in a common use case.
  // FIXME: Select flag table based on toolset instead of VS version.
  if (this->LocalGenerator->GetVersion() >=
      cmGlobalVisualStudioGenerator::VS14) {
    cmGlobalVisualStudio10Generator* gg =
      static_cast<cmGlobalVisualStudio10Generator*>(this->GlobalGenerator);
    const char* toolset = gg->GetPlatformToolset();
    if (toolset && (cmHasLiteralPrefix(toolset, "v100") ||
                    cmHasLiteralPrefix(toolset, "v110") ||
                    cmHasLiteralPrefix(toolset, "v120"))) {
      if (const char* debug =
            linkOptions.GetFlag("GenerateDebugInformation")) {
        // Convert value from enumeration back to boolean for older toolsets.
        if (strcmp(debug, "No") == 0) {
          linkOptions.AddFlag("GenerateDebugInformation", "false");
        } else if (strcmp(debug, "Debug") == 0) {
          linkOptions.AddFlag("GenerateDebugInformation", "true");
        }
      }
    }
  }

  this->LinkOptions[config] = pOptions.release();
  return true;
}

void cmVisualStudio10TargetGenerator::WriteLinkOptions(
  std::string const& config)
{
  if (this->GeneratorTarget->GetType() == cmState::STATIC_LIBRARY ||
      this->GeneratorTarget->GetType() > cmState::MODULE_LIBRARY) {
    return;
  }
  Options& linkOptions = *(this->LinkOptions[config]);
  this->WriteString("<Link>\n", 2);

  linkOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");
  linkOptions.OutputFlagMap(*this->BuildFileStream, "      ");

  this->WriteString("</Link>\n", 2);
  if (!this->GlobalGenerator->NeedLinkLibraryDependencies(
        this->GeneratorTarget)) {
    this->WriteString("<ProjectReference>\n", 2);
    this->WriteString(
      "<LinkLibraryDependencies>false</LinkLibraryDependencies>\n", 3);
    this->WriteString("</ProjectReference>\n", 2);
  }
}

void cmVisualStudio10TargetGenerator::AddLibraries(
  cmComputeLinkInformation& cli, std::vector<std::string>& libVec)
{
  typedef cmComputeLinkInformation::ItemVector ItemVector;
  ItemVector libs = cli.GetItems();
  for (ItemVector::const_iterator l = libs.begin(); l != libs.end(); ++l) {
    if (l->IsPath) {
      std::string path = this->LocalGenerator->Convert(
        l->Value.c_str(), cmOutputConverter::START_OUTPUT,
        cmOutputConverter::UNCHANGED);
      this->ConvertToWindowsSlash(path);
      libVec.push_back(path);
    } else if (!l->Target ||
               l->Target->GetType() != cmState::INTERFACE_LIBRARY) {
      libVec.push_back(l->Value);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteMidlOptions(
  std::string const& /*config*/, std::vector<std::string> const& includes)
{
  if (!this->MSTools) {
    return;
  }

  // This processes *any* of the .idl files specified in the project's file
  // list (and passed as the item metadata %(Filename) expressing the rule
  // input filename) into output files at the per-config *build* dir
  // ($(IntDir)) each.
  //
  // IOW, this MIDL section is intended to provide a fully generic syntax
  // content suitable for most cases (read: if you get errors, then it's quite
  // probable that the error is on your side of the .idl setup).
  //
  // Also, note that the marked-as-generated _i.c file in the Visual Studio
  // generator case needs to be referred to as $(IntDir)\foo_i.c at the
  // project's file list, otherwise the compiler-side processing won't pick it
  // up (for non-directory form, it ends up looking in project binary dir
  // only).  Perhaps there's something to be done to make this more automatic
  // on the CMake side?
  this->WriteString("<Midl>\n", 2);
  this->WriteString("<AdditionalIncludeDirectories>", 3);
  for (std::vector<std::string>::const_iterator i = includes.begin();
       i != includes.end(); ++i) {
    *this->BuildFileStream << cmVS10EscapeXML(*i) << ";";
  }
  this->WriteString("%(AdditionalIncludeDirectories)"
                    "</AdditionalIncludeDirectories>\n",
                    0);
  this->WriteString("<OutputDirectory>$(ProjectDir)/$(IntDir)"
                    "</OutputDirectory>\n",
                    3);
  this->WriteString("<HeaderFileName>%(Filename).h</HeaderFileName>\n", 3);
  this->WriteString("<TypeLibraryName>%(Filename).tlb</TypeLibraryName>\n", 3);
  this->WriteString("<InterfaceIdentifierFileName>"
                    "%(Filename)_i.c</InterfaceIdentifierFileName>\n",
                    3);
  this->WriteString("<ProxyFileName>%(Filename)_p.c</ProxyFileName>\n", 3);
  this->WriteString("</Midl>\n", 2);
}

void cmVisualStudio10TargetGenerator::WriteItemDefinitionGroups()
{
  for (std::vector<std::string>::const_iterator i =
         this->Configurations.begin();
       i != this->Configurations.end(); ++i) {
    std::vector<std::string> includes;
    this->LocalGenerator->GetIncludeDirectories(
      includes, this->GeneratorTarget, "C", i->c_str());
    for (std::vector<std::string>::iterator ii = includes.begin();
         ii != includes.end(); ++ii) {
      this->ConvertToWindowsSlash(*ii);
    }
    this->WritePlatformConfigTag("ItemDefinitionGroup", i->c_str(), 1);
    *this->BuildFileStream << "\n";
    //    output cl compile flags <ClCompile></ClCompile>
    if (this->GeneratorTarget->GetType() <= cmState::OBJECT_LIBRARY) {
      this->WriteClOptions(*i, includes);
      //    output rc compile flags <ResourceCompile></ResourceCompile>
      this->WriteRCOptions(*i, includes);
      this->WriteMasmOptions(*i, includes);
    }
    //    output midl flags       <Midl></Midl>
    this->WriteMidlOptions(*i, includes);
    // write events
    this->WriteEvents(*i);
    //    output link flags       <Link></Link>
    this->WriteLinkOptions(*i);
    //    output lib flags       <Lib></Lib>
    this->WriteLibOptions(*i);
    //    output manifest flags  <Manifest></Manifest>
    this->WriteManifestOptions(*i);
    if (this->NsightTegra &&
        this->GeneratorTarget->GetType() == cmState::EXECUTABLE &&
        this->GeneratorTarget->GetPropertyAsBool("ANDROID_GUI")) {
      this->WriteAntBuildOptions(*i);
    }
    this->WriteString("</ItemDefinitionGroup>\n", 1);
  }
}

void cmVisualStudio10TargetGenerator::WriteEvents(
  std::string const& configName)
{
  bool addedPrelink = false;
  if (this->GeneratorTarget->GetType() == cmState::SHARED_LIBRARY &&
      this->Makefile->IsOn("CMAKE_SUPPORT_WINDOWS_EXPORT_ALL_SYMBOLS")) {
    if (this->GeneratorTarget->GetPropertyAsBool(
          "WINDOWS_EXPORT_ALL_SYMBOLS")) {
      addedPrelink = true;
      std::vector<cmCustomCommand> commands =
        this->GeneratorTarget->GetPreLinkCommands();
      this->GlobalGenerator->AddSymbolExportCommand(this->GeneratorTarget,
                                                    commands, configName);
      this->WriteEvent("PreLinkEvent", commands, configName);
    }
  }
  if (!addedPrelink) {
    this->WriteEvent("PreLinkEvent",
                     this->GeneratorTarget->GetPreLinkCommands(), configName);
  }
  this->WriteEvent("PreBuildEvent",
                   this->GeneratorTarget->GetPreBuildCommands(), configName);
  this->WriteEvent("PostBuildEvent",
                   this->GeneratorTarget->GetPostBuildCommands(), configName);
}

void cmVisualStudio10TargetGenerator::WriteEvent(
  const char* name, std::vector<cmCustomCommand> const& commands,
  std::string const& configName)
{
  if (commands.empty()) {
    return;
  }
  this->WriteString("<", 2);
  (*this->BuildFileStream) << name << ">\n";
  cmLocalVisualStudio7Generator* lg = this->LocalGenerator;
  std::string script;
  const char* pre = "";
  std::string comment;
  for (std::vector<cmCustomCommand>::const_iterator i = commands.begin();
       i != commands.end(); ++i) {
    cmCustomCommandGenerator ccg(*i, configName, this->LocalGenerator);
    comment += pre;
    comment += lg->ConstructComment(ccg);
    script += pre;
    pre = "\n";
    script += cmVS10EscapeXML(lg->ConstructScript(ccg));
  }
  comment = cmVS10EscapeComment(comment);
  this->WriteString("<Message>", 3);
  (*this->BuildFileStream) << cmVS10EscapeXML(comment) << "</Message>\n";
  this->WriteString("<Command>", 3);
  (*this->BuildFileStream) << script;
  (*this->BuildFileStream) << "</Command>"
                           << "\n";
  this->WriteString("</", 2);
  (*this->BuildFileStream) << name << ">\n";
}

void cmVisualStudio10TargetGenerator::WriteProjectReferences()
{
  cmGlobalGenerator::TargetDependSet const& unordered =
    this->GlobalGenerator->GetTargetDirectDepends(this->GeneratorTarget);
  typedef cmGlobalVisualStudioGenerator::OrderedTargetDependSet
    OrderedTargetDependSet;
  OrderedTargetDependSet depends(unordered, CMAKE_CHECK_BUILD_SYSTEM_TARGET);
  this->WriteString("<ItemGroup>\n", 1);
  for (OrderedTargetDependSet::const_iterator i = depends.begin();
       i != depends.end(); ++i) {
    cmGeneratorTarget const* dt = *i;
    if (dt->GetType() == cmState::INTERFACE_LIBRARY) {
      continue;
    }
    // skip fortran targets as they can not be processed by MSBuild
    // the only reference will be in the .sln file
    if (static_cast<cmGlobalVisualStudioGenerator*>(this->GlobalGenerator)
          ->TargetIsFortranOnly(dt)) {
      continue;
    }
    this->WriteString("<ProjectReference Include=\"", 2);
    cmLocalGenerator* lg = dt->GetLocalGenerator();
    std::string name = dt->GetName();
    std::string path;
    const char* p = dt->GetProperty("EXTERNAL_MSPROJECT");
    if (p) {
      path = p;
    } else {
      path = lg->GetCurrentBinaryDirectory();
      path += "/";
      path += dt->GetName();
      path += ".vcxproj";
    }
    (*this->BuildFileStream) << cmVS10EscapeXML(path) << "\">\n";
    this->WriteString("<Project>", 3);
    (*this->BuildFileStream) << this->GlobalGenerator->GetGUID(name.c_str())
                             << "</Project>\n";
    this->WriteString("</ProjectReference>\n", 2);
  }
  this->WriteString("</ItemGroup>\n", 1);
}

void cmVisualStudio10TargetGenerator::WritePlatformExtensions()
{
  // This only applies to Windows 10 apps
  if (this->GlobalGenerator->TargetsWindowsStore() &&
      cmHasLiteralPrefix(this->GlobalGenerator->GetSystemVersion(), "10.0")) {
    const char* desktopExtensionsVersion =
      this->GeneratorTarget->GetProperty("VS_DESKTOP_EXTENSIONS_VERSION");
    if (desktopExtensionsVersion) {
      this->WriteSinglePlatformExtension("WindowsDesktop",
                                         desktopExtensionsVersion);
    }
    const char* mobileExtensionsVersion =
      this->GeneratorTarget->GetProperty("VS_MOBILE_EXTENSIONS_VERSION");
    if (mobileExtensionsVersion) {
      this->WriteSinglePlatformExtension("WindowsMobile",
                                         mobileExtensionsVersion);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteSinglePlatformExtension(
  std::string const& extension, std::string const& version)
{
  this->WriteString("<Import Project=", 2);
  (*this->BuildFileStream)
    << "\"$([Microsoft.Build.Utilities.ToolLocationHelper]"
    << "::GetPlatformExtensionSDKLocation(`" << extension
    << ", Version=" << version
    << "`, $(TargetPlatformIdentifier), $(TargetPlatformVersion), null, "
    << "$(ExtensionSDKDirectoryRoot), null))"
    << "\\DesignTime\\CommonConfiguration\\Neutral\\" << extension
    << ".props\" "
    << "Condition=\"exists('$("
    << "[Microsoft.Build.Utilities.ToolLocationHelper]"
    << "::GetPlatformExtensionSDKLocation(`" << extension
    << ", Version=" << version
    << "`, $(TargetPlatformIdentifier), $(TargetPlatformVersion), null, "
    << "$(ExtensionSDKDirectoryRoot), null))"
    << "\\DesignTime\\CommonConfiguration\\Neutral\\" << extension
    << ".props')\" />\n";
}

void cmVisualStudio10TargetGenerator::WriteSDKReferences()
{
  // This only applies to Windows 10 apps
  if (this->GlobalGenerator->TargetsWindowsStore() &&
      cmHasLiteralPrefix(this->GlobalGenerator->GetSystemVersion(), "10.0")) {
    const char* desktopExtensionsVersion =
      this->GeneratorTarget->GetProperty("VS_DESKTOP_EXTENSIONS_VERSION");
    const char* mobileExtensionsVersion =
      this->GeneratorTarget->GetProperty("VS_MOBILE_EXTENSIONS_VERSION");
    const char* iotExtensionsVersion =
      this->GeneratorTarget->GetProperty("VS_IOT_EXTENSIONS_VERSION");

    if (desktopExtensionsVersion || mobileExtensionsVersion ||
        iotExtensionsVersion) {
      this->WriteString("<ItemGroup>\n", 1);
      if (desktopExtensionsVersion) {
        this->WriteSingleSDKReference("WindowsDesktop",
                                      desktopExtensionsVersion);
      }
      if (mobileExtensionsVersion) {
        this->WriteSingleSDKReference("WindowsMobile",
                                      mobileExtensionsVersion);
      }
      if (iotExtensionsVersion) {
        this->WriteSingleSDKReference("WindowsIoT", iotExtensionsVersion);
      }
      this->WriteString("</ItemGroup>\n", 1);
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteSingleSDKReference(
  std::string const& extension, std::string const& version)
{
  this->WriteString("<SDKReference Include=\"", 2);
  (*this->BuildFileStream) << extension << ", Version=" << version
                           << "\" />\n";
}

void cmVisualStudio10TargetGenerator::WriteWinRTPackageCertificateKeyFile()
{
  if ((this->GlobalGenerator->TargetsWindowsStore() ||
       this->GlobalGenerator->TargetsWindowsPhone()) &&
      (cmState::EXECUTABLE == this->GeneratorTarget->GetType())) {
    std::string pfxFile;
    std::vector<cmSourceFile const*> certificates;
    this->GeneratorTarget->GetCertificates(certificates, "");
    for (std::vector<cmSourceFile const*>::const_iterator si =
           certificates.begin();
         si != certificates.end(); ++si) {
      pfxFile = this->ConvertPath((*si)->GetFullPath(), false);
      this->ConvertToWindowsSlash(pfxFile);
      break;
    }

    if (this->IsMissingFiles &&
        !(this->GlobalGenerator->TargetsWindowsPhone() &&
          this->GlobalGenerator->GetSystemVersion() == "8.0")) {
      // Move the manifest to a project directory to avoid clashes
      std::string artifactDir =
        this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
      this->ConvertToWindowsSlash(artifactDir);
      this->WriteString("<PropertyGroup>\n", 1);
      this->WriteString("<AppxPackageArtifactsDir>", 2);
      (*this->BuildFileStream) << cmVS10EscapeXML(artifactDir)
                               << "\\</AppxPackageArtifactsDir>\n";
      this->WriteString("<ProjectPriFullPath>"
                        "$(TargetDir)resources.pri</ProjectPriFullPath>\n",
                        2);

      // If we are missing files and we don't have a certificate and
      // aren't targeting WP8.0, add a default certificate
      if (pfxFile.empty()) {
        std::string templateFolder =
          cmSystemTools::GetCMakeRoot() + "/Templates/Windows";
        pfxFile = this->DefaultArtifactDir + "/Windows_TemporaryKey.pfx";
        cmSystemTools::CopyAFile(templateFolder + "/Windows_TemporaryKey.pfx",
                                 pfxFile, false);
        this->ConvertToWindowsSlash(pfxFile);
        this->AddedFiles.push_back(pfxFile);
      }

      this->WriteString("<", 2);
      (*this->BuildFileStream) << "PackageCertificateKeyFile>" << pfxFile
                               << "</PackageCertificateKeyFile>\n";
      std::string thumb = cmSystemTools::ComputeCertificateThumbprint(pfxFile);
      if (!thumb.empty()) {
        this->WriteString("<PackageCertificateThumbprint>", 2);
        (*this->BuildFileStream) << thumb
                                 << "</PackageCertificateThumbprint>\n";
      }
      this->WriteString("</PropertyGroup>\n", 1);
    } else if (!pfxFile.empty()) {
      this->WriteString("<PropertyGroup>\n", 1);
      this->WriteString("<", 2);
      (*this->BuildFileStream) << "PackageCertificateKeyFile>" << pfxFile
                               << "</PackageCertificateKeyFile>\n";
      std::string thumb = cmSystemTools::ComputeCertificateThumbprint(pfxFile);
      if (!thumb.empty()) {
        this->WriteString("<PackageCertificateThumbprint>", 2);
        (*this->BuildFileStream) << thumb
                                 << "</PackageCertificateThumbprint>\n";
      }
      this->WriteString("</PropertyGroup>\n", 1);
    }
  }
}

bool cmVisualStudio10TargetGenerator::IsResxHeader(
  const std::string& headerFile)
{
  std::set<std::string> expectedResxHeaders;
  this->GeneratorTarget->GetExpectedResxHeaders(expectedResxHeaders, "");

  std::set<std::string>::const_iterator it =
    expectedResxHeaders.find(headerFile);
  return it != expectedResxHeaders.end();
}

bool cmVisualStudio10TargetGenerator::IsXamlHeader(
  const std::string& headerFile)
{
  std::set<std::string> expectedXamlHeaders;
  this->GeneratorTarget->GetExpectedXamlHeaders(expectedXamlHeaders, "");

  std::set<std::string>::const_iterator it =
    expectedXamlHeaders.find(headerFile);
  return it != expectedXamlHeaders.end();
}

bool cmVisualStudio10TargetGenerator::IsXamlSource(
  const std::string& sourceFile)
{
  std::set<std::string> expectedXamlSources;
  this->GeneratorTarget->GetExpectedXamlSources(expectedXamlSources, "");

  std::set<std::string>::const_iterator it =
    expectedXamlSources.find(sourceFile);
  return it != expectedXamlSources.end();
}

void cmVisualStudio10TargetGenerator::WriteApplicationTypeSettings()
{
  cmGlobalVisualStudio10Generator* gg =
    static_cast<cmGlobalVisualStudio10Generator*>(this->GlobalGenerator);
  bool isAppContainer = false;
  bool const isWindowsPhone = this->GlobalGenerator->TargetsWindowsPhone();
  bool const isWindowsStore = this->GlobalGenerator->TargetsWindowsStore();
  std::string const& v = this->GlobalGenerator->GetSystemVersion();
  if (isWindowsPhone || isWindowsStore) {
    this->WriteString("<ApplicationType>", 2);
    (*this->BuildFileStream)
      << (isWindowsPhone ? "Windows Phone" : "Windows Store")
      << "</ApplicationType>\n";
    this->WriteString("<DefaultLanguage>en-US"
                      "</DefaultLanguage>\n",
                      2);
    if (cmHasLiteralPrefix(v, "10.0")) {
      this->WriteString("<ApplicationTypeRevision>", 2);
      (*this->BuildFileStream) << cmVS10EscapeXML("10.0")
                               << "</ApplicationTypeRevision>\n";
      // Visual Studio 14.0 is necessary for building 10.0 apps
      this->WriteString("<MinimumVisualStudioVersion>14.0"
                        "</MinimumVisualStudioVersion>\n",
                        2);

      if (this->GeneratorTarget->GetType() < cmState::UTILITY) {
        isAppContainer = true;
      }
    } else if (v == "8.1") {
      this->WriteString("<ApplicationTypeRevision>", 2);
      (*this->BuildFileStream) << cmVS10EscapeXML(v)
                               << "</ApplicationTypeRevision>\n";
      // Visual Studio 12.0 is necessary for building 8.1 apps
      this->WriteString("<MinimumVisualStudioVersion>12.0"
                        "</MinimumVisualStudioVersion>\n",
                        2);

      if (this->GeneratorTarget->GetType() < cmState::UTILITY) {
        isAppContainer = true;
      }
    } else if (v == "8.0") {
      this->WriteString("<ApplicationTypeRevision>", 2);
      (*this->BuildFileStream) << cmVS10EscapeXML(v)
                               << "</ApplicationTypeRevision>\n";
      // Visual Studio 11.0 is necessary for building 8.0 apps
      this->WriteString("<MinimumVisualStudioVersion>11.0"
                        "</MinimumVisualStudioVersion>\n",
                        2);

      if (isWindowsStore &&
          this->GeneratorTarget->GetType() < cmState::UTILITY) {
        isAppContainer = true;
      } else if (isWindowsPhone &&
                 this->GeneratorTarget->GetType() == cmState::EXECUTABLE) {
        this->WriteString("<XapOutputs>true</XapOutputs>\n", 2);
        this->WriteString("<XapFilename>", 2);
        (*this->BuildFileStream)
          << cmVS10EscapeXML(this->Name.c_str())
          << "_$(Configuration)_$(Platform).xap</XapFilename>\n";
      }
    }
  }
  if (isAppContainer) {
    this->WriteString("<AppContainerApplication>true"
                      "</AppContainerApplication>\n",
                      2);
  } else if (this->Platform == "ARM") {
    this->WriteString("<WindowsSDKDesktopARMSupport>true"
                      "</WindowsSDKDesktopARMSupport>\n",
                      2);
  }
  std::string const& targetPlatformVersion =
    gg->GetWindowsTargetPlatformVersion();
  if (!targetPlatformVersion.empty()) {
    this->WriteString("<WindowsTargetPlatformVersion>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(targetPlatformVersion)
                             << "</WindowsTargetPlatformVersion>\n";
  }
  const char* targetPlatformMinVersion = this->GeneratorTarget->GetProperty(
    "VS_WINDOWS_TARGET_PLATFORM_MIN_VERSION");
  if (targetPlatformMinVersion) {
    this->WriteString("<WindowsTargetPlatformMinVersion>", 2);
    (*this->BuildFileStream) << cmVS10EscapeXML(targetPlatformMinVersion)
                             << "</WindowsTargetPlatformMinVersion>\n";
  } else if (isWindowsStore && cmHasLiteralPrefix(v, "10.0")) {
    // If the min version is not set, then use the TargetPlatformVersion
    if (!targetPlatformVersion.empty()) {
      this->WriteString("<WindowsTargetPlatformMinVersion>", 2);
      (*this->BuildFileStream) << cmVS10EscapeXML(targetPlatformVersion)
                               << "</WindowsTargetPlatformMinVersion>\n";
    }
  }

  // Added IoT Startup Task support
  if (this->GeneratorTarget->GetPropertyAsBool("VS_IOT_STARTUP_TASK")) {
    this->WriteString("<ContainsStartupTask>true</ContainsStartupTask>\n", 2);
  }
}

void cmVisualStudio10TargetGenerator::VerifyNecessaryFiles()
{
  // For Windows and Windows Phone executables, we will assume that if a
  // manifest is not present that we need to add all the necessary files
  if (this->GeneratorTarget->GetType() == cmState::EXECUTABLE) {
    std::vector<cmSourceFile const*> manifestSources;
    this->GeneratorTarget->GetAppManifest(manifestSources, "");
    {
      std::string const& v = this->GlobalGenerator->GetSystemVersion();
      if (this->GlobalGenerator->TargetsWindowsPhone()) {
        if (v == "8.0") {
          // Look through the sources for WMAppManifest.xml
          std::vector<cmSourceFile const*> extraSources;
          this->GeneratorTarget->GetExtraSources(extraSources, "");
          bool foundManifest = false;
          for (std::vector<cmSourceFile const*>::const_iterator si =
                 extraSources.begin();
               si != extraSources.end(); ++si) {
            // Need to do a lowercase comparison on the filename
            if ("wmappmanifest.xml" ==
                cmSystemTools::LowerCase((*si)->GetLocation().GetName())) {
              foundManifest = true;
              break;
            }
          }
          if (!foundManifest) {
            this->IsMissingFiles = true;
          }
        } else if (v == "8.1") {
          if (manifestSources.empty()) {
            this->IsMissingFiles = true;
          }
        }
      } else if (this->GlobalGenerator->TargetsWindowsStore()) {
        if (manifestSources.empty()) {
          if (v == "8.0") {
            this->IsMissingFiles = true;
          } else if (v == "8.1" || cmHasLiteralPrefix(v, "10.0")) {
            this->IsMissingFiles = true;
          }
        }
      }
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteMissingFiles()
{
  std::string const& v = this->GlobalGenerator->GetSystemVersion();
  if (this->GlobalGenerator->TargetsWindowsPhone()) {
    if (v == "8.0") {
      this->WriteMissingFilesWP80();
    } else if (v == "8.1") {
      this->WriteMissingFilesWP81();
    }
  } else if (this->GlobalGenerator->TargetsWindowsStore()) {
    if (v == "8.0") {
      this->WriteMissingFilesWS80();
    } else if (v == "8.1") {
      this->WriteMissingFilesWS81();
    } else if (cmHasLiteralPrefix(v, "10.0")) {
      this->WriteMissingFilesWS10_0();
    }
  }
}

void cmVisualStudio10TargetGenerator::WriteMissingFilesWP80()
{
  std::string templateFolder =
    cmSystemTools::GetCMakeRoot() + "/Templates/Windows";

  // For WP80, the manifest needs to be in the same folder as the project
  // this can cause an overwrite problem if projects aren't organized in
  // folders
  std::string manifestFile =
    this->LocalGenerator->GetCurrentBinaryDirectory() +
    std::string("/WMAppManifest.xml");
  std::string artifactDir =
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  this->ConvertToWindowsSlash(artifactDir);
  std::string artifactDirXML = cmVS10EscapeXML(artifactDir);
  std::string targetNameXML =
    cmVS10EscapeXML(this->GeneratorTarget->GetName());

  cmGeneratedFileStream fout(manifestFile.c_str());
  fout.SetCopyIfDifferent(true);

  /* clang-format off */
  fout <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<Deployment"
    " xmlns=\"http://schemas.microsoft.com/windowsphone/2012/deployment\""
    " AppPlatformVersion=\"8.0\">\n"
    "\t<DefaultLanguage xmlns=\"\" code=\"en-US\"/>\n"
    "\t<App xmlns=\"\" ProductID=\"{" << this->GUID << "}\""
    " Title=\"CMake Test Program\" RuntimeType=\"Modern Native\""
    " Version=\"1.0.0.0\" Genre=\"apps.normal\"  Author=\"CMake\""
    " Description=\"Default CMake App\" Publisher=\"CMake\""
    " PublisherID=\"{" << this->GUID << "}\">\n"
    "\t\t<IconPath IsRelative=\"true\" IsResource=\"false\">"
       << artifactDirXML << "\\ApplicationIcon.png</IconPath>\n"
    "\t\t<Capabilities/>\n"
    "\t\t<Tasks>\n"
    "\t\t\t<DefaultTask Name=\"_default\""
    " ImagePath=\"" << targetNameXML << ".exe\" ImageParams=\"\" />\n"
    "\t\t</Tasks>\n"
    "\t\t<Tokens>\n"
    "\t\t\t<PrimaryToken TokenID=\"" << targetNameXML << "Token\""
    " TaskName=\"_default\">\n"
    "\t\t\t\t<TemplateFlip>\n"
    "\t\t\t\t\t<SmallImageURI IsRelative=\"true\" IsResource=\"false\">"
       << artifactDirXML << "\\SmallLogo.png</SmallImageURI>\n"
    "\t\t\t\t\t<Count>0</Count>\n"
    "\t\t\t\t\t<BackgroundImageURI IsRelative=\"true\" IsResource=\"false\">"
       << artifactDirXML << "\\Logo.png</BackgroundImageURI>\n"
    "\t\t\t\t</TemplateFlip>\n"
    "\t\t\t</PrimaryToken>\n"
    "\t\t</Tokens>\n"
    "\t\t<ScreenResolutions>\n"
    "\t\t\t<ScreenResolution Name=\"ID_RESOLUTION_WVGA\" />\n"
    "\t\t</ScreenResolutions>\n"
    "\t</App>\n"
    "</Deployment>\n";
  /* clang-format on */

  std::string sourceFile = this->ConvertPath(manifestFile, false);
  this->ConvertToWindowsSlash(sourceFile);
  this->WriteString("<Xml Include=\"", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(sourceFile) << "\">\n";
  this->WriteString("<SubType>Designer</SubType>\n", 3);
  this->WriteString("</Xml>\n", 2);
  this->AddedFiles.push_back(sourceFile);

  std::string smallLogo = this->DefaultArtifactDir + "/SmallLogo.png";
  cmSystemTools::CopyAFile(templateFolder + "/SmallLogo.png", smallLogo,
                           false);
  this->ConvertToWindowsSlash(smallLogo);
  this->WriteString("<Image Include=\"", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(smallLogo) << "\" />\n";
  this->AddedFiles.push_back(smallLogo);

  std::string logo = this->DefaultArtifactDir + "/Logo.png";
  cmSystemTools::CopyAFile(templateFolder + "/Logo.png", logo, false);
  this->ConvertToWindowsSlash(logo);
  this->WriteString("<Image Include=\"", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(logo) << "\" />\n";
  this->AddedFiles.push_back(logo);

  std::string applicationIcon =
    this->DefaultArtifactDir + "/ApplicationIcon.png";
  cmSystemTools::CopyAFile(templateFolder + "/ApplicationIcon.png",
                           applicationIcon, false);
  this->ConvertToWindowsSlash(applicationIcon);
  this->WriteString("<Image Include=\"", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(applicationIcon) << "\" />\n";
  this->AddedFiles.push_back(applicationIcon);
}

void cmVisualStudio10TargetGenerator::WriteMissingFilesWP81()
{
  std::string manifestFile =
    this->DefaultArtifactDir + "/package.appxManifest";
  std::string artifactDir =
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  this->ConvertToWindowsSlash(artifactDir);
  std::string artifactDirXML = cmVS10EscapeXML(artifactDir);
  std::string targetNameXML =
    cmVS10EscapeXML(this->GeneratorTarget->GetName());

  cmGeneratedFileStream fout(manifestFile.c_str());
  fout.SetCopyIfDifferent(true);

  /* clang-format off */
  fout <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<Package xmlns=\"http://schemas.microsoft.com/appx/2010/manifest\""
    " xmlns:m2=\"http://schemas.microsoft.com/appx/2013/manifest\""
    " xmlns:mp=\"http://schemas.microsoft.com/appx/2014/phone/manifest\">\n"
    "\t<Identity Name=\"" << this->GUID << "\" Publisher=\"CN=CMake\""
    " Version=\"1.0.0.0\" />\n"
    "\t<mp:PhoneIdentity PhoneProductId=\"" << this->GUID << "\""
    " PhonePublisherId=\"00000000-0000-0000-0000-000000000000\"/>\n"
    "\t<Properties>\n"
    "\t\t<DisplayName>" << targetNameXML << "</DisplayName>\n"
    "\t\t<PublisherDisplayName>CMake</PublisherDisplayName>\n"
    "\t\t<Logo>" << artifactDirXML << "\\StoreLogo.png</Logo>\n"
    "\t</Properties>\n"
    "\t<Prerequisites>\n"
    "\t\t<OSMinVersion>6.3.1</OSMinVersion>\n"
    "\t\t<OSMaxVersionTested>6.3.1</OSMaxVersionTested>\n"
    "\t</Prerequisites>\n"
    "\t<Resources>\n"
    "\t\t<Resource Language=\"x-generate\" />\n"
    "\t</Resources>\n"
    "\t<Applications>\n"
    "\t\t<Application Id=\"App\""
    " Executable=\"" << targetNameXML << ".exe\""
    " EntryPoint=\"" << targetNameXML << ".App\">\n"
    "\t\t\t<m2:VisualElements\n"
    "\t\t\t\tDisplayName=\"" << targetNameXML << "\"\n"
    "\t\t\t\tDescription=\"" << targetNameXML << "\"\n"
    "\t\t\t\tBackgroundColor=\"#336699\"\n"
    "\t\t\t\tForegroundText=\"light\"\n"
    "\t\t\t\tSquare150x150Logo=\"" << artifactDirXML << "\\Logo.png\"\n"
    "\t\t\t\tSquare30x30Logo=\"" << artifactDirXML << "\\SmallLogo.png\">\n"
    "\t\t\t\t<m2:DefaultTile ShortName=\"" << targetNameXML << "\">\n"
    "\t\t\t\t\t<m2:ShowNameOnTiles>\n"
    "\t\t\t\t\t\t<m2:ShowOn Tile=\"square150x150Logo\" />\n"
    "\t\t\t\t\t</m2:ShowNameOnTiles>\n"
    "\t\t\t\t</m2:DefaultTile>\n"
    "\t\t\t\t<m2:SplashScreen"
    " Image=\"" << artifactDirXML << "\\SplashScreen.png\" />\n"
    "\t\t\t</m2:VisualElements>\n"
    "\t\t</Application>\n"
    "\t</Applications>\n"
    "</Package>\n";
  /* clang-format on */

  this->WriteCommonMissingFiles(manifestFile);
}

void cmVisualStudio10TargetGenerator::WriteMissingFilesWS80()
{
  std::string manifestFile =
    this->DefaultArtifactDir + "/package.appxManifest";
  std::string artifactDir =
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  this->ConvertToWindowsSlash(artifactDir);
  std::string artifactDirXML = cmVS10EscapeXML(artifactDir);
  std::string targetNameXML =
    cmVS10EscapeXML(this->GeneratorTarget->GetName());

  cmGeneratedFileStream fout(manifestFile.c_str());
  fout.SetCopyIfDifferent(true);

  /* clang-format off */
  fout <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<Package xmlns=\"http://schemas.microsoft.com/appx/2010/manifest\">\n"
    "\t<Identity Name=\"" << this->GUID << "\" Publisher=\"CN=CMake\""
    " Version=\"1.0.0.0\" />\n"
    "\t<Properties>\n"
    "\t\t<DisplayName>" << targetNameXML << "</DisplayName>\n"
    "\t\t<PublisherDisplayName>CMake</PublisherDisplayName>\n"
    "\t\t<Logo>" << artifactDirXML << "\\StoreLogo.png</Logo>\n"
    "\t</Properties>\n"
    "\t<Prerequisites>\n"
    "\t\t<OSMinVersion>6.2.1</OSMinVersion>\n"
    "\t\t<OSMaxVersionTested>6.2.1</OSMaxVersionTested>\n"
    "\t</Prerequisites>\n"
    "\t<Resources>\n"
    "\t\t<Resource Language=\"x-generate\" />\n"
    "\t</Resources>\n"
    "\t<Applications>\n"
    "\t\t<Application Id=\"App\""
    " Executable=\"" << targetNameXML << ".exe\""
    " EntryPoint=\"" << targetNameXML << ".App\">\n"
    "\t\t\t<VisualElements"
    " DisplayName=\"" << targetNameXML << "\""
    " Description=\"" << targetNameXML << "\""
    " BackgroundColor=\"#336699\" ForegroundText=\"light\""
    " Logo=\"" << artifactDirXML << "\\Logo.png\""
    " SmallLogo=\"" << artifactDirXML << "\\SmallLogo.png\">\n"
    "\t\t\t\t<DefaultTile ShowName=\"allLogos\""
    " ShortName=\"" << targetNameXML << "\" />\n"
    "\t\t\t\t<SplashScreen"
    " Image=\"" << artifactDirXML << "\\SplashScreen.png\" />\n"
    "\t\t\t</VisualElements>\n"
    "\t\t</Application>\n"
    "\t</Applications>\n"
    "</Package>\n";
  /* clang-format on */

  this->WriteCommonMissingFiles(manifestFile);
}

void cmVisualStudio10TargetGenerator::WriteMissingFilesWS81()
{
  std::string manifestFile =
    this->DefaultArtifactDir + "/package.appxManifest";
  std::string artifactDir =
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  this->ConvertToWindowsSlash(artifactDir);
  std::string artifactDirXML = cmVS10EscapeXML(artifactDir);
  std::string targetNameXML =
    cmVS10EscapeXML(this->GeneratorTarget->GetName());

  cmGeneratedFileStream fout(manifestFile.c_str());
  fout.SetCopyIfDifferent(true);

  /* clang-format off */
  fout <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<Package xmlns=\"http://schemas.microsoft.com/appx/2010/manifest\""
    " xmlns:m2=\"http://schemas.microsoft.com/appx/2013/manifest\">\n"
    "\t<Identity Name=\"" << this->GUID << "\" Publisher=\"CN=CMake\""
    " Version=\"1.0.0.0\" />\n"
    "\t<Properties>\n"
    "\t\t<DisplayName>" << targetNameXML << "</DisplayName>\n"
    "\t\t<PublisherDisplayName>CMake</PublisherDisplayName>\n"
    "\t\t<Logo>" << artifactDirXML << "\\StoreLogo.png</Logo>\n"
    "\t</Properties>\n"
    "\t<Prerequisites>\n"
    "\t\t<OSMinVersion>6.3</OSMinVersion>\n"
    "\t\t<OSMaxVersionTested>6.3</OSMaxVersionTested>\n"
    "\t</Prerequisites>\n"
    "\t<Resources>\n"
    "\t\t<Resource Language=\"x-generate\" />\n"
    "\t</Resources>\n"
    "\t<Applications>\n"
    "\t\t<Application Id=\"App\""
    " Executable=\"" << targetNameXML << ".exe\""
    " EntryPoint=\"" << targetNameXML << ".App\">\n"
    "\t\t\t<m2:VisualElements\n"
    "\t\t\t\tDisplayName=\"" << targetNameXML << "\"\n"
    "\t\t\t\tDescription=\"" << targetNameXML << "\"\n"
    "\t\t\t\tBackgroundColor=\"#336699\"\n"
    "\t\t\t\tForegroundText=\"light\"\n"
    "\t\t\t\tSquare150x150Logo=\"" << artifactDirXML << "\\Logo.png\"\n"
    "\t\t\t\tSquare30x30Logo=\"" << artifactDirXML << "\\SmallLogo.png\">\n"
    "\t\t\t\t<m2:DefaultTile ShortName=\"" << targetNameXML << "\">\n"
    "\t\t\t\t\t<m2:ShowNameOnTiles>\n"
    "\t\t\t\t\t\t<m2:ShowOn Tile=\"square150x150Logo\" />\n"
    "\t\t\t\t\t</m2:ShowNameOnTiles>\n"
    "\t\t\t\t</m2:DefaultTile>\n"
    "\t\t\t\t<m2:SplashScreen"
    " Image=\"" << artifactDirXML << "\\SplashScreen.png\" />\n"
    "\t\t\t</m2:VisualElements>\n"
    "\t\t</Application>\n"
    "\t</Applications>\n"
    "</Package>\n";
  /* clang-format on */

  this->WriteCommonMissingFiles(manifestFile);
}

void cmVisualStudio10TargetGenerator::WriteMissingFilesWS10_0()
{
  std::string manifestFile =
    this->DefaultArtifactDir + "/package.appxManifest";
  std::string artifactDir =
    this->LocalGenerator->GetTargetDirectory(this->GeneratorTarget);
  this->ConvertToWindowsSlash(artifactDir);
  std::string artifactDirXML = cmVS10EscapeXML(artifactDir);
  std::string targetNameXML =
    cmVS10EscapeXML(this->GeneratorTarget->GetName());

  cmGeneratedFileStream fout(manifestFile.c_str());
  fout.SetCopyIfDifferent(true);

  /* clang-format off */
  fout <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n"
    "<Package\n\t"
    "xmlns=\"http://schemas.microsoft.com/appx/manifest/foundation/windows10\""
    "\txmlns:mp=\"http://schemas.microsoft.com/appx/2014/phone/manifest\"\n"
    "\txmlns:uap=\"http://schemas.microsoft.com/appx/manifest/uap/windows10\""
    "\n\tIgnorableNamespaces=\"uap mp\">\n\n"
    "\t<Identity Name=\"" << this->GUID << "\" Publisher=\"CN=CMake\""
    " Version=\"1.0.0.0\" />\n"
    "\t<mp:PhoneIdentity PhoneProductId=\"" << this->GUID <<
    "\" PhonePublisherId=\"00000000-0000-0000-0000-000000000000\"/>\n"
    "\t<Properties>\n"
    "\t\t<DisplayName>" << targetNameXML << "</DisplayName>\n"
    "\t\t<PublisherDisplayName>CMake</PublisherDisplayName>\n"
    "\t\t<Logo>" << artifactDirXML << "\\StoreLogo.png</Logo>\n"
    "\t</Properties>\n"
    "\t<Dependencies>\n"
    "\t\t<TargetDeviceFamily Name=\"Windows.Universal\" "
    "MinVersion=\"10.0.0.0\" MaxVersionTested=\"10.0.0.0\" />\n"
    "\t</Dependencies>\n"

    "\t<Resources>\n"
    "\t\t<Resource Language=\"x-generate\" />\n"
    "\t</Resources>\n"
    "\t<Applications>\n"
    "\t\t<Application Id=\"App\""
    " Executable=\"" << targetNameXML << ".exe\""
    " EntryPoint=\"" << targetNameXML << ".App\">\n"
    "\t\t\t<uap:VisualElements\n"
    "\t\t\t\tDisplayName=\"" << targetNameXML << "\"\n"
    "\t\t\t\tDescription=\"" << targetNameXML << "\"\n"
    "\t\t\t\tBackgroundColor=\"#336699\"\n"
    "\t\t\t\tSquare150x150Logo=\"" << artifactDirXML << "\\Logo.png\"\n"
    "\t\t\t\tSquare44x44Logo=\"" << artifactDirXML <<
    "\\SmallLogo44x44.png\">\n"
    "\t\t\t\t<uap:SplashScreen"
    " Image=\"" << artifactDirXML << "\\SplashScreen.png\" />\n"
    "\t\t\t</uap:VisualElements>\n"
    "\t\t</Application>\n"
    "\t</Applications>\n"
    "</Package>\n";
  /* clang-format on */

  this->WriteCommonMissingFiles(manifestFile);
}

void cmVisualStudio10TargetGenerator::WriteCommonMissingFiles(
  const std::string& manifestFile)
{
  std::string templateFolder =
    cmSystemTools::GetCMakeRoot() + "/Templates/Windows";

  std::string sourceFile = this->ConvertPath(manifestFile, false);
  this->ConvertToWindowsSlash(sourceFile);
  this->WriteString("<AppxManifest Include=\"", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(sourceFile) << "\">\n";
  this->WriteString("<SubType>Designer</SubType>\n", 3);
  this->WriteString("</AppxManifest>\n", 2);
  this->AddedFiles.push_back(sourceFile);

  std::string smallLogo = this->DefaultArtifactDir + "/SmallLogo.png";
  cmSystemTools::CopyAFile(templateFolder + "/SmallLogo.png", smallLogo,
                           false);
  this->ConvertToWindowsSlash(smallLogo);
  this->WriteString("<Image Include=\"", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(smallLogo) << "\" />\n";
  this->AddedFiles.push_back(smallLogo);

  std::string smallLogo44 = this->DefaultArtifactDir + "/SmallLogo44x44.png";
  cmSystemTools::CopyAFile(templateFolder + "/SmallLogo44x44.png", smallLogo44,
                           false);
  this->ConvertToWindowsSlash(smallLogo44);
  this->WriteString("<Image Include=\"", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(smallLogo44) << "\" />\n";
  this->AddedFiles.push_back(smallLogo44);

  std::string logo = this->DefaultArtifactDir + "/Logo.png";
  cmSystemTools::CopyAFile(templateFolder + "/Logo.png", logo, false);
  this->ConvertToWindowsSlash(logo);
  this->WriteString("<Image Include=\"", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(logo) << "\" />\n";
  this->AddedFiles.push_back(logo);

  std::string storeLogo = this->DefaultArtifactDir + "/StoreLogo.png";
  cmSystemTools::CopyAFile(templateFolder + "/StoreLogo.png", storeLogo,
                           false);
  this->ConvertToWindowsSlash(storeLogo);
  this->WriteString("<Image Include=\"", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(storeLogo) << "\" />\n";
  this->AddedFiles.push_back(storeLogo);

  std::string splashScreen = this->DefaultArtifactDir + "/SplashScreen.png";
  cmSystemTools::CopyAFile(templateFolder + "/SplashScreen.png", splashScreen,
                           false);
  this->ConvertToWindowsSlash(splashScreen);
  this->WriteString("<Image Include=\"", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(splashScreen) << "\" />\n";
  this->AddedFiles.push_back(splashScreen);

  // This file has already been added to the build so don't copy it
  std::string keyFile = this->DefaultArtifactDir + "/Windows_TemporaryKey.pfx";
  this->ConvertToWindowsSlash(keyFile);
  this->WriteString("<None Include=\"", 2);
  (*this->BuildFileStream) << cmVS10EscapeXML(keyFile) << "\" />\n";
}

bool cmVisualStudio10TargetGenerator::ForceOld(const std::string& source) const
{
  HANDLE h =
    CreateFileW(cmSystemTools::ConvertToWindowsExtendedPath(source).c_str(),
                FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, 0, OPEN_EXISTING,
                FILE_FLAG_BACKUP_SEMANTICS, 0);
  if (!h) {
    return false;
  }

  FILETIME const ftime_20010101 = { 3365781504u, 29389701u };
  if (!SetFileTime(h, &ftime_20010101, &ftime_20010101, &ftime_20010101)) {
    CloseHandle(h);
    return false;
  }

  CloseHandle(h);
  return true;
}
