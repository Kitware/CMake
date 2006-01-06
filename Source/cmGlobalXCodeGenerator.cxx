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
#include "cmGlobalXCodeGenerator.h"
#include "cmGlobalXCode21Generator.h"
#include "cmLocalXCodeGenerator.h"
#include "cmMakefile.h"
#include "cmXCodeObject.h"
#include "cmXCode21Object.h"
#include "cmake.h"
#include "cmGeneratedFileStream.h"
#include "cmSourceFile.h"
#include "cmOrderLinkDirectories.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cmXMLParser.h"

// parse the xml file storing the installed version of Xcode on
// the machine
class cmXcodeVersionParser : public cmXMLParser
{
public:
  void StartElement(const char* , const char** )
    {
      m_Data = "";
    }
  void EndElement(const char* name)
    {
      if(strcmp(name, "key") == 0)
        {
        m_Key = m_Data;
        }
      else if(strcmp(name, "string") == 0)
        {
        if(m_Key == "CFBundleShortVersionString")
          {
          m_Version = (int)(10.0 * atof(m_Data.c_str()));
          }
        }
    }
  void CharacterDataHandler(const char* data, int length)
    {
      m_Data.append(data, length);
    }
  int m_Version;
  std::string m_Key;
  std::string m_Data;
};
#endif


//TODO
// add OSX application stuff

//----------------------------------------------------------------------------
cmGlobalXCodeGenerator::cmGlobalXCodeGenerator()
{
  m_FindMakeProgramFile = "CMakeFindXCode.cmake";
  m_RootObject = 0;
  m_MainGroupChildren = 0;
  m_SourcesGroupChildren = 0;
  m_CurrentMakefile = 0;
  m_CurrentLocalGenerator = 0;
  m_XcodeVersion = 15;
}

//----------------------------------------------------------------------------
cmGlobalGenerator* cmGlobalXCodeGenerator::New()
{ 
#if defined(CMAKE_BUILD_WITH_CMAKE)  
  cmXcodeVersionParser parser;
  parser.ParseFile("/Developer/Applications/Xcode.app/Contents/version.plist");
  if(parser.m_Version == 15)
    {
    return new cmGlobalXCodeGenerator;
    }
  else if (parser.m_Version == 20)
    {
    cmSystemTools::Message("Xcode 2.0 not really supported by cmake, "
                           "using Xcode 15 generator\n");
    return new cmGlobalXCodeGenerator;
    }
  cmGlobalXCodeGenerator* ret = new cmGlobalXCode21Generator;
  ret->SetVersion(parser.m_Version);
  return ret;
#else
  std::cerr 
    << "CMake should be built with cmake to use XCode, default to Xcode 1.5\n";
  return new cmGlobalXCodeGenerator;
#endif
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::EnableLanguage(std::vector<std::string>const&
                                            lang,
                                            cmMakefile * mf)
{ 
  mf->AddDefinition("XCODE","1");
  if(m_XcodeVersion == 15)
    {
    mf->AddDefinition("CMAKE_CFG_INTDIR",".");
    }
  else
    {
    mf->AddDefinition("CMAKE_CFG_INTDIR","$(CONFIGURATION)");
    mf->AddCacheDefinition(
      "CMAKE_CONFIGURATION_TYPES",
      "Debug;Release;MinSizeRel;RelWithDebInfo",
      "Semicolon separated list of supported configuration types, "
      "only supports Debug, Release, MinSizeRel, and RelWithDebInfo, "
      "anything else will be ignored.",
      cmCacheManager::STRING);
    }
  mf->AddDefinition("CMAKE_GENERATOR_CC", "gcc");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "g++");
  mf->AddDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV", "1");
  this->cmGlobalGenerator::EnableLanguage(lang, mf);
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::GenerateBuildCommand(const char* makeProgram,
  const char *projectName, const char* additionalOptions, const char *targetName,
  const char* config, bool ignoreErrors)
{
  // Config is not used yet
  (void) config;
  (void) ignoreErrors;

  // now build the test
  if(makeProgram == 0 || !strlen(makeProgram))
    {
    cmSystemTools::Error(
      "Generator cannot find the appropriate make command.");
    return "";
    }
  std::string makeCommand = 
    cmSystemTools::ConvertToOutputPath(makeProgram);
  std::string lowerCaseCommand = makeCommand;
  cmSystemTools::LowerCase(lowerCaseCommand);

  makeCommand += " -project ";
  makeCommand += projectName;
  makeCommand += ".xcode";
  if(m_XcodeVersion > 20)
    {
    makeCommand += "proj";
    }

  bool clean = false;
  if ( targetName && strcmp(targetName, "clean") == 0 )
    {
    clean = true;
    targetName = "ALL_BUILD";
    }
  if(clean)
    {
    makeCommand += " clean";
    }
  else
    {
    makeCommand += " build";
    }
  makeCommand += " -target ";
  if (targetName && strlen(targetName))
    {
    makeCommand += targetName;
    }
  else
    {
    makeCommand += "ALL_BUILD";
    }
  if(m_XcodeVersion == 15)
    {
    makeCommand += " -buildstyle Development ";
    }
  else
    {
    makeCommand += " -configuration Debug";
    }
  if ( additionalOptions )
    {
    makeCommand += " ";
    makeCommand += additionalOptions;
    }
  makeCommand += " OBJROOT=.";
  return makeCommand;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::ConfigureOutputPaths()
{
  // Format the library and executable output paths.
  m_LibraryOutputPath = 
    m_CurrentMakefile->GetSafeDefinition("LIBRARY_OUTPUT_PATH");
  if(m_LibraryOutputPath.size() == 0)
    {
    m_LibraryOutputPath = m_CurrentMakefile->GetCurrentOutputDirectory();
    }
  // make sure there is a trailing slash
  if(m_LibraryOutputPath.size() && 
     m_LibraryOutputPath[m_LibraryOutputPath.size()-1] != '/')
    {
    m_LibraryOutputPath += "/";
    if(!cmSystemTools::MakeDirectory(m_LibraryOutputPath.c_str()))
      {
      cmSystemTools::Error("Error creating directory ",
                           m_LibraryOutputPath.c_str());
      }
    }
  m_CurrentMakefile->AddLinkDirectory(m_LibraryOutputPath.c_str());
  m_ExecutableOutputPath = 
    m_CurrentMakefile->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
  if(m_ExecutableOutputPath.size() == 0)
    {
    m_ExecutableOutputPath = m_CurrentMakefile->GetCurrentOutputDirectory();
    }
  // make sure there is a trailing slash
  if(m_ExecutableOutputPath.size() && 
     m_ExecutableOutputPath[m_ExecutableOutputPath.size()-1] != '/')
    {
    m_ExecutableOutputPath += "/";
    if(!cmSystemTools::MakeDirectory(m_ExecutableOutputPath.c_str()))
      {
      cmSystemTools::Error("Error creating directory ",
                           m_ExecutableOutputPath.c_str());
      }
    }
}

//----------------------------------------------------------------------------
///! Create a local generator appropriate to this Global Generator
cmLocalGenerator *cmGlobalXCodeGenerator::CreateLocalGenerator()
{
  cmLocalGenerator *lg = new cmLocalXCodeGenerator;
  lg->SetGlobalGenerator(this);
  return lg;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::Generate()
{
  this->cmGlobalGenerator::Generate();
  std::map<cmStdString, std::vector<cmLocalGenerator*> >::iterator it;
  for(it = m_ProjectMap.begin(); it!= m_ProjectMap.end(); ++it)
    { 
    cmLocalGenerator* root = it->second[0];
    m_CurrentProject = root->GetMakefile()->GetProjectName();
    this->SetCurrentLocalGenerator(root);
    m_OutputDir = m_CurrentMakefile->GetHomeOutputDirectory();
    m_OutputDir = cmSystemTools::CollapseFullPath(m_OutputDir.c_str());
    cmSystemTools::SplitPath(m_OutputDir.c_str(),
                             m_ProjectOutputDirectoryComponents);
    m_CurrentLocalGenerator = root;
    // add ALL_BUILD, INSTALL, etc
    this->AddExtraTargets(root, it->second);
    // now create the project
    this->OutputXCodeProject(root, it->second);
    }
}

//----------------------------------------------------------------------------
void 
cmGlobalXCodeGenerator::AddExtraTargets(cmLocalGenerator* root, 
                                        std::vector<cmLocalGenerator*>& gens)
{
  cmMakefile* mf = root->GetMakefile();
  // Add ALL_BUILD
  const char* no_output = 0;
  std::vector<std::string> no_depends;
  mf->AddUtilityCommand("ALL_BUILD", false, no_output, no_depends,
                        "echo", "Build all projects");
  cmTarget* allbuild = mf->FindTarget("ALL_BUILD");
  // ADD install
  std::string cmake_command = mf->GetRequiredDefinition("CMAKE_COMMAND");
  if(m_XcodeVersion == 15)
    {
    mf->AddUtilityCommand("install", false, no_output, no_depends,
                          cmake_command.c_str(),
                          "-P", "cmake_install.cmake"); 
    }
  else
    { 
    mf->AddUtilityCommand("install", false, no_output, no_depends,
                          cmake_command.c_str(), 
                          "-DBUILD_TYPE=$(CONFIGURATION)",
                          "-P", "cmake_install.cmake"); 
    }
  
  const char* noall =
    mf->GetDefinition("CMAKE_SKIP_INSTALL_ALL_DEPENDENCY");
  if(!noall || cmSystemTools::IsOff(noall))
    {
    cmTarget* install = mf->FindTarget("install");
    install->AddUtility("ALL_BUILD");
    }
  
  // Add RUN_TESTS target if testing has been enabled
  std::string fname;
  fname = mf->GetStartOutputDirectory();
  fname += "/";
  fname += "DartTestfile.txt";
  if (cmSystemTools::FileExists(fname.c_str()))
    {
    std::string ctest_command = 
      mf->GetRequiredDefinition("CMAKE_CTEST_COMMAND");
    mf->AddUtilityCommand("RUN_TESTS", false, no_output, no_depends,
                          ctest_command.c_str());
    }
  // Add XCODE depend helper 
  std::string dir = mf->GetCurrentOutputDirectory();
  m_CurrentXCodeHackMakefile = dir;
  m_CurrentXCodeHackMakefile += "/CMakeScripts";
  cmSystemTools::MakeDirectory(m_CurrentXCodeHackMakefile.c_str());
  m_CurrentXCodeHackMakefile += "/XCODE_DEPEND_HELPER.make";
  cmCustomCommandLine makecommand;
  makecommand.push_back("make");
  makecommand.push_back("-C");
  makecommand.push_back(dir.c_str());
  makecommand.push_back("-f");
  makecommand.push_back(m_CurrentXCodeHackMakefile.c_str());
  cmCustomCommandLines commandLines;
  commandLines.push_back(makecommand);
  mf->AddUtilityCommand("XCODE_DEPEND_HELPER", false, no_output, no_depends,
                        commandLines);

  // Add Re-Run CMake rules
  this->CreateReRunCMakeFile(root);

  // now make the allbuild depend on all the non-utility targets
  // in the project
  for(std::vector<cmLocalGenerator*>::iterator i = gens.begin();
      i != gens.end(); ++i)
    {
    cmLocalGenerator* lg = *i; 
    if(this->IsExcluded(root, *i))
      {
      continue;
      }
    cmTargets& tgts = lg->GetMakefile()->GetTargets();
    for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
      {
      cmTarget& target = l->second;
      // make all exe, shared libs and modules depend
      // on the XCODE_DEPEND_HELPER target
      if((target.GetType() == cmTarget::EXECUTABLE ||
          target.GetType() == cmTarget::SHARED_LIBRARY ||
          target.GetType() == cmTarget::MODULE_LIBRARY))
        {
        target.AddUtility("XCODE_DEPEND_HELPER");
        }
      if(target.IsInAll())
        {
        allbuild->AddUtility(target.GetName());
        }
      }
    }
}


//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateReRunCMakeFile(cmLocalGenerator* root)
{
  cmMakefile* mf = root->GetMakefile();
  std::vector<std::string> lfiles = mf->GetListFiles();
  // sort the array
  std::sort(lfiles.begin(), lfiles.end(), std::less<std::string>()); 
  std::vector<std::string>::iterator new_end = 
    std::unique(lfiles.begin(), lfiles.end());
  lfiles.erase(new_end, lfiles.end());
  std::string dir = mf->GetHomeOutputDirectory();
  m_CurrentReRunCMakeMakefile = dir;
  m_CurrentReRunCMakeMakefile += "/CMakeScripts";
  cmSystemTools::MakeDirectory(m_CurrentReRunCMakeMakefile.c_str());
  m_CurrentReRunCMakeMakefile += "/ReRunCMake.make";
  cmGeneratedFileStream makefileStream(m_CurrentReRunCMakeMakefile.c_str());
  makefileStream.SetCopyIfDifferent(true);
  makefileStream << "# Generated by CMake, DO NOT EDIT\n";
  makefileStream << "CMakeFiles/cmake.check_cache: ";
  for(std::vector<std::string>::const_iterator i = lfiles.begin();
      i !=  lfiles.end(); ++i)
    {
    makefileStream << "\\\n" << this->ConvertToRelativeForMake(i->c_str());
    }
  std::string cmake = mf->GetRequiredDefinition("CMAKE_COMMAND");
  makefileStream << "\n\t" << this->ConvertToRelativeForMake(cmake.c_str()) 
                 << " -H" << this->ConvertToRelativeForMake(
                   mf->GetHomeDirectory())
                 << " -B" << this->ConvertToRelativeForMake(
                   mf->GetHomeOutputDirectory()) << "\n";
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::ClearXCodeObjects()
{
  m_TargetDoneSet.clear();
  for(unsigned int i = 0; i < m_XCodeObjects.size(); ++i)
    {
    delete m_XCodeObjects[i];
    }
  m_XCodeObjects.clear();
  m_GroupMap.clear();
  m_GroupNameMap.clear();
  m_TargetGroup.clear();
}

//----------------------------------------------------------------------------
cmXCodeObject* 
cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::PBXType ptype)
{
  cmXCodeObject* obj;
  if(m_XcodeVersion == 15)
    {
    obj = new cmXCodeObject(ptype, cmXCodeObject::OBJECT);
    }
  else
    {
    obj = new cmXCode21Object(ptype, cmXCodeObject::OBJECT);
    }
  m_XCodeObjects.push_back(obj);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject* 
cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::Type type)
{
  cmXCodeObject* obj = new cmXCodeObject(cmXCodeObject::None, type);
  m_XCodeObjects.push_back(obj);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateString(const char* s)
{
  cmXCodeObject* obj = this->CreateObject(cmXCodeObject::STRING);
  obj->SetString(s);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject* cmGlobalXCodeGenerator::CreateObjectReference(cmXCodeObject* ref)
{
  cmXCodeObject* obj = this->CreateObject(cmXCodeObject::OBJECT_REF);
  obj->SetObject(ref);
  return obj;
}

//----------------------------------------------------------------------------
cmXCodeObject* 
cmGlobalXCodeGenerator::CreateXCodeSourceFile(cmLocalGenerator* lg, 
                                              cmSourceFile* sf)
{
  std::string flags;
  // Add flags from source file properties.
  lg->AppendFlags(flags, sf->GetProperty("COMPILE_FLAGS"));

  cmXCodeObject* fileRef = this->CreateObject(cmXCodeObject::PBXFileReference);

  cmXCodeObject* group = m_GroupMap[sf];
  cmXCodeObject* children = group->GetObject("children");
  children->AddObject(fileRef);
  cmXCodeObject* buildFile = this->CreateObject(cmXCodeObject::PBXBuildFile);
  std::string fname = sf->GetSourceName();
  fname += ".";
  fname += sf->GetSourceExtension();
  std::string comment = fname;
  comment += " in ";
  std::string gname = group->GetObject("name")->GetString();
  comment += gname.substr(1, gname.size()-2);
  buildFile->SetComment(comment.c_str());
  fileRef->SetComment(fname.c_str());
  buildFile->AddAttribute("fileRef", this->CreateObjectReference(fileRef));
  cmXCodeObject* settings = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  settings->AddAttribute("COMPILER_FLAGS", this->CreateString(flags.c_str()));
  buildFile->AddAttribute("settings", settings);
  fileRef->AddAttribute("fileEncoding", this->CreateString("4"));
  const char* lang = 
    this->GetLanguageFromExtension(sf->GetSourceExtension().c_str());
  std::string sourcecode = "sourcecode";
  std::string ext = sf->GetSourceExtension();
  ext = cmSystemTools::LowerCase(ext);
  if(ext == "o")
    {
    sourcecode = "compiled.mach-o.objfile";
    }
  else if(ext == "mm")
    {
    sourcecode += ".cpp.objcpp";
    }
  else if(ext == "m")
    {
    sourcecode += ".cpp.objc";
    }
  else if(ext == "plist")
    {
    sourcecode += ".text.plist";
    }
  else if(!lang)
    {
    sourcecode += ext;
    sourcecode += ".";
    sourcecode += ext;
    }
  else if(strcmp(lang, "C") == 0)
    {
    sourcecode += ".c.c";
    }
  else
    {
    sourcecode += ".cpp.cpp";
    }

  fileRef->AddAttribute("lastKnownFileType", 
                        this->CreateString(sourcecode.c_str()));
  std::string path = 
    this->ConvertToRelativeForXCode(sf->GetFullPath().c_str());
  std::string dir;
  std::string file;
  cmSystemTools::SplitProgramPath(sf->GetFullPath().c_str(),
                                  dir, file);
  
  fileRef->AddAttribute("name", this->CreateString(file.c_str()));
  fileRef->AddAttribute("path", this->CreateString(path.c_str()));
  if(m_XcodeVersion == 15)
    {
    fileRef->AddAttribute("refType", this->CreateString("4"));
    }
  if(path.size() > 1 && path[0] == '.' && path[1] == '.')
    {
    fileRef->AddAttribute("sourceTree", this->CreateString("<group>"));
    }
  else
    {
    fileRef->AddAttribute("sourceTree", this->CreateString("<absolute>"));
    }
  return buildFile;
}

//----------------------------------------------------------------------------
bool cmGlobalXCodeGenerator::SpecialTargetEmitted(std::string const& tname)
{
  if(tname == "ALL_BUILD" || tname == "XCODE_DEPEND_HELPER" ||
     tname == "install" || tname == "RUN_TESTS" )
    {
    if(m_TargetDoneSet.find(tname) != m_TargetDoneSet.end())
      {
      return true;
      }
    m_TargetDoneSet.insert(tname);
    return false;
    }
  return false;
}


void cmGlobalXCodeGenerator::SetCurrentLocalGenerator(cmLocalGenerator* gen)
{
  m_CurrentLocalGenerator = gen;
  m_CurrentMakefile = gen->GetMakefile();
  std::string outdir =
    cmSystemTools::CollapseFullPath(m_CurrentMakefile->
                                    GetCurrentOutputDirectory());
  cmSystemTools::SplitPath(outdir.c_str(), m_CurrentOutputDirectoryComponents);
}


//----------------------------------------------------------------------------
void 
cmGlobalXCodeGenerator::CreateXCodeTargets(cmLocalGenerator* gen,
                                           std::vector<cmXCodeObject*>& 
                                           targets)
{
  this->SetCurrentLocalGenerator(gen);
  cmTargets &tgts = gen->GetMakefile()->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
    { 
    cmTarget& cmtarget = l->second;
    // make sure ALL_BUILD, INSTALL, etc are only done once
    if(this->SpecialTargetEmitted(l->first.c_str()))
      {
      continue;
      }
    if(cmtarget.GetType() == cmTarget::UTILITY ||
       cmtarget.GetType() == cmTarget::INSTALL_FILES ||
       cmtarget.GetType() == cmTarget::INSTALL_PROGRAMS)
      {
      if(cmtarget.GetType() == cmTarget::UTILITY)
        {
        targets.push_back(this->CreateUtilityTarget(cmtarget));
        }
      continue;
      }

    // create source build phase
    cmXCodeObject* sourceBuildPhase = 
      this->CreateObject(cmXCodeObject::PBXSourcesBuildPhase);
    sourceBuildPhase->AddAttribute("buildActionMask", 
                                   this->CreateString("2147483647"));
    sourceBuildPhase->SetComment("Sources");
    cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    sourceBuildPhase->AddAttribute("files", buildFiles);
    sourceBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                                   this->CreateString("0"));
    std::vector<cmSourceFile*> &classes = l->second.GetSourceFiles();
    // add all the sources
    std::vector<cmXCodeObject*> externalObjFiles;
    std::vector<cmXCodeObject*> headerFiles;
    for(std::vector<cmSourceFile*>::iterator i = classes.begin(); 
        i != classes.end(); ++i)
      {
      cmXCodeObject* xsf = this->CreateXCodeSourceFile(gen, *i);
      cmXCodeObject* fr = xsf->GetObject("fileRef");
      cmXCodeObject* filetype = 
        fr->GetObject()->GetObject("lastKnownFileType");
      if(strcmp(filetype->GetString(), "\"compiled.mach-o.objfile\"") == 0)
        {
        externalObjFiles.push_back(xsf);
        }
      else if((*i)->GetPropertyAsBool("HEADER_FILE_ONLY"))
        {
        headerFiles.push_back(xsf);
        }
      else
        {
        buildFiles->AddObject(xsf);
        }
      }
    // create header build phase
    cmXCodeObject* headerBuildPhase = 
      this->CreateObject(cmXCodeObject::PBXHeadersBuildPhase);
    headerBuildPhase->SetComment("Headers");
    headerBuildPhase->AddAttribute("buildActionMask",
                                   this->CreateString("2147483647"));
    buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    for(std::vector<cmXCodeObject*>::iterator i = headerFiles.begin();
        i != headerFiles.end(); ++i)
      {
      buildFiles->AddObject(*i);
      }
    headerBuildPhase->AddAttribute("files", buildFiles);
    headerBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                   this->CreateString("0"));
    
    // create framework build phase
    cmXCodeObject* frameworkBuildPhase =
      this->CreateObject(cmXCodeObject::PBXFrameworksBuildPhase);
    frameworkBuildPhase->SetComment("Frameworks");
    frameworkBuildPhase->AddAttribute("buildActionMask",
                                      this->CreateString("2147483647"));
    buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    frameworkBuildPhase->AddAttribute("files", buildFiles);
    for(std::vector<cmXCodeObject*>::iterator i =  externalObjFiles.begin();
        i != externalObjFiles.end(); ++i)
      {
      buildFiles->AddObject(*i);
      }
    frameworkBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                                      this->CreateString("0"));
    cmXCodeObject* buildPhases = 
      this->CreateObject(cmXCodeObject::OBJECT_LIST);
    this->CreateCustomCommands(buildPhases, sourceBuildPhase,
                               headerBuildPhase, frameworkBuildPhase,
                               cmtarget);
    targets.push_back(this->CreateXCodeTarget(l->second, buildPhases));
    }
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateBuildPhase(const char* name, 
                                         const char* name2,
                                         cmTarget& cmtarget,
                                         const std::vector<cmCustomCommand>&
                                         commands)
{
  if(commands.size() == 0 && strcmp(name, "CMake ReRun") != 0)
    {
    return 0;
    }
  cmXCodeObject* buildPhase = 
    this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase);
  buildPhase->AddAttribute("buildActionMask",
                           this->CreateString("2147483647"));
  cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  buildPhase->AddAttribute("files", buildFiles);
  buildPhase->AddAttribute("name", 
                           this->CreateString(name));
  buildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                           this->CreateString("0"));
  buildPhase->AddAttribute("shellPath",
                           this->CreateString("/bin/sh"));
  this->AddCommandsToBuildPhase(buildPhase, cmtarget, commands,
                                name2);
  return buildPhase;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateCustomCommands(cmXCodeObject* buildPhases,
                                                  cmXCodeObject*
                                                  sourceBuildPhase,
                                                  cmXCodeObject*
                                                  headerBuildPhase,
                                                  cmXCodeObject*
                                                  frameworkBuildPhase,
                                                  cmTarget& cmtarget)
{
  std::vector<cmCustomCommand> const & prebuild 
    = cmtarget.GetPreBuildCommands();
  std::vector<cmCustomCommand> const & prelink 
    = cmtarget.GetPreLinkCommands();
  std::vector<cmCustomCommand> const & postbuild 
    = cmtarget.GetPostBuildCommands();
  cmtarget.TraceVSDependencies(cmtarget.GetName(), m_CurrentMakefile);
  std::vector<cmSourceFile*> &classes = cmtarget.GetSourceFiles();
  // add all the sources
  std::vector<cmCustomCommand> commands;
  for(std::vector<cmSourceFile*>::iterator i = classes.begin(); 
      i != classes.end(); ++i)
    {
    if((*i)->GetCustomCommand())
      {
      commands.push_back(*(*i)->GetCustomCommand());
      }
    }
  std::vector<cmCustomCommand> reruncom;
  cmXCodeObject* cmakeReRunPhase =  this->CreateBuildPhase("CMake ReRun",
                                                           "cmakeReRunPhase",
                                                           cmtarget, reruncom);
  buildPhases->AddObject(cmakeReRunPhase);
  // create prebuild phase
  cmXCodeObject* cmakeRulesBuildPhase =
    this->CreateBuildPhase("CMake Rules",
                           "cmakeRulesBuildPhase",
                           cmtarget, commands);
  // create prebuild phase
  cmXCodeObject* preBuildPhase = this->CreateBuildPhase("CMake PreBuild Rules",
                                                        "preBuildCommands",
                                                        cmtarget, prebuild);
  // create prebuild phase
  cmXCodeObject* preLinkPhase = this->CreateBuildPhase("CMake PreLink Rules",
                                                       "preLinkCommands",
                                                       cmtarget, prelink);
  // create prebuild phase
  cmXCodeObject* postBuildPhase = 
    this->CreateBuildPhase("CMake PostBuild Rules",
                           "postBuildPhase",
                           cmtarget, postbuild);
  // the order here is the order they will be built in
  if(preBuildPhase)
    {
    buildPhases->AddObject(preBuildPhase);
    }
  if(cmakeRulesBuildPhase)
    {
    buildPhases->AddObject(cmakeRulesBuildPhase);
    }
  if(sourceBuildPhase)
    {
    buildPhases->AddObject(sourceBuildPhase);
    }
  if(headerBuildPhase)
    {
    buildPhases->AddObject(headerBuildPhase);
    }
  if(preLinkPhase)
    {
    buildPhases->AddObject(preLinkPhase);
    }
  if(frameworkBuildPhase)
    {
    buildPhases->AddObject(frameworkBuildPhase);
    }
  if(postBuildPhase)
    {
    buildPhases->AddObject(postBuildPhase);
    }
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::ExtractFlag(const char* flag,
                                                std::string& flags)
{
  std::string retFlag;
  std::string::size_type pos = flags.find(flag);
  if(pos != flags.npos)
    {
    retFlag = flag;
    // remove the flag
    flags[pos]=' ';
    flags[pos+1]=' ';
    char pos2 = flags[pos+2];
    // if the pos after the option 
    if(pos2 != ' ' && pos2 != 0 )
      {
      retFlag += pos2;
      // remove the next part of the flag
      flags[pos+2] = ' ';
      }
    }
  return retFlag;
}
//----------------------------------------------------------------------------
void 
cmGlobalXCodeGenerator::AddCommandsToBuildPhase(cmXCodeObject* buildphase,
                                                cmTarget& target,
                                                std::vector<cmCustomCommand> 
                                                const & commands,
                                                const char* name)
{
  if(strcmp(name, "cmakeReRunPhase") == 0)
    {
    std::string cdir = m_CurrentMakefile->GetHomeOutputDirectory();
    cdir = this->ConvertToRelativeForMake(cdir.c_str());
    std::string makecmd = "make -C ";
    makecmd += cdir;
    makecmd += " -f ";
    makecmd += 
      this->ConvertToRelativeForMake(m_CurrentReRunCMakeMakefile.c_str());
    cmSystemTools::ReplaceString(makecmd, "\\ ", "\\\\ ");
    buildphase->AddAttribute("shellScript",
                             this->CreateString(makecmd.c_str()));
    return;
    }
  
  std::string dir = m_CurrentMakefile->GetCurrentOutputDirectory();
  dir += "/CMakeScripts";
  cmSystemTools::MakeDirectory(dir.c_str());
  std::string makefile = dir;
  makefile += "/";
  makefile += target.GetName();
  makefile += "_";
  makefile += name;
  makefile += ".make";
  cmGeneratedFileStream makefileStream(makefile.c_str());
  if(!makefileStream)
    {
    return;
    }
  makefileStream.SetCopyIfDifferent(true);
  makefileStream << "# Generated by CMake, DO NOT EDIT\n";
  makefileStream << "# Custom rules for " << target.GetName() << "\n";
  
  // have all depend on all outputs
  makefileStream << "all: ";
  std::map<const cmCustomCommand*, cmStdString> tname;
  int count = 0;
  for(std::vector<cmCustomCommand>::const_iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    cmCustomCommand const& cc = *i; 
    if(!cc.GetCommandLines().empty())
      {
      if(cc.GetOutput()[0])
        {
        makefileStream << "\\\n\t"  << this->
          ConvertToRelativeForMake(cc.GetOutput());
        }
      else
        {
        char c = '1' + count++;
        tname[&cc] = std::string(target.GetName()) + c;
        makefileStream << "\\\n\t" << tname[&cc];
        }
      }
    }
  makefileStream << "\n\n";
  
  for(std::vector<cmCustomCommand>::const_iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    cmCustomCommand const& cc = *i; 
    if(!cc.GetCommandLines().empty())
      {
      
      makefileStream << "\n#" << "Custom command rule: " << 
        cc.GetComment() << "\n";
      if(cc.GetOutput()[0])
        {
        makefileStream << this
          ->ConvertToRelativeForMake(cc.GetOutput()) << ": ";
        }
      else
        {
        makefileStream << tname[&cc] << ": ";
        }
      for(std::vector<std::string>::const_iterator d = cc.GetDepends().begin();
          d != cc.GetDepends().end(); ++d)
        {
        if(!this->FindTarget(m_CurrentProject.c_str(),
                             d->c_str()))
          {
          // if the depend is not a target but
          // is a full path then use it, if not then
          // just skip it
          if(cmSystemTools::FileIsFullPath(d->c_str()))
            {
            makefileStream << "\\\n" << this
              ->ConvertToRelativeForMake(d->c_str());
            }
          }
        else
          {
          // if the depend is a target then make 
          // the target with the source that is a custom command
          // depend on the that target via a AddUtility call
          target.AddUtility(d->c_str());
          }
        }
      makefileStream << "\n";

      // Add each command line to the set of commands.
      for(cmCustomCommandLines::const_iterator cl = 
            cc.GetCommandLines().begin();
          cl != cc.GetCommandLines().end(); ++cl)
        {
        // Build the command line in a single string.
        const cmCustomCommandLine& commandLine = *cl;
        std::string cmd = commandLine[0];
        cmSystemTools::ReplaceString(cmd, "/./", "/");
        cmd = this->ConvertToRelativeForMake(cmd.c_str());
        for(unsigned int j=1; j < commandLine.size(); ++j)
          {
          cmd += " ";
          cmd += cmSystemTools::EscapeSpaces(commandLine[j].c_str());
          }
        makefileStream << "\t" << cmd.c_str() << "\n";
        }
      }
    }
  std::string cdir = m_CurrentMakefile->GetCurrentOutputDirectory();
  cdir = this->ConvertToRelativeForXCode(cdir.c_str());
  std::string makecmd = "make -C ";
  makecmd += cdir;
  makecmd += " -f ";
  makecmd += this->ConvertToRelativeForMake(makefile.c_str());
  cmSystemTools::ReplaceString(makecmd, "\\ ", "\\\\ ");
  buildphase->AddAttribute("shellScript", this->CreateString(makecmd.c_str()));
}


//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateBuildSettings(cmTarget& target,
                                                 cmXCodeObject* buildSettings,
                                                 std::string& fileType,
                                                 std::string& productType,
                                                 std::string& productName,
                                                 const char* buildtype)
{
  this->ConfigureOutputPaths();
  std::string flags;
  std::string defFlags;
  bool shared = ((target.GetType() == cmTarget::SHARED_LIBRARY) ||
                 (target.GetType() == cmTarget::MODULE_LIBRARY));
  if(shared)
    {
    defFlags += "-D";
    if(const char* custom_export_name = target.GetProperty("DEFINE_SYMBOL"))
      {
      defFlags += custom_export_name;
      }
    else
      {
      std::string in = target.GetName();
      in += "_EXPORTS";
      defFlags += cmSystemTools::MakeCindentifier(in.c_str());
      }
    }
  const char* lang = target.GetLinkerLanguage(this);
  std::string cflags;
  if(lang)
    {
    if(buildtype)
      {
      m_CurrentMakefile->AddDefinition("CMAKE_BUILD_TYPE", buildtype);
      }
    // for c++ projects get the c flags as well
    if(strcmp(lang, "CXX") == 0)
      {
      m_CurrentLocalGenerator->AddLanguageFlags(cflags, "C");
      m_CurrentLocalGenerator->AddSharedFlags(cflags, lang, shared);
      }
    // Add language-specific flags.
    m_CurrentLocalGenerator->AddLanguageFlags(flags, lang);
    
    // Add shared-library flags if needed.
    m_CurrentLocalGenerator->AddSharedFlags(flags, lang, shared);
    m_CurrentMakefile->AddDefinition("CMAKE_BUILD_TYPE", "");
    }

  // Add define flags
  m_CurrentLocalGenerator->AppendFlags(defFlags,
                                       m_CurrentMakefile->GetDefineFlags());
  cmSystemTools::ReplaceString(defFlags, "\"", "\\\"");
  cmSystemTools::ReplaceString(flags, "\"", "\\\"");
  cmSystemTools::ReplaceString(cflags, "\"", "\\\"");
  if(m_XcodeVersion > 15)
    {
    buildSettings->
      AddAttribute("GCC_PREPROCESSOR_DEFINITIONS", 
                   this->CreateString("CMAKE_INTDIR=\\\\\\\"$(CONFIGURATION)\\\\\\\""));
    
    }
  productName = target.GetName();
  std::string extraLinkOptions;
  if(target.GetType() == cmTarget::EXECUTABLE)
    {
    extraLinkOptions = 
      m_CurrentMakefile->GetRequiredDefinition("CMAKE_EXE_LINKER_FLAGS");
    }
  if(target.GetType() == cmTarget::SHARED_LIBRARY)
    {
    extraLinkOptions = 
      m_CurrentMakefile->GetRequiredDefinition("CMAKE_SHARED_LINKER_FLAGS");
    }
  if(target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    extraLinkOptions = 
      m_CurrentMakefile->GetRequiredDefinition("CMAKE_MODULE_LINKER_FLAGS");
    }
  
  const char* targetLinkFlags = target.GetProperty("LINK_FLAGS");
  if(targetLinkFlags)
    {
    extraLinkOptions += " ";
    extraLinkOptions += targetLinkFlags;
    }

  switch(target.GetType())
    {
    case cmTarget::STATIC_LIBRARY:
    {
    if(m_LibraryOutputPath.size())
      {
      buildSettings->AddAttribute("SYMROOT", 
                                  this->CreateString
                                  (m_LibraryOutputPath.c_str()));
      }
    productName += ".a";
    std::string t = "lib";
    t += productName;
    productName = t;  
    productType = "com.apple.product-type.library.static";
    fileType = "archive.ar";
    buildSettings->AddAttribute("LIBRARY_STYLE", 
                                this->CreateString("STATIC"));
    break;
    }
    
    case cmTarget::MODULE_LIBRARY:
    {
    if(m_LibraryOutputPath.size())
      {
      buildSettings->AddAttribute("SYMROOT", 
                                  this->CreateString
                                  (m_LibraryOutputPath.c_str()));
      }
      
    buildSettings->AddAttribute("EXECUTABLE_PREFIX", 
                                this->CreateString("lib"));
    buildSettings->AddAttribute("EXECUTABLE_EXTENSION", 
                                this->CreateString("so"));
    buildSettings->AddAttribute("LIBRARY_STYLE", 
                                this->CreateString("BUNDLE"));
    productName += ".so";
    std::string t = "lib";
    t += productName;
    productName = t;
    if(m_XcodeVersion >= 22)
      {
      buildSettings->AddAttribute("MACH_O_TYPE", 
                                  this->CreateString("mh_bundle"));
      buildSettings->AddAttribute("GCC_DYNAMIC_NO_PIC", 
                                  this->CreateString("NO"));
      buildSettings->AddAttribute("GCC_SYMBOLS_PRIVATE_EXTERN", 
                                  this->CreateString("NO"));
      buildSettings->AddAttribute("GCC_INLINES_ARE_PRIVATE_EXTERN", 
                                  this->CreateString("NO"));
      std::string outflag = "-o \\\"$(CONFIGURATION_BUILD_DIR)/";
      outflag += productName;
      outflag += "\\\"";
      extraLinkOptions += " ";
      extraLinkOptions += outflag;
      productType = "com.apple.product-type.tool";
      fileType = "compiled.mach-o.executable";
      }
    else
      {
      extraLinkOptions += " -bundle";
      productType = "com.apple.product-type.library.dynamic";
      fileType = "compiled.mach-o.dylib";
      }
    break;
    }
    case cmTarget::SHARED_LIBRARY:
    {
    if(m_LibraryOutputPath.size())
      {
      buildSettings->AddAttribute("SYMROOT", 
                                  this->CreateString
                                  (m_LibraryOutputPath.c_str()));
      }
    buildSettings->AddAttribute("LIBRARY_STYLE", 
                                this->CreateString("DYNAMIC"));
    productName += ".dylib";
    std::string t = "lib";
    t += productName;
    productName = t;
    buildSettings->AddAttribute("DYLIB_COMPATIBILITY_VERSION", 
                                this->CreateString("1"));
    buildSettings->AddAttribute("DYLIB_CURRENT_VERSION", 
                                this->CreateString("1"));
    extraLinkOptions += " -dynamiclib";
    productType = "com.apple.product-type.library.dynamic";
    fileType = "compiled.mach-o.dylib";
    break;
    }
    case cmTarget::EXECUTABLE:
    {
    const char* outname = target.GetProperty("OUTPUT_NAME");
    std::string name;
    if(outname)
      {
      productName = outname;
      name = outname;
      }
    else
      {
      name = target.GetName();
      }
    std::string symRoot;
    if(m_ExecutableOutputPath.size())
      {
      std::string path = m_ExecutableOutputPath;
      symRoot = path;
      }
    fileType = "compiled.mach-o.executable";
    if(target.GetPropertyAsBool("MACOSX_BUNDLE"))
      {
      std::string f1 =
        m_CurrentMakefile->GetModulesFile("MacOSXBundleInfo.plist.in");
      if ( f1.size() == 0 )
        {
        cmSystemTools::Error("could not find Mac OSX bundle template file.");
        }
      std::string f2 = m_CurrentMakefile->GetCurrentOutputDirectory();
      f2 += "/Info.plist";
      m_CurrentMakefile->ConfigureFile(f1.c_str(), f2.c_str(),
                                       false, false, false);
      productType = "com.apple.product-type.application";
      std::string path = 
        this->ConvertToRelativeForXCode(f2.c_str());
      buildSettings->AddAttribute("INFOPLIST_FILE", 
                                  this->CreateString(path.c_str()));

      }
    else
      {
      productType = "com.apple.product-type.tool";
      }
    if(symRoot.size())
      {
      buildSettings->AddAttribute("SYMROOT", 
                                  this->CreateString
                                  (symRoot.c_str()));
      }
    }
    break;
    case cmTarget::UTILITY:
      
      break;
    case cmTarget::INSTALL_FILES:
      break;
    case cmTarget::INSTALL_PROGRAMS:
      break;
    }
  
  std::string dirs;
  std::vector<std::string>& includes =
    m_CurrentMakefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i = includes.begin();
  std::string fdirs;
  std::set<cmStdString> emitted;
  for(;i != includes.end(); ++i)
    {
    if(cmSystemTools::IsPathToFramework(i->c_str()))
      {
      std::string frameworkDir = *i;
      frameworkDir += "/../";
      frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir.c_str());
      if(emitted.insert(frameworkDir).second)
        {
        fdirs += this->XCodeEscapePath(frameworkDir.c_str());
        fdirs += " ";
        }
      }
    else
      {
      std::string incpath = 
        this->XCodeEscapePath(i->c_str());
      dirs += incpath + " ";
      }
    }
  std::vector<std::string>& frameworks = target.GetFrameworks();
  if(frameworks.size())
    {
    for(std::vector<std::string>::iterator fmIt = frameworks.begin();
        fmIt != frameworks.end(); ++fmIt)
      {
      if(emitted.insert(*fmIt).second)
        {
        fdirs += this->XCodeEscapePath(fmIt->c_str());
        fdirs += " ";
        }
      }
    }
  if(fdirs.size())
    {
    buildSettings->AddAttribute("FRAMEWORK_SEARCH_PATHS", 
                                this->CreateString(fdirs.c_str()));
    }
  if(dirs.size())
    {
    buildSettings->AddAttribute("HEADER_SEARCH_PATHS", 
                                this->CreateString(dirs.c_str()));
    }
  std::string oflagc = this->ExtractFlag("-O", cflags);
  char optLevel[2];
  optLevel[0] = '0';
  optLevel[1] = 0;
  if(oflagc.size() == 3)
    {
    optLevel[0] = oflagc[2];
    }
  if(oflagc.size() == 2)
    {
    optLevel[0] = '1';
    }
  std::string oflag = this->ExtractFlag("-O", flags);
  if(oflag.size() == 3)
    {
    optLevel[0] = oflag[2];
    }
  if(oflag.size() == 2)
    {
    optLevel[0] = '1';
    }
  std::string gflagc = this->ExtractFlag("-g", cflags);
  std::string gflag = this->ExtractFlag("-g", flags);
  const char* debugStr = "YES";
  if(gflagc.size() ==0  && gflag.size() == 0)
    {
    debugStr = "NO";
    }    
  buildSettings->AddAttribute("GCC_GENERATE_DEBUGGING_SYMBOLS",
                              this->CreateString(debugStr));
  buildSettings->AddAttribute("GCC_OPTIMIZATION_LEVEL", 
                              this->CreateString(optLevel));
  buildSettings->AddAttribute("INSTALL_PATH", 
                              this->CreateString(""));
  buildSettings->AddAttribute("OPTIMIZATION_CFLAGS", 
                              this->CreateString(oflagc.c_str()));
  if(lang && strcmp(lang, "CXX") == 0)
    {
    flags += " ";
    flags += defFlags;
    buildSettings->AddAttribute("OTHER_CPLUSPLUSFLAGS", 
                                this->CreateString(flags.c_str()));
    cflags += " ";
    cflags += defFlags;
    buildSettings->AddAttribute("OTHER_CFLAGS", 
                                this->CreateString(cflags.c_str()));

    }
  else
    {
    flags += " ";
    flags += defFlags;
    buildSettings->AddAttribute("OTHER_CFLAGS", 
                                this->CreateString(flags.c_str()));
    }
  buildSettings->AddAttribute("OTHER_LDFLAGS", 
                              this->CreateString(extraLinkOptions.c_str()));

  buildSettings->AddAttribute("OTHER_REZFLAGS", 
                              this->CreateString(""));
  buildSettings->AddAttribute("SECTORDER_FLAGS",
                              this->CreateString(""));
  buildSettings->AddAttribute("USE_HEADERMAP",
                              this->CreateString("NO"));
  buildSettings->AddAttribute("WARNING_CFLAGS",
                              this->CreateString(
                                "-Wmost -Wno-four-char-constants"
                                " -Wno-unknown-pragmas"));
  std::string pname;
  if(target.GetType() == cmTarget::SHARED_LIBRARY)
    {
    pname = "lib";
    }
  pname += target.GetName();
  if(target.GetType() == cmTarget::EXECUTABLE 
     && target.GetProperty("OUTPUT_NAME") )
    {
    pname = target.GetProperty("OUTPUT_NAME");
    }
  
  buildSettings->AddAttribute("PRODUCT_NAME", 
                              this->CreateString(pname.c_str()));
}

//----------------------------------------------------------------------------
cmXCodeObject* 
cmGlobalXCodeGenerator::CreateUtilityTarget(cmTarget& cmtarget)
{
  cmXCodeObject* shellBuildPhase =
    this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase);
  shellBuildPhase->AddAttribute("buildActionMask", 
                                this->CreateString("2147483647"));
  cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  shellBuildPhase->AddAttribute("files", buildFiles);
  cmXCodeObject* inputPaths = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  shellBuildPhase->AddAttribute("inputPaths", inputPaths);
  cmXCodeObject* outputPaths = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  shellBuildPhase->AddAttribute("outputPaths", outputPaths);
  shellBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                this->CreateString("0"));
  shellBuildPhase->AddAttribute("shellPath",
                                this->CreateString("/bin/sh"));
  shellBuildPhase->AddAttribute("shellScript",
                                this->CreateString(
                                  "# shell script goes here\nexit 0"));
  cmXCodeObject* target = 
    this->CreateObject(cmXCodeObject::PBXAggregateTarget);
  target->SetComment(cmtarget.GetName());
  cmXCodeObject* buildPhases = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  this->CreateCustomCommands(buildPhases, 0, 0, 0, cmtarget);
  target->AddAttribute("buildPhases", buildPhases);
  cmXCodeObject* buildSettings =
    this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  std::string fileTypeString;
  std::string productTypeString;
  std::string productName;
  this->CreateBuildSettings(cmtarget, 
                            buildSettings, fileTypeString, 
                            productTypeString, productName, 0);
  if(m_XcodeVersion > 20)
    {
    this->AddConfigurations(target, cmtarget);
    }
  target->AddAttribute("buildSettings", buildSettings);
  cmXCodeObject* dependencies = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(productName.c_str()));
  target->AddAttribute("productName",this->CreateString(productName.c_str()));
  target->SetcmTarget(&cmtarget);
  return target;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AddConfigurations(cmXCodeObject* target,
                                               cmTarget& cmtarget)
{
  std::string configTypes = m_CurrentMakefile->GetRequiredDefinition("CMAKE_CONFIGURATION_TYPES");
  std::vector<std::string> configVectorIn;
  std::vector<std::string> configVector;
  configVectorIn.push_back(configTypes);
  cmSystemTools::ExpandList(configVectorIn, configVector);
  cmXCodeObject* configlist = this->CreateObject(cmXCodeObject::XCConfigurationList);
  cmXCodeObject* buildConfigurations =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  configlist->AddAttribute("buildConfigurations", buildConfigurations);
  std::string comment = "Build configuration list for ";
  comment += cmXCodeObject::PBXTypeNames[target->GetIsA()];
  comment += " \"";
  comment += cmtarget.GetName();
  comment += "\"";
  configlist->SetComment(comment.c_str());
  target->AddAttribute("buildConfigurationList", 
                       this->CreateObjectReference(configlist));
  for(unsigned int i = 0; i < configVector.size(); ++i)
    {
    cmXCodeObject* config = this->CreateObject(cmXCodeObject::XCBuildConfiguration);
    buildConfigurations->AddObject(config);
    cmXCodeObject* buildSettings =
      this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
    std::string fileTypeString;
    std::string productTypeString;
    std::string productName;
    std::string buildtype = cmSystemTools::UpperCase(configVector[i]);
    this->CreateBuildSettings(cmtarget, 
                              buildSettings, fileTypeString, 
                              productTypeString, productName, buildtype.c_str());
    config->AddAttribute("name", this->CreateString(configVector[i].c_str()));
    config->SetComment(configVector[i].c_str());
    config->AddAttribute("buildSettings", buildSettings);
    }
  if(configVector.size())
    {
    configlist->AddAttribute("defaultConfigurationName", 
                             this->CreateString(configVector[0].c_str()));
    configlist->AddAttribute("defaultConfigurationIsVisible", this->CreateString("0"));
    }
}

//----------------------------------------------------------------------------
cmXCodeObject*
cmGlobalXCodeGenerator::CreateXCodeTarget(cmTarget& cmtarget,
                                          cmXCodeObject* buildPhases)
{
  cmXCodeObject* target = 
    this->CreateObject(cmXCodeObject::PBXNativeTarget);
  target->AddAttribute("buildPhases", buildPhases);
  cmXCodeObject* buildRules = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("buildRules", buildRules);
  cmXCodeObject* buildSettings =
    this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  std::string fileTypeString;
  std::string productTypeString;
  std::string productName;
  if(m_XcodeVersion > 20)
    {
    this->AddConfigurations(target, cmtarget);
    }
  this->CreateBuildSettings(cmtarget, 
                            buildSettings, fileTypeString, 
                            productTypeString, productName, 0);
  target->AddAttribute("buildSettings", buildSettings);
  cmXCodeObject* dependencies = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(productName.c_str()));
  target->AddAttribute("productName",this->CreateString(productName.c_str()));

  cmXCodeObject* fileRef = this->CreateObject(cmXCodeObject::PBXFileReference);
  fileRef->AddAttribute("explicitFileType", 
                        this->CreateString(fileTypeString.c_str()));
  fileRef->AddAttribute("path", this->CreateString(productName.c_str()));
  fileRef->AddAttribute("refType", this->CreateString("0"));
  fileRef->AddAttribute("sourceTree",
                        this->CreateString("BUILT_PRODUCTS_DIR"));
  fileRef->SetComment(cmtarget.GetName());
  target->AddAttribute("productReference", 
                       this->CreateObjectReference(fileRef));
  target->AddAttribute("productType", 
                       this->CreateString(productTypeString.c_str()));
  target->SetcmTarget(&cmtarget);
  return target;
}

//----------------------------------------------------------------------------
cmXCodeObject* cmGlobalXCodeGenerator::FindXCodeTarget(cmTarget* t)
{
  if(!t)
    {
    return 0;
    }
  for(std::vector<cmXCodeObject*>::iterator i = m_XCodeObjects.begin();
      i != m_XCodeObjects.end(); ++i)
    {
    cmXCodeObject* o = *i;
    if(o->GetcmTarget() == t)
      {
      return o;
      }
    }
  return 0;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AddDependTarget(cmXCodeObject* target,
                                             cmXCodeObject* dependTarget)
{
  // make sure a target does not depend on itself
  if(target == dependTarget)
    {
    return;
    }
  // now avoid circular references if dependTarget already
  // depends on target then skip it.  Circular references crashes
  // xcode
  cmXCodeObject* dependTargetDepends = dependTarget->GetObject("dependencies");
  if(dependTargetDepends)
    {
    if(dependTargetDepends->HasObject(target->GetPBXTargetDependency()))
      { 
      return;
      }
    }
  
  cmXCodeObject* targetdep = dependTarget->GetPBXTargetDependency();
  if(!targetdep)
    {
    cmXCodeObject* container = 
      this->CreateObject(cmXCodeObject::PBXContainerItemProxy);
    container->SetComment("PBXContainerItemProxy");
    container->AddAttribute("containerPortal",
                            this->CreateObjectReference(m_RootObject));
    container->AddAttribute("proxyType", this->CreateString("1"));
    container->AddAttribute("remoteGlobalIDString",
                            this->CreateObjectReference(dependTarget));
    container->AddAttribute("remoteInfo", 
                            this->CreateString(
                              dependTarget->GetcmTarget()->GetName()));
    targetdep = 
      this->CreateObject(cmXCodeObject::PBXTargetDependency);
    targetdep->SetComment("PBXTargetDependency");
    targetdep->AddAttribute("target",
                            this->CreateObjectReference(dependTarget));
    targetdep->AddAttribute("targetProxy", 
                            this->CreateObjectReference(container));
    dependTarget->SetPBXTargetDependency(targetdep);
    }
    
  cmXCodeObject* depends = target->GetObject("dependencies");
  if(!depends)
    {
    cmSystemTools::
      Error("target does not have dependencies attribute error..");
    
    }
  else
    {
    depends->AddUniqueObject(targetdep);
    }
}


//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AppendOrAddBuildSetting(cmXCodeObject* settings,
                                                     const char* attribute,
                                                     const char* value)
{
  if(settings)
    {
    cmXCodeObject* attr = settings->GetObject(attribute);
    if(!attr)
      {
      settings->AddAttribute(attribute, this->CreateString(value));
      }
    else
      {
      std::string oldValue = attr->GetString();
      cmSystemTools::ReplaceString(oldValue, "\"", "");
      oldValue += " ";
      oldValue += value;
      attr->SetString(oldValue.c_str());
      }
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AppendBuildSettingAttribute(cmXCodeObject* target,
                                                         const char* attribute,
                                                         const char* value)
{
  if(m_XcodeVersion < 21)
    {
    this->AppendOrAddBuildSetting(target->GetObject("buildSettings"),
                                  attribute, value);
    }
  else
    {
    cmXCodeObject* configurationList = target->GetObject("buildConfigurationList")->GetObject();
    cmXCodeObject* buildConfigs = configurationList->GetObject("buildConfigurations");
    std::vector<cmXCodeObject*> list = buildConfigs->GetObjectList();
    // each configuration and the target itself has a buildSettings in it 
    list.push_back(target);
    for(std::vector<cmXCodeObject*>::iterator i = list.begin(); i != list.end(); ++i)
      {
      cmXCodeObject* settings = (*i)->GetObject("buildSettings");
      this->AppendOrAddBuildSetting(settings, attribute, value);
      }
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AddLinkLibrary(cmXCodeObject* target,
                                            const char* library,
                                            cmTarget* dtarget)
{
  if(dtarget)
    {
    target->AddDependLibrary(this->GetTargetFullPath(dtarget).c_str());
    }
  
  // if the library is not a full path then add it with a -l flag
  // to the settings of the target
  cmsys::RegularExpression reg("^([ \t]*\\-[lLWRBF])|([ \t]*\\-framework)|(\\${)|([ \t]*\\-pthread)|([ \t]*`)");
  // if the library is not already in the form required by the compiler
  // add a -l infront of the name
  std::string link;
  if(!reg.find(library))
    {
    link += "-l";
    }
  link += library;
  this->AppendBuildSettingAttribute(target, "OTHER_LDFLAGS", link.c_str());
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::GetTargetFullPath(cmTarget* target)
{
  std::string libPath;
  cmXCodeObject* xtarget = this->FindXCodeTarget(target);
  cmXCodeObject* bset = xtarget->GetObject("buildSettings");
  cmXCodeObject* spath = bset->GetObject("SYMROOT");
  if(m_XcodeVersion > 15)
    {
    libPath += "$(CONFIGURATION)/";
    }
  
  libPath = spath->GetString();
  libPath = libPath.substr(1, libPath.size()-2);
  if(target->GetType() == cmTarget::STATIC_LIBRARY)
    {
    libPath += "lib";
    libPath += target->GetName();
    libPath += ".a";
    }
  else if(target->GetType() == cmTarget::SHARED_LIBRARY)
    {
    libPath += "lib";
    libPath += target->GetName();
    libPath += ".dylib";
    }
  else
    {
    libPath += target->GetName();
    }
  return libPath;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AddDependAndLinkInformation(cmXCodeObject* target)
{
  cmTarget* cmtarget = target->GetcmTarget();
  if(!cmtarget)
    {
    cmSystemTools::Error("Error no target on xobject\n");
    return;
    }
  // compute the correct order for link libraries
  cmOrderLinkDirectories orderLibs;
  std::string ext = 
    m_CurrentMakefile->GetSafeDefinition("CMAKE_STATIC_LIBRARY_SUFFIX");
  if(ext.size())
    {
    orderLibs.AddLinkExtension(ext.c_str());
    }
  ext = 
    m_CurrentMakefile->GetSafeDefinition("CMAKE_STATIC_LIBRARY_PREFIX");
  if(ext.size())
    {
    orderLibs.SetLinkPrefix(ext.c_str());
    }
  ext = 
    m_CurrentMakefile->GetSafeDefinition("CMAKE_SHARED_LIBRARY_SUFFIX");
  if(ext.size())
    {
    orderLibs.AddLinkExtension(ext.c_str());
    }
  ext = 
    m_CurrentMakefile->GetSafeDefinition("CMAKE_LINK_LIBRARY_SUFFIX");
  if(ext.size())
    {
    orderLibs.AddLinkExtension(ext.c_str());
    }
  const char* targetLibrary = cmtarget->GetName();
  if(cmtarget->GetType() == cmTarget::EXECUTABLE)
    {
    targetLibrary = 0;
    }
  orderLibs.SetLinkInformation(*cmtarget, cmTarget::GENERAL, targetLibrary);
  orderLibs.DetermineLibraryPathOrder();
  std::vector<cmStdString> libdirs;
  std::vector<cmStdString> linkItems;
  orderLibs.GetLinkerInformation(libdirs, linkItems);
  std::string linkDirs;
  // add the library search paths
  for(std::vector<cmStdString>::const_iterator libDir = libdirs.begin();
      libDir != libdirs.end(); ++libDir)
    {
    if(libDir->size() && *libDir != "/usr/lib")
      {
      if(m_XcodeVersion > 15)
        {
        // now add the same one but append $(CONFIGURATION) to it:
        linkDirs += " ";
        linkDirs += this->XCodeEscapePath(libDir->c_str());
        linkDirs += "/$(CONFIGURATION)";
        }
      linkDirs += " ";
      linkDirs += this->XCodeEscapePath(libDir->c_str());
      }
    }
  this->AppendBuildSettingAttribute(target, "LIBRARY_SEARCH_PATHS", linkDirs.c_str());
  // now add the link libraries
  for(std::vector<cmStdString>::iterator lib = linkItems.begin();
      lib != linkItems.end(); ++lib)
    {
    cmTarget* t = this->FindTarget(m_CurrentProject.c_str(),
                                   lib->c_str());
    cmXCodeObject* dptarget = this->FindXCodeTarget(t);
    if(dptarget)
      {
      this->AddDependTarget(target, dptarget);
      if(cmtarget->GetType() != cmTarget::STATIC_LIBRARY)
        {
        this->AddLinkLibrary(target, t->GetName(), t);
        }
      }
    else
      {
      if(cmtarget->GetType() != cmTarget::STATIC_LIBRARY)
        {
        this->AddLinkLibrary(target, lib->c_str());
        }
      }
    }
  
  // write utility dependencies.
  for(std::set<cmStdString>::const_iterator i
        = cmtarget->GetUtilities().begin();
      i != cmtarget->GetUtilities().end(); ++i)
    {
    cmTarget* t = this->FindTarget(m_CurrentProject.c_str(),
                                   i->c_str());
    // if the target is in this project then make target depend
    // on it.  It may not be in this project if this is a sub
    // project from the top.
    if(t)
      {
      cmXCodeObject* dptarget = this->FindXCodeTarget(t);
      if(dptarget)
        {
        this->AddDependTarget(target, dptarget);
        }
      else
        {
        std::string m = "Error Utility: ";
        m += i->c_str();
        m += "\n";
        m += "cmtarget ";
        if(t)
          {
          m += t->GetName();
          }
        m += "\n";
        m += "Is on the target ";
        m += cmtarget->GetName();
        m += "\n";
        m += "But it has no xcode target created yet??\n";
        m += "Current project is ";
        m += m_CurrentProject.c_str();
        cmSystemTools::Error(m.c_str());
        }
      } 
    }
  std::vector<cmStdString> fullPathLibs;
  orderLibs.GetFullPathLibraries(fullPathLibs);
  for(std::vector<cmStdString>::iterator i = fullPathLibs.begin();
      i != fullPathLibs.end(); ++i)
    {
    target->AddDependLibrary(i->c_str());
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateGroups(cmLocalGenerator* root,
                                          std::vector<cmLocalGenerator*>&
                                          generators)
{
  for(std::vector<cmLocalGenerator*>::iterator i = generators.begin();
      i != generators.end(); ++i)
    {
    if(this->IsExcluded(root, *i))
      {
      continue;
      }
    cmMakefile* mf = (*i)->GetMakefile();
    std::vector<cmSourceGroup> sourceGroups = mf->GetSourceGroups();
    cmTargets &tgts = mf->GetTargets();
    for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); l++)
      { 
      cmTarget& cmtarget = l->second;
      // add the soon to be generated Info.plist file as a source for a MACOSX_BUNDLE
      // file
      if(cmtarget.GetPropertyAsBool("MACOSX_BUNDLE"))
        {
        cmSourceFile file;
        file.SetName("Info",
                     m_CurrentMakefile->GetCurrentOutputDirectory(),
                     "plist", false);
        cmtarget.GetSourceFiles().push_back(m_CurrentMakefile->AddSource(file));
        }

      std::vector<cmSourceFile*> & classes = cmtarget.GetSourceFiles();
      for(std::vector<cmSourceFile*>::const_iterator s = classes.begin(); 
          s != classes.end(); s++)
        {
        cmSourceFile* sf = *s;
        // Add the file to the list of sources.
        std::string const& source = sf->GetFullPath();
        cmSourceGroup& sourceGroup = 
          mf->FindSourceGroup(source.c_str(), sourceGroups);
        cmXCodeObject* pbxgroup = this->CreateOrGetPBXGroup(cmtarget, &sourceGroup);
        m_GroupMap[sf] = pbxgroup;
        }
      }
    } 
}
//----------------------------------------------------------------------------
cmXCodeObject* cmGlobalXCodeGenerator::CreateOrGetPBXGroup(cmTarget& cmtarget,
                                                           cmSourceGroup* sg)
{
  cmStdString s = cmtarget.GetName();
  s += "/";
  s += sg->GetName();
  std::map<cmStdString, cmXCodeObject* >::iterator i =  m_GroupNameMap.find(s);
  if(i != m_GroupNameMap.end())
    {
    return i->second;
    }
  i = m_TargetGroup.find(cmtarget.GetName());
  cmXCodeObject* tgroup = 0;
  if(i != m_TargetGroup.end())
    {
    tgroup = i->second;
    }
  else
    {
    tgroup = this->CreateObject(cmXCodeObject::PBXGroup);
    m_TargetGroup[cmtarget.GetName()] = tgroup;
    cmXCodeObject* tgroupChildren = 
      this->CreateObject(cmXCodeObject::OBJECT_LIST);
    tgroup->AddAttribute("name", this->CreateString(cmtarget.GetName()));
    tgroup->AddAttribute("children", tgroupChildren);
    if(m_XcodeVersion == 15)
      {
      tgroup->AddAttribute("refType", this->CreateString("4"));
      }
    tgroup->AddAttribute("sourceTree", this->CreateString("<group>"));
    m_SourcesGroupChildren->AddObject(tgroup);
    }
  
  cmXCodeObject* tgroupChildren = tgroup->GetObject("children");
  cmXCodeObject* group = this->CreateObject(cmXCodeObject::PBXGroup);
  cmXCodeObject* groupChildren = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  group->AddAttribute("name", this->CreateString(sg->GetName()));
  group->AddAttribute("children", groupChildren);
  if(m_XcodeVersion == 15)
    {
    group->AddAttribute("refType", this->CreateString("4"));
    }
  group->AddAttribute("sourceTree", this->CreateString("<group>"));
  tgroupChildren->AddObject(group);
  m_GroupNameMap[s] = group;
  return group;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateXCodeObjects(cmLocalGenerator* root,
                                                std::vector<cmLocalGenerator*>&
                                                generators
  )
{
  this->ClearXCodeObjects(); 
  m_RootObject = 0;
  m_SourcesGroupChildren = 0;
  m_MainGroupChildren = 0;
  cmXCodeObject* group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("NO"));
  cmXCodeObject* developBuildStyle = 
    this->CreateObject(cmXCodeObject::PBXBuildStyle);
  if(m_XcodeVersion == 15)
    {
    developBuildStyle->AddAttribute("name", this->CreateString("Development"));
    }
  else
    {
    developBuildStyle->AddAttribute("name", this->CreateString("Debug"));
    developBuildStyle->SetComment("Debug");
    }
  developBuildStyle->AddAttribute("buildSettings", group);
  
  group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("YES"));
  cmXCodeObject* deployBuildStyle =
    this->CreateObject(cmXCodeObject::PBXBuildStyle);
  if(m_XcodeVersion == 15)
    {
    deployBuildStyle->AddAttribute("name", this->CreateString("Deployment"));
    }
  else
    {
    deployBuildStyle->AddAttribute("name", this->CreateString("Release"));
    deployBuildStyle->SetComment("Release");
    }

  deployBuildStyle->AddAttribute("buildSettings", group);

  cmXCodeObject* listObjs = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  listObjs->AddObject(developBuildStyle);
  listObjs->AddObject(deployBuildStyle);
  cmXCodeObject* mainGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  m_MainGroupChildren = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  mainGroup->AddAttribute("children", m_MainGroupChildren);
  if(m_XcodeVersion == 15)
    {
    mainGroup->AddAttribute("refType", this->CreateString("4"));
    }
  mainGroup->AddAttribute("sourceTree", this->CreateString("<group>"));

  cmXCodeObject* sourcesGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  m_SourcesGroupChildren = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  sourcesGroup->AddAttribute("name", this->CreateString("Sources"));
  sourcesGroup->AddAttribute("children", m_SourcesGroupChildren);
  if(m_XcodeVersion == 15)
    {
    sourcesGroup->AddAttribute("refType", this->CreateString("4"));
    }
  sourcesGroup->AddAttribute("sourceTree", this->CreateString("<group>"));
  m_MainGroupChildren->AddObject(sourcesGroup);
  // now create the cmake groups 
  this->CreateGroups(root, generators);
  
  cmXCodeObject* productGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  productGroup->AddAttribute("name", this->CreateString("Products"));
  if(m_XcodeVersion == 15)
    {
    productGroup->AddAttribute("refType", this->CreateString("4"));
    }
  productGroup->AddAttribute("sourceTree", this->CreateString("<group>"));
  cmXCodeObject* productGroupChildren = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  productGroup->AddAttribute("children", productGroupChildren);
  m_MainGroupChildren->AddObject(productGroup);
  
  
  m_RootObject = this->CreateObject(cmXCodeObject::PBXProject);
  m_RootObject->SetComment("Project object");
  group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  m_RootObject->AddAttribute("mainGroup", 
                             this->CreateObjectReference(mainGroup));
  m_RootObject->AddAttribute("buildSettings", group);
  m_RootObject->AddAttribute("buildStyles", listObjs);
  m_RootObject->AddAttribute("hasScannedForEncodings",
                             this->CreateString("0"));
  cmXCodeObject* configlist = this->CreateObject(cmXCodeObject::XCConfigurationList);
  cmXCodeObject* configDebug = this->CreateObject(cmXCodeObject::XCBuildConfiguration);
  cmXCodeObject* configRelease = this->CreateObject(cmXCodeObject::XCBuildConfiguration);
  cmXCodeObject* buildConfigurations =
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  buildConfigurations->AddObject(configDebug);
  buildConfigurations->AddObject(configRelease);
  configlist->AddAttribute("buildConfigurations", buildConfigurations);
  std::string comment = "Build configuration list for PBXProject ";
  comment += " \"";
  comment += m_CurrentProject;
  comment += "\"";
  configlist->SetComment(comment.c_str());
  configlist->AddAttribute("defaultConfigurationIsVisible", this->CreateString("0"));
  configlist->AddAttribute("defaultConfigurationName", this->CreateString("Debug"));
  cmXCodeObject* buildSettings =
    this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  configDebug->AddAttribute("name", this->CreateString("Debug"));
  configDebug->AddAttribute("buildSettings", buildSettings);
  configRelease->AddAttribute("name", this->CreateString("Release"));
  configRelease->AddAttribute("buildSettings", buildSettings);

  m_RootObject->AddAttribute("buildConfigurationList", 
                             this->CreateObjectReference(configlist));

  std::vector<cmXCodeObject*> targets;
  for(std::vector<cmLocalGenerator*>::iterator i = generators.begin();
      i != generators.end(); ++i)
    {
    if(!this->IsExcluded(root, *i))
      {
      this->CreateXCodeTargets(*i, targets);
      }
    }
  // loop over all targets and add link and depend info
  for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
      i != targets.end(); ++i)
    {
    cmXCodeObject* t = *i;
    this->AddDependAndLinkInformation(t);
    }
  // now create xcode depend hack makefile
  this->CreateXCodeDependHackTarget(targets);
  // now add all targets to the root object
  cmXCodeObject* allTargets = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
      i != targets.end(); ++i)
    { 
    cmXCodeObject* t = *i;
    allTargets->AddObject(t);
    cmXCodeObject* productRef = t->GetObject("productReference");
    if(productRef)
      {
      productGroupChildren->AddObject(productRef->GetObject());
      }
    }
  m_RootObject->AddAttribute("targets", allTargets);
}


//----------------------------------------------------------------------------
void 
cmGlobalXCodeGenerator::CreateXCodeDependHackTarget(
  std::vector<cmXCodeObject*>& targets)
{ 
  cmGeneratedFileStream makefileStream(m_CurrentXCodeHackMakefile.c_str());
  if(!makefileStream)
    {
    cmSystemTools::Error("Could not create",
                         m_CurrentXCodeHackMakefile.c_str());
    return;
    }
  makefileStream.SetCopyIfDifferent(true);
  // one more pass for external depend information not handled
  // correctly by xcode
  makefileStream << "# DO NOT EDIT\n";
  makefileStream << "# This makefile makes sure all linkable targets are \n";
  makefileStream 
    << "# up-to-date with anything they link to,avoiding a bug in XCode 1.5\n";
  makefileStream << "all: ";
  for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
      i != targets.end(); ++i)
    {
    cmXCodeObject* target = *i;
    cmTarget* t =target->GetcmTarget();
    if(t->GetType() == cmTarget::EXECUTABLE ||
       t->GetType() == cmTarget::SHARED_LIBRARY ||
       t->GetType() == cmTarget::MODULE_LIBRARY)
      {
      makefileStream << "\\\n\t"
                     << this->AddConfigDir(this->
                                           ConvertToRelativeForMake(this->GetTargetFullPath(target->GetcmTarget()).c_str()).c_str());
      }
    }
  makefileStream << "\n\n"; 
  makefileStream 
    << "# For each target create a dummy rule "
    "so the target does not have to exist\n";
  std::set<cmStdString> emitted;
  for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
      i != targets.end(); ++i)
    {
    cmXCodeObject* target = *i;
    std::vector<cmStdString> const& deplibs = target->GetDependLibraries();
    for(std::vector<cmStdString>::const_iterator d = deplibs.begin();
        d != deplibs.end(); ++d)
      {
      if(emitted.insert(*d).second)
        {
        makefileStream << 
          this->AddConfigDir(this->ConvertToRelativeForMake(d->c_str()).c_str())
                       << ":\n";
        }
      }
    }
  makefileStream << "\n\n";
  makefileStream << 
    "# Each linkable target depends on everything it links to.\n";
  makefileStream 
    << "#And the target is removed if it is older than what it linkes to\n";
  
  for(std::vector<cmXCodeObject*>::iterator i = targets.begin();
      i != targets.end(); ++i)
    {
    cmXCodeObject* target = *i;
    cmTarget* t =target->GetcmTarget();
    if(t->GetType() == cmTarget::EXECUTABLE ||
       t->GetType() == cmTarget::SHARED_LIBRARY ||
       t->GetType() == cmTarget::MODULE_LIBRARY)
      {
      std::vector<cmStdString> const& deplibs = target->GetDependLibraries();
      std::string tfull = this->GetTargetFullPath(target->GetcmTarget());
      makefileStream << this->AddConfigDir(
        this->ConvertToRelativeForMake(tfull.c_str()).c_str()) << ": ";
      for(std::vector<cmStdString>::const_iterator d = deplibs.begin();
          d != deplibs.end(); ++d)
        {
        makefileStream << "\\\n\t" << 
          this->AddConfigDir(
            this->ConvertToRelativeForMake(d->c_str()).c_str());
        }
      makefileStream << "\n";
      makefileStream << "\t/bin/rm -f "
                     << 
        this->AddConfigDir(
          this->ConvertToRelativeForMake(tfull.c_str()).c_str())
                     << "\n";
      makefileStream << "\n\n";
      }
    }
}

//----------------------------------------------------------------------------
std::string
cmGlobalXCodeGenerator::AddConfigDir(const char* d)
{
  if(m_XcodeVersion == 15)
    {
    return std::string(d);
    }
  std::string dir = d;
  if(cmSystemTools::FileExists(d))
    {
    return dir;
    }
  std::string::size_type pos = dir.rfind("/");
  if(pos == dir.npos)
    {
    return dir;
    }
  std::string ret = dir.substr(0, pos);
  ret += "/${CONFIGURATION}";
  ret += dir.substr(pos);
  return ret;
}
//----------------------------------------------------------------------------
void
cmGlobalXCodeGenerator::OutputXCodeProject(cmLocalGenerator* root,
                                           std::vector<cmLocalGenerator*>& 
                                           generators)
{
  if(generators.size() == 0)
    {
    return;
    }
  this->CreateXCodeObjects(root,
                           generators);
  std::string xcodeDir = root->GetMakefile()->GetStartOutputDirectory();
  xcodeDir += "/";
  xcodeDir += root->GetMakefile()->GetProjectName();
  xcodeDir += ".xcode";
  if(m_XcodeVersion > 20)
    {
    xcodeDir += "proj";
    }  
  cmSystemTools::MakeDirectory(xcodeDir.c_str());
  std::string xcodeProjFile = xcodeDir + "/project.pbxproj";
  cmGeneratedFileStream fout(xcodeProjFile.c_str());
  fout.SetCopyIfDifferent(true);
  if(!fout)
    {
    return;
    }
  this->WriteXCodePBXProj(fout, root, generators);
  this->ClearXCodeObjects();
}

//----------------------------------------------------------------------------
void 
cmGlobalXCodeGenerator::WriteXCodePBXProj(std::ostream& fout,
                                          cmLocalGenerator* ,
                                          std::vector<cmLocalGenerator*>& )
{
  fout << "// !$*UTF8*$!\n";
  fout << "{\n";
  cmXCodeObject::Indent(1, fout);
  fout << "archiveVersion = 1;\n";
  cmXCodeObject::Indent(1, fout);
  fout << "classes = {\n";
  cmXCodeObject::Indent(1, fout);
  fout << "};\n";
  cmXCodeObject::Indent(1, fout);
  fout << "objectVersion = 39;\n";
  cmXCodeObject::PrintList(m_XCodeObjects, fout);
  cmXCodeObject::Indent(1, fout);
  fout << "rootObject = " << m_RootObject->GetId() << ";\n";
  fout << "}\n";
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::GetDocumentation(cmDocumentationEntry& entry)
  const
{
  entry.name = this->GetName();
  entry.brief = "Generate XCode project files.";
  entry.full = "";
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::ConvertToRelativeForMake(const char* p)
{
  if ( !m_CurrentMakefile->IsOn("CMAKE_USE_RELATIVE_PATHS") )
    {
    return cmSystemTools::ConvertToOutputPath(p);
    }
  else
    {
    std::string ret = 
      this->ConvertToRelativePath(m_CurrentOutputDirectoryComponents, p);
    return cmSystemTools::ConvertToOutputPath(ret.c_str());
    }
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::ConvertToRelativeForXCode(const char* p)
{
  if ( !m_CurrentMakefile->IsOn("CMAKE_USE_RELATIVE_PATHS") )
    {
    return cmSystemTools::ConvertToOutputPath(p);
    }
  else
    {
    std::string ret = 
      this->ConvertToRelativePath(m_ProjectOutputDirectoryComponents, p);
    return cmSystemTools::ConvertToOutputPath(ret.c_str());
    }
}

std::string cmGlobalXCodeGenerator::XCodeEscapePath(const char* p)
{
  std::string ret = p;
  if(ret.find(' ') != ret.npos)
    {
    std::string t = ret;
    ret = "\\\"";
    ret += t;
    ret += "\\\"";
    }
  return ret;
}
