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
#include "cmLocalXCodeGenerator.h"
#include "cmMakefile.h"
#include "cmXCodeObject.h"
#include "cmake.h"
#include "cmGeneratedFileStream.h"
#include "cmSourceFile.h"


//TODO
// RUN_TESTS
// add a group for Sources Headers, and other cmake group stuff

// for each target create a custom build phase that is run first
// it can have inputs and outputs should just be a makefile

// per file flags howto
// 115281011528101152810000 = {
//    fileEncoding = 4;
// isa = PBXFileReference;
// lastKnownFileType = sourcecode.cpp.cpp;
// path = /Users/kitware/Bill/CMake/Source/cmakemain.cxx;
// refType = 0;
// sourceTree = "<absolute>";
// };
// 115285011528501152850000 = {
// fileRef = 115281011528101152810000;
// isa = PBXBuildFile;
// settings = {
// COMPILER_FLAGS = "-Dcmakemakeindefflag";
// };
// };

// custom commands and clean up custom targets
// do I need an ALL_BUILD yes
// exe/lib output paths

//----------------------------------------------------------------------------
cmGlobalXCodeGenerator::cmGlobalXCodeGenerator()
{
  m_FindMakeProgramFile = "CMakeFindXCode.cmake";
  m_RootObject = 0;
  m_MainGroupChildren = 0;
  m_SourcesGroupChildren = 0;
  m_CurrentMakefile = 0;
  m_CurrentLocalGenerator = 0;
  m_ExternalGroupChildren = 0;
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::EnableLanguage(std::vector<std::string>const&
                                            lang,
                                            cmMakefile * mf)
{ 
  mf->AddDefinition("CMAKE_CFG_INTDIR",".");
  mf->AddDefinition("CMAKE_GENERATOR_CC", "gcc");
  mf->AddDefinition("CMAKE_GENERATOR_CXX", "g++");
  mf->AddDefinition("CMAKE_GENERATOR_NO_COMPILER_ENV", "1");
  this->cmGlobalGenerator::EnableLanguage(lang, mf);
}

//----------------------------------------------------------------------------
int cmGlobalXCodeGenerator::Build(
  const char *, 
  const char *bindir, 
  const char *projectName,
  const char *targetName,
  std::string *output,
  const char *makeCommandCSTR,
  const char *,
  bool )
{
  // now build the test
  if(makeCommandCSTR == 0 || !strlen(makeCommandCSTR))
    {
    cmSystemTools::Error(
      "Generator cannot find the appropriate make command.");
    return 1;
    }
  std::string makeCommand = 
    cmSystemTools::ConvertToOutputPath(makeCommandCSTR);
  std::string lowerCaseCommand = makeCommand;
  cmSystemTools::LowerCase(lowerCaseCommand);

  /**
   * Run an executable command and put the stdout in output.
   */
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  cmSystemTools::ChangeDirectory(bindir);

  makeCommand += " -project ";
  makeCommand += projectName;
  makeCommand += ".xcode";
  makeCommand += " build -target ";
  if (targetName)
    {
    makeCommand += targetName;
    }
  else
    {
    makeCommand += "ALL_BUILD";
    }
  makeCommand += " -buildstyle Development ";
  makeCommand += " SYMROOT=";
  makeCommand += cmSystemTools::ConvertToOutputPath(bindir);
  int retVal;
  int timeout = cmGlobalGenerator::s_TryCompileTimeout;
  if (!cmSystemTools::RunSingleCommand(makeCommand.c_str(), output, &retVal, 
                                       0, false, timeout))
    {
    cmSystemTools::Error("Generator: execution of xcodebuild failed.");
    // return to the original directory
    cmSystemTools::ChangeDirectory(cwd.c_str());
    return 1;
    }
  cmSystemTools::ChangeDirectory(cwd.c_str());
  return retVal;
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
    m_CurrentMakefile = root->GetMakefile();
    m_OutputDir = m_CurrentMakefile->GetHomeOutputDirectory();
    m_OutputDir = cmSystemTools::CollapseFullPath(m_OutputDir.c_str());
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
  mf->AddUtilityCommand("install", false, no_output, no_depends,
                        cmake_command.c_str(),
                        "-P", "cmake_install.cmake");
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
  makecommand.push_back(this->ConvertToRelativeOutputPath(dir.c_str()));
  makecommand.push_back("-f");
  makecommand.push_back(this->ConvertToRelativeOutputPath(
                          m_CurrentXCodeHackMakefile.c_str()));
  cmCustomCommandLines commandLines;
  commandLines.push_back(makecommand);
  mf->AddUtilityCommand("XCODE_DEPEND_HELPER", false, no_output, no_depends,
                        commandLines);
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
void cmGlobalXCodeGenerator::ClearXCodeObjects()
{
  m_TargetDoneSet.clear();
  for(unsigned int i = 0; i < m_XCodeObjects.size(); ++i)
    {
    delete m_XCodeObjects[i];
    }
  m_XCodeObjects.clear();
}

//----------------------------------------------------------------------------
cmXCodeObject* 
cmGlobalXCodeGenerator::CreateObject(cmXCodeObject::PBXType ptype)
{
  cmXCodeObject* obj = new cmXCodeObject(ptype, cmXCodeObject::OBJECT);
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
  m_SourcesGroupChildren->AddObject(fileRef);
  cmXCodeObject* buildFile = this->CreateObject(cmXCodeObject::PBXBuildFile);
  buildFile->AddAttribute("fileRef", this->CreateObjectReference(fileRef));
  cmXCodeObject* settings = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  settings->AddAttribute("COMPILER_FLAGS", this->CreateString(flags.c_str()));
  buildFile->AddAttribute("settings", settings);
  fileRef->AddAttribute("fileEncoding", this->CreateString("4"));
  const char* lang = 
    this->GetLanguageFromExtension(sf->GetSourceExtension().c_str());
  std::string sourcecode = "sourcecode";
  if(sf->GetSourceExtension() == "o")
    {
    sourcecode = "compiled.mach-o.objfile";
    }
  else if(sf->GetSourceExtension() == "mm")
    {
    sourcecode += ".cpp.objcpp";
    }
  else if(!lang)
    {
    std::string ext = ".";
    ext = sf->GetSourceExtension();
    sourcecode += ext;
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
    this->ConvertToRelativeOutputPath(sf->GetFullPath().c_str());
  fileRef->AddAttribute("path", this->CreateString(path.c_str()));
  fileRef->AddAttribute("refType", this->CreateString("4"));
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
     tname == "install" || tname == "RUN_TESTS")
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

  
//----------------------------------------------------------------------------
void 
cmGlobalXCodeGenerator::CreateXCodeTargets(cmLocalGenerator* gen,
                                           std::vector<cmXCodeObject*>& 
                                           targets)
{
  m_CurrentLocalGenerator = gen;
  m_CurrentMakefile = gen->GetMakefile();
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
    cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    sourceBuildPhase->AddAttribute("files", buildFiles);
    sourceBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                                   this->CreateString("0"));
    std::vector<cmSourceFile*> &classes = l->second.GetSourceFiles();
    // add all the sources
    std::vector<cmXCodeObject*> externalObjFiles;
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
      else
        {
        buildFiles->AddObject(xsf);
        }
      }
    // create header build phase
    cmXCodeObject* headerBuildPhase = 
      this->CreateObject(cmXCodeObject::PBXHeadersBuildPhase);
    headerBuildPhase->AddAttribute("buildActionMask",
                                   this->CreateString("2147483647"));
    buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    headerBuildPhase->AddAttribute("files", buildFiles);
    headerBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing",
                                   this->CreateString("0"));
    
    // create framework build phase
    cmXCodeObject* frameworkBuildPhase =
      this->CreateObject(cmXCodeObject::PBXFrameworksBuildPhase);
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
  
  // create prebuild phase
  cmXCodeObject* cmakeRulesBuildPhase = 0;
  if(commands.size())
    {
    cmakeRulesBuildPhase = 
      this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase);
    cmakeRulesBuildPhase->AddAttribute("buildActionMask",
                                this->CreateString("2147483647"));
    cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    cmakeRulesBuildPhase->AddAttribute("files", buildFiles);
    cmakeRulesBuildPhase->AddAttribute("name", 
                                this->CreateString("CMake Rules"));
    cmakeRulesBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                                this->CreateString("0"));
    cmakeRulesBuildPhase->AddAttribute("shellPath",
                                       this->CreateString("/bin/sh"));
    this->AddCommandsToBuildPhase(cmakeRulesBuildPhase, cmtarget, commands,
      "cmakeRulesCommands");
    }
  // create prebuild phase
  cmXCodeObject* preBuildPhase = 0;
  if(prebuild.size())
    {
    preBuildPhase = 
      this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase);
    preBuildPhase->AddAttribute("buildActionMask",
                                this->CreateString("2147483647"));
    cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    preBuildPhase->AddAttribute("files", buildFiles);
    preBuildPhase->AddAttribute("name", 
                                this->CreateString("CMake PreBuild Rules"));
    preBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                                this->CreateString("0"));
    preBuildPhase->AddAttribute("shellPath", this->CreateString("/bin/sh"));
    this->AddCommandsToBuildPhase(preBuildPhase, cmtarget, prebuild,
      "preBuildCommands");
    }
  // create prebuild phase
  cmXCodeObject* preLinkPhase = 0;
  if(prelink.size())
    {
    preLinkPhase = 
      this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase);
    preLinkPhase->AddAttribute("buildActionMask",
                               this->CreateString("2147483647"));
    cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    preLinkPhase->AddAttribute("files", buildFiles);
    preLinkPhase->AddAttribute("name", 
                               this->CreateString("CMake PreLink Rules"));
    preLinkPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                               this->CreateString("0"));
    preLinkPhase->AddAttribute("shellPath", this->CreateString("/bin/sh"));
    this->AddCommandsToBuildPhase(preLinkPhase, cmtarget, prelink,
                                  "preLinkCommands");
    }
  // create prebuild phase
  cmXCodeObject* postBuildPhase = 0;
  if(postbuild.size())
    {
    postBuildPhase = 
      this->CreateObject(cmXCodeObject::PBXShellScriptBuildPhase);
    postBuildPhase->AddAttribute("buildActionMask",
                                 this->CreateString("2147483647"));
    cmXCodeObject* buildFiles = this->CreateObject(cmXCodeObject::OBJECT_LIST);
    postBuildPhase->AddAttribute("files", buildFiles);
    postBuildPhase->AddAttribute("name", 
                                 this->CreateString("CMake PostBuild Rules"));
    postBuildPhase->AddAttribute("runOnlyForDeploymentPostprocessing", 
                                 this->CreateString("0"));
    postBuildPhase->AddAttribute("shellPath", this->CreateString("/bin/sh"));
    this->AddCommandsToBuildPhase(postBuildPhase, cmtarget, postbuild,
                                  "postBuildCommands");
    }
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
void 
cmGlobalXCodeGenerator::AddCommandsToBuildPhase(cmXCodeObject* buildphase,
                                                cmTarget& target,
                                                std::vector<cmCustomCommand> 
                                                const & commands,
                                                const char* name)
{
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
          ConvertToRelativeOutputPath(cc.GetOutput());
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
          ->ConvertToRelativeOutputPath(cc.GetOutput()) << ": ";
        }
      else
        {
        makefileStream << tname[&cc] << ": ";
        }
      for(std::vector<std::string>::const_iterator d = cc.GetDepends().begin();
          d != cc.GetDepends().end(); ++d)
        {
        if(!this->FindTarget(d->c_str()))
          {
          makefileStream << "\\\n" << *d;
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
      for(cmCustomCommandLines::const_iterator cl = cc.GetCommandLines().begin();
          cl != cc.GetCommandLines().end(); ++cl)
        {
        // Build the command line in a single string.
        const cmCustomCommandLine& commandLine = *cl;
        std::string cmd = commandLine[0];
        cmSystemTools::ReplaceString(cmd, "/./", "/");
        cmd = this->ConvertToRelativeOutputPath(cmd.c_str());
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
  cdir = this->ConvertToRelativeOutputPath(cdir.c_str());
  std::string makecmd = "make -C ";
  makecmd += cdir;
  makecmd += " -f ";
  makecmd += this->ConvertToRelativeOutputPath(makefile.c_str());
  cmSystemTools::ReplaceString(makecmd, "\\ ", "\\\\ ");
  buildphase->AddAttribute("shellScript", this->CreateString(makecmd.c_str()));
}


//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateBuildSettings(cmTarget& target,
                                                 cmXCodeObject* buildSettings,
                                                 std::string& fileType,
                                                 std::string& productType,
                                                 std::string& productName)
{
  this->ConfigureOutputPaths();
  std::string flags;
  bool shared = ((target.GetType() == cmTarget::SHARED_LIBRARY) ||
                 (target.GetType() == cmTarget::MODULE_LIBRARY));
  if(shared)
    {
    flags += "-D";
    if(const char* custom_export_name = target.GetProperty("DEFINE_SYMBOL"))
      {
        flags += custom_export_name;
      }
    else
      {
      std::string in = target.GetName();
      in += "_EXPORTS";
      flags += cmSystemTools::MakeCindentifier(in.c_str());
      }
    }
  const char* lang = target.GetLinkerLanguage(this);
  if(lang)
    {
    // Add language-specific flags.
    m_CurrentLocalGenerator->AddLanguageFlags(flags, lang);
    
    // Add shared-library flags if needed.
    m_CurrentLocalGenerator->AddSharedFlags(flags, lang, shared);
    }

  // Add define flags
  m_CurrentLocalGenerator->AppendFlags(flags,
                                       m_CurrentMakefile->GetDefineFlags());
  cmSystemTools::ReplaceString(flags, "\"", "\\\"");
  productName = target.GetName();
  bool needLinkDirs = true;
  
  switch(target.GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      {
      needLinkDirs = false;
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
      buildSettings->AddAttribute("OTHER_LDFLAGS",
                                  this->CreateString("-bundle"));
      productType = "com.apple.product-type.library.dynamic";
      fileType = "compiled.mach-o.dylib";
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
      buildSettings->AddAttribute("OTHER_LDFLAGS",
                                  this->CreateString("-dynamiclib"));
      productType = "com.apple.product-type.library.dynamic";
      fileType = "compiled.mach-o.dylib";
      break;
      }
    case cmTarget::EXECUTABLE:
      if(m_ExecutableOutputPath.size())
        {
        buildSettings->AddAttribute("SYMROOT", 
                                    this->CreateString
                                    (m_ExecutableOutputPath.c_str()));
        }
      fileType = "compiled.mach-o.executable";
      productType = "com.apple.product-type.tool";
      break;
    case cmTarget::UTILITY:
      
      break;
    case cmTarget::INSTALL_FILES:
      break;
    case cmTarget::INSTALL_PROGRAMS:
      break;
    }
  
  std::string dirs;
  if(needLinkDirs)
    {
    // Try to emit each search path once
    std::set<cmStdString> emitted;
    // Some search paths should never be emitted
    emitted.insert("");
    emitted.insert("/usr/lib");
    std::vector<std::string> const& linkdirs =
      target.GetLinkDirectories();
    for(std::vector<std::string>::const_iterator l = linkdirs.begin();
        l != linkdirs.end(); ++l)
      {
      std::string libpath = 
        this->ConvertToRelativeOutputPath(l->c_str());
      if(emitted.insert(libpath).second)
        {
        dirs += libpath + " ";
        }
      }
    if(dirs.size())
      {
      buildSettings->AddAttribute("LIBRARY_SEARCH_PATHS", 
                                  this->CreateString(dirs.c_str()));
      }
    }
  dirs = "";
  std::vector<std::string>& includes =
    m_CurrentMakefile->GetIncludeDirectories();
  std::vector<std::string>::iterator i = includes.begin();
  for(;i != includes.end(); ++i)
    {
    std::string incpath = 
      this->ConvertToRelativeOutputPath(i->c_str());
    dirs += incpath + " ";
    }
  if(dirs.size())
    {
    buildSettings->AddAttribute("HEADER_SEARCH_PATHS", 
                                this->CreateString(dirs.c_str()));
    }
  buildSettings->AddAttribute("GCC_OPTIMIZATION_LEVEL", 
                              this->CreateString("0"));
  buildSettings->AddAttribute("INSTALL_PATH", 
                              this->CreateString(""));
  buildSettings->AddAttribute("OPTIMIZATION_CFLAGS", 
                              this->CreateString(""));
  buildSettings->AddAttribute("OTHER_CFLAGS", 
                              this->CreateString(flags.c_str()));
  buildSettings->AddAttribute("OTHER_LDFLAGS",
                              this->CreateString(""));
  buildSettings->AddAttribute("OTHER_REZFLAGS", 
                              this->CreateString(""));
  buildSettings->AddAttribute("SECTORDER_FLAGS",
                              this->CreateString(""));
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
                            productTypeString, productName);
  target->AddAttribute("buildSettings", buildSettings);
  cmXCodeObject* dependencies = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(cmtarget.GetName()));
  target->AddAttribute("productName",this->CreateString(cmtarget.GetName()));
  target->SetcmTarget(&cmtarget);
  return target;
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
  this->CreateBuildSettings(cmtarget, 
                            buildSettings, fileTypeString, 
                            productTypeString, productName);
  target->AddAttribute("buildSettings", buildSettings);
  cmXCodeObject* dependencies = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  target->AddAttribute("dependencies", dependencies);
  target->AddAttribute("name", this->CreateString(cmtarget.GetName()));
  target->AddAttribute("productName",this->CreateString(cmtarget.GetName()));

  cmXCodeObject* fileRef = this->CreateObject(cmXCodeObject::PBXFileReference);
  fileRef->AddAttribute("explicitFileType", 
                        this->CreateString(fileTypeString.c_str()));
  fileRef->AddAttribute("path", this->CreateString(productName.c_str()));
  fileRef->AddAttribute("refType", this->CreateString("0"));
  fileRef->AddAttribute("sourceTree",
                        this->CreateString("BUILT_PRODUCTS_DIR"));
  
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
    targetdep->AddAttribute("target",
                            this->CreateObjectReference(dependTarget));
    targetdep->AddAttribute("targetProxy", 
                            this->CreateObjectReference(container));
    dependTarget->SetPBXTargetDependency(targetdep);
    }
    
  cmXCodeObject* depends = target->GetObject("dependencies");
  if(!depends)
    {
    std::cerr << "target does not have dependencies attribute error...\n";
    }
  else
    {
    depends->AddUniqueObject(targetdep);
    }
}

//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::AddLinkLibrary(cmXCodeObject* target,
                                            const char* library,
                                            cmTarget* dtarget)
{
  // if the library is a full path then create a file reference
  // and build file and add them to the PBXFrameworksBuildPhase
  // for the target
  std::string file = library;
  if(cmSystemTools::FileIsFullPath(library))
    {
    target->AddDependLibrary(library);
    std::string dir;
    cmSystemTools::SplitProgramPath(library, dir, file);
    // add the library path to the search path for the target
    cmXCodeObject* bset = target->GetObject("buildSettings");
    if(bset)
      {
      cmXCodeObject* spath = bset->GetObject("LIBRARY_SEARCH_PATHS");
      if(spath)
        {
        std::string libs = spath->GetString();
        // remove double quotes
        libs = libs.substr(1, libs.size()-2);
        libs += " ";
        libs += this->ConvertToRelativeOutputPath(dir.c_str());
        spath->SetString(libs.c_str());
        }
      else
        {
        std::string libs = this->ConvertToRelativeOutputPath(dir.c_str());
        bset->AddAttribute("LIBRARY_SEARCH_PATHS",
                           this->CreateString(libs.c_str()));
        }
      }
    cmsys::RegularExpression libname("^lib([^/]*)(\\.a|\\.dylib).*");
    if(libname.find(file))
      {
      file = libname.match(1);
      }
    }
  else
    {
    if(dtarget)
      {
      target->AddDependLibrary(this->GetTargetFullPath(dtarget).c_str());
      }
    }
  
  // if the library is not a full path then add it with a -l flag
  // to the settings of the target
  cmXCodeObject* settings = target->GetObject("buildSettings");
  cmXCodeObject* ldflags = settings->GetObject("OTHER_LDFLAGS");
  std::string link = ldflags->GetString();
  cmSystemTools::ReplaceString(link, "\"", "");
  cmsys::RegularExpression reg("^([ \t]*\\-[lWRB])|([ \t]*\\-framework)|(\\${)|([ \t]*\\-pthread)|([ \t]*`)");
  // if the library is not already in the form required by the compiler
  // add a -l infront of the name
  link += " ";
  if(!reg.find(file))
    {
    link += "-l";
    }
  link +=  file;
  ldflags->SetString(link.c_str());
}

//----------------------------------------------------------------------------
std::string cmGlobalXCodeGenerator::GetTargetFullPath(cmTarget* target)
{
  std::string libPath;
  cmXCodeObject* xtarget = this->FindXCodeTarget(target);
  cmXCodeObject* bset = xtarget->GetObject("buildSettings");
  cmXCodeObject* spath = bset->GetObject("SYMROOT");
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
    std::cerr << "Error no target on xobject\n";
    return;
    }
  
  cmTarget::LinkLibraries::const_iterator j, jend;
  j = cmtarget->GetLinkLibraries().begin();
  jend = cmtarget->GetLinkLibraries().end();
  for(;j!= jend; ++j)
    {
    cmTarget* t = this->FindTarget(j->first.c_str());
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
        this->AddLinkLibrary(target, j->first.c_str());
        }
      }
    }
  std::set<cmStdString>::const_iterator i, end;
  // write utility dependencies.
  i = cmtarget->GetUtilities().begin();
  end = cmtarget->GetUtilities().end();
  for(;i!= end; ++i)
    {
    cmTarget* t = this->FindTarget(i->c_str());
    cmXCodeObject* dptarget = this->FindXCodeTarget(t);
    if(dptarget)
      {
      this->AddDependTarget(target, dptarget);
      }
    else
      {
      std::cerr << "Error Utility: " << i->c_str() << "\n";
      std::cerr << "Is on the target " << cmtarget->GetName() << "\n";
      std::cerr << "But it has no xcode target created yet??\n";
      std::cerr << "Current project is "
                << m_CurrentMakefile->GetProjectName() << "\n";
      }
    }
}


//----------------------------------------------------------------------------
void cmGlobalXCodeGenerator::CreateXCodeObjects(cmLocalGenerator* root,
                                                std::vector<cmLocalGenerator*>&
                                                generators
  )
{
  this->ClearXCodeObjects(); 
  m_RootObject = 0;
  m_ExternalGroupChildren = 0;
  m_SourcesGroupChildren = 0;
  m_MainGroupChildren = 0;
  cmXCodeObject* group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("NO"));
  cmXCodeObject* developBuildStyle = 
    this->CreateObject(cmXCodeObject::PBXBuildStyle);
  developBuildStyle->AddAttribute("name", this->CreateString("Development"));
  developBuildStyle->AddAttribute("buildSettings", group);
  
  group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  group->AddAttribute("COPY_PHASE_STRIP", this->CreateString("YES"));
  cmXCodeObject* deployBuildStyle =
    this->CreateObject(cmXCodeObject::PBXBuildStyle);
  deployBuildStyle->AddAttribute("name", this->CreateString("Deployment"));
  deployBuildStyle->AddAttribute("buildSettings", group);

  cmXCodeObject* listObjs = this->CreateObject(cmXCodeObject::OBJECT_LIST);
  listObjs->AddObject(developBuildStyle);
  listObjs->AddObject(deployBuildStyle);
  
  cmXCodeObject* mainGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  m_MainGroupChildren = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  mainGroup->AddAttribute("children", m_MainGroupChildren);
  mainGroup->AddAttribute("refType", this->CreateString("4"));
  mainGroup->AddAttribute("sourceTree", this->CreateString("<group>"));

  cmXCodeObject* sourcesGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  m_SourcesGroupChildren = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  sourcesGroup->AddAttribute("name", this->CreateString("Sources"));
  sourcesGroup->AddAttribute("children", m_SourcesGroupChildren);
  sourcesGroup->AddAttribute("refType", this->CreateString("4"));
  sourcesGroup->AddAttribute("sourceTree", this->CreateString("<group>"));
  m_MainGroupChildren->AddObject(sourcesGroup);
  
  cmXCodeObject* externalGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  m_ExternalGroupChildren = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  externalGroup->AddAttribute("name", 
                              this->CreateString("External Libraries and Frameworks"));
  externalGroup->AddAttribute("children", m_ExternalGroupChildren);
  externalGroup->AddAttribute("refType", this->CreateString("4"));
  externalGroup->AddAttribute("sourceTree", this->CreateString("<group>"));
  m_MainGroupChildren->AddObject(externalGroup);
  
  cmXCodeObject* productGroup = this->CreateObject(cmXCodeObject::PBXGroup);
  productGroup->AddAttribute("name", this->CreateString("Products"));
  productGroup->AddAttribute("refType", this->CreateString("4"));
  productGroup->AddAttribute("sourceTree", this->CreateString("<group>"));
  cmXCodeObject* productGroupChildren = 
    this->CreateObject(cmXCodeObject::OBJECT_LIST);
  productGroup->AddAttribute("children", productGroupChildren);
  m_MainGroupChildren->AddObject(productGroup);
  
  
  m_RootObject = this->CreateObject(cmXCodeObject::PBXProject);
  group = this->CreateObject(cmXCodeObject::ATTRIBUTE_GROUP);
  m_RootObject->AddAttribute("mainGroup", 
                             this->CreateObjectReference(mainGroup));
  m_RootObject->AddAttribute("buildSettings", group);
  m_RootObject->AddAttribute("buildSyles", listObjs);
  m_RootObject->AddAttribute("hasScannedForEncodings",
                             this->CreateString("0"));
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
                     << this->GetTargetFullPath(target->GetcmTarget());
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
        makefileStream << *d << ":\n";
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
      makefileStream << tfull << ": ";
      for(std::vector<cmStdString>::const_iterator d = deplibs.begin();
          d != deplibs.end(); ++d)
        {
        makefileStream << "\\\n\t" << *d;
        }
      makefileStream << "\n";
      makefileStream << "\t/bin/rm -f " << tfull << "\n";
      makefileStream << "\n\n";
      }
    }
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
  cmSystemTools::MakeDirectory(xcodeDir.c_str());
  xcodeDir += "/project.pbxproj";
  cmGeneratedFileStream fout(xcodeDir.c_str());
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
std::string cmGlobalXCodeGenerator::ConvertToRelativeOutputPath(const char* p)
{
  if ( !m_CurrentMakefile->IsOn("CMAKE_USE_RELATIVE_PATHS") )
    {
    return cmSystemTools::ConvertToOutputPath(p);
    }
  // NOTE, much of this was copied from 
  // cmLocalGenerator::ConvertToRelativeOutputPath
  // fixes here should be made there as well.
  //
  // copy to a string class
  std::string pathIn = p;
  // check to see if the path is already relative, it is
  // considered relative if one of the following is true
  // - has no / in it at all
  // - does not start with / or drive leter :
  // - starts with a ".."
  if(pathIn.find('/') == pathIn.npos ||        
     (pathIn[0] != '/' && pathIn[1] != ':') || 
     pathIn.find("..") == 0)
    {
    return cmSystemTools::ConvertToOutputPath(p);
    } 
  // Given that we are in m_CurrentOutputDirectory how to we
  // get to pathIn with a relative path, store in ret
  std::string ret = 
    cmSystemTools::RelativePath(m_OutputDir.c_str(), pathIn.c_str());
  
  // If the path is 0 sized make it a .
  // this happens when pathIn is the same as m_CurrentOutputDirectory
  if(ret.size() == 0)
    {
    ret = ".";
    }
  // if there was a trailing / there still is one, and
  // if there was not one, there still is not one
  if(ret[ret.size()-1] == '/' && 
     pathIn[pathIn.size()-1] != '/')
    {
    ret.erase(ret.size()-1, 1);
    }
  if(ret[ret.size()-1] != '/' && 
     pathIn[pathIn.size()-1] == '/')
    {
    ret += "/";
    }
  // Now convert the relative path to an output path
  ret = cmSystemTools::ConvertToOutputPath(ret.c_str());
  // finally return the path 
  // at this point it should be relative and in the correct format
  // for the native build system.  (i.e. \ for windows and / for unix,
  // and correct escaping/quoting of spaces in the path
  return ret;
}
