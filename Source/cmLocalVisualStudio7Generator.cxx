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
#include "cmGlobalVisualStudio7Generator.h"
#include "cmLocalVisualStudio7Generator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"
#include "cmSourceFile.h"
#include "cmCacheManager.h"
#include "cmake.h"

#include <ctype.h> // for isspace

cmLocalVisualStudio7Generator::cmLocalVisualStudio7Generator()
{
  this->Version = 7;
  this->PlatformName = "Win32";
}

cmLocalVisualStudio7Generator::~cmLocalVisualStudio7Generator()
{
}


void cmLocalVisualStudio7Generator::Generate()
{
  std::set<cmStdString> lang;
  lang.insert("C");
  lang.insert("CXX");
  lang.insert("RC");
  lang.insert("IDL");
  lang.insert("DEF");
  this->CreateCustomTargetsAndCommands(lang);
  this->FixGlobalTargets();
  this->OutputVCProjFile();
}

void cmLocalVisualStudio7Generator::FixGlobalTargets()
{
  // Visual Studio .NET 2003 Service Pack 1 will not run post-build
  // commands for targets in which no sources are built.  Add dummy
  // rules to force these targets to build.
  cmTargets &tgts = this->Makefile->GetTargets();
  for(cmTargets::iterator l = tgts.begin();
      l != tgts.end(); l++)
    {
    cmTarget& tgt = l->second;
    if(tgt.GetType() == cmTarget::GLOBAL_TARGET)
      {
      std::vector<std::string> no_depends;
      cmCustomCommandLine force_command;
      force_command.push_back(";");
      cmCustomCommandLines force_commands;
      force_commands.push_back(force_command);
      const char* no_main_dependency = 0;
      std::string force = this->Makefile->GetStartOutputDirectory();
      force += cmake::GetCMakeFilesDirectory();
      force += "/";
      force += tgt.GetName();
      force += "_force";
      this->Makefile->AddCustomCommandToOutput(force.c_str(), no_depends,
                                               no_main_dependency,
                                               force_commands, " ", 0, true);
      if(cmSourceFile* file =
         this->Makefile->GetSourceFileWithOutput(force.c_str()))
        {
        tgt.GetSourceFiles().push_back(file);
        }
      }
    }
}

// TODO
// for CommandLine= need to repleace quotes with &quot
// write out configurations
void cmLocalVisualStudio7Generator::OutputVCProjFile()
{
  // If not an in source build, then create the output directory
  if(strcmp(this->Makefile->GetStartOutputDirectory(),
            this->Makefile->GetHomeDirectory()) != 0)
    {
    if(!cmSystemTools::MakeDirectory
       (this->Makefile->GetStartOutputDirectory()))
      {
      cmSystemTools::Error("Error creating directory ",
                           this->Makefile->GetStartOutputDirectory());
      }
    }

  this->LibraryOutputPath = "";
  if (this->Makefile->GetDefinition("LIBRARY_OUTPUT_PATH"))
    {
    this->LibraryOutputPath = 
      this->Makefile->GetDefinition("LIBRARY_OUTPUT_PATH");
    }
  if(this->LibraryOutputPath.size())
    {
    // make sure there is a trailing slash
    if(this->LibraryOutputPath[this->LibraryOutputPath.size()-1] != '/')
      {
      this->LibraryOutputPath += "/";
      }
    }
  this->ExecutableOutputPath = "";
  if (this->Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH"))
    {
    this->ExecutableOutputPath = 
      this->Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH");
    }
  if(this->ExecutableOutputPath.size())
    {
    // make sure there is a trailing slash
    if(this->ExecutableOutputPath[this->ExecutableOutputPath.size()-1] != '/')
      {
      this->ExecutableOutputPath += "/";
      }
    }

  // Create the VCProj or set of VCProj's for libraries and executables

  // clear project names
  this->CreatedProjectNames.clear();

  // Call TraceVSDependencies on all targets
  cmTargets &tgts = this->Makefile->GetTargets();
  for(cmTargets::iterator l = tgts.begin();
      l != tgts.end(); l++)
    {
    // Add a rule to regenerate the build system when the target
    // specification source changes.
    const char* suppRegenRule =
      this->Makefile->GetDefinition("CMAKE_SUPPRESS_REGENERATION");
    if (!cmSystemTools::IsOn(suppRegenRule) &&
        (strcmp(l->first.c_str(), CMAKE_CHECK_BUILD_SYSTEM_TARGET) != 0))
      {
      this->AddVCProjBuildRule(l->second);
      }

    // INCLUDE_EXTERNAL_MSPROJECT command only affects the workspace
    // so don't build a projectfile for it
    if ((l->second.GetType() != cmTarget::INSTALL_FILES)
        && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS)
        && (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) != 0))
      {
      cmTarget& target = l->second;
      target.TraceVSDependencies(target.GetName(), this->Makefile);
      }
    }

  for(cmTargets::iterator l = tgts.begin();
      l != tgts.end(); l++)
    {
    // INCLUDE_EXTERNAL_MSPROJECT command only affects the workspace
    // so don't build a projectfile for it
    if ((l->second.GetType() != cmTarget::INSTALL_FILES)
        && (l->second.GetType() != cmTarget::INSTALL_PROGRAMS)
        && (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) != 0))
      {
      this->CreateSingleVCProj(l->first.c_str(),l->second);
      }
    }
}

void cmLocalVisualStudio7Generator
::CreateSingleVCProj(const char *lname, cmTarget &target)
{
  // add to the list of projects
  std::string pname = lname;
  this->CreatedProjectNames.push_back(pname);
  // create the dsp.cmake file
  std::string fname;
  fname = this->Makefile->GetStartOutputDirectory();
  fname += "/";
  fname += lname;
  fname += ".vcproj";
  // save the name of the real dsp file
  std::string realVCProj = fname;
  fname += ".cmake";
  std::ofstream fout(fname.c_str());
  if(!fout)
    {
    cmSystemTools::Error("Error Writing ", fname.c_str());
    }
  this->WriteVCProjFile(fout,lname,target);
  fout.close();
  // if the dsp file has changed, then write it.
  cmSystemTools::CopyFileIfDifferent(fname.c_str(), realVCProj.c_str());
}


void cmLocalVisualStudio7Generator::AddVCProjBuildRule(cmTarget& tgt)
{
  std::string dspname = tgt.GetName();
  dspname += ".vcproj.cmake";
  const char* dsprule = 
    this->Makefile->GetRequiredDefinition("CMAKE_COMMAND");
  cmCustomCommandLine commandLine;
  commandLine.push_back(dsprule);
  std::string makefileIn = this->Makefile->GetStartDirectory();
  makefileIn += "/";
  makefileIn += "CMakeLists.txt";
  makefileIn = cmSystemTools::CollapseFullPath(makefileIn.c_str());
  std::string comment = "Building Custom Rule ";
  comment += makefileIn;
  std::string args;
  args = "-H";
  args += this->Convert(this->Makefile->GetHomeDirectory(), 
                        START_OUTPUT, UNCHANGED, true);
  commandLine.push_back(args);
  args = "-B";
  args +=
    this->Convert(this->Makefile->GetHomeOutputDirectory(),
                  START_OUTPUT, UNCHANGED, true);
  commandLine.push_back(args);

  std::string configFile =
    this->Makefile->GetRequiredDefinition("CMAKE_ROOT");
  configFile += "/Templates/CMakeWindowsSystemConfig.cmake";
  std::vector<std::string> listFiles = this->Makefile->GetListFiles();
  bool found = false;
  for(std::vector<std::string>::iterator i = listFiles.begin();
      i != listFiles.end(); ++i)
    {
    if(*i == configFile)
      {
      found  = true;
      }
    }
  if(!found)
    {
    listFiles.push_back(configFile);
    }

  cmCustomCommandLines commandLines;
  commandLines.push_back(commandLine);
  const char* no_working_directory = 0;
  this->Makefile->AddCustomCommandToOutput(dspname.c_str(), listFiles,
                                           makefileIn.c_str(), commandLines,
                                           comment.c_str(),
                                           no_working_directory, true);
  if(cmSourceFile* file = this->Makefile->GetSource(makefileIn.c_str()))
    {
    tgt.GetSourceFiles().push_back(file);
    }
  else
    {
    cmSystemTools::Error("Error adding rule for ", makefileIn.c_str());
    }
}


void cmLocalVisualStudio7Generator::WriteConfigurations(std::ostream& fout,
                                                        const char *libName,
                                                        cmTarget &target)
{
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();

  fout << "\t<Configurations>\n";
  for( std::vector<std::string>::iterator i = configs->begin();
       i != configs->end(); ++i)
    {
    this->WriteConfiguration(fout, i->c_str(), libName, target);
    }
  fout << "\t</Configurations>\n";
}

// This is a table mapping XML tag IDE names to command line options
struct cmVS7FlagTable
{
  const char* IDEName;  // name used in the IDE xml file
  const char* commandFlag; // command line flag
  const char* comment;     // comment
  const char* value; // string value
};

// fill the table here currently the comment field is not used for
// anything other than documentation NOTE: Make sure the longer
// commandFlag comes FIRST!
cmVS7FlagTable cmLocalVisualStudio7GeneratorFlagTable[] =
{
  // option flags (some flags map to the same option)
  {"BasicRuntimeChecks", "GZ", "Stack frame checks",                      "1"},
  {"BasicRuntimeChecks", "RTCsu", "Both stack and uninitialized checks", "3"},
  {"BasicRuntimeChecks", "RTCs", "Stack frame checks",                    "1"},
  {"BasicRuntimeChecks", "RTCu", "Uninitialized Variables ",              "2"},
  {"BasicRuntimeChecks", "RTC1", "Both stack and uninitialized checks ",  "3"},
  {"DebugInformationFormat", "Z7", "debug format", "1"},
  {"DebugInformationFormat", "Zd", "debug format", "2"},
  {"DebugInformationFormat", "Zi", "debug format", "3"},
  {"DebugInformationFormat", "ZI", "debug format", "4"},
  {"EnableEnhancedInstructionSet", "arch:SSE2", "Use sse2 instructions", "2"},
  {"EnableEnhancedInstructionSet", "arch:SSE", "Use sse instructions",   "1"},
  {"FavorSizeOrSpeed",  "Ot", "Favor fast code",  "1"},
  {"FavorSizeOrSpeed",  "Os", "Favor small code", "2"},
  {"CompileAs", "TC", "Compile as c code",        "1"},
  {"CompileAs", "TP", "Compile as c++ code",      "2"},
  {"Optimization", "Od", "Non Debug",        "0"},
  {"Optimization", "O1", "Min Size",         "1"},
  {"Optimization", "O2", "Max Speed",        "2"},
  {"Optimization", "Ox", "Max Optimization", "3"},
  {"OptimizeForProcessor", "GB", "Blended processor mode", "0"},
  {"OptimizeForProcessor", "G5", "Pentium",                "1"},
  {"OptimizeForProcessor", "G6", "PPro PII PIII",          "2"},
  {"OptimizeForProcessor", "G7", "Pentium 4 or Athlon",    "3"},
  {"InlineFunctionExpansion", "Ob0", "no inlines",              "0"},
  {"InlineFunctionExpansion", "Ob1", "when inline keyword",     "1"},
  {"InlineFunctionExpansion", "Ob2", "any time you can inline", "2"},
  {"RuntimeLibrary", "MTd", "Multithreded debug",     "1"},
  {"RuntimeLibrary", "MT", "Multithreded", "0"},
  {"RuntimeLibrary", "MDd", "Multithreded dll debug", "3"},
  {"RuntimeLibrary", "MD", "Multithreded dll",        "2"},
  {"RuntimeLibrary", "MLd", "Sinble Thread debug",    "5"},
  {"RuntimeLibrary", "ML", "Sinble Thread",           "4"},
  {"StructMemberAlignment", "Zp16", "struct align 16 byte ",   "5"},
  {"StructMemberAlignment", "Zp1", "struct align 1 byte ",     "1"},
  {"StructMemberAlignment", "Zp2", "struct align 2 byte ",     "2"},
  {"StructMemberAlignment", "Zp4", "struct align 4 byte ",     "3"},
  {"StructMemberAlignment", "Zp8", "struct align 8 byte ",     "4"},
  {"WarningLevel", "W1", "Warning level", "1"},
  {"WarningLevel", "W2", "Warning level", "2"},
  {"WarningLevel", "W3", "Warning level", "3"},
  {"WarningLevel", "W4", "Warning level", "4"},

  // boolean flags
  {"BufferSecurityCheck", "GS", "Buffer security check", "TRUE"},
  {"EnableFibreSafeOptimization", "GT", "OmitFramePointers", "TRUE"},
  {"EnableFunctionLevelLinking", "Gy", "EnableFunctionLevelLinking", "TRUE"},
  {"EnableIntrinsicFunctions", "Oi", "EnableIntrinsicFunctions", "TRUE"},
  {"ExceptionHandling", "EHsc", "enable c++ exceptions", "TRUE"},
  {"ExceptionHandling", "EHa", "enable c++ exceptions", "2"},
  {"ExceptionHandling", "GX", "enable c++ exceptions", "TRUE"},
  {"GlobalOptimizations", "Og", "Global Optimize", "TRUE"},
  {"ImproveFloatingPointConsistency", "Op", 
   "ImproveFloatingPointConsistency", "TRUE"},
  {"MinimalRebuild", "Gm", "minimal rebuild", "TRUE"},
  {"OmitFramePointers", "Oy", "OmitFramePointers", "TRUE"},
  {"OptimizeForWindowsApplication", "GA", "Optimize for windows", "TRUE"},
  {"RuntimeTypeInfo", "GR", 
   "Turn on Run time type information for c++", "TRUE"},
  {"SmallerTypeCheck", "RTCc", "smaller type check", "TRUE"},
  {"SuppressStartupBanner", "nologo", "SuppressStartupBanner", "TRUE"},
  {"WarnAsError", "WX", "Treat warnings as errors", "TRUE"},
  {0,0,0,0 }
};


cmVS7FlagTable cmLocalVisualStudio7GeneratorLinkFlagTable[] =
{
  // option flags (some flags map to the same option)
  {"GenerateManifest", "MANIFEST:NO", "disable manifest generation", "FALSE"},
  {"GenerateManifest", "MANIFEST", "enable manifest generation", "TRUE"},
  {"LinkIncremental", "INCREMENTAL:NO", "link incremental", "1"},
  {"LinkIncremental", "INCREMENTAL:YES", "link incremental", "2"},
  {0,0,0,0 }
};








void cmLocalVisualStudio7Generator::WriteConfiguration(std::ostream& fout,
                                                       const char* configName,
                                                       const char *libName,
                                                       cmTarget &target)
{
  // create a map of xml tags to the values they should have in the output
  // for example, "BufferSecurityCheck" = "TRUE"
  // first fill this table with the values for the configuration
  // Debug, Release, etc,
  // Then parse the command line flags specified in CMAKE_CXX_FLAGS
  // and CMAKE_C_FLAGS
  // and overwrite or add new values to this map
  std::map<cmStdString, cmStdString> flagMap;
  // since the default is on for this, but if /EHsc is found
  // in the flags it will be turned on and we have /EHSC on by
  // default in the CXX flags, then this is the only way to turn this off
  flagMap["ExceptionHandling"] = "FALSE";
  const char* mfcFlag = this->Makefile->GetDefinition("CMAKE_MFC_FLAG");
  if(!mfcFlag)
    {
    mfcFlag = "0";
    }
  fout << "\t\t<Configuration\n"
       << "\t\t\tName=\"" << configName << "|" << this->PlatformName << "\"\n"
       << "\t\t\tOutputDirectory=\"" << configName << "\"\n";
  // This is an internal type to Visual Studio, it seems that:
  // 4 == static library
  // 2 == dll
  // 1 == executable
  // 10 == utility
  const char* configType = "10";
  switch(target.GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      configType = "4";
      break;
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
      configType = "2";
      break;
    case cmTarget::EXECUTABLE:
      configType = "1";
      break;
    case cmTarget::UTILITY:
    case cmTarget::GLOBAL_TARGET:
      configType = "10";
    default:
      break;
    }

  std::string flags;
  if(strcmp(configType, "10") != 0)
    {
    const char* linkLanguage = 
      target.GetLinkerLanguage(this->GetGlobalGenerator());
    if(!linkLanguage)
      {
      cmSystemTools::Error
        ("CMake can not determine linker language for target:",
                           target.GetName());
      return;
      }
    if(strcmp(linkLanguage, "C") == 0 || strcmp(linkLanguage, "CXX") == 0)
      {
      std::string baseFlagVar = "CMAKE_";
      baseFlagVar += linkLanguage;
      baseFlagVar += "_FLAGS";
      flags = this->Makefile->GetRequiredDefinition(baseFlagVar.c_str());
      std::string flagVar = baseFlagVar + std::string("_") +
        cmSystemTools::UpperCase(configName);
      flags += " ";
      flags += this->Makefile->GetRequiredDefinition(flagVar.c_str());
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

  // Add the target-specific flags.
  if(const char* targetFlags = target.GetProperty("COMPILE_FLAGS"))
    {
    flags += " ";
    flags += targetFlags;
    }

  // The intermediate directory name consists of a directory for the
  // target and a subdirectory for the configuration name.
  std::string intermediateDir = this->GetTargetDirectory(target);
  intermediateDir += "/";
  intermediateDir += configName;
  fout << "\t\t\tIntermediateDirectory=\""
       << this->ConvertToXMLOutputPath(intermediateDir.c_str())
       << "\"\n"
       << "\t\t\tConfigurationType=\"" << configType << "\"\n"
       << "\t\t\tUseOfMFC=\"" << mfcFlag << "\"\n"
       << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"\n";
  // if -D_UNICODE or /D_UNICODE is found in the flags
  // change the character set to unicode, if not then
  // default to MBCS
  std::string defs = this->Makefile->GetDefineFlags();
  if(flags.find("D_UNICODE") != flags.npos ||
     defs.find("D_UNICODE") != flags.npos)
    {
    fout << "\t\t\tCharacterSet=\"1\">\n";
    }
  else
    {
    fout << "\t\t\tCharacterSet=\"2\">\n";
    }

  fout << "\t\t\t<Tool\n"
       << "\t\t\t\tName=\"VCCLCompilerTool\"\n"
       << "\t\t\t\tAdditionalOptions=\"";
  // now fill the flagMap from the command line flags, and
  // if a flag is used, it will be removed from the flags string by
  // this function call
  this->FillFlagMapFromCommandFlags
    (flagMap, &cmLocalVisualStudio7GeneratorFlagTable[0], flags);
  std::string defineFlags = this->Makefile->GetDefineFlags();

  // now check the define flags for flags other than -D and
  // put them in the map, the -D flags will be left in the defineFlags
  // variable as -D is not in the flagMap
  this->FillFlagMapFromCommandFlags
    (flagMap, &cmLocalVisualStudio7GeneratorFlagTable[0], defineFlags);

  // output remaining flags that were not mapped to anything
  fout << this->EscapeForXML(flags.c_str()).c_str();
  fout << " -DCMAKE_INTDIR=\\&quot;" << configName << "\\&quot;"
       << "\"\n";
  fout << "\t\t\t\tAdditionalIncludeDirectories=\"";
  std::vector<std::string> includes;
  this->GetIncludeDirectories(includes);
  std::vector<std::string>::iterator i = includes.begin();
  for(;i != includes.end(); ++i)
    {
    std::string ipath = this->ConvertToXMLOutputPath(i->c_str());
    fout << ipath << ";";
    }
  fout << "\"\n";
  // set a few cmake specific flags
  if(this->Makefile->IsOn("CMAKE_CXX_USE_RTTI"))
    {
    flagMap["RuntimeTypeInfo"] = "TRUE";
    }
  if ( this->Makefile->GetDefinition("CMAKE_CXX_WARNING_LEVEL") )
    {
    flagMap["WarningLevel"] = 
      this->Makefile->GetDefinition("CMAKE_CXX_WARNING_LEVEL");
    }

  // Now copy the flag map into the xml for the file
  for(std::map<cmStdString, cmStdString>::iterator m = flagMap.begin();
      m != flagMap.end(); ++m)
    {
    fout << "\t\t\t\t" << m->first << "=\"" << m->second << "\"\n";
    }
  fout << "\t\t\t\tPreprocessorDefinitions=\"";
  if(target.GetType() == cmTarget::SHARED_LIBRARY
     || target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    std::string exportSymbol;
    if (const char* custom_export_name = 
        target.GetProperty("DEFINE_SYMBOL"))
      {
      exportSymbol = custom_export_name;
      }
    else
      {
      std::string id = libName;
      id += "_EXPORTS";
      exportSymbol = cmSystemTools::MakeCindentifier(id.c_str());
      }
    fout << "," << exportSymbol;
    }
  this->OutputDefineFlags(defineFlags.c_str(), fout);
  fout << "\"\n";
  fout << "\t\t\t\tAssemblerListingLocation=\"" << configName << "\"\n";
  fout << "\t\t\t\tObjectFile=\"$(IntDir)\\\"\n";
  std::map<cmStdString, cmStdString>::iterator mi = 
    flagMap.find("DebugInformationFormat");
  if(mi != flagMap.end() && mi->second != "1")
    {
    fout <<  "\t\t\t\tProgramDataBaseFileName=\""
         << this->LibraryOutputPath
         << "$(OutDir)/" << libName << ".pdb\"\n";
    }
  fout << "/>\n";  // end of <Tool Name=VCCLCompilerTool
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCCustomBuildTool\"/>\n";
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCResourceCompilerTool\"\n"
       << "\t\t\t\tAdditionalIncludeDirectories=\"";
  for(i = includes.begin();i != includes.end(); ++i)
    {
    std::string ipath = this->ConvertToXMLOutputPath(i->c_str());
    fout << ipath << ";";
    }
  // add the -D flags to the RC tool
  fout << "\"\n"
       << "\t\t\t\tPreprocessorDefinitions=\"";
  this->OutputDefineFlags(defineFlags.c_str(), fout);
  fout << "\" />\n";

  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCMIDLTool\"\n";
  fout << "\t\t\t\tPreprocessorDefinitions=\"";
  this->OutputDefineFlags(defineFlags.c_str(), fout);
  fout << "\"\n";
  fout << "\t\t\t\tMkTypLibCompatible=\"FALSE\"\n";
  if( this->PlatformName == "x64" )
    {
    fout << "\t\t\t\tTargetEnvironment=\"3\"\n";
    }
  else if( this->PlatformName == "ia64" )
    {
    fout << "\t\t\t\tTargetEnvironment=\"2\"\n";
    }
  else
    {
    fout << "\t\t\t\tTargetEnvironment=\"1\"\n";
    }
  fout << "\t\t\t\tGenerateStublessProxies=\"TRUE\"\n";
  fout << "\t\t\t\tTypeLibraryName=\"$(InputName).tlb\"\n";
  fout << "\t\t\t\tOutputDirectory=\"$(IntDir)\"\n";
  fout << "\t\t\t\tHeaderFileName=\"$(InputName).h\"\n";
  fout << "\t\t\t\tDLLDataFileName=\"\"\n";
  fout << "\t\t\t\tInterfaceIdentifierFileName=\"$(InputName)_i.c\"\n";
  fout << "\t\t\t\tProxyFileName=\"$(InputName)_p.c\"/>\n";
  // end of <Tool Name=VCMIDLTool

  // If we are building a version 8 project file, add a flag telling the
  // manifest tool to use a workaround for FAT32 file systems, which can cause
  // an empty manifest to be embedded into the resulting executable.
  // See CMake bug #2617.
  if ( this->Version == 8 )
    {
    fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCManifestTool\"\n"
         << "\t\t\t\tUseFAT32Workaround=\"true\"\n"
         << "\t\t\t/>\n";
    }

  this->OutputTargetRules(fout, target, libName);
  this->OutputBuildTool(fout, configName, libName, target);
  fout << "\t\t</Configuration>\n";
}

void cmLocalVisualStudio7Generator::FillFlagMapFromCommandFlags(
  std::map<cmStdString, cmStdString>& flagMap,
  cmVS7FlagTable* flagTable,
  std::string& flags)
{
  std::string replace;
  std::string option;
  while(flagTable->IDEName)
    {
    option.reserve(strlen(flagTable->commandFlag)+2);
    // first do the - version
    option = "-";
    option += flagTable->commandFlag;
    while(flags.find(option) != flags.npos)
      {
      // replace the flag
      cmSystemTools::ReplaceString(flags, option.c_str(), "");
      // now put value into flag map
      flagMap[flagTable->IDEName] = flagTable->value;
      }
    // now do the / version
    option[0] = '/';
    while(flags.find(option) != flags.npos)
      {
      // replace the flag
      cmSystemTools::ReplaceString(flags, option.c_str(), "");
      // now put value into flag map
      flagMap[flagTable->IDEName] = flagTable->value;
      }
    // move to next flag
    flagTable++;
    }

  // If verbose makefiles have been requested and the /nologo option
  // was not given explicitly in the flags we want to add an attribute
  // to the generated project to disable logo suppression.  Otherwise
  // the GUI default is to enable suppression.
  if(this->Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"))
    {
    if(flagMap.find("SuppressStartupBanner") == flagMap.end())
      {
      flagMap["SuppressStartupBanner"] = "FALSE";
      }
    }
}

//----------------------------------------------------------------------------
std::string
cmLocalVisualStudio7Generator
::GetBuildTypeLinkerFlags(std::string rootLinkerFlags, const char* configName)
{
  std::string configTypeUpper = cmSystemTools::UpperCase(configName);
  std::string extraLinkOptionsBuildTypeDef = 
    rootLinkerFlags + "_" + configTypeUpper;

  std::string extraLinkOptionsBuildType =
    this->Makefile->GetRequiredDefinition
    (extraLinkOptionsBuildTypeDef.c_str());

  return extraLinkOptionsBuildType;
}

void cmLocalVisualStudio7Generator::OutputBuildTool(std::ostream& fout,
                                                    const char* configName,
                                                    const char *libName,
                                                    cmTarget &target)
{
  std::string targetFullName = target.GetFullName(configName);
  std::string temp;
  std::string extraLinkOptions;
  if(target.GetType() == cmTarget::EXECUTABLE)
    {
    extraLinkOptions = 
      this->Makefile->GetRequiredDefinition("CMAKE_EXE_LINKER_FLAGS") 
      + std::string(" ") 
      + GetBuildTypeLinkerFlags("CMAKE_EXE_LINKER_FLAGS", configName);
    }
  if(target.GetType() == cmTarget::SHARED_LIBRARY)
    {
    extraLinkOptions = 
      this->Makefile->GetRequiredDefinition("CMAKE_SHARED_LINKER_FLAGS") 
      + std::string(" ") 
      + GetBuildTypeLinkerFlags("CMAKE_SHARED_LINKER_FLAGS", configName);
    }
  if(target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    extraLinkOptions = 
      this->Makefile->GetRequiredDefinition("CMAKE_MODULE_LINKER_FLAGS") 
      + std::string(" ") 
      + GetBuildTypeLinkerFlags("CMAKE_MODULE_LINKER_FLAGS", configName);
    }

  const char* targetLinkFlags = target.GetProperty("LINK_FLAGS");
  if(targetLinkFlags)
    {
    extraLinkOptions += " ";
    extraLinkOptions += targetLinkFlags;
    }
  std::string configTypeUpper = cmSystemTools::UpperCase(configName);
  std::string linkFlagsConfig = "LINK_FLAGS_";
  linkFlagsConfig += configTypeUpper;
  targetLinkFlags = target.GetProperty(linkFlagsConfig.c_str());
  if(targetLinkFlags)
    {
    extraLinkOptions += " ";
    extraLinkOptions += targetLinkFlags;
    }
  std::map<cmStdString, cmStdString> flagMap;
  this->FillFlagMapFromCommandFlags
    (flagMap, &cmLocalVisualStudio7GeneratorLinkFlagTable[0],
                                extraLinkOptions);

  switch(target.GetType())
    {
    case cmTarget::STATIC_LIBRARY:
    {
    std::string libpath = this->LibraryOutputPath +
      "$(OutDir)/" + targetFullName;
    fout << "\t\t\t<Tool\n"
         << "\t\t\t\tName=\"VCLibrarianTool\"\n";
    if(const char* libflags = target.GetProperty("STATIC_LIBRARY_FLAGS"))
      {
      fout << "\t\t\t\tAdditionalOptions=\"" << libflags << "\"\n";
      }
    fout << "\t\t\t\tOutputFile=\""
         << this->ConvertToXMLOutputPathSingle(libpath.c_str()) << "\"/>\n";
    break;
    }
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::MODULE_LIBRARY:
    {
    // Compute the link library and directory information.
    std::vector<cmStdString> linkLibs;
    std::vector<cmStdString> linkDirs;
    this->ComputeLinkInformation(target, configName, linkLibs, linkDirs);

    // Get the language to use for linking.
    const char* linkLanguage = 
      target.GetLinkerLanguage(this->GetGlobalGenerator());
    if(!linkLanguage)
      {
      cmSystemTools::Error
        ("CMake can not determine linker language for target:",
                           target.GetName());
      return;
      }

    // Compute the variable name to lookup standard libraries for this
    // language.
    std::string standardLibsVar = "CMAKE_";
    standardLibsVar += linkLanguage;
    standardLibsVar += "_STANDARD_LIBRARIES";

    fout << "\t\t\t<Tool\n"
         << "\t\t\t\tName=\"VCLinkerTool\"\n"
         << "\t\t\t\tAdditionalOptions=\"/MACHINE:I386";
    if(extraLinkOptions.size())
      {
      fout << " " << cmLocalVisualStudio7Generator::EscapeForXML(
        extraLinkOptions.c_str()).c_str();
      }
    // Use the NOINHERIT macro to avoid getting VS project default
    // libraries which may be set by the user to something bad.
    fout << "\"\n"
         << "\t\t\t\tAdditionalDependencies=\"$(NOINHERIT) "
         << this->Makefile->GetSafeDefinition(standardLibsVar.c_str())
         << " ";
    this->OutputLibraries(fout, linkLibs);
    fout << "\"\n";
    temp = this->LibraryOutputPath;
    temp += configName;
    temp += "/";
    temp += targetFullName;
    fout << "\t\t\t\tOutputFile=\""
         << this->ConvertToXMLOutputPathSingle(temp.c_str()) << "\"\n";
    for(std::map<cmStdString, cmStdString>::iterator i = flagMap.begin();
        i != flagMap.end(); ++i)
      {
      fout << "\t\t\t\t" << i->first << "=\"" << i->second << "\"\n";
      }
    fout << "\t\t\t\tAdditionalLibraryDirectories=\"";
    this->OutputLibraryDirectories(fout, linkDirs);
    fout << "\"\n";
    this->OutputModuleDefinitionFile(fout, target);
    temp = this->LibraryOutputPath;
    temp += "$(OutDir)/";
    temp += libName;
    temp += ".pdb";
    fout << "\t\t\t\tProgramDataBaseFile=\"" <<
      this->ConvertToXMLOutputPathSingle(temp.c_str()) << "\"\n";
    if(strcmp(configName, "Debug") == 0
       || strcmp(configName, "RelWithDebInfo") == 0)
      {
      fout << "\t\t\t\tGenerateDebugInformation=\"TRUE\"\n";
      }
    std::string stackVar = "CMAKE_";
    stackVar += linkLanguage;
    stackVar += "_STACK_SIZE";
    const char* stackVal = this->Makefile->GetDefinition(stackVar.c_str());
    if(stackVal)
      {
      fout << "\t\t\t\tStackReserveSize=\"" << stackVal  << "\"\n";
      }
    temp = this->LibraryOutputPath;
    temp += configName;
    temp += "/";
    temp += 
      cmSystemTools::GetFilenameWithoutLastExtension(targetFullName.c_str());
    temp += ".lib";
    fout << "\t\t\t\tImportLibrary=\"" 
         << this->ConvertToXMLOutputPathSingle(temp.c_str()) << "\"/>\n";
    }
    break;
    case cmTarget::EXECUTABLE:
    {
    // Compute the link library and directory information.
    std::vector<cmStdString> linkLibs;
    std::vector<cmStdString> linkDirs;
    this->ComputeLinkInformation(target, configName, linkLibs, linkDirs);

    // Get the language to use for linking.
    const char* linkLanguage = 
      target.GetLinkerLanguage(this->GetGlobalGenerator());
    if(!linkLanguage)
      {
      cmSystemTools::Error
        ("CMake can not determine linker language for target:",
                           target.GetName());
      return;
      }

    // Compute the variable name to lookup standard libraries for this
    // language.
    std::string standardLibsVar = "CMAKE_";
    standardLibsVar += linkLanguage;
    standardLibsVar += "_STANDARD_LIBRARIES";

    fout << "\t\t\t<Tool\n"
         << "\t\t\t\tName=\"VCLinkerTool\"\n"
         << "\t\t\t\tAdditionalOptions=\"";
    if(extraLinkOptions.size())
      {
      fout << " " << cmLocalVisualStudio7Generator::EscapeForXML(
        extraLinkOptions.c_str()).c_str();
      }
    // Use the NOINHERIT macro to avoid getting VS project default
    // libraries which may be set by the user to something bad.
    fout << "\"\n"
         << "\t\t\t\tAdditionalDependencies=\"$(NOINHERIT) "
         << this->Makefile->GetSafeDefinition(standardLibsVar.c_str())
         << " ";
    this->OutputLibraries(fout, linkLibs);
    fout << "\"\n";
    temp = this->ExecutableOutputPath;
    temp += configName;
    temp += "/";
    temp += targetFullName;
    fout << "\t\t\t\tOutputFile=\"" 
         << this->ConvertToXMLOutputPathSingle(temp.c_str()) << "\"\n";
    for(std::map<cmStdString, cmStdString>::iterator i = flagMap.begin();
        i != flagMap.end(); ++i)
      {
      fout << "\t\t\t\t" << i->first << "=\"" << i->second << "\"\n";
      }
    fout << "\t\t\t\tAdditionalLibraryDirectories=\"";
    this->OutputLibraryDirectories(fout, linkDirs);
    fout << "\"\n";
    fout << "\t\t\t\tProgramDataBaseFile=\"" << this->LibraryOutputPath
         << "$(OutDir)\\" << libName << ".pdb\"\n";
    if(strcmp(configName, "Debug") == 0
       || strcmp(configName, "RelWithDebInfo") == 0)
      {
      fout << "\t\t\t\tGenerateDebugInformation=\"TRUE\"\n";
      }
    if ( target.GetPropertyAsBool("WIN32_EXECUTABLE") )
      {
      fout << "\t\t\t\tSubSystem=\"2\"\n";
      }
    else
      {
      fout << "\t\t\t\tSubSystem=\"1\"\n";
      }
    std::string stackVar = "CMAKE_";
    stackVar += linkLanguage;
    stackVar += "_STACK_SIZE";
    const char* stackVal = this->Makefile->GetDefinition(stackVar.c_str());
    if(stackVal)
      {
      fout << "\t\t\t\tStackReserveSize=\"" << stackVal << "\"";
      }
    fout << "/>\n";
    break;
    }
    case cmTarget::UTILITY:
    case cmTarget::GLOBAL_TARGET:
      break;
    }
}

void cmLocalVisualStudio7Generator
::OutputModuleDefinitionFile(std::ostream& fout,
                                                               cmTarget &target)
{
  std::vector<cmSourceFile*> const& classes = target.GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin();
      i != classes.end(); i++)
    {
    if(cmSystemTools::UpperCase((*i)->GetSourceExtension()) == "DEF")
      {
      fout << "\t\t\t\tModuleDefinitionFile=\""
           << this->ConvertToXMLOutputPath((*i)->GetFullPath().c_str())
           << "\"\n";
      return;
      }
    }

}

//----------------------------------------------------------------------------
void
cmLocalVisualStudio7Generator
::OutputLibraries(std::ostream& fout,
                  std::vector<cmStdString> const& libs)
{
  for(std::vector<cmStdString>::const_iterator l = libs.begin();
      l != libs.end(); ++l)
    {
    fout << this->ConvertToXMLOutputPath(l->c_str()) << " ";
    }
}

//----------------------------------------------------------------------------
void
cmLocalVisualStudio7Generator
::OutputLibraryDirectories(std::ostream& fout,
                           std::vector<cmStdString> const& dirs)
{
  const char* comma = "";
  for(std::vector<cmStdString>::const_iterator d = dirs.begin();
      d != dirs.end(); ++d)
    {
    // Remove any trailing slash and skip empty paths.
    std::string dir = *d;
    if(dir[dir.size()-1] == '/')
      {
      dir = dir.substr(0, dir.size()-1);
      }
    if(dir.empty())
        {
      continue;
        }

    // Switch to a relative path specification if it is shorter.
    if(cmSystemTools::FileIsFullPath(dir.c_str()))
      {
      std::string rel = this->Convert(dir.c_str(), START_OUTPUT, UNCHANGED);
      if(rel.size() < dir.size())
        {
        dir = rel;
      }
      }

    // First search a configuration-specific subdirectory and then the
    // original directory.
    fout << comma << this->ConvertToXMLOutputPath((dir+"/$(OutDir)").c_str())
         << "," << this->ConvertToXMLOutputPath(dir.c_str());
    comma = ",";
    }
}

//----------------------------------------------------------------------------
void cmLocalVisualStudio7Generator::OutputDefineFlags(const char* flags,
                                                      std::ostream& fout)
{
  std::string defs = flags;
  cmSystemTools::ReplaceString(defs, "/D","-D");
  std::string::size_type pos = defs.find("-D");
  bool done = pos == std::string::npos;
  if(!done)
    {
    fout << ",";
    }
  while(!done)
    {
    std::string::size_type nextpos = defs.find("-D", pos+2);
    std::string define;
    if(nextpos != std::string::npos)
      {
      define = defs.substr(pos+2, nextpos - pos -3);
      }
    else
      {
      define = defs.substr(pos+2);
      done = true;
      }

    // Remove trailing whitespace from the definition.
    while(!define.empty() && isspace(define[define.size()-1]))
      {
      define = define.substr(0, define.size()-1);
      }

    // Double-quotes in the value of the definition must be escaped
    // with a backslash.  The entire definition should be quoted in
    // the generated xml attribute to avoid confusing the VS parser.
    define = this->EscapeForXML(define.c_str());
    // if the define has something in it that is not a letter or a number
    // then quote it
    if(define.
       find_first_not_of(
         "-_abcdefghigklmnopqrstuvwxyz1234567890ABCDEFGHIGKLMNOPQRSTUVWXYZ")
       != define.npos)
      {
    fout << "&quot;" << define << "&quot;,";
      }
    else
      {
      fout << define << ",";
      }
    if(!done)
      {
      pos = defs.find("-D", nextpos);
      }
    }
}


void cmLocalVisualStudio7Generator::WriteVCProjFile(std::ostream& fout,
                                                    const char *libName,
                                                    cmTarget &target)
{
  // get the configurations
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();

  // trace the visual studio dependencies
  std::string name = libName;
  name += ".vcproj.cmake";

  // We may be modifying the source groups temporarily, so make a copy.
  std::vector<cmSourceGroup> sourceGroups = this->Makefile->GetSourceGroups();

  // get the classes from the source lists then add them to the groups
  std::vector<cmSourceFile*> & classes = target.GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin();
      i != classes.end(); i++)
    {
    // Add the file to the list of sources.
    std::string source = (*i)->GetFullPath();
    if(cmSystemTools::UpperCase((*i)->GetSourceExtension()) == "DEF")
      {
      this->ModuleDefinitionFile = (*i)->GetFullPath();
      }
    cmSourceGroup& sourceGroup =
      this->Makefile->FindSourceGroup(source.c_str(), sourceGroups);
    sourceGroup.AssignSource(*i);
    }

  // Compute which sources need unique object computation.
  this->ComputeObjectNameRequirements(sourceGroups);

  // open the project
  this->WriteProjectStart(fout, libName, target, sourceGroups);
  // write the configuration information
  this->WriteConfigurations(fout, libName, target);

  fout << "\t<Files>\n";


  // Loop through every source group.
  for(unsigned int i = 0; i < sourceGroups.size(); ++i)
    {
    cmSourceGroup sg = sourceGroups[i];
    this->WriteGroup(&sg, target, fout, libName, configs);
    }

  //}

  fout << "\t</Files>\n";

  // Write the VCProj file's footer.
  this->WriteVCProjFooter(fout);
}

void cmLocalVisualStudio7Generator
::WriteGroup(const cmSourceGroup *sg, cmTarget target, 
             std::ostream &fout, const char *libName, 
             std::vector<std::string> *configs)
{
  const std::vector<const cmSourceFile *> &sourceFiles =
    sg->GetSourceFiles();
  // If the group is empty, don't write it at all.
  if(sourceFiles.empty() && sg->GetGroupChildren().empty())
    {
    return;
    }

  // If the group has a name, write the header.
  std::string name = sg->GetName();
  if(name != "")
    {
    this->WriteVCProjBeginGroup(fout, name.c_str(), "");
    }

  // Loop through each source in the source group.
  std::string objectName;
  for(std::vector<const cmSourceFile *>::const_iterator sf =
        sourceFiles.begin(); sf != sourceFiles.end(); ++sf)
    {
    std::string source = (*sf)->GetFullPath();
    const cmCustomCommand *command = (*sf)->GetCustomCommand();
    std::string compileFlags;
    std::string additionalDeps;
    if(this->NeedObjectName.find(*sf) != this->NeedObjectName.end())
      {
      objectName = this->GetObjectFileNameWithoutTarget(*(*sf));
      }
    else
      {
      objectName = "";
      }

    // Add per-source flags.
    const char* cflags = (*sf)->GetProperty("COMPILE_FLAGS");
    if(cflags)
      {
      compileFlags += " ";
      compileFlags += cflags;
      }
    const char* lang = this->GlobalGenerator->GetLanguageFromExtension
      ((*sf)->GetSourceExtension().c_str());
    const char* linkLanguage = target.GetLinkerLanguage
      (this->GetGlobalGenerator());

    // if the source file does not match the linker language
    // then force c or c++
    if(linkLanguage && lang && strcmp(lang, linkLanguage) != 0)
      {
      if(strcmp(lang, "CXX") == 0)
      {
      // force a C++ file type
      compileFlags += " /TP ";
        }
      else if(strcmp(lang, "C") == 0)
        {
        // force to c 
        compileFlags += " /TC ";
        }
      }
    // Check for extra object-file dependencies.
    const char* deps = (*sf)->GetProperty("OBJECT_DEPENDS");
    if(deps)
      {
      std::vector<std::string> depends;
      cmSystemTools::ExpandListArgument(deps, depends);
      if(!depends.empty())
        {
        std::vector<std::string>::iterator i = depends.begin();
        additionalDeps = this->ConvertToXMLOutputPath(i->c_str());
        for(++i;i != depends.end(); ++i)
          {
          additionalDeps += ";";
          additionalDeps += this->ConvertToXMLOutputPath(i->c_str());
          }
        }
      }
    if (source != libName || target.GetType() == cmTarget::UTILITY ||
      target.GetType() == cmTarget::GLOBAL_TARGET )
      {
      fout << "\t\t\t<File\n";
      std::string d = this->ConvertToXMLOutputPathSingle(source.c_str());
      // Tell MS-Dev what the source is.  If the compiler knows how to
      // build it, then it will.
      fout << "\t\t\t\tRelativePath=\"" << d << "\">\n";
      if (command)
        {
        // Construct the entire set of commands in one string.
        std::string script = 
          this->ConstructScript(command->GetCommandLines(),
                                command->GetWorkingDirectory(),
                                command->GetEscapeOldStyle(),
                                command->GetEscapeAllowMakeVars());
        std::string comment = this->ConstructComment(*command);
        const char* flags = compileFlags.size() ? compileFlags.c_str(): 0;
        this->WriteCustomRule(fout, source.c_str(), script.c_str(),
                              comment.c_str(), command->GetDepends(),
                              command->GetOutputs(), flags);
        }
      else if(compileFlags.size() || additionalDeps.length() 
              || objectName.size())
        {
        const char* aCompilerTool = "VCCLCompilerTool";
        std::string ext = (*sf)->GetSourceExtension();
        ext = cmSystemTools::LowerCase(ext);
        if(ext == "idl")
          {
          aCompilerTool = "VCMIDLTool";
          }
        if(ext == "rc")
          {
          aCompilerTool = "VCResourceCompilerTool";
          }
        if(ext == "def")
          {
          aCompilerTool = "VCCustomBuildTool";
          }
        for(std::vector<std::string>::iterator i = configs->begin();
            i != configs->end(); ++i)
          {
          fout << "\t\t\t\t<FileConfiguration\n"
               << "\t\t\t\t\tName=\""  << *i 
               << "|" << this->PlatformName << "\">\n"
               << "\t\t\t\t\t<Tool\n"
               << "\t\t\t\t\tName=\"" << aCompilerTool << "\"\n";
          if(compileFlags.size())
            {
            std::string compileFlagsCopy = compileFlags;
            std::map<cmStdString, cmStdString> fileFlagMap;
            this->FillFlagMapFromCommandFlags
              (fileFlagMap, 
               &cmLocalVisualStudio7GeneratorFlagTable[0], 
               compileFlagsCopy);
            if(compileFlagsCopy.size() && 
               compileFlagsCopy.find_first_not_of(" ") 
               != compileFlagsCopy.npos)
              {
            fout << "\t\t\t\t\tAdditionalOptions=\""
                   << this->EscapeForXML(compileFlagsCopy.c_str()) << "\"\n"; 
              }
            for(std::map<cmStdString, 
                  cmStdString>::iterator m = fileFlagMap.begin();
                m != fileFlagMap.end(); ++m)
              {
              fout << "\t\t\t\t\t" << m->first << "=\"" 
                   << m->second << "\"\n";
              }
            }
          if(additionalDeps.length())
            {
            fout << "\t\t\t\t\tAdditionalDependencies=\""
                 << additionalDeps.c_str() << "\"\n";
            }
          if(objectName.size())
            {
            fout << "\t\t\t\t\tObjectFile=\"$(IntDir)/"
                 << objectName.c_str() << "\"\n";
            }
          fout << "\t\t\t\t\t/>\n"
               << "\t\t\t\t</FileConfiguration>\n";
          }
        }
      fout << "\t\t\t</File>\n";
      }
    }

  std::vector<cmSourceGroup> children  = sg->GetGroupChildren();

  for(unsigned int i=0;i<children.size();++i)
    {
    this->WriteGroup(&children[i], target, fout, libName, configs);
    }

  // If the group has a name, write the footer.
  if(name != "")
    {
    this->WriteVCProjEndGroup(fout);
    }
}

void cmLocalVisualStudio7Generator::
WriteCustomRule(std::ostream& fout,
                const char* source,
                const char* command,
                const char* comment,
                const std::vector<std::string>& depends,
                const std::vector<std::string>& outputs,
                const char* compileFlags)
{
  // Write the rule for each configuration.
  std::vector<std::string>::iterator i;
  std::vector<std::string> *configs =
    static_cast<cmGlobalVisualStudio7Generator *>
    (this->GlobalGenerator)->GetConfigurations();

  for(i = configs->begin(); i != configs->end(); ++i)
    {
    fout << "\t\t\t\t<FileConfiguration\n";
    fout << "\t\t\t\t\tName=\"" << *i << "|" << this->PlatformName << "\">\n";
    if(compileFlags)
      {
      fout << "\t\t\t\t\t<Tool\n"
           << "\t\t\t\t\tName=\"VCCLCompilerTool\"\n"
           << "\t\t\t\t\tAdditionalOptions=\""
           << this->EscapeForXML(compileFlags) << "\"/>\n";
      }
    fout << "\t\t\t\t\t<Tool\n"
         << "\t\t\t\t\tName=\"VCCustomBuildTool\"\n"
         << "\t\t\t\t\tDescription=\"" 
         << this->EscapeForXML(comment) << "\"\n"
         << "\t\t\t\t\tCommandLine=\"" 
         << this->EscapeForXML(command) << "\"\n"
         << "\t\t\t\t\tAdditionalDependencies=\"";
    if(depends.empty())
      {
      // There are no real dependencies.  Produce an artificial one to
      // make sure the rule runs reliably.
      if(!cmSystemTools::FileExists(source))
        {
        std::ofstream depout(source);
        depout << "Artificial dependency for a custom command.\n";
        }
      fout << this->ConvertToXMLOutputPath(source);
      }
    else
      {
    // Write out the dependencies for the rule.
    std::string temp;
    for(std::vector<std::string>::const_iterator d = depends.begin();
        d != depends.end(); ++d)
      {
        // Get the real name of the dependency in case it is a CMake target.
      std::string dep = this->GetRealDependency(d->c_str(), i->c_str());
      fout << this->ConvertToXMLOutputPath(dep.c_str())
           << ";";
      }
      }
    fout << "\"\n";
    fout << "\t\t\t\t\tOutputs=\"";
    if(outputs.empty())
      {
      fout << source << "_force";
      }
    else
      {
      // Write a rule for the output generated by this command.
      const char* sep = "";
      for(std::vector<std::string>::const_iterator o = outputs.begin();
          o != outputs.end(); ++o)
        {
        fout << sep << this->ConvertToXMLOutputPathSingle(o->c_str());
        sep = ";";
        }
      }
    fout << "\"/>\n";
    fout << "\t\t\t\t</FileConfiguration>\n";
    }
}


void cmLocalVisualStudio7Generator::WriteVCProjBeginGroup(std::ostream& fout,
                                                          const char* group,
                                                          const char* )
{
  fout << "\t\t<Filter\n"
       << "\t\t\tName=\"" << group << "\"\n"
       << "\t\t\tFilter=\"\">\n";
}


void cmLocalVisualStudio7Generator::WriteVCProjEndGroup(std::ostream& fout)
{
  fout << "\t\t</Filter>\n";
}


// look for custom rules on a target and collect them together
void cmLocalVisualStudio7Generator
::OutputTargetRules(std::ostream& fout,
                                                      cmTarget &target,
                                                      const char * /*libName*/)
{
  if (target.GetType() > cmTarget::GLOBAL_TARGET)
    {
    return;
    }

  // add the pre build rules
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCPreBuildEventTool\"";
  bool init = false;
  for (std::vector<cmCustomCommand>::const_iterator cr =
         target.GetPreBuildCommands().begin();
       cr != target.GetPreBuildCommands().end(); ++cr)
    {
    if(!init)
      {
      fout << "\nCommandLine=\"";
      init = true;
      }
    std::string script = 
      this->ConstructScript(cr->GetCommandLines(),
                            cr->GetWorkingDirectory(),
                            cr->GetEscapeOldStyle(),
                            cr->GetEscapeAllowMakeVars());
    fout << this->EscapeForXML(script.c_str()).c_str();
    }
  if (init)
    {
    fout << "\"";
    }
  fout << "/>\n";

  // add the pre Link rules
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCPreLinkEventTool\"";
  init = false;
  for (std::vector<cmCustomCommand>::const_iterator cr =
         target.GetPreLinkCommands().begin();
       cr != target.GetPreLinkCommands().end(); ++cr)
    {
    if(!init)
      {
      fout << "\nCommandLine=\"";
      init = true;
      }
    std::string script =
      this->ConstructScript(cr->GetCommandLines(),
                            cr->GetWorkingDirectory(),
                            cr->GetEscapeOldStyle(),
                            cr->GetEscapeAllowMakeVars());
    fout << this->EscapeForXML(script.c_str()).c_str();
    }
  if (init)
    {
    fout << "\"";
    }
  fout << "/>\n";

  // add the PostBuild rules
  fout << "\t\t\t<Tool\n\t\t\t\tName=\"VCPostBuildEventTool\"";
  init = false;
  for (std::vector<cmCustomCommand>::const_iterator cr =
         target.GetPostBuildCommands().begin();
       cr != target.GetPostBuildCommands().end(); ++cr)
    {
    if(!init)
      {
      fout << "\nCommandLine=\"";
      init = true;
      }
    std::string script = 
      this->ConstructScript(cr->GetCommandLines(),
                            cr->GetWorkingDirectory(),
                            cr->GetEscapeOldStyle(),
                            cr->GetEscapeAllowMakeVars());
    fout << this->EscapeForXML(script.c_str()).c_str();
    }
  if (init)
    {
    fout << "\"";
    }
  fout << "/>\n";
}

void
cmLocalVisualStudio7Generator::WriteProjectStart(std::ostream& fout,
                                                 const char *libName,
                                                 cmTarget & target,
                                                 std::vector<cmSourceGroup> &)
{
  fout << "<?xml version=\"1.0\" encoding = \"Windows-1252\"?>\n"
       << "<VisualStudioProject\n"
       << "\tProjectType=\"Visual C++\"\n";
  if(this->Version == 71)
    {
    fout << "\tVersion=\"7.10\"\n";
    }
  else
    {
    if (this->Version == 8)
      {
      fout << "\tVersion=\"8.00\"\n";
      }
    else
      {
      fout << "\tVersion=\"7.00\"\n";
      }
    }
  const char* projLabel = target.GetProperty("PROJECT_LABEL");
  if(!projLabel)
    {
    projLabel = libName;
    }
  const char* keyword = target.GetProperty("VS_KEYWORD");
  if(!keyword)
    {
    keyword = "Win32Proj";
    }
  cmGlobalVisualStudio7Generator* gg =
    static_cast<cmGlobalVisualStudio7Generator *>(this->GlobalGenerator);
  fout << "\tName=\"" << projLabel << "\"\n";
  if(this->Version == 8)
    {
    fout << "\tProjectGUID=\"{" << gg->GetGUID(libName) << "}\"\n";
    }
  fout << "\tSccProjectName=\"\"\n"
       << "\tSccLocalPath=\"\"\n"
       << "\tKeyword=\"" << keyword << "\">\n"
       << "\t<Platforms>\n"
       << "\t\t<Platform\n\t\t\tName=\"" << this->PlatformName << "\"/>\n"
       << "\t</Platforms>\n";
}


void cmLocalVisualStudio7Generator::WriteVCProjFooter(std::ostream& fout)
{
  fout << "\t<Globals>\n"
       << "\t</Globals>\n"
       << "</VisualStudioProject>\n";
}


std::string cmLocalVisualStudio7Generator::EscapeForXML(const char* s)
{
  std::string ret = s;
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "\"", "&quot;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  cmSystemTools::ReplaceString(ret, "\n", "&#x0D;&#x0A;");
  return ret;
}

std::string cmLocalVisualStudio7Generator
::ConvertToXMLOutputPath(const char* path)
{
  std::string ret = this->ConvertToOptionallyRelativeOutputPath(path);
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "\"", "&quot;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  return ret;
}

std::string cmLocalVisualStudio7Generator
::ConvertToXMLOutputPathSingle(const char* path)
{
  std::string ret = this->ConvertToOptionallyRelativeOutputPath(path);
  cmSystemTools::ReplaceString(ret, "\"", "");
  cmSystemTools::ReplaceString(ret, "&", "&amp;");
  cmSystemTools::ReplaceString(ret, "<", "&lt;");
  cmSystemTools::ReplaceString(ret, ">", "&gt;");
  return ret;
}

void cmLocalVisualStudio7Generator::ConfigureFinalPass()
{
  cmLocalGenerator::ConfigureFinalPass();
  cmTargets &tgts = this->Makefile->GetTargets();

  cmGlobalVisualStudio7Generator* gg =
    static_cast<cmGlobalVisualStudio7Generator *>(this->GlobalGenerator);
  for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
    {
    if (strncmp(l->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) == 0)
      {
      cmCustomCommand cc = l->second.GetPostBuildCommands()[0];
      const cmCustomCommandLines& cmds = cc.GetCommandLines();
      std::string project_name = cmds[0][0];
      gg->CreateGUID(project_name.c_str());
      }
    else
      {
      gg->CreateGUID(l->first.c_str());
      }
    }

}

//----------------------------------------------------------------------------
std::string cmLocalVisualStudio7Generator
::GetTargetDirectory(cmTarget& target)
{
  std::string dir;
  dir += target.GetName();
  dir += ".dir";
  return dir;
}
