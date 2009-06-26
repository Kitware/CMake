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
#include "cmVisualStudio10TargetGenerator.h"
#include "cmGlobalVisualStudio7Generator.h"
#include "cmTarget.h"
#include "cmComputeLinkInformation.h"
#include "cmGeneratedFileStream.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmVisualStudioGeneratorOptions.h"
#include "cmLocalVisualStudio7Generator.h"

#include "cmVS10CLFlagTable.h"
#include "cmVS10LinkFlagTable.h"



cmVisualStudio10TargetGenerator::
cmVisualStudio10TargetGenerator(cmTarget* target,
                                cmGlobalVisualStudio7Generator* gg)
{
  this->GlobalGenerator = gg;
  this->GlobalGenerator->CreateGUID(target->GetName());
  this->GUID = this->GlobalGenerator->GetGUID(target->GetName());
  this->Target = target;
  this->Makefile = target->GetMakefile();
  this->LocalGenerator =  
    (cmLocalVisualStudio7Generator*)
    this->Makefile->GetLocalGenerator();
  this->Platform = "|Win32";
}

cmVisualStudio10TargetGenerator::~cmVisualStudio10TargetGenerator()
{
  delete this->BuildFileStream;
}

void cmVisualStudio10TargetGenerator::WritePlatformConfigTag(
  const char* tag,
  const char* config,
  int indentLevel,
  const char* attribute,
  const char* end,
  std::ostream* stream)

{
  if(!stream)
    {
    stream = this->BuildFileStream;
    }
  stream->fill(' ');
  stream->width(indentLevel*2 ); 
  (*stream ) << "";
  (*stream ) << "<" << tag 
                            << " Condition=\"'$(Configuration)|$(Platform)'=='";
  (*stream ) << config << this->Platform << "'\"";
  if(attribute)
    {
    (*stream ) << attribute;
    }
  // close the tag
  (*stream ) << ">";
  if(end)
    {
    (*stream ) << end;
    }
}

void cmVisualStudio10TargetGenerator::WriteString(const char* line,
                                                  int indentLevel)
{
  this->BuildFileStream->fill(' ');
  this->BuildFileStream->width(indentLevel*2 );
  // write an empty string to get the fill level indent to print
  (*this->BuildFileStream ) << "";
  (*this->BuildFileStream ) << line;
}

void cmVisualStudio10TargetGenerator::Generate()
{
  // Tell the global generator the name of the project file
  this->Target->SetProperty("GENERATOR_FILE_NAME",this->Target->GetName());
  this->Target->SetProperty("GENERATOR_FILE_NAME_EXT",
                            ".vcxproj");
  cmMakefile* mf = this->Target->GetMakefile();
  std::string path =  mf->GetStartOutputDirectory();
  path += "/";
  path += this->Target->GetName();
  path += ".vcxproj";
  this->BuildFileStream =
    new cmGeneratedFileStream(path.c_str());
  this->BuildFileStream->SetCopyIfDifferent(true);
  
  // Write the encoding header into the file
  char magic[] = {0xEF,0xBB, 0xBF};
  this->BuildFileStream->write(magic, 3);
  this->WriteString("<Project DefaultTargets=\"Build\" "
                    "ToolsVersion=\"4.0\" "
                    "xmlns=\"http://schemas.microsoft.com/"
                    "developer/msbuild/2003\">\n",
                    0);
  this->WriteProjectConfigurations();
  this->WriteString("<PropertyGroup Label=\"Globals\">\n", 1);
  this->WriteString("<ProjectGUID>", 2);
  (*this->BuildFileStream) <<  "{" << this->GUID << "}</ProjectGUID>\n";

  this->WriteString("<SccProjectName />\n", 2);
  this->WriteString("<SccLocalPath />\n", 2);
  this->WriteString("<Keyword>Win32Proj</Keyword>\n", 2);
  this->WriteString("</PropertyGroup>\n", 1);
  this->WriteString("<Import Project="
                    "\"$(VCTargetsPath)\\Microsoft.Cpp.Default.props\" />\n",
                    1);
  this->WriteProjectConfigurationValues();
  this->WriteString(
    "<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.props\" />\n", 1);
  this->WriteString("<ImportGroup Label=\"ExtensionSettings\">\n", 1);
  this->WriteString("</ImportGroup>\n", 1);
  this->WriteString("<ImportGroup Label=\"PropertySheets\">\n", 1);
  this->WriteString("<Import Project="
                    "\"$(LocalAppData)\\Microsoft\\VisualStudio\\10.0\\"
                    "Microsoft.Cpp.$(Platform).user.props\" "
                    "Condition=\"exists('$(LocalAppData)\\Microsoft"
                    "\\VisualStudio\\10.0\\"
                    "Microsoft.Cpp.$(Platform).user.props')\" />\n", 2);
  this->WriteString("</ImportGroup>\n", 1);
  this->WriteString("<PropertyGroup Label=\"UserMacros\" />\n", 1);
  this->WritePathAndIncrementalLinkOptions();
  this->WriteItemDefinitionGroups();
  this->WriteCustomCommands();
  this->WriteObjSources();
  this->WriteCLSources();
  this->WriteProjectReferences();
  this->WriteString(
    "<Import Project=\"$(VCTargetsPath)\\Microsoft.Cpp.targets\""
    " />\n", 1);
  this->WriteString("<ImportGroup Label=\"ExtensionTargets\">\n", 1);
  this->WriteString("</ImportGroup>\n", 1);
  this->WriteString("</Project>", 0);
  // The groups are stored in a separate file for VS 10
  this->WriteGroups();
}

// ConfigurationType Application, Utility StaticLibrary DynamicLibrary

void cmVisualStudio10TargetGenerator::WriteProjectConfigurations()
{
  this->WriteString("<ItemGroup Label=\"ProjectConfigurations\">\n", 1);
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();
  for(std::vector<std::string>::iterator i = configs->begin();
      i != configs->end(); ++i)
    {
    this->WriteString("<ProjectConfiguration Include=\"", 2);
    (*this->BuildFileStream ) <<  *i << this->Platform << "\">\n";
    this->WriteString("<Configuration>", 3);
    (*this->BuildFileStream ) <<  *i << "</Configuration>\n";
    this->WriteString("<Platform>Win32</Platform>\n", 3);
    this->WriteString("</ProjectConfiguration>\n", 2);
    }
  this->WriteString("</ItemGroup>\n", 1);
}

void cmVisualStudio10TargetGenerator::WriteProjectConfigurationValues()
{
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();
  for(std::vector<std::string>::iterator i = configs->begin();
      i != configs->end(); ++i)
    {
    this->WritePlatformConfigTag("PropertyGroup",
                                 i->c_str(),
                                 1, " Label=\"Configuration\"", "\n");
    std::string configType = "<ConfigurationType>";
    switch(this->Target->GetType())
      {
      case cmTarget::SHARED_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
        configType += "DynamicLibrary";
        break;
      case cmTarget::STATIC_LIBRARY:
        configType += "StaticLibrary";
        break;
      case cmTarget::EXECUTABLE:
        configType += "Application";
        break;
      case cmTarget::UTILITY:
        configType += "Utility";
        break;
      }
    configType += "</ConfigurationType>\n";
    this->WriteString(configType.c_str(), 2); 
    const char* mfcFlag = 
      this->Target->GetMakefile()->GetDefinition("CMAKE_MFC_FLAG");
    if(mfcFlag)
      {
      this->WriteString("<UseOfMfc>true</UseOfMfc>\n", 2);
      }
    else
      {
      this->WriteString("<UseOfMfc>false</UseOfMfc>\n", 2);
      }
    this->WriteString("<CharacterSet>MultiByte</CharacterSet>\n", 2);
    this->WriteString("</PropertyGroup>\n", 1);
    }
}

void cmVisualStudio10TargetGenerator::WriteCustomCommands()
{ 
  this->WriteString("<ItemGroup>\n", 1); 
  std::vector<cmSourceFile*>const & sources = this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
      source != sources.end(); ++source)
    {
    if(cmCustomCommand const* command = (*source)->GetCustomCommand())
      {
      this->WriteCustomRule(*source, *command);
      }
    } 
  this->WriteString("</ItemGroup>\n", 1);
}

void 
cmVisualStudio10TargetGenerator::WriteCustomRule(cmSourceFile* source,
                                                 cmCustomCommand const & 
                                                 command)
{
  std::string sourcePath = source->GetFullPath();
  // the rule file seems to need to exist for vs10
  if (source->GetExtension() == "rule")
    {
    if(!cmSystemTools::FileExists(sourcePath.c_str()))
      {
      std::ofstream fout(sourcePath.c_str());
      if(fout)
        {
        fout << "# generated from CMake\n";
        fout.flush();
        fout.close();
        }
      }
    }
  cmLocalVisualStudio7Generator* lg = this->LocalGenerator;
  std::string comment = lg->ConstructComment(command);
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations(); 
  this->WriteString("<CustomBuild Include=\"", 2);
  (*this->BuildFileStream ) << 
    cmSystemTools::RelativePath(this->Makefile->GetCurrentOutputDirectory(),
                                sourcePath.c_str()) << "\">\n";
  for(std::vector<std::string>::iterator i = configs->begin();
      i != configs->end(); ++i)
    {
    std::string script = lg->ConstructScript(command.GetCommandLines(),
                                             command.GetWorkingDirectory(),
                                             i->c_str(),
                                             command.GetEscapeOldStyle(),
                                             command.GetEscapeAllowMakeVars());
    this->WritePlatformConfigTag("Message",i->c_str(), 3);
    (*this->BuildFileStream ) << comment << "</Message>\n";
    this->WritePlatformConfigTag("Command", i->c_str(), 3);
    (*this->BuildFileStream ) << script << "</Command>\n";
    this->WritePlatformConfigTag("AdditionalInputs", i->c_str(), 3);
    
    (*this->BuildFileStream ) << source->GetFullPath();
    for(std::vector<std::string>::const_iterator d = 
          command.GetDepends().begin();
        d != command.GetDepends().end(); 
        ++d)
      {
      std::string dep = this->LocalGenerator->
        GetRealDependency(d->c_str(), i->c_str());
      this->ConvertToWindowsSlash(dep);
      (*this->BuildFileStream ) << ";" << dep;
      }
    (*this->BuildFileStream ) << ";%(AdditionalInputs)</AdditionalInputs>\n";
    this->WritePlatformConfigTag("Outputs", i->c_str(), 3);
    const char* sep = "";
    for(std::vector<std::string>::const_iterator o = 
          command.GetOutputs().begin();
        o != command.GetOutputs().end(); 
        ++o)
      {
      std::string out = *o;
      this->ConvertToWindowsSlash(out);
      (*this->BuildFileStream ) << sep << out;
      sep = ";";
      }
    (*this->BuildFileStream ) << ";%(Outputs)</Outputs>\n";
    }
  this->WriteString("</CustomBuild>\n", 2);
}

void cmVisualStudio10TargetGenerator::ConvertToWindowsSlash(std::string& s)
{
  // first convert all of the slashes
  std::string::size_type pos = 0;
  while((pos = s.find('/', pos)) != std::string::npos)
    {
    s[pos] = '\\';
    pos++;
    }
}
void cmVisualStudio10TargetGenerator::WriteGroups()
{
  // This should create a target.vcxproj.filters file
  // something like this:
  
/*
  ﻿<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="4.0" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <ItemGroup>
    <CustomBuild Include="..\CMakeLists.txt" />
  </ItemGroup>
  <ItemGroup>
    <Filter Include="Source Files">
      <UniqueIdentifier>{05072589-c7be-439a-8fd7-5db6ee5008a9}</UniqueIdentifier>
    </Filter>
  </ItemGroup>
  <ItemGroup>
    <ClCompile Include="..\foo.c">
      <Filter>Source Files</Filter>
    </ClCompile>
    <ClCompile Include="..\testCCompiler.c">
      <Filter>Source Files</Filter>
    </ClCompile>
  </ItemGroup>
</Project>
*/
}


void cmVisualStudio10TargetGenerator::WriteObjSources()
{ 
  if(this->Target->GetType() > cmTarget::MODULE_LIBRARY)
    {
    return;
    }
  bool first = true;
  std::vector<cmSourceFile*>const & sources = this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
      source != sources.end(); ++source)
    {
    std::cerr << (*source)->GetExtension() << "\n";
    std::cerr << (*source)->GetFullPath() << "\n";
    if((*source)->GetExtension() == "obj")
      {
      if(first)
        {
        this->WriteString("<ItemGroup>\n", 1);
        first = false;
        }
      this->WriteString("<None Include=\"", 2);
      (*this->BuildFileStream ) << (*source)->GetFullPath() << "\" />\n";
      }
    }
  if(!first)
    {
    this->WriteString("</ItemGroup>\n", 1); 
    }
}


void cmVisualStudio10TargetGenerator::WriteCLSources()
{
  this->WriteString("<ItemGroup>\n", 1);
  if(this->Target->GetType() > cmTarget::MODULE_LIBRARY)
    {
    this->WriteString("<None Include=\"", 2);
    (*this->BuildFileStream ) << this->Target->GetDirectory()
                              << "\\" << this->Target->GetName() 
                              << "\" />\n";
    }
  else
    {
    std::vector<cmSourceFile*>const & sources = this->Target->GetSourceFiles();
    for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
        source != sources.end(); ++source)
      {
      // if it is not a custom command then add it as a c file,
      // TODO: need to check for idl or rc, or exclude from build
      if(!(*source)->GetCustomCommand()
         && !(*source)->GetPropertyAsBool("HEADER_FILE_ONLY")
         && !this->GlobalGenerator->IgnoreFile
         ((*source)->GetExtension().c_str()))
        {
        const char* lang = (*source)->GetLanguage();
        if(lang && (strcmp(lang, "C") == 0 || strcmp(lang, "CXX") ==0))
          {
          std::string sourceFile = (*source)->GetFullPath();
          // output the source file
          this->WriteString("<ClCompile Include=\"", 2);
          (*this->BuildFileStream ) << sourceFile << "\"";
          // ouput any flags specific to this source file
          if(this->OutputSourceSpecificFlags(*source))
            {
            // if the source file has specific flags the tag
            // is ended on a new line
            this->WriteString("</ClCompile>\n", 2);
            }
          else
            {
            (*this->BuildFileStream ) << " />\n";
            }
          }
        }
      }
    }
  this->WriteString("</ItemGroup>\n", 1);
}

bool cmVisualStudio10TargetGenerator::OutputSourceSpecificFlags(
  cmSourceFile* source)
{
  cmSourceFile& sf = *source;
  std::string flags;
  std::string defines;
  if(const char* cflags = sf.GetProperty("COMPILE_FLAGS"))
    {
    flags += cflags;
    } 
  if(const char* cdefs = sf.GetProperty("COMPILE_DEFINITIONS"))
    {
    defines += cdefs;
    }
  const char* lang =
    this->GlobalGenerator->GetLanguageFromExtension
    (sf.GetExtension().c_str());
  const char* sourceLang = this->LocalGenerator->GetSourceFileLanguage(sf);
  const char* linkLanguage = this->Target->GetLinkerLanguage
    (this->LocalGenerator->GetGlobalGenerator());
  bool needForceLang = false;
  // source file does not match its extension language
  if(lang && sourceLang && strcmp(lang, sourceLang) != 0)
    {
    needForceLang = true;
    lang = sourceLang;
    }  
  // if the source file does not match the linker language
  // then force c or c++
  if(needForceLang || (linkLanguage && lang
                       && strcmp(lang, linkLanguage) != 0))
    {
    if(strcmp(lang, "CXX") == 0)
      {
      // force a C++ file type
      flags += " /TP ";
      }
    else if(strcmp(lang, "C") == 0)
      {
      // force to c
      flags += " /TC ";
      }
    }
  // for the first time we need a new line if there is something
  // produced here.
  const char* firstString = ">\n";
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();
  bool hasFlags = false;
  for( std::vector<std::string>::iterator config = configs->begin();
       config != configs->end(); ++config)
    { 
    std::string configUpper = cmSystemTools::UpperCase(*config);
    std::string configDefines = defines;
    std::string defPropName = "COMPILE_DEFINITIONS_";
    defPropName += configUpper;
    if(const char* ccdefs = sf.GetProperty(defPropName.c_str()))
      {
      if(configDefines.size())
        {
        configDefines += ",";
        }
      configDefines += ccdefs;
      }
    // if we have flags or defines for this config then 
    // use them
    if(flags.size() || configDefines.size())
      {
      (*this->BuildFileStream ) << firstString;
      firstString = ""; // only do firstString once
      hasFlags = true;
      cmVisualStudioGeneratorOptions 
        clOptions(this->LocalGenerator,
                  10, cmVisualStudioGeneratorOptions::Compiler,
                  cmVS10CLFlagTable, 0, this);
      clOptions.Parse(flags.c_str());
      clOptions.AddDefines(configDefines.c_str());
      clOptions.SetConfiguration((*config).c_str());
      clOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");
      clOptions.OutputFlagMap(*this->BuildFileStream, "      "); 
      clOptions.OutputPreprocessorDefinitions(*this->BuildFileStream, "      ", 
                                              "\n");
      
      }
    }
  return hasFlags;
}


void cmVisualStudio10TargetGenerator::WritePathAndIncrementalLinkOptions()
{
  this->WriteString("<PropertyGroup>\n", 2);
  this->WriteString("<_ProjectFileVersion>10.0.20506.1"
                    "</_ProjectFileVersion>\n", 3);
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();
  for(std::vector<std::string>::iterator config = configs->begin();
      config != configs->end(); ++config)
    {
    std::string targetNameFull = 
      this->Target->GetFullName(config->c_str());
    std::string intermediateDir = this->LocalGenerator->
      GetTargetDirectory(*this->Target);
    intermediateDir += "/";
    intermediateDir += *config;
    intermediateDir += "/";
    this->ConvertToWindowsSlash(intermediateDir);
    std::string outDir = this->Target->GetDirectory(config->c_str());
    this->ConvertToWindowsSlash(outDir);
    this->WritePlatformConfigTag("OutDir", config->c_str(), 3);
    *this->BuildFileStream << outDir
                           << "\\"
                           << "</OutDir>\n";
    this->WritePlatformConfigTag("IntDir", config->c_str(), 3); 
    *this->BuildFileStream << intermediateDir
                           << "</IntDir>\n";
    this->WritePlatformConfigTag("TargetName", config->c_str(), 3);
    *this->BuildFileStream << cmSystemTools::GetFilenameWithoutExtension(
      targetNameFull.c_str())
                           << "</TargetName>\n";
    
    this->WritePlatformConfigTag("TargetExt", config->c_str(), 3);
    *this->BuildFileStream << cmSystemTools::GetFilenameLastExtension(
      targetNameFull.c_str())
                           << "</TargetExt>\n";
    this->OutputLinkIncremental(*config);
    }
  this->WriteString("</PropertyGroup>\n", 2);
        
}



void 
cmVisualStudio10TargetGenerator::
OutputLinkIncremental(std::string const& configName)
{ 
  std::string CONFIG = cmSystemTools::UpperCase(configName);
  // static libraries and things greater than modules do not need
  // to set this option
  if(this->Target->GetType() == cmTarget::STATIC_LIBRARY
     || this->Target->GetType() > cmTarget::MODULE_LIBRARY)
    {
    return;
    }
  const char* linkType = "SHARED";
  if(this->Target->GetType() == cmTarget::EXECUTABLE)
    {
    linkType = "EXE";
    }
  
  // assume incremental linking
  const char* incremental = "true";
  const char* linkLanguage = 
    this->Target->GetLinkerLanguage(this->GlobalGenerator);
  if(!linkLanguage)
    {
    cmSystemTools::Error
      ("CMake can not determine linker language for target:",
       this->Target->GetName());
    return;
    }
  std::string linkFlagVarBase = "CMAKE_";
  linkFlagVarBase += linkType;
  linkFlagVarBase += "_LINKER_FLAGS";
  std::string flags = this->
    Target->GetMakefile()->GetRequiredDefinition(linkFlagVarBase.c_str());
  std::string linkFlagVar = linkFlagVarBase + "_" + CONFIG;
  flags += this->
    Target->GetMakefile()->GetRequiredDefinition(linkFlagVar.c_str());
  if(strcmp(linkLanguage, "C") == 0 || strcmp(linkLanguage, "CXX") == 0
     || strcmp(linkLanguage, "Fortran") == 0)
    {
    std::string baseFlagVar = "CMAKE_";
    baseFlagVar += linkLanguage;
    baseFlagVar += "_FLAGS";
    flags += this->
      Target->GetMakefile()->GetRequiredDefinition(baseFlagVar.c_str());
    std::string flagVar = baseFlagVar + std::string("_") + CONFIG;
    flags += 
      Target->GetMakefile()->GetRequiredDefinition(flagVar.c_str());
    }
  if(flags.find("INCREMENTAL:NO") != flags.npos)
    {
    incremental = "false";
    }
  this->WritePlatformConfigTag("LinkIncremental", configName.c_str(), 3);
  *this->BuildFileStream << incremental
                         << "</LinkIncremental>\n"; 
}


void 
cmVisualStudio10TargetGenerator::
WriteClOptions(std::string const& configName,
               std::vector<std::string> const & includes)
{
  
  // much of this was copied from here:
  // copied from cmLocalVisualStudio7Generator.cxx 805

  this->WriteString("<ClCompile>\n", 2);
  cmVisualStudioGeneratorOptions 
    clOptions(this->LocalGenerator,
              10, cmVisualStudioGeneratorOptions::Compiler,
              cmVS10CLFlagTable);
  std::string flags;
  // collect up flags for 
  if(this->Target->GetType() < cmTarget::UTILITY)
    {
    const char* linkLanguage = 
      this->Target->GetLinkerLanguage(this->GlobalGenerator);
    if(!linkLanguage)
      {
      cmSystemTools::Error
        ("CMake can not determine linker language for target:",
         this->Target->GetName());
      return;
      }
    if(strcmp(linkLanguage, "C") == 0 || strcmp(linkLanguage, "CXX") == 0
       || strcmp(linkLanguage, "Fortran") == 0)
      {
      std::string baseFlagVar = "CMAKE_";
      baseFlagVar += linkLanguage;
      baseFlagVar += "_FLAGS";
      flags = this->
        Target->GetMakefile()->GetRequiredDefinition(baseFlagVar.c_str());
      std::string flagVar = baseFlagVar + std::string("_") +
        cmSystemTools::UpperCase(configName);
      flags += " ";
      flags += this->
        Target->GetMakefile()->GetRequiredDefinition(flagVar.c_str());
      }
    // set the correct language
    if(strcmp(linkLanguage, "C") == 0)
      {
      flags += " /TC ";
      }
    if(strcmp(linkLanguage, "CXX") == 0)
      {
      flags += " /TP ";
      }
    }
  std::string configUpper = cmSystemTools::UpperCase(configName);
  std::string defPropName = "COMPILE_DEFINITIONS_";
  defPropName += configUpper;

  // Get preprocessor definitions for this directory.
  std::string defineFlags = this->Target->GetMakefile()->GetDefineFlags();
  clOptions.FixExceptionHandlingDefault();
  clOptions.Parse(flags.c_str());
  clOptions.Parse(defineFlags.c_str());
  clOptions.AddDefines
    (this->Makefile->GetProperty("COMPILE_DEFINITIONS"));
  clOptions.AddDefines(this->Target->GetProperty("COMPILE_DEFINITIONS"));
  clOptions.AddDefines(this->Makefile->GetProperty(defPropName.c_str()));
  clOptions.AddDefines(this->Target->GetProperty(defPropName.c_str()));
  clOptions.SetVerboseMakefile(
    this->Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"));

  // Add a definition for the configuration name.
  std::string configDefine = "CMAKE_INTDIR=\"";
  configDefine += configName;
  configDefine += "\"";
  clOptions.AddDefine(configDefine);
  if(const char* exportMacro = this->Target->GetExportMacro())
    {
    clOptions.AddDefine(exportMacro);
    }
  clOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");
  this->OutputIncludes(includes);
  clOptions.OutputFlagMap(*this->BuildFileStream, "      ");
  clOptions.OutputPreprocessorDefinitions(*this->BuildFileStream, "      ", 
                                          "\n");
  this->WriteString("<AssemblerListingLocation>", 3);
  *this->BuildFileStream << configName 
                         << "</AssemblerListingLocation>\n";
  this->WriteString("<ObjectFileName>$(IntDir)</ObjectFileName>\n", 3);
  this->WriteString("<ProgramDataBaseFileName>", 3);
  *this->BuildFileStream << this->Target->GetDirectory(configName.c_str())
                         << "/" 
                         << this->Target->GetPDBName(configName.c_str())
                         << "</ProgramDataBaseFileName>\n";
  this->WriteString("</ClCompile>\n", 2);
}

void cmVisualStudio10TargetGenerator::
OutputIncludes(std::vector<std::string> const & includes)
{
  this->WriteString("<AdditionalIncludeDirectories>", 3);
  for(std::vector<std::string>::const_iterator i =  includes.begin();
      i != includes.end(); ++i)
    {
    *this->BuildFileStream << *i << ";";
    }
  this->WriteString("%(AdditionalIncludeDirectories)"
                    "</AdditionalIncludeDirectories>\n", 0);
}
  


void cmVisualStudio10TargetGenerator::
WriteRCOptions(std::string const& ,
               std::vector<std::string> const & includes)
{
  this->WriteString("<ResourceCompile>\n", 2);
  this->OutputIncludes(includes);
  this->WriteString("</ResourceCompile>\n", 2);
}


void cmVisualStudio10TargetGenerator::WriteLinkOptions(std::string const&
                                                       config)
{
  
  // static libraries and things greater than modules do not need
  // to set this option
  if(this->Target->GetType() == cmTarget::STATIC_LIBRARY
     || this->Target->GetType() > cmTarget::MODULE_LIBRARY)
    {
    return;
    }
  const char* linkLanguage = 
    this->Target->GetLinkerLanguage(this->GlobalGenerator);
  if(!linkLanguage)
    {
    cmSystemTools::Error
      ("CMake can not determine linker language for target:",
       this->Target->GetName());
    return;
    }

  this->WriteString("<Link>\n", 2);
  std::string CONFIG = cmSystemTools::UpperCase(config);
  
  const char* linkType = "SHARED";
  if(this->Target->GetType() == cmTarget::MODULE_LIBRARY)
    {
    linkType = "MODULE";
    }
  if(this->Target->GetType() == cmTarget::EXECUTABLE)
    {
    linkType = "EXE";
    }
  std::string stackVar = "CMAKE_";
  stackVar += linkLanguage;
  stackVar += "_STACK_SIZE";
  const char* stackVal = this->Makefile->GetDefinition(stackVar.c_str());
  std::string flags;
  if(stackVal)
    {
    flags += " ";
    flags += stackVal;
    }
  // assume incremental linking
  std::string linkFlagVarBase = "CMAKE_";
  linkFlagVarBase += linkType;
  linkFlagVarBase += "_LINKER_FLAGS";
  flags += " ";
  flags += this->
    Target->GetMakefile()->GetRequiredDefinition(linkFlagVarBase.c_str());
  std::string linkFlagVar = linkFlagVarBase + "_" + CONFIG;
  flags += " ";
  flags += this->
    Target->GetMakefile()->GetRequiredDefinition(linkFlagVar.c_str());
  const char* targetLinkFlags = this->Target->GetProperty("LINK_FLAGS");
  if(targetLinkFlags)
    {
    flags += " ";
    flags += targetLinkFlags;
    }
  cmVisualStudioGeneratorOptions 
    linkOptions(this->LocalGenerator, 
              10, cmVisualStudioGeneratorOptions::Compiler,
                cmVS10LinkFlagTable);
  if ( this->Target->GetPropertyAsBool("WIN32_EXECUTABLE") )
    {
    flags += " /SUBSYSTEM:WINDOWS";
    }
  else
    {
    flags += " /SUBSYSTEM:CONSOLE";
    }
  cmSystemTools::ReplaceString(flags, "/INCREMENTAL:YES", "");
  cmSystemTools::ReplaceString(flags, "/INCREMENTAL:NO", "");
  std::string standardLibsVar = "CMAKE_";
  standardLibsVar += linkLanguage;
  standardLibsVar += "_STANDARD_LIBRARIES";
  std::string 
    libs = this->Makefile->GetSafeDefinition(standardLibsVar.c_str());
  // Remove trailing spaces from libs
  std::string::size_type pos = libs.size()-1;
  if(libs.size() != 0)
    {
    while(libs[pos] == ' ')
      {
      pos--;
      }
    }
  if(pos != libs.size()-1)
    {
    libs = libs.substr(0, pos+1);
    }
  // Replace spaces in libs with ;
  cmSystemTools::ReplaceString(libs, " ", ";");
  cmComputeLinkInformation* pcli =
    this->Target->GetLinkInformation(config.c_str());
  if(!pcli)
    {
    cmSystemTools::Error
      ("CMake can not compute cmComputeLinkInformation for target:",
       this->Target->GetName());
    return;
    }
  // add the libraries for the target to libs string
  cmComputeLinkInformation& cli = *pcli;
  this->AddLibraries(cli, libs);
  linkOptions.AddFlag("AdditionalDependencies", libs.c_str());

  std::vector<std::string> const& ldirs = cli.GetDirectories();
  const char* sep = "";
  std::string linkDirs;
  for(std::vector<std::string>::const_iterator d = ldirs.begin();
      d != ldirs.end(); ++d)
    {
    linkDirs += sep;
    linkDirs += *d;
    sep = ";";
    }
  linkDirs += "%(AdditionalLibraryDirectories)";
  linkOptions.AddFlag("AdditionalLibraryDirectories", linkDirs.c_str());
  linkOptions.AddFlag("AdditionalDependencies", libs.c_str());
  linkOptions.AddFlag("Version", "0.0");
  if(linkOptions.IsDebug() || flags.find("/debug") != flags.npos)
    {
    linkOptions.AddFlag("GenerateDebugInformation", "true");
    }
  else
    {
    linkOptions.AddFlag("GenerateDebugInformation", "false");
    } 
  std::string targetName;
  std::string targetNameSO;
  std::string targetNameFull;
  std::string targetNameImport;
  std::string targetNamePDB;
  if(this->Target->GetType() == cmTarget::EXECUTABLE)
    {
    this->Target->GetExecutableNames(targetName, targetNameFull,
                                     targetNameImport, targetNamePDB, 
                                     config.c_str());
    }
  else
    {
    this->Target->GetLibraryNames(targetName, targetNameSO, targetNameFull,
                                  targetNameImport, targetNamePDB, 
                                  config.c_str());
    }
  std::string dir = this->Target->GetDirectory(config.c_str());
  dir += "/";
  std::string imLib = dir;
  imLib += targetNameImport;
  std::string pdb = dir;
  pdb += targetNamePDB;
  linkOptions.AddFlag("ImportLibrary", imLib.c_str());
  linkOptions.AddFlag("ProgramDataBaseFileName", pdb.c_str());
  linkOptions.Parse(flags.c_str());
  linkOptions.OutputAdditionalOptions(*this->BuildFileStream, "      ", "");
  linkOptions.OutputFlagMap(*this->BuildFileStream, "      ");
  
  this->WriteString("</Link>\n", 2);
}

void cmVisualStudio10TargetGenerator::AddLibraries(
  cmComputeLinkInformation& cli,
  std::string& libstring)
{ 
  typedef cmComputeLinkInformation::ItemVector ItemVector;
  ItemVector libs = cli.GetItems();
  const char* sep = ";";
  for(ItemVector::const_iterator l = libs.begin(); l != libs.end(); ++l)
    {
    if(l->IsPath)
      {
      std::string path = this->LocalGenerator->
        Convert(l->Value.c_str(),
                cmLocalGenerator::START_OUTPUT,
                cmLocalGenerator::UNCHANGED);
      libstring += sep;
      libstring += path;
      }
    else
      {
      libstring += sep;
      libstring += l->Value;
      }
    }
}


void cmVisualStudio10TargetGenerator::
WriteMidlOptions(std::string const& /*config*/,
                 std::vector<std::string> const & includes)
{
  this->WriteString("<Midl>\n", 2);
  this->OutputIncludes(includes);
  // Need this stuff, but there is an midl.xml file...
  // should we look for .idl language?, and flags?
  /*
       <MkTypLibCompatible>false</MkTypLibCompatible>
      <TargetEnvironment>Win32</TargetEnvironment>
      <GenerateStublessProxies>true</GenerateStublessProxies>
      <TypeLibraryName>%(FileName).tlb</TypeLibraryName>
      <OutputDirectory>$(IntDir)\</OutputDirectory>
      <HeaderFileName>%(FileName).h</HeaderFileName>
      <DllDataFileName>
      </DllDataFileName>
      <InterfaceIdentifierFileName>%(FileName)_i.c</InterfaceIdentifierFileName>
      <ProxyFileName>%(FileName)_p.c</ProxyFileName>
  */
  this->WriteString("</Midl>\n", 2);
}
  
void cmVisualStudio10TargetGenerator::WriteItemDefinitionGroups()
{  
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();
  std::vector<std::string> includes;
  this->LocalGenerator->GetIncludeDirectories(includes);
  for(std::vector<std::string>::iterator i = configs->begin();
      i != configs->end(); ++i)
    {
    this->WritePlatformConfigTag("ItemDefinitionGroup", i->c_str(), 1);
    *this->BuildFileStream << "\n";
    //    output cl compile flags <ClCompile></ClCompile>
    if(this->Target->GetType() <= cmTarget::MODULE_LIBRARY)
      {
      this->WriteClOptions(*i, includes);
      //    output rc compile flags <ResourceCompile></ResourceCompile>
      this->WriteRCOptions(*i, includes);
      }
    //    output midl flags       <Midl></Midl>
    this->WriteMidlOptions(*i, includes);
    //    output link flags       <Link></Link> or <Lib></Lib>
    this->WriteLinkOptions(*i);
    // TODO:  need a WriteLibOptions for static
    this->WriteString("</ItemDefinitionGroup>\n", 1);
    }
}


void cmVisualStudio10TargetGenerator::WriteProjectReferences()
{
  // TODO
  // This should have dependent targets listed like this:
  /*
 <ItemGroup>
    <ProjectReference Include="ZERO_CHECK.vcxproj">
      <Project>{2f1e4f3c-0a51-46c3-aaf9-e486599604f2}</Project>
    </ProjectReference>
  </ItemGroup>
  */
}
