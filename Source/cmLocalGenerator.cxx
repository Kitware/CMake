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
#include "cmLocalGenerator.h"

#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmInstallGenerator.h"
#include "cmInstallFilesGenerator.h"
#include "cmInstallScriptGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmMakefile.h"
#include "cmOrderLinkDirectories.h"
#include "cmSourceFile.h"
#include "cmTest.h"
#include "cmake.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
# define CM_LG_ENCODE_OBJECT_NAMES
# include <cmsys/MD5.h>
#endif

#include <cmsys/System.h>

#include <ctype.h> // for isalpha

#include <assert.h>

cmLocalGenerator::cmLocalGenerator()
{
  this->Makefile = new cmMakefile;
  this->Makefile->SetLocalGenerator(this);
  this->Parent = 0;
  this->WindowsShell = false;
  this->WindowsVSIDE = false;
  this->WatcomWMake = false;
  this->MinGWMake = false;
  this->NMake = false;
  this->MSYSShell = false;
  this->IgnoreLibPrefix = false;
  this->UseRelativePaths = false;
  this->Configured = false;
  this->EmitUniversalBinaryFlags = true;
  this->IsMakefileGenerator = false;
  this->RelativePathsConfigured = false;
  this->PathConversionsSetup = false;
  this->BackwardsCompatibility = 0;
  this->BackwardsCompatibilityFinal = false;
}

cmLocalGenerator::~cmLocalGenerator()
{
  delete this->Makefile;
}

void cmLocalGenerator::Configure()
{
  cmLocalGenerator* previousLg = 
                        this->GetGlobalGenerator()->GetCurrentLocalGenerator();
  this->GetGlobalGenerator()->SetCurrentLocalGenerator(this);

  // make sure the CMakeFiles dir is there
  std::string filesDir = this->Makefile->GetStartOutputDirectory();
  filesDir += cmake::GetCMakeFilesDirectory();
  cmSystemTools::MakeDirectory(filesDir.c_str());
  
  // find & read the list file
  std::string currentStart = this->Makefile->GetStartDirectory();
  currentStart += "/CMakeLists.txt";
  this->Makefile->ReadListFile(currentStart.c_str());

  // at the end of the ReadListFile handle any old style subdirs
  // first get all the subdirectories
  std::vector<cmLocalGenerator *> subdirs = this->GetChildren();
  
  // for each subdir recurse
  std::vector<cmLocalGenerator *>::iterator sdi = subdirs.begin();
  for (; sdi != subdirs.end(); ++sdi)
    {
    if (!(*sdi)->Configured)
      {
      this->Makefile->ConfigureSubDirectory(*sdi);
      }
    }  

  // Check whether relative paths should be used for optionally
  // relative paths.
  this->UseRelativePaths = this->Makefile->IsOn("CMAKE_USE_RELATIVE_PATHS");

  this->Configured = true;

  this->GetGlobalGenerator()->SetCurrentLocalGenerator(previousLg);
}

void cmLocalGenerator::SetupPathConversions()
{  
  // Setup the current output directory components for use by
  // Convert
  std::string outdir; 
  outdir =
    cmSystemTools::CollapseFullPath(this->Makefile->GetHomeDirectory());
  cmSystemTools::SplitPath(outdir.c_str(), this->HomeDirectoryComponents);
  outdir =
    cmSystemTools::CollapseFullPath(this->Makefile->GetStartDirectory());
  cmSystemTools::SplitPath(outdir.c_str(), this->StartDirectoryComponents);

  outdir = cmSystemTools::CollapseFullPath
    (this->Makefile->GetHomeOutputDirectory());
  cmSystemTools::SplitPath(outdir.c_str(), 
                           this->HomeOutputDirectoryComponents);

  outdir = cmSystemTools::CollapseFullPath
    (this->Makefile->GetStartOutputDirectory());
  cmSystemTools::SplitPath(outdir.c_str(), 
                           this->StartOutputDirectoryComponents);
}


void cmLocalGenerator::SetGlobalGenerator(cmGlobalGenerator *gg)
{
  this->GlobalGenerator = gg;

  // setup the home directories
  this->Makefile->GetProperties().SetCMakeInstance(gg->GetCMakeInstance());
  this->Makefile->SetHomeDirectory(
    gg->GetCMakeInstance()->GetHomeDirectory());
  this->Makefile->SetHomeOutputDirectory(
    gg->GetCMakeInstance()->GetHomeOutputDirectory());
}

void cmLocalGenerator::ConfigureFinalPass()
{
  this->Makefile->ConfigureFinalPass();
}

void cmLocalGenerator::TraceDependencies()
{
  // Generate the rule files for each target.
  cmTargets& targets = this->Makefile->GetTargets();
  for(cmTargets::iterator t = targets.begin(); t != targets.end(); ++t)
    {
    // INCLUDE_EXTERNAL_MSPROJECT command only affects the workspace
    // so don't build a projectfile for it
    if (strncmp(t->first.c_str(), "INCLUDE_EXTERNAL_MSPROJECT", 26) != 0)
      {
      const char* projectFilename = 0;
      if (this->IsMakefileGenerator == false)  // only use of this variable
        {
        projectFilename = t->second.GetName();
        }
      t->second.TraceDependencies(projectFilename);
      }
    }
}

void cmLocalGenerator::GenerateTestFiles()
{
  if ( !this->Makefile->IsOn("CMAKE_TESTING_ENABLED") )
    {
    return;
    }
  std::string file = this->Makefile->GetStartOutputDirectory();
  file += "/";
  file += "CTestTestfile.cmake";

  cmGeneratedFileStream fout(file.c_str());
  fout.SetCopyIfDifferent(true);

  fout << "# CMake generated Testfile for " << std::endl
       << "# Source directory: " 
       << this->Makefile->GetStartDirectory() << std::endl
       << "# Build directory: " 
       << this->Makefile->GetStartOutputDirectory() << std::endl
       << "# " << std::endl
       << "# This file replicates the SUBDIRS() and ADD_TEST() commands "
       << "from the source" << std::endl
       << "# tree CMakeLists.txt file, skipping any SUBDIRS() or "
       << "ADD_TEST() commands" << std::endl
       << "# that are excluded by CMake control structures, i.e. IF() "
       << "commands." << std::endl;
  
  const char* testIncludeFile = 
    this->Makefile->GetProperty("TEST_INCLUDE_FILE");
  if ( testIncludeFile )
    {
    fout << "INCLUDE(\"" << testIncludeFile << "\")" << std::endl;
    }
  
  const std::vector<cmTest*> *tests = this->Makefile->GetTests();
  std::vector<cmTest*>::const_iterator it;
  for ( it = tests->begin(); it != tests->end(); ++ it )
    {
    cmTest* test = *it;
    fout << "ADD_TEST(";
    fout << test->GetName() << " \"" << test->GetCommand() << "\"";
    
    std::vector<cmStdString>::const_iterator argit;
    for (argit = test->GetArguments().begin();
         argit != test->GetArguments().end(); ++argit)
      {
      // Just double-quote all arguments so they are re-parsed
      // correctly by the test system.
      fout << " \"";
      for(std::string::const_iterator c = argit->begin(); 
          c != argit->end(); ++c)
        {
        // Escape quotes within arguments.  We should escape
        // backslashes too but we cannot because it makes the result
        // inconsistent with previous behavior of this command.
        if((*c == '"'))
          {
          fout << '\\';
          }
        fout << *c;
        }
      fout << "\"";
      }
    fout << ")" << std::endl;
    cmPropertyMap::const_iterator pit;
    cmPropertyMap* mpit = &test->GetProperties();
    if ( mpit->size() )
      {
      fout << "SET_TESTS_PROPERTIES(" << test->GetName() << " PROPERTIES ";
      for ( pit = mpit->begin(); pit != mpit->end(); ++ pit )
        {
        fout << " " << pit->first.c_str() << " \"";
        const char* value = pit->second.GetValue();
        for ( ; *value; ++ value )
          {
          switch ( *value )
            {
          case '\\':
          case '"':
          case ' ':
          case '#':
          case '(':
          case ')':
          case '$':
          case '^':
            fout << "\\" << *value;
            break;
          case '\t':
            fout << "\\t";
            break;
          case '\n':
            fout << "\\n";
            break;
          case '\r':
            fout << "\\r";
            break;
          default:
            fout << *value;
            }
          }
        fout << "\"";
        }
      fout << ")" << std::endl;
      }
    }
  if ( this->Children.size())
    {
    fout << "SUBDIRS(";
    size_t i;
    std::string outDir = this->Makefile->GetStartOutputDirectory();
    outDir += "/";
    for(i = 0; i < this->Children.size(); ++i)
      {
      std::string binP = 
        this->Children[i]->GetMakefile()->GetStartOutputDirectory();
      cmSystemTools::ReplaceString(binP, outDir.c_str(), "");
      if ( i > 0 )
        {
        fout << " ";
        }
      fout << binP.c_str();
      }
    fout << ")" << std::endl << std::endl;;
    }
}

//----------------------------------------------------------------------------
void cmLocalGenerator::GenerateInstallRules()
{
  // Compute the install prefix.
  const char* prefix = this->Makefile->GetDefinition("CMAKE_INSTALL_PREFIX");
#if defined(_WIN32) && !defined(__CYGWIN__)
  std::string prefix_win32;
  if(!prefix)
    {
    if(!cmSystemTools::GetEnv("SystemDrive", prefix_win32))
      {
      prefix_win32 = "C:";
      }
    const char* project_name = this->Makefile->GetDefinition("PROJECT_NAME");
    if(project_name && project_name[0])
      {
      prefix_win32 += "/Program Files/";
      prefix_win32 += project_name;
      }
    else
      {
      prefix_win32 += "/InstalledCMakeProject";
      }
    prefix = prefix_win32.c_str();
    }
#else
  if (!prefix)
    {
    prefix = "/usr/local";
    }
#endif

  // Compute the set of configurations.
  std::vector<std::string> configurationTypes;
  if(const char* types = 
     this->Makefile->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
    {
    cmSystemTools::ExpandListArgument(types, configurationTypes);
    }
  const char* config = 0;
  if(configurationTypes.empty())
    {
    config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE");
    }

  // Choose a default install configuration.
  const char* default_config = config;
  const char* default_order[] = {"RELEASE", "MINSIZEREL",
                                 "RELWITHDEBINFO", "DEBUG", 0};
  for(const char** c = default_order; *c && !default_config; ++c)
    {
    for(std::vector<std::string>::iterator i = configurationTypes.begin();
        i != configurationTypes.end(); ++i)
      {
      if(cmSystemTools::UpperCase(*i) == *c)
        {
        default_config = i->c_str();
        }
      }
    }
  if(!default_config && !configurationTypes.empty())
    {
    default_config = configurationTypes[0].c_str();
    }
  if(!default_config)
    {
    default_config = "Release";
    }

  // Create the install script file.
  std::string file = this->Makefile->GetStartOutputDirectory();
  std::string homedir = this->Makefile->GetHomeOutputDirectory();
  std::string currdir = this->Makefile->GetCurrentOutputDirectory();
  cmSystemTools::ConvertToUnixSlashes(file);
  cmSystemTools::ConvertToUnixSlashes(homedir);
  cmSystemTools::ConvertToUnixSlashes(currdir);
  int toplevel_install = 0;
  if ( currdir == homedir )
    {
    toplevel_install = 1;
    }
  file += "/cmake_install.cmake";
  cmGeneratedFileStream fout(file.c_str());
  fout.SetCopyIfDifferent(true);

  // Write the header.
  fout << "# Install script for directory: "
       << this->Makefile->GetCurrentDirectory() << std::endl << std::endl;
  fout << "# Set the install prefix" << std::endl
       << "IF(NOT DEFINED CMAKE_INSTALL_PREFIX)" << std::endl
       << "  SET(CMAKE_INSTALL_PREFIX \"" << prefix << "\")" << std::endl
       << "ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)" << std::endl
       << "STRING(REGEX REPLACE \"/$\" \"\" CMAKE_INSTALL_PREFIX "
       << "\"${CMAKE_INSTALL_PREFIX}\")" << std::endl
       << std::endl;

  // Write support code for generating per-configuration install rules.
  fout <<
    "# Set the install configuration name.\n"
    "IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)\n"
    "  IF(BUILD_TYPE)\n"
    "    STRING(REGEX REPLACE \"^[^A-Za-z0-9_]+\" \"\"\n"
    "           CMAKE_INSTALL_CONFIG_NAME \"${BUILD_TYPE}\")\n"
    "  ELSE(BUILD_TYPE)\n"
    "    SET(CMAKE_INSTALL_CONFIG_NAME \"" << default_config << "\")\n"
    "  ENDIF(BUILD_TYPE)\n"
    "  MESSAGE(STATUS \"Install configuration: "
    "\\\"${CMAKE_INSTALL_CONFIG_NAME}\\\"\")\n"
    "ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)\n"
    "\n";

  // Write support code for dealing with component-specific installs.
  fout <<
    "# Set the component getting installed.\n"
    "IF(NOT CMAKE_INSTALL_COMPONENT)\n"
    "  IF(COMPONENT)\n"
    "    MESSAGE(STATUS \"Install component: \\\"${COMPONENT}\\\"\")\n"
    "    SET(CMAKE_INSTALL_COMPONENT \"${COMPONENT}\")\n"
    "  ELSE(COMPONENT)\n"
    "    SET(CMAKE_INSTALL_COMPONENT)\n"
    "  ENDIF(COMPONENT)\n"
    "ENDIF(NOT CMAKE_INSTALL_COMPONENT)\n"
    "\n";

  // Copy user-specified install options to the install code.
  if(const char* so_no_exe =
     this->Makefile->GetDefinition("CMAKE_INSTALL_SO_NO_EXE"))
    {
    fout <<
      "# Install shared libraries without execute permission?\n"
      "IF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)\n"
      "  SET(CMAKE_INSTALL_SO_NO_EXE \"" << so_no_exe << "\")\n"
      "ENDIF(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)\n"
      "\n";
    }

  // Ask each install generator to write its code.
  std::vector<cmInstallGenerator*> const& installers =
    this->Makefile->GetInstallGenerators();
  for(std::vector<cmInstallGenerator*>::const_iterator 
        gi = installers.begin();
      gi != installers.end(); ++gi)
    {
    (*gi)->Generate(fout, config, configurationTypes);
    }

  // Write rules from old-style specification stored in targets.
  this->GenerateTargetInstallRules(fout, config, configurationTypes);

  // Include install scripts from subdirectories.
  if(!this->Children.empty())
    {
    fout << "IF(NOT CMAKE_INSTALL_LOCAL_ONLY)\n";
    fout << "  # Include the install script for each subdirectory.\n";
    for(std::vector<cmLocalGenerator*>::const_iterator
          ci = this->Children.begin(); ci != this->Children.end(); ++ci)
      {
      if(!(*ci)->GetMakefile()->GetPropertyAsBool("EXCLUDE_FROM_ALL"))
        {
        std::string odir = (*ci)->GetMakefile()->GetStartOutputDirectory();
        cmSystemTools::ConvertToUnixSlashes(odir);
        fout << "  INCLUDE(\"" <<  odir.c_str()
             << "/cmake_install.cmake\")" << std::endl;
        }
      }
    fout << "\n";
    fout << "ENDIF(NOT CMAKE_INSTALL_LOCAL_ONLY)\n\n";
    }

  // Record the install manifest.
  if ( toplevel_install )
    {
    fout <<
      "IF(CMAKE_INSTALL_COMPONENT)\n"
      "  SET(CMAKE_INSTALL_MANIFEST \"install_manifest_"
      "${CMAKE_INSTALL_COMPONENT}.txt\")\n"
      "ELSE(CMAKE_INSTALL_COMPONENT)\n"
      "  SET(CMAKE_INSTALL_MANIFEST \"install_manifest.txt\")\n"
      "ENDIF(CMAKE_INSTALL_COMPONENT)\n\n";
    fout
      << "FILE(WRITE \""
      << homedir.c_str() << "/${CMAKE_INSTALL_MANIFEST}\" "
      << "\"\")" << std::endl;
    fout
      << "FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})" << std::endl
      << "  FILE(APPEND \""
      << homedir.c_str() << "/${CMAKE_INSTALL_MANIFEST}\" "
      << "\"${file}\\n\")" << std::endl
      << "ENDFOREACH(file)" << std::endl;
    }
}

//----------------------------------------------------------------------------
void cmLocalGenerator::GenerateTargetManifest(cmTargetManifest& manifest)
{
  // Collect the set of configuration types.
  std::vector<std::string> configNames;
  if(const char* configurationTypes =
     this->Makefile->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
    {
    cmSystemTools::ExpandListArgument(configurationTypes, configNames);
    }
  else if(const char* buildType =
          this->Makefile->GetDefinition("CMAKE_BUILD_TYPE"))
    {
    if(*buildType)
      {
      configNames.push_back(buildType);
      }
    }

  // Add our targets to the manifest for each configuration.
  cmTargets& targets = this->Makefile->GetTargets();
  for(cmTargets::iterator t = targets.begin(); t != targets.end(); ++t)
    {
    cmTarget& target = t->second;
    cmTarget::TargetType type = target.GetType();
    if(type == cmTarget::STATIC_LIBRARY ||
       type == cmTarget::SHARED_LIBRARY ||
       type == cmTarget::MODULE_LIBRARY ||
       type == cmTarget::EXECUTABLE)
      {
      if(configNames.empty())
        {
        manifest[""].insert(target.GetFullPath(0, false));
        if(type == cmTarget::SHARED_LIBRARY &&
           this->Makefile->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX"))
          {
          manifest[""].insert(target.GetFullPath(0, true));
          }
        }
      else
        {
        for(std::vector<std::string>::iterator ci = configNames.begin();
            ci != configNames.end(); ++ci)
          {
          const char* config = ci->c_str();
          manifest[config].insert(target.GetFullPath(config, false));
          if(type == cmTarget::SHARED_LIBRARY &&
             this->Makefile->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX"))
            {
            manifest[config].insert(target.GetFullPath(config, true));
            }
          }
        }
      }
    }
}

void cmLocalGenerator::AddCustomCommandToCreateObject(const char* ofname, 
                                                      const char* lang, 
                                                      cmSourceFile& source,
                                                      cmTarget& )
{ 
  std::string objectDir = cmSystemTools::GetFilenamePath(std::string(ofname));
  objectDir = this->Convert(objectDir.c_str(),START_OUTPUT,SHELL);
  std::string objectFile = this->Convert(ofname,START_OUTPUT,SHELL);
  std::string sourceFile = 
    this->Convert(source.GetFullPath().c_str(),START_OUTPUT,SHELL,true);
  std::string varString = "CMAKE_";
  varString += lang;
  varString += "_COMPILE_OBJECT";
  std::vector<std::string> rules;
  rules.push_back(this->Makefile->GetRequiredDefinition(varString.c_str()));
  varString = "CMAKE_";
  varString += lang;
  varString += "_FLAGS";
  std::string flags;
  flags += this->Makefile->GetSafeDefinition(varString.c_str());
  flags += " ";
  flags += this->GetIncludeFlags(lang);

  // Construct the command lines.
  cmCustomCommandLines commandLines;
  std::vector<std::string> commands;
  cmSystemTools::ExpandList(rules, commands);
  cmLocalGenerator::RuleVariables vars;
  vars.Language = lang;
  vars.Source = sourceFile.c_str();
  vars.Object = objectFile.c_str();
  vars.ObjectDir = objectDir.c_str();
  vars.Flags = flags.c_str();
  for(std::vector<std::string>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    // Expand the full command line string.
    this->ExpandRuleVariables(*i, vars);

    // Parse the string to get the custom command line.
    cmCustomCommandLine commandLine;
    std::vector<cmStdString> cmd = cmSystemTools::ParseArguments(i->c_str());
    for(std::vector<cmStdString>::iterator a = cmd.begin();
        a != cmd.end(); ++a)
      {
      commandLine.push_back(*a);
      }

    // Store this command line.
    commandLines.push_back(commandLine);
    }

  // Check for extra object-file dependencies.
  std::vector<std::string> depends;
  const char* additionalDeps = source.GetProperty("OBJECT_DEPENDS");
  if(additionalDeps)
    {
    cmSystemTools::ExpandListArgument(additionalDeps, depends);
    }

  // Generate a meaningful comment for the command.
  std::string comment = "Building ";
  comment += lang;
  comment += " object ";
  comment += this->Convert(ofname, START_OUTPUT);

  // Add the custom command to build the object file.
  this->Makefile->AddCustomCommandToOutput(
    ofname,
    depends,
    source.GetFullPath().c_str(),
    commandLines,
    comment.c_str(),
    this->Makefile->GetStartOutputDirectory()
    );
}

void cmLocalGenerator::AddBuildTargetRule(const char* llang, cmTarget& target)
{
  cmStdString objs;
  std::vector<std::string> objVector;
  // Add all the sources outputs to the depends of the target
  std::vector<cmSourceFile*> const& classes = target.GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin();
      i != classes.end(); ++i)
    {
    cmSourceFile* sf = *i;
    if(!sf->GetCustomCommand() &&
       !sf->GetPropertyAsBool("HEADER_FILE_ONLY") &&
       !sf->GetPropertyAsBool("EXTERNAL_OBJECT"))
      {
      std::string::size_type dir_len = 0;
      dir_len += strlen(this->Makefile->GetCurrentOutputDirectory());
      dir_len += 1;
      std::string obj = this->GetObjectFileNameWithoutTarget(*sf, dir_len);
      if(!obj.empty())
        {
        std::string ofname = this->Makefile->GetCurrentOutputDirectory();
        ofname += "/";
        ofname += obj;
        objVector.push_back(ofname);
        this->AddCustomCommandToCreateObject(ofname.c_str(), 
                                             llang, *(*i), target);
        objs += this->Convert(ofname.c_str(),START_OUTPUT,MAKEFILE);
        objs += " ";
        }
      }
    }
  std::string createRule = "CMAKE_";
  createRule += llang;
  createRule += target.GetCreateRuleVariable();
  std::string targetName = target.GetFullName();
  // Executable :
  // Shared Library:
  // Static Library:
  // Shared Module:
  std::string linkLibs; // should be set
  std::string flags; // should be set
  std::string linkFlags; // should be set 
  this->GetTargetFlags(linkLibs, flags, linkFlags, target);
  cmLocalGenerator::RuleVariables vars;
  vars.Language = llang;
  vars.Objects = objs.c_str();
  vars.ObjectDir = ".";
  vars.Target = targetName.c_str();
  vars.LinkLibraries = linkLibs.c_str();
  vars.Flags = flags.c_str();
  vars.LinkFlags = linkFlags.c_str();
 
  std::string langFlags;
  this->AddLanguageFlags(langFlags, llang, 0);
  vars.LanguageCompileFlags = langFlags.c_str();
  
  cmCustomCommandLines commandLines;
  std::vector<std::string> rules;
  rules.push_back(this->Makefile->GetRequiredDefinition(createRule.c_str()));
  std::vector<std::string> commands;
  cmSystemTools::ExpandList(rules, commands);  
  for(std::vector<std::string>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    // Expand the full command line string.
    this->ExpandRuleVariables(*i, vars);
    // Parse the string to get the custom command line.
    cmCustomCommandLine commandLine;
    std::vector<cmStdString> cmd = cmSystemTools::ParseArguments(i->c_str());
    for(std::vector<cmStdString>::iterator a = cmd.begin();
        a != cmd.end(); ++a)
      {
      commandLine.push_back(*a);
      }

    // Store this command line.
    commandLines.push_back(commandLine);
    }
  std::string targetFullPath = target.GetFullPath();
  // Generate a meaningful comment for the command.
  std::string comment = "Linking ";
  comment += llang;
  comment += " target ";
  comment += this->Convert(targetFullPath.c_str(), START_OUTPUT);
  this->Makefile->AddCustomCommandToOutput(
    targetFullPath.c_str(),
    objVector,
    0,
    commandLines,
    comment.c_str(),
    this->Makefile->GetStartOutputDirectory()
    );
  target.AddSourceFile
    (this->Makefile->GetSource(targetFullPath.c_str()));
}

  
void cmLocalGenerator
::CreateCustomTargetsAndCommands(std::set<cmStdString> const& lang)
{ 
  cmTargets &tgts = this->Makefile->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); 
      l != tgts.end(); l++)
    {
    cmTarget& target = l->second;
    switch(target.GetType())
      { 
      case cmTarget::STATIC_LIBRARY:
      case cmTarget::SHARED_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
      case cmTarget::EXECUTABLE: 
        {
        const char* llang = 
          target.GetLinkerLanguage(this->GetGlobalGenerator());
        if(!llang)
          {
          cmSystemTools::Error
            ("CMake can not determine linker language for target:",
             target.GetName());
          return;
          }
        // if the language is not in the set lang then create custom
        // commands to build the target
        if(lang.count(llang) == 0)
          {
          this->AddBuildTargetRule(llang, target);
          }
        }
        break; 
      case cmTarget::UTILITY:
      case cmTarget::GLOBAL_TARGET:
      case cmTarget::INSTALL_FILES:
      case cmTarget::INSTALL_PROGRAMS:
      case cmTarget::INSTALL_DIRECTORY:
        break;
      }
    }
}

// List of variables that are replaced when
// rules are expanced.  These variables are
// replaced in the form <var> with GetSafeDefinition(var).
// ${LANG} is replaced in the variable first with all enabled 
// languages.
static const char* ruleReplaceVars[] =
{
  "CMAKE_${LANG}_COMPILER",
  "CMAKE_SHARED_LIBRARY_CREATE_${LANG}_FLAGS",
  "CMAKE_SHARED_MODULE_CREATE_${LANG}_FLAGS",
  "CMAKE_SHARED_MODULE_${LANG}_FLAGS", 
  "CMAKE_SHARED_LIBRARY_${LANG}_FLAGS",
  "CMAKE_${LANG}_LINK_FLAGS",
  "CMAKE_SHARED_LIBRARY_SONAME_${LANG}_FLAG",
  "CMAKE_${LANG}_ARCHIVE",
  "CMAKE_AR",
  "CMAKE_CURRENT_SOURCE_DIR",
  "CMAKE_CURRENT_BINARY_DIR",
  "CMAKE_RANLIB",
  "CMAKE_LINKER",
  0
};

std::string
cmLocalGenerator::ExpandRuleVariable(std::string const& variable,
                                     const RuleVariables& replaceValues)
{
  if(replaceValues.LinkFlags)
    {
    if(variable == "LINK_FLAGS")
      {
      return replaceValues.LinkFlags;
      }
    }
  if(replaceValues.Flags)
    {
    if(variable == "FLAGS")
      {
      return replaceValues.Flags;
      }
    }
    
  if(replaceValues.Source)
    {
    if(variable == "SOURCE")
      {
      return replaceValues.Source;
      }
    }
  if(replaceValues.PreprocessedSource)
    {
    if(variable == "PREPROCESSED_SOURCE")
      {
      return replaceValues.PreprocessedSource;
      }
    }
  if(replaceValues.AssemblySource)
    {
    if(variable == "ASSEMBLY_SOURCE")
      {
      return replaceValues.AssemblySource;
      }
    }
  if(replaceValues.Object)
    {
    if(variable == "OBJECT")
      {
      return replaceValues.Object;
      }
    }
  if(replaceValues.ObjectDir)
    {
    if(variable == "OBJECT_DIR")
      {
      return replaceValues.ObjectDir;
      }
    }
  if(replaceValues.Objects)
    {
    if(variable == "OBJECTS")
      {
      return replaceValues.Objects;
      }
    }
  if(replaceValues.ObjectsQuoted)
    {
    if(variable == "OBJECTS_QUOTED")
      {
      return replaceValues.ObjectsQuoted;
      }
    }
  if(replaceValues.Defines && variable == "DEFINES")
    {
    return replaceValues.Defines;
    }
  if(replaceValues.TargetPDB )
    {
    if(variable == "TARGET_PDB")
      {
      return replaceValues.TargetPDB;
      }
    }

  if(replaceValues.Target)
    { 
    if(variable == "TARGET_QUOTED")
      {
      std::string targetQuoted = replaceValues.Target;
      if(targetQuoted.size() && targetQuoted[0] != '\"')
        {
        targetQuoted = '\"';
        targetQuoted += replaceValues.Target;
        targetQuoted += '\"';
        }
      return targetQuoted;
      }
    if(replaceValues.LanguageCompileFlags)
      {
      if(variable == "LANGUAGE_COMPILE_FLAGS")
        {
        return replaceValues.LanguageCompileFlags;
        }
      }
    if(replaceValues.Target)
      {
      if(variable == "TARGET")
        {
        return replaceValues.Target;
        }
      }
    if(variable == "TARGET_IMPLIB")
      {
      return this->TargetImplib;
      }
    if(variable == "TARGET_VERSION_MAJOR")
      {
      if(replaceValues.TargetVersionMajor)
        {
        return replaceValues.TargetVersionMajor;
        }
      else
        {
        return "0";
        }
      }
    if(variable == "TARGET_VERSION_MINOR")
      {
      if(replaceValues.TargetVersionMinor)
        {
        return replaceValues.TargetVersionMinor;
        }
      else
        {
        return "0";
        }
      }
    if(replaceValues.Target)
      {
      if(variable == "TARGET_BASE")
        {
        // Strip the last extension off the target name.
        std::string targetBase = replaceValues.Target;
        std::string::size_type pos = targetBase.rfind(".");
        if(pos != targetBase.npos)
          {
        return targetBase.substr(0, pos);
          }
        else
          {
          return targetBase;
          }
        }
      }
    }
  if(replaceValues.TargetSOName)
    {
    if(variable == "TARGET_SONAME")
      {
      if(replaceValues.Language)
        {
        std::string name = "CMAKE_SHARED_LIBRARY_SONAME_";
        name += replaceValues.Language;
        name += "_FLAG";
        if(this->Makefile->GetDefinition(name.c_str()))
          {
          return replaceValues.TargetSOName;
          }
        }
      return "";
      }
    }
  if(replaceValues.TargetInstallNameDir)
    {
    if(variable == "TARGET_INSTALLNAME_DIR")
      {
      return replaceValues.TargetInstallNameDir;
      }
    }
  if(replaceValues.LinkLibraries)
    {
    if(variable == "LINK_LIBRARIES")
      {
      return replaceValues.LinkLibraries;
      }
    }
  std::vector<std::string> enabledLanguages;
  this->GlobalGenerator->GetEnabledLanguages(enabledLanguages);
  // loop over language specific replace variables
  int pos = 0;
  while(ruleReplaceVars[pos])
    {
    for(std::vector<std::string>::iterator i = enabledLanguages.begin();   
        i != enabledLanguages.end(); ++i)   
      { 
      const char* lang = i->c_str();
      std::string actualReplace = ruleReplaceVars[pos];
      // If this is the compiler then look for the extra variable
      // _COMPILER_ARG1 which must be the first argument to the compiler 
      const char* compilerArg1 = 0;
      if(actualReplace == "CMAKE_${LANG}_COMPILER")
        {
        std::string arg1 = actualReplace + "_ARG1";
        cmSystemTools::ReplaceString(arg1, "${LANG}", lang);
        compilerArg1 = this->Makefile->GetDefinition(arg1.c_str());
        }
      if(actualReplace.find("${LANG}") != actualReplace.npos)
        {
        cmSystemTools::ReplaceString(actualReplace, "${LANG}", lang);
        }
      if(actualReplace == variable)
        {
        std::string replace = 
          this->Makefile->GetSafeDefinition(variable.c_str());
        // if the variable is not a FLAG then treat it like a path
        if(variable.find("_FLAG") == variable.npos)
          {
          std::string ret = this->ConvertToOutputForExisting(replace.c_str());
          // if there is a required first argument to the compiler add it
          // to the compiler string
          if(compilerArg1)
            {
            ret += " ";
            ret += compilerArg1;
            }
          return ret;
          }
        return replace;
        }
      }
    pos++;
    }
  return variable;
}


void 
cmLocalGenerator::ExpandRuleVariables(std::string& s,
                                      const RuleVariables& replaceValues)
{
  std::vector<std::string> enabledLanguages;
  this->GlobalGenerator->GetEnabledLanguages(enabledLanguages);
  std::string::size_type start = s.find('<');
  // no variables to expand
  if(start == s.npos)
    {
    return;
    }
  std::string::size_type pos = 0;
  std::string expandedInput;
  while(start != s.npos && start < s.size()-2)
    {
    std::string::size_type end = s.find('>', start);
    // if we find a < with no > we are done
    if(end == s.npos)
      {
      return;
      }
    char c = s[start+1];
    // if the next char after the < is not A-Za-z then
    // skip it and try to find the next < in the string
    if(!isalpha(c))
      {
      start = s.find('<', start+1);
      }
    else
      {
      // extract the var
      std::string var = s.substr(start+1,  end - start-1);
      std::string replace = this->ExpandRuleVariable(var,
                                                     replaceValues);
      expandedInput += s.substr(pos, start-pos);
      expandedInput += replace;
      // move to next one
      start = s.find('<', start+var.size()+2);
      pos = end+1;
      }
    }
  // add the rest of the input
  expandedInput += s.substr(pos, s.size()-pos);
  s = expandedInput;
}


std::string 
cmLocalGenerator::ConvertToOutputForExisting(const char* p)
{
  std::string ret = p;
  if(this->WindowsShell && ret.find(' ') != ret.npos 
     && cmSystemTools::FileExists(p))
    {
    if(cmSystemTools::GetShortPath(p, ret))
      {
      return  this->Convert(ret.c_str(), NONE, SHELL, true);
      }
    }
  return this->Convert(p, START_OUTPUT, SHELL, true);
}

const char* cmLocalGenerator::GetIncludeFlags(const char* lang)
{
  if(!lang)
    {
    return "";
    }
  if(this->LanguageToIncludeFlags.count(lang))
    {
    return this->LanguageToIncludeFlags[lang].c_str();
    }
  cmOStringStream includeFlags;
  std::vector<std::string> includes;
  this->GetIncludeDirectories(includes);
  std::vector<std::string>::iterator i;

  std::string flagVar = "CMAKE_INCLUDE_FLAG_";
  flagVar += lang;
  const char* includeFlag = 
    this->Makefile->GetSafeDefinition(flagVar.c_str());
  flagVar = "CMAKE_INCLUDE_FLAG_SEP_";
  flagVar += lang;
  const char* sep = this->Makefile->GetDefinition(flagVar.c_str());
  bool quotePaths = false;
  if(this->Makefile->GetDefinition("CMAKE_QUOTE_INCLUDE_PATHS"))
    {
    quotePaths = true;
    }
  bool repeatFlag = true; 
  // should the include flag be repeated like ie. -IA -IB
  if(!sep)
    {
    sep = " ";
    }
  else
    {
    // if there is a separator then the flag is not repeated but is only
    // given once i.e.  -classpath a:b:c
    repeatFlag = false;
    }

  // Support special system include flag if it is available and the
  // normal flag is repeated for each directory.
  std::string sysFlagVar = "CMAKE_INCLUDE_SYSTEM_FLAG_";
  sysFlagVar += lang;
  const char* sysIncludeFlag = 0;
  if(repeatFlag)
    {
    sysIncludeFlag = this->Makefile->GetDefinition(sysFlagVar.c_str());
    }

  bool flagUsed = false;
  std::set<cmStdString> emitted;
#ifdef __APPLE__
  emitted.insert("/System/Library/Frameworks");
#endif
  for(i = includes.begin(); i != includes.end(); ++i)
    {
#ifdef __APPLE__
    if(cmSystemTools::IsPathToFramework(i->c_str()))
      {
      std::string frameworkDir = *i;
      frameworkDir += "/../";
      frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir.c_str());
      if(emitted.insert(frameworkDir).second)
        {
        includeFlags 
          << "-F" 
          << this->ConvertToOutputForExisting(frameworkDir.c_str()) << " ";
        }
      continue;
      }
#endif
    std::string include = *i;
    if(!flagUsed || repeatFlag)
      {
      if(sysIncludeFlag &&
         this->Makefile->IsSystemIncludeDirectory(i->c_str()))
        {
        includeFlags << sysIncludeFlag;
        }
      else
        {
        includeFlags << includeFlag;
        }
      flagUsed = true;
      }
    std::string includePath = this->ConvertToOutputForExisting(i->c_str());
    if(quotePaths && includePath.size() && includePath[0] != '\"')
      {
      includeFlags << "\"";
      }
    includeFlags << includePath;
    if(quotePaths && includePath.size() && includePath[0] != '\"')
      {
      includeFlags << "\"";
      }
    includeFlags << sep;
    }
  std::string flags = includeFlags.str();
  // remove trailing separators
  if((sep[0] != ' ') && flags[flags.size()-1] == sep[0])
    {
    flags[flags.size()-1] = ' ';
    }
  std::string defineFlags = this->Makefile->GetDefineFlags();
  flags += defineFlags;
  this->LanguageToIncludeFlags[lang] = flags;

  // Use this temorary variable for the return value to work-around a
  // bogus GCC 2.95 warning.
  const char* ret = this->LanguageToIncludeFlags[lang].c_str();
  return ret;
}

//----------------------------------------------------------------------------
void cmLocalGenerator::GetIncludeDirectories(std::vector<std::string>& dirs,
                                             bool filter_system_dirs)
{
  // Need to decide whether to automatically include the source and
  // binary directories at the beginning of the include path.
  bool includeSourceDir = false;
  bool includeBinaryDir = false;

  // When automatic include directories are requested for a build then
  // include the source and binary directories at the beginning of the
  // include path to approximate include file behavior for an
  // in-source build.  This does not account for the case of a source
  // file in a subdirectory of the current source directory but we
  // cannot fix this because not all native build tools support
  // per-source-file include paths.
  if(this->Makefile->IsOn("CMAKE_INCLUDE_CURRENT_DIR"))
    {
    includeSourceDir = true;
    includeBinaryDir = true;
    }

  // CMake versions below 2.0 would add the source tree to the -I path
  // automatically.  Preserve compatibility.
  const char* versionValue =
    this->Makefile->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
  int major = 0;
  int minor = 0;
  if(versionValue && sscanf(versionValue, "%d.%d", &major, &minor) != 2)
    {
    versionValue = 0;
    }
  if(versionValue && major < 2)
    {
    includeSourceDir = true;
    }

  // Hack for VTK 4.0 - 4.4 which depend on the old behavior but do
  // not set the backwards compatibility level automatically.
  const char* vtkSourceDir =
    this->Makefile->GetDefinition("VTK_SOURCE_DIR");
  if(vtkSourceDir)
    {
    const char* vtk_major = 
      this->Makefile->GetDefinition("VTK_MAJOR_VERSION");
    const char* vtk_minor = 
      this->Makefile->GetDefinition("VTK_MINOR_VERSION");
    vtk_major = vtk_major? vtk_major : "4";
    vtk_minor = vtk_minor? vtk_minor : "4";
    int vmajor = 0;
    int vminor = 0;
    if(sscanf(vtk_major, "%d", &vmajor) && 
       sscanf(vtk_minor, "%d", &vminor) && vmajor == 4 && vminor <= 4)
      {
      includeSourceDir = true;
      }
    }

  // Do not repeat an include path.
  std::set<cmStdString> emitted;

  // Store the automatic include paths.
  if(includeBinaryDir)
    {
    dirs.push_back(this->Makefile->GetStartOutputDirectory());
    emitted.insert(this->Makefile->GetStartOutputDirectory());
    }
  if(includeSourceDir)
    {
    if(emitted.find(this->Makefile->GetStartDirectory()) == emitted.end())
      {
      dirs.push_back(this->Makefile->GetStartDirectory());
      emitted.insert(this->Makefile->GetStartDirectory());
      }
    }

  if(filter_system_dirs)
    {
    // Do not explicitly add the standard include path "/usr/include".
    // This can cause problems with certain standard library
    // implementations because the wrong headers may be found first.
    emitted.insert("/usr/include");
    if(const char* implicitIncludes = this->Makefile->GetDefinition
       ("CMAKE_PLATFORM_IMPLICIT_INCLUDE_DIRECTORIES"))
      {
      std::vector<std::string> implicitIncludeVec;
      cmSystemTools::ExpandListArgument(implicitIncludes, implicitIncludeVec);
      for(unsigned int k = 0; k < implicitIncludeVec.size(); ++k)
        {
        emitted.insert(implicitIncludeVec[k]);
        }
      }
    }

  // Get the project-specified include directories.
  std::vector<std::string>& includes = 
    this->Makefile->GetIncludeDirectories();

  // Support putting all the in-project include directories first if
  // it is requested by the project.
  if(this->Makefile->IsOn("CMAKE_INCLUDE_DIRECTORIES_PROJECT_BEFORE"))
    {
    const char* topSourceDir = this->Makefile->GetHomeDirectory();
    const char* topBinaryDir = this->Makefile->GetHomeOutputDirectory();
    for(std::vector<std::string>::iterator i = includes.begin();
        i != includes.end(); ++i)
      {
      // Emit this directory only if it is a subdirectory of the
      // top-level source or binary tree.
      if(cmSystemTools::ComparePath(i->c_str(), topSourceDir) ||
         cmSystemTools::ComparePath(i->c_str(), topBinaryDir) ||
         cmSystemTools::IsSubDirectory(i->c_str(), topSourceDir) ||
         cmSystemTools::IsSubDirectory(i->c_str(), topBinaryDir))
        {
        if(emitted.insert(*i).second)
          {
          dirs.push_back(*i);
          }
        }
      }
    }

  // Construct the final ordered include directory list.
  for(std::vector<std::string>::iterator i = includes.begin();
      i != includes.end(); ++i)
    {
    if(emitted.insert(*i).second)
      {
      dirs.push_back(*i);
      }
    }
}

void cmLocalGenerator::GetTargetFlags(std::string& linkLibs,
                                 std::string& flags,
                                 std::string& linkFlags,
                                 cmTarget& target)
{
  std::string buildType =  
    this->Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
  buildType = cmSystemTools::UpperCase(buildType); 
  const char* libraryLinkVariable = 
    "CMAKE_SHARED_LINKER_FLAGS"; // default to shared library
  
  switch(target.GetType())
    {
    case cmTarget::STATIC_LIBRARY: 
      {
      const char* targetLinkFlags = 
        target.GetProperty("STATIC_LIBRARY_FLAGS");
      if(targetLinkFlags)
        {
        linkFlags += targetLinkFlags;
        linkFlags += " ";
        }
      }
      break; 
    case cmTarget::MODULE_LIBRARY:
      libraryLinkVariable = "CMAKE_MODULE_LINKER_FLAGS";
    case cmTarget::SHARED_LIBRARY:
      { 
      linkFlags = this->Makefile->GetSafeDefinition(libraryLinkVariable);
      linkFlags += " ";
      if(buildType.size())
        {
        std::string build = libraryLinkVariable;
        build += "_";
        build += buildType;
        linkFlags += this->Makefile->GetSafeDefinition(build.c_str());
        linkFlags += " ";
        }  
      if(this->Makefile->IsOn("WIN32") && 
         !(this->Makefile->IsOn("CYGWIN") || this->Makefile->IsOn("MINGW")))
        {
        const std::vector<cmSourceFile*>& sources = target.GetSourceFiles();
        for(std::vector<cmSourceFile*>::const_iterator i = sources.begin();
            i != sources.end(); ++i)
          {
          cmSourceFile* sf = *i;
          if(sf->GetExtension() == "def")
            {
            linkFlags += 
              this->Makefile->GetSafeDefinition("CMAKE_LINK_DEF_FILE_FLAG");
            linkFlags += this->Convert(sf->GetFullPath().c_str(),
                                       START_OUTPUT, SHELL);
            linkFlags += " ";
            }
          }
        } 
      const char* targetLinkFlags = target.GetProperty("LINK_FLAGS");
      if(targetLinkFlags)
        {
        linkFlags += targetLinkFlags;
        linkFlags += " ";
        std::string configLinkFlags = targetLinkFlags;
        configLinkFlags += buildType;
        targetLinkFlags = target.GetProperty(configLinkFlags.c_str());
        if(targetLinkFlags)
          { 
          linkFlags += targetLinkFlags;
          linkFlags += " ";
          }
        }  
      cmOStringStream linklibsStr;
      this->OutputLinkLibraries(linklibsStr, target, false);
      linkLibs = linklibsStr.str();
      }
      break;
    case cmTarget::EXECUTABLE:
      {
      linkFlags += 
        this->Makefile->GetSafeDefinition("CMAKE_EXE_LINKER_FLAGS");
      linkFlags += " ";
      if(buildType.size())
        {
        std::string build = "CMAKE_EXE_LINKER_FLAGS_";
        build += buildType;
        linkFlags += this->Makefile->GetSafeDefinition(build.c_str());
        linkFlags += " ";
        } 
      const char* linkLanguage = 
        target.GetLinkerLanguage(this->GetGlobalGenerator());
      if(!linkLanguage)
        {
        cmSystemTools::Error
          ("CMake can not determine linker language for target:",
           target.GetName());
        return;
        }
      std::string langVar = "CMAKE_";
      langVar += linkLanguage;
      std::string flagsVar = langVar + "_FLAGS";
      std::string sharedFlagsVar = "CMAKE_SHARED_LIBRARY_";
      sharedFlagsVar += linkLanguage;
      sharedFlagsVar += "_FLAGS";
      flags += this->Makefile->GetSafeDefinition(flagsVar.c_str());
      flags += " ";
      flags += this->Makefile->GetSafeDefinition(sharedFlagsVar.c_str());
      flags += " ";
      cmOStringStream linklibs;
      this->OutputLinkLibraries(linklibs, target, false);
      linkLibs = linklibs.str();
      if(cmSystemTools::IsOn
         (this->Makefile->GetDefinition("BUILD_SHARED_LIBS")))
        {
        std::string sFlagVar = std::string("CMAKE_SHARED_BUILD_") 
          + linkLanguage + std::string("_FLAGS");
        linkFlags += this->Makefile->GetSafeDefinition(sFlagVar.c_str());
        linkFlags += " ";
        }
      if ( target.GetPropertyAsBool("WIN32_EXECUTABLE") )
        {
        linkFlags +=
          this->Makefile->GetSafeDefinition("CMAKE_CREATE_WIN32_EXE");
        linkFlags += " ";
        }
      else
        {
        linkFlags +=  
          this->Makefile->GetSafeDefinition("CMAKE_CREATE_CONSOLE_EXE");
        linkFlags += " ";
        }
      const char* targetLinkFlags = target.GetProperty("LINK_FLAGS");
      if(targetLinkFlags)
        {
        linkFlags += targetLinkFlags;
        linkFlags += " ";  
        std::string configLinkFlags = targetLinkFlags;
        configLinkFlags += buildType;
        targetLinkFlags = target.GetProperty(configLinkFlags.c_str());
        if(targetLinkFlags)
          { 
          linkFlags += targetLinkFlags;
          linkFlags += " ";
          }
        }
      }
      break; 
    case cmTarget::UTILITY:
    case cmTarget::GLOBAL_TARGET:
    case cmTarget::INSTALL_FILES:
    case cmTarget::INSTALL_PROGRAMS:
    case cmTarget::INSTALL_DIRECTORY:
      break;
    }
}

bool cmLocalGenerator::GetLinkerArgs(std::string& rpath, 
                                     std::string& linkLibs,
                                     cmTarget& tgt,
                                     bool relink,
                                     unsigned int minRpathSize)
{
  rpath = "";
  // collect all the flags needed for linking libraries
  linkLibs = "";

  // Try to emit each search path once
  std::set<cmStdString> emitted;
  // Embed runtime search paths if possible and if required.
  bool outputRuntime = true;
  std::string runtimeFlag;
  std::string runtimeSep;

  const char* config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE");
  const char* linkLanguage = 
      tgt.GetLinkerLanguage(this->GetGlobalGenerator());
  if(!linkLanguage)
    {
    cmSystemTools::
      Error("CMake can not determine linker language for target:",
            tgt.GetName());
    return false;
    }
  std::string runTimeFlagVar = "CMAKE_SHARED_LIBRARY_RUNTIME_";
  runTimeFlagVar += linkLanguage;
  runTimeFlagVar += "_FLAG";
  std::string runTimeFlagSepVar = runTimeFlagVar + "_SEP";
  runtimeFlag = this->Makefile->GetSafeDefinition(runTimeFlagVar.c_str());
  runtimeSep = this->Makefile->GetSafeDefinition(runTimeFlagSepVar.c_str());
  
  // concatenate all paths or no?
  bool runtimeConcatenate = ( runtimeSep!="" );
  if(runtimeFlag == "" || this->Makefile->IsOn("CMAKE_SKIP_RPATH"))
    {
    outputRuntime = false;
    }

  // Some search paths should never be emitted
  emitted.insert("");
  if(const char* implicitLinks =
     (this->Makefile->GetDefinition
      ("CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES")))
    {
    std::vector<std::string> implicitLinkVec;
    cmSystemTools::ExpandListArgument(implicitLinks, implicitLinkVec);
    for(unsigned int k = 0; k < implicitLinkVec.size(); ++k)
      {
      emitted.insert(implicitLinkVec[k]);
      }
    }
  std::string libPathFlag = 
    this->Makefile->GetRequiredDefinition("CMAKE_LIBRARY_PATH_FLAG");
  std::string libPathTerminator = 
    this->Makefile->GetSafeDefinition("CMAKE_LIBRARY_PATH_TERMINATOR");
  std::string libLinkFlag = 
    this->Makefile->GetSafeDefinition("CMAKE_LINK_LIBRARY_FLAG");
  
  // Flags to link an executable to shared libraries.
  std::string linkFlagsVar = "CMAKE_SHARED_LIBRARY_LINK_";
  linkFlagsVar += linkLanguage;
  linkFlagsVar += "_FLAGS";
  if( tgt.GetType() == cmTarget::EXECUTABLE )
    {
    linkLibs = this->Makefile->GetSafeDefinition(linkFlagsVar.c_str());
    linkLibs += " ";
    }

  // Compute the link library and directory information.
  std::vector<cmStdString> libNames;
  std::vector<cmStdString> libDirs;
  this->ComputeLinkInformation(tgt, config, libNames, libDirs);

  // Select whether to generate an rpath for the install tree or the
  // build tree.
  bool linking_for_install =
    relink || tgt.GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH");
  bool use_install_rpath =
    outputRuntime && tgt.HaveInstallTreeRPATH() && linking_for_install;
  bool use_build_rpath =
    outputRuntime && tgt.HaveBuildTreeRPATH() && !linking_for_install;
  bool use_link_rpath =
    outputRuntime && linking_for_install &&
    tgt.GetPropertyAsBool("INSTALL_RPATH_USE_LINK_PATH");

  // Construct the RPATH.
  std::vector<std::string> runtimeDirs;
  if(use_install_rpath)
    {
    const char* install_rpath = tgt.GetProperty("INSTALL_RPATH");
    cmSystemTools::ExpandListArgument(install_rpath, runtimeDirs);
    for(unsigned int i=0; i < runtimeDirs.size(); ++i)
      {
      runtimeDirs[i] =
        this->Convert(runtimeDirs[i].c_str(), FULL, SHELL, false);
      }
    }

  // Append the library search path flags.
  for(std::vector<cmStdString>::const_iterator libDir = libDirs.begin();
      libDir != libDirs.end(); ++libDir)
    {
   std::string libpath = this->ConvertToOutputForExisting(libDir->c_str());
    if(emitted.insert(libpath).second)
      {
      std::string fullLibPath;
      if(!this->WindowsShell && this->UseRelativePaths)
        {
        fullLibPath = "\"`cd ";
        }
      fullLibPath += libpath;
      if(!this->WindowsShell && this->UseRelativePaths)
        {
        fullLibPath += ";pwd`\"";
        }
      std::string::size_type pos = libDir->find(libPathFlag.c_str());
      if((pos == std::string::npos || pos > 0)
         && libDir->find("${") == std::string::npos)
        {
        linkLibs += libPathFlag;
        linkLibs += fullLibPath;
        linkLibs += libPathTerminator;
        linkLibs += " ";

        // Put this directory in the rpath if using build-tree rpath
        // support or if using the link path as an rpath.
        if(use_build_rpath)
          {
          runtimeDirs.push_back(fullLibPath);
          }
        else if(use_link_rpath)
          {
          // Do not add any path inside the source or build tree.
          const char* topSourceDir = this->Makefile->GetHomeDirectory();
          const char* topBinaryDir = this->Makefile->GetHomeOutputDirectory();
          if(!cmSystemTools::ComparePath(libDir->c_str(), topSourceDir) &&
             !cmSystemTools::ComparePath(libDir->c_str(), topBinaryDir) &&
             !cmSystemTools::IsSubDirectory(libDir->c_str(), topSourceDir) &&
             !cmSystemTools::IsSubDirectory(libDir->c_str(), topBinaryDir))
            {
            runtimeDirs.push_back(fullLibPath);
            }
          }
        }
      }
    }

  // Append the link libraries.
  for(std::vector<cmStdString>::iterator lib = libNames.begin();
      lib != libNames.end(); ++lib)
    {
    linkLibs += *lib;
    linkLibs += " ";
    }

  if(!runtimeDirs.empty())
    {
    // For the runtime search directories, do a "-Wl,-rpath,a:b:c" or
    // a "-R a -R b -R c" type link line
    rpath += runtimeFlag;
    std::vector<std::string>::iterator itr = runtimeDirs.begin();
    rpath += *itr;
    ++itr;
    for( ; itr != runtimeDirs.end(); ++itr )
      {
      if(runtimeConcatenate)
        {
        rpath += runtimeSep;
        rpath += *itr;
        }
      else
        {
        rpath += " ";
        rpath += runtimeFlag;
        rpath += *itr;
        }
      }
    }

  while (rpath.size() < minRpathSize)
    {
    if (rpath.size()==0)
      {
      rpath += runtimeFlag;
      }

    rpath += runtimeSep;
    }
  return true;
}

/**
 * Output the linking rules on a command line.  For executables,
 * targetLibrary should be a NULL pointer.  For libraries, it should point
 * to the name of the library.  This will not link a library against itself.
 */
void cmLocalGenerator::OutputLinkLibraries(std::ostream& fout,
                                           cmTarget& tgt,
                                           bool relink)
{
  std::string rpath;
  std::string linkLibs;
  unsigned int minBuildRpathSize = 0;
  
  if ((relink==false) 
    && this->Makefile->IsOn("CMAKE_USE_CHRPATH") 
    && (tgt.IsChrpathAvailable()))
    {
    std::string installRpath;
    std::string dummy;
    this->GetLinkerArgs(installRpath, dummy, tgt, true, 0);
    minBuildRpathSize = static_cast<unsigned int>(installRpath.size());
    }

  if (!this->GetLinkerArgs(rpath, linkLibs, tgt, relink, minBuildRpathSize))
    {
    return;
    }

  const char* linkLanguage = 
    tgt.GetLinkerLanguage(this->GetGlobalGenerator());
  if(!linkLanguage)
    {
    cmSystemTools::
      Error("CMake can not determine linker language for target:",
            tgt.GetName());
    return;
    }

  fout << linkLibs;
  fout << rpath << " ";

  // Add standard libraries for this language.
  std::string standardLibsVar = "CMAKE_";
  standardLibsVar += linkLanguage;
  standardLibsVar += "_STANDARD_LIBRARIES";
  if(const char* stdLibs =
     this->Makefile->GetDefinition(standardLibsVar.c_str()))
    {
    fout << stdLibs << " ";
    }
}

//----------------------------------------------------------------------------
void cmLocalGenerator
::ComputeLinkInformation(cmTarget& target,
                         const char* config,
                         std::vector<cmStdString>& outLibs,
                         std::vector<cmStdString>& outDirs,
                         std::vector<cmStdString>* fullPathLibs)
{
  // Compute which library configuration to link.
  cmTarget::LinkLibraryType linkType = cmTarget::OPTIMIZED;
  if(config && cmSystemTools::UpperCase(config) == "DEBUG")
    {
    linkType = cmTarget::DEBUG;
    }

  // Get the language used for linking.
  const char* linkLanguage =
    target.GetLinkerLanguage(this->GetGlobalGenerator());

  // Check whether we should use an import library for linking a target.
  bool implib =
    this->Makefile->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX")?true:false;

  // On platforms without import libraries there may be a special flag
  // to use when creating a plugin (module) that obtains symbols from
  // the program that will load it.
  const char* loader_flag = 0;
  if(!implib && target.GetType() == cmTarget::MODULE_LIBRARY)
    {
    if(!linkLanguage)
      {
      cmSystemTools::
        Error("CMake can not determine linker language for target:",
          target.GetName());
      return;
      }
    std::string loader_flag_var = "CMAKE_SHARED_MODULE_LOADER_";
    loader_flag_var += linkLanguage;
    loader_flag_var += "_FLAG";
    loader_flag = this->Makefile->GetDefinition(loader_flag_var.c_str());
    }

  // Get the list of libraries against which this target wants to link.
  std::vector<std::string> linkLibraries;
  const cmTarget::LinkLibraryVectorType& inLibs = target.GetLinkLibraries();
  for(cmTarget::LinkLibraryVectorType::const_iterator j = inLibs.begin();
      j != inLibs.end(); ++j)
    {
    // For backwards compatibility variables may have been expanded
    // inside library names.  Clean up the resulting name.
    std::string lib = j->first;
    std::string::size_type pos = lib.find_first_not_of(" \t\r\n");
    if(pos != lib.npos)
      {
      lib = lib.substr(pos, lib.npos);
      }
    pos = lib.find_last_not_of(" \t\r\n");
    if(pos != lib.npos)
      { 
      lib = lib.substr(0, pos+1); 
      }
    if(lib.empty())
      {
      continue;
      }
    // Link to a library if it is not the same target and is meant for
    // this configuration type.
    if((target.GetType() == cmTarget::EXECUTABLE ||
        lib != target.GetName()) &&
       (j->second == cmTarget::GENERAL || j->second == linkType))
      {
      // Compute the proper name to use to link this library.
      cmTarget* tgt = this->GlobalGenerator->FindTarget(0, lib.c_str(), false);
      bool impexe = (tgt &&
                     tgt->GetType() == cmTarget::EXECUTABLE &&
                     tgt->GetPropertyAsBool("ENABLE_EXPORTS"));
      if(impexe && !implib && !loader_flag)
        {
        // Skip linking to executables on platforms with no import
        // libraries or loader flags.
        continue;
        }
      else if(tgt && (tgt->GetType() == cmTarget::STATIC_LIBRARY ||
                      tgt->GetType() == cmTarget::SHARED_LIBRARY ||
                      tgt->GetType() == cmTarget::MODULE_LIBRARY ||
                      impexe))
        {
        // This is a CMake target.  Ask the target for its real name.
        std::string linkItem;
        if(impexe && loader_flag)
          {
          // This link item is an executable that may provide symbols
          // used by this target.  A special flag is needed on this
          // platform.  Add it now.
          std::string exe = tgt->GetFullPath(config, implib);
          linkItem += loader_flag;
          linkItem += this->Convert(exe.c_str(), NONE, SHELL, false);
          }
        else
          {
          // Pass the full path to the target file but purposely leave
          // off the per-configuration subdirectory.  The link directory
          // ordering knows how to deal with this.
          linkItem += tgt->GetDirectory(0, implib);
          // on apple if the FRAMEWORK prop is set, then
          // do not add the target full name but just use the directory
          // name
#ifdef __APPLE__
          if (!(tgt->GetType() == cmTarget::SHARED_LIBRARY &&
                tgt->GetPropertyAsBool("FRAMEWORK")))
#endif
            { 
            linkItem += "/";
            linkItem += tgt->GetFullName(config, implib);
            }
          }
        linkLibraries.push_back(linkItem);
        // For full path, use the true location.
        if(fullPathLibs)
          {
          fullPathLibs->push_back(tgt->GetFullPath(config, implib));
          }
        }
      else
        {
        // This is not a CMake target.  Use the name given.
        linkLibraries.push_back(lib);

        // Add to the list of full paths if this library is one.
        if(fullPathLibs &&
           cmSystemTools::FileIsFullPath(lib.c_str()) &&
           !cmSystemTools::FileIsDirectory(lib.c_str()))
          {
          fullPathLibs->push_back(lib);
          }
        }
      }
    }

  // Get the list of directories the target wants to search for libraries.
  const std::vector<std::string>&
    linkDirectories = target.GetLinkDirectories();

  // Lookup link type selection flags.
  const char* static_link_type_flag = 0;
  const char* shared_link_type_flag = 0;
  const char* target_type_str = 0;
  switch(target.GetType())
    {
    case cmTarget::EXECUTABLE:     target_type_str = "EXE"; break;
    case cmTarget::SHARED_LIBRARY: target_type_str = "SHARED_LIBRARY"; break;
    case cmTarget::MODULE_LIBRARY: target_type_str = "SHARED_MODULE"; break;
    default: break;
    }
  if(target_type_str)
    {
    if(!linkLanguage)
      {
      cmSystemTools::
        Error("CMake can not determine linker language for target:",
          target.GetName());
      return;
      }
    std::string static_link_type_flag_var = "CMAKE_";
    static_link_type_flag_var += target_type_str;
    static_link_type_flag_var += "_LINK_STATIC_";
    static_link_type_flag_var += linkLanguage;
    static_link_type_flag_var += "_FLAGS";
    static_link_type_flag =
      this->Makefile->GetDefinition(static_link_type_flag_var.c_str());

    std::string shared_link_type_flag_var = "CMAKE_";
    shared_link_type_flag_var += target_type_str;
    shared_link_type_flag_var += "_LINK_DYNAMIC_";
    shared_link_type_flag_var += linkLanguage;
    shared_link_type_flag_var += "_FLAGS";
    shared_link_type_flag =
      this->Makefile->GetDefinition(shared_link_type_flag_var.c_str());
    }

  // Compute the link directory order needed to link the libraries.
  cmOrderLinkDirectories orderLibs;
  orderLibs.SetLinkTypeInformation(cmOrderLinkDirectories::LinkShared,
                                   static_link_type_flag,
                                   shared_link_type_flag);
  orderLibs.AddLinkPrefix(
    this->Makefile->GetDefinition("CMAKE_STATIC_LIBRARY_PREFIX"));
  orderLibs.AddLinkPrefix(
    this->Makefile->GetDefinition("CMAKE_SHARED_LIBRARY_PREFIX"));

  // Import library names should be matched and treated as shared
  // libraries for the purposes of linking.
  orderLibs.AddLinkExtension(
    this->Makefile->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX"),
    cmOrderLinkDirectories::LinkShared);
  orderLibs.AddLinkExtension(
    this->Makefile->GetDefinition("CMAKE_STATIC_LIBRARY_SUFFIX"),
    cmOrderLinkDirectories::LinkStatic);
  orderLibs.AddLinkExtension(
    this->Makefile->GetDefinition("CMAKE_SHARED_LIBRARY_SUFFIX"),
    cmOrderLinkDirectories::LinkShared);
  orderLibs.AddLinkExtension(
    this->Makefile->GetDefinition("CMAKE_LINK_LIBRARY_SUFFIX"));
  if(const char* linkSuffixes =
     this->Makefile->GetDefinition("CMAKE_EXTRA_LINK_EXTENSIONS"))
    {
    std::vector<std::string> linkSuffixVec;
    cmSystemTools::ExpandListArgument(linkSuffixes, linkSuffixVec);
    for(std::vector<std::string>::iterator i = linkSuffixVec.begin();
        i != linkSuffixVec.end(); ++i)
      {
      orderLibs.AddLinkExtension(i->c_str());
      }
    }
  std::string configSubdir;
  cmGlobalGenerator* gg = this->GetGlobalGenerator();
  gg->AppendDirectoryForConfig("", config, "", configSubdir);
  orderLibs.SetLinkInformation(target.GetName(),
                               linkLibraries,
                               linkDirectories,
                               gg->GetTargetManifest(),
                               configSubdir.c_str());
  orderLibs.DetermineLibraryPathOrder();
  std::vector<cmStdString> orderedLibs;
  orderLibs.GetLinkerInformation(outDirs, orderedLibs);

  // Make sure libraries are linked with the proper syntax.
  std::string libLinkFlag =
    this->Makefile->GetSafeDefinition("CMAKE_LINK_LIBRARY_FLAG");
  std::string libLinkSuffix =
    this->Makefile->GetSafeDefinition("CMAKE_LINK_LIBRARY_SUFFIX");
  for(std::vector<cmStdString>::iterator l = orderedLibs.begin();
      l != orderedLibs.end(); ++l)
    {
    std::string lib = *l;
    if(lib[0] == '-' || lib[0] == '$' || lib[0] == '`')
      {
      // The library is linked with special syntax by the user.
      outLibs.push_back(lib);
      }
    else
      {
      // Generate the proper link syntax.
      lib = libLinkFlag;
      lib += *l;
      lib += libLinkSuffix;
      outLibs.push_back(lib);
      }
    }
}

//----------------------------------------------------------------------------
void cmLocalGenerator::AddLanguageFlags(std::string& flags,
                                        const char* lang,
                                        const char* config)
{
  // Add language-specific flags.
  std::string flagsVar = "CMAKE_";
  flagsVar += lang;
  flagsVar += "_FLAGS";
  if(this->EmitUniversalBinaryFlags)
    {
    const char* osxArch = 
      this->Makefile->GetDefinition("CMAKE_OSX_ARCHITECTURES");
    const char* sysroot = 
      this->Makefile->GetDefinition("CMAKE_OSX_SYSROOT");
    if(osxArch && sysroot  && lang && lang[0] =='C')
      { 
      std::vector<std::string> archs;
      cmSystemTools::ExpandListArgument(std::string(osxArch),
                                        archs);
      bool addArchFlag = true;
      if(archs.size() == 1)
        {
        const char* archOrig = 
          this->Makefile->GetSafeDefinition("CMAKE_OSX_ARCHITECTURES_DEFAULT");
        if(archs[0] == archOrig)
          {
          addArchFlag = false;
          }
        }
      // if there is more than one arch add the -arch and
      // -isysroot flags, or if there is one arch flag, but
      // it is not the default -arch flag for the system, then
      // add it.  Otherwize do not add -arch and -isysroot
      if(addArchFlag)
        {
        for( std::vector<std::string>::iterator i = archs.begin();
             i != archs.end(); ++i)
          {
          flags += " -arch ";
          flags += *i;
          }
        flags += " -isysroot ";
        flags += sysroot;
        }
      }
    }
  this->AddConfigVariableFlags(flags, flagsVar.c_str(), config);
}

//----------------------------------------------------------------------------
std::string cmLocalGenerator::GetRealDependency(const char* inName,
                                                const char* config)
{
  // Older CMake code may specify the dependency using the target
  // output file rather than the target name.  Such code would have
  // been written before there was support for target properties that
  // modify the name so stripping down to just the file name should
  // produce the target name in this case.
  std::string name = cmSystemTools::GetFilenameName(inName);
  if(cmSystemTools::GetFilenameLastExtension(name) == ".exe")
    {
    name = cmSystemTools::GetFilenameWithoutLastExtension(name);
    }

  // Look for a CMake target with the given name.
  if(cmTarget* target = this->GlobalGenerator->FindTarget(0,name.c_str(),true))
    {
    // make sure it is not just a coincidence that the target name
    // found is part of the inName
    if(cmSystemTools::FileIsFullPath(inName))
      {
      std::string tLocation = target->GetLocation(config);
      tLocation = cmSystemTools::GetFilenamePath(tLocation);
      std::string depLocation = cmSystemTools::GetFilenamePath(
        std::string(inName));
      depLocation = cmSystemTools::CollapseFullPath(depLocation.c_str());
      tLocation = cmSystemTools::CollapseFullPath(tLocation.c_str());
      if(depLocation != tLocation)
        {
        // it is a full path to a depend that has the same name
        // as a target but is in a different location so do not use
        // the target as the depend
        return inName;
        }
      }
    switch (target->GetType())
      {
      case cmTarget::EXECUTABLE:
      case cmTarget::STATIC_LIBRARY:
      case cmTarget::SHARED_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
        {
        // Get the location of the target's output file and depend on it.
        if(const char* location = target->GetLocation(config))
          {
          return location;
          }
        }
        break;
      case cmTarget::UTILITY:
      case cmTarget::GLOBAL_TARGET:
        // Depending on a utility target may not work but just trust
        // the user to have given a valid name.
        return inName;
      case cmTarget::INSTALL_FILES:
      case cmTarget::INSTALL_PROGRAMS:
      case cmTarget::INSTALL_DIRECTORY:
        break;
      }
    }

  // The name was not that of a CMake target.  It must name a file.
  if(cmSystemTools::FileIsFullPath(inName))
    {
    // This is a full path.  Return it as given.
    return inName;
    }
  // Treat the name as relative to the source directory in which it
  // was given.
  name = this->Makefile->GetCurrentDirectory();
  name += "/";
  name += inName;
  return name;
}

//----------------------------------------------------------------------------
std::string cmLocalGenerator::GetRealLocation(const char* inName,
                                              const char* config)
{
  std::string outName=inName;
  // Look for a CMake target with the given name, which is an executable 
  // and which can be run
  cmTarget* target = this->GlobalGenerator->FindTarget(0, inName, true);
  if ((target != 0)
       && (target->GetType() == cmTarget::EXECUTABLE)
       && ((this->Makefile->IsOn("CMAKE_CROSSCOMPILING") == false) 
            || (target->IsImported() == true)))
    {
    outName = target->GetLocation( config );
    }
  return outName;
}

//----------------------------------------------------------------------------
void cmLocalGenerator::AddSharedFlags(std::string& flags,
                                      const char* lang,
                                      bool shared)
{
  std::string flagsVar;

  // Add flags for dealing with shared libraries for this language.
  if(shared)
    {
    flagsVar = "CMAKE_SHARED_LIBRARY_";
    flagsVar += lang;
    flagsVar += "_FLAGS";
    this->AppendFlags(flags, this->Makefile->GetDefinition(flagsVar.c_str()));
    }

  // Add flags specific to shared builds.
  if(cmSystemTools::IsOn(this->Makefile->GetDefinition("BUILD_SHARED_LIBS")))
    {
    flagsVar = "CMAKE_SHARED_BUILD_";
    flagsVar += lang;
    flagsVar += "_FLAGS";
    this->AppendFlags(flags, this->Makefile->GetDefinition(flagsVar.c_str()));
    }
}

//----------------------------------------------------------------------------
void cmLocalGenerator::AddConfigVariableFlags(std::string& flags,
                                              const char* var,
                                              const char* config)
{
  // Add the flags from the variable itself.
  std::string flagsVar = var;
  this->AppendFlags(flags, this->Makefile->GetDefinition(flagsVar.c_str()));
  // Add the flags from the build-type specific variable.
  if(config && *config)
    {
    flagsVar += "_";
    flagsVar += cmSystemTools::UpperCase(config);
    this->AppendFlags(flags, this->Makefile->GetDefinition(flagsVar.c_str()));
    }
}

//----------------------------------------------------------------------------
void cmLocalGenerator::AppendFlags(std::string& flags,
                                                const char* newFlags)
{
  if(newFlags && *newFlags)
    {
    std::string newf = newFlags;
    if(flags.size())
      {
      flags += " ";
      }
    flags += newFlags;
    }
}

//----------------------------------------------------------------------------
void cmLocalGenerator::AppendDefines(std::string& defines,
                                     const char* defines_list,
                                     const char* lang)
{
  // Short-circuit if there are no definitions.
  if(!defines_list)
    {
    return;
    }

  // Expand the list of definitions.
  std::vector<std::string> defines_vec;
  cmSystemTools::ExpandListArgument(defines_list, defines_vec);

  // Short-circuit if there are no definitions.
  if(defines_vec.empty())
    {
    return;
    }

  // Lookup the define flag for the current language.
  std::string dflag = "-D";
  if(lang)
    {
    std::string defineFlagVar = "CMAKE_";
    defineFlagVar += lang;
    defineFlagVar += "_DEFINE_FLAG";
    const char* df = this->Makefile->GetDefinition(defineFlagVar.c_str());
    if(df && *df)
      {
      dflag = df;
      }
    }

  // Add each definition to the command line with appropriate escapes.
  const char* dsep = defines.empty()? "" : " ";
  for(std::vector<std::string>::const_iterator di = defines_vec.begin();
      di != defines_vec.end(); ++di)
    {
    // Skip unsupported definitions.
    if(!this->CheckDefinition(*di))
      {
      continue;
      }

    // Separate from previous definitions.
    defines += dsep;
    dsep = " ";

    // Append the definition with proper escaping.
    defines += dflag;
    if(this->WatcomWMake)
      {
      // The Watcom compiler does its own command line parsing instead
      // of using the windows shell rules.  Definitions are one of
      //   -DNAME
      //   -DNAME=<cpp-token>
      //   -DNAME="c-string with spaces and other characters(?@#$)"
      //
      // Watcom will properly parse each of these cases from the
      // command line without any escapes.  However we still have to
      // get the '$' and '#' characters through WMake as '$$' and
      // '$#'.
      for(const char* c = di->c_str(); *c; ++c)
        {
        if(*c == '$' || *c == '#')
          {
          defines += '$';
          }
        defines += *c;
        }
      }
    else
      {
      // Make the definition appear properly on the command line.
      defines += this->EscapeForShell(di->c_str(), true);
      }
    }
}

//----------------------------------------------------------------------------
std::string
cmLocalGenerator::ConstructComment(const cmCustomCommand& cc,
                                   const char* default_comment)
{
  // Check for a comment provided with the command.
  if(cc.GetComment())
    {
    return cc.GetComment();
    }

  // Construct a reasonable default comment if possible.
  if(!cc.GetOutputs().empty())
    {
    std::string comment;
    comment = "Generating ";
    const char* sep = "";
    for(std::vector<std::string>::const_iterator o = cc.GetOutputs().begin();
        o != cc.GetOutputs().end(); ++o)
      {
      comment += sep;
      comment += this->Convert(o->c_str(), cmLocalGenerator::START_OUTPUT);
      sep = ", ";
      }
    return comment;
    }

  // Otherwise use the provided default.
  return default_comment;
}

//----------------------------------------------------------------------------
std::string
cmLocalGenerator::ConvertToOptionallyRelativeOutputPath(const char* remote)
{
  return this->Convert(remote, START_OUTPUT, SHELL, true);
}

//----------------------------------------------------------------------------
std::string cmLocalGenerator::Convert(const char* source, 
                                      RelativeRoot relative,
                                      OutputFormat output,
                                      bool optional)
{
  // Make sure the relative path conversion components are set.
  if(!this->PathConversionsSetup)
    {
    this->SetupPathConversions();
    this->PathConversionsSetup = true;
    }

  // Convert the path to a relative path.
  std::string result = source;

  if (!optional || this->UseRelativePaths)
    {
    switch (relative)
      {
      case HOME:
        //result = cmSystemTools::CollapseFullPath(result.c_str());
        result = this->ConvertToRelativePath(this->HomeDirectoryComponents,
                                             result.c_str());
        break;
      case START:
        //result = cmSystemTools::CollapseFullPath(result.c_str());
        result = this->ConvertToRelativePath(this->StartDirectoryComponents,
                                             result.c_str());
        break;
      case HOME_OUTPUT:
        //result = cmSystemTools::CollapseFullPath(result.c_str());
        result =
          this->ConvertToRelativePath(this->HomeOutputDirectoryComponents,
                                      result.c_str());
        break;
      case START_OUTPUT:
        //result = cmSystemTools::CollapseFullPath(result.c_str());
        result =
          this->ConvertToRelativePath(this->StartOutputDirectoryComponents,
                                      result.c_str());
        break;
      case FULL:
        result = cmSystemTools::CollapseFullPath(result.c_str());
        break;
      case NONE:
        break;
      }
    }
  // Now convert it to an output path.
  if (output == MAKEFILE)
    {
    result = cmSystemTools::ConvertToOutputPath(result.c_str());
    }
  else if( output == SHELL)
    {
        // For the MSYS shell convert drive letters to posix paths, so
    // that c:/some/path becomes /c/some/path.  This is needed to
    // avoid problems with the shell path translation.
    if(this->MSYSShell)
      {
      if(result.size() > 2 && result[1] == ':')
        {
        result[1] = result[0];
        result[0] = '/';
        }
      }
    if(this->WindowsShell)
      {
      std::string::size_type pos = 0;
      while((pos = result.find('/', pos)) != std::string::npos)
        {
        result[pos] = '\\';
        pos++;
        }
      }
    result = this->EscapeForShell(result.c_str(), true, false);
    }
  return result;
}

//----------------------------------------------------------------------------
std::string cmLocalGenerator::FindRelativePathTopSource()
{
  // Relative path conversion within a single tree managed by CMake is
  // safe.  We can use our parent relative path top if and only if
  // this is a subdirectory of that top.
  if(cmLocalGenerator* parent = this->GetParent())
    {
    std::string parentTop = parent->FindRelativePathTopSource();
    if(cmSystemTools::IsSubDirectory(
         this->Makefile->GetStartDirectory(), parentTop.c_str()))
      {
      return parentTop;
      }
    }

  // Otherwise this directory itself is the new top.
  return this->Makefile->GetStartDirectory();
}

//----------------------------------------------------------------------------
std::string cmLocalGenerator::FindRelativePathTopBinary()
{
  // Relative path conversion within a single tree managed by CMake is
  // safe.  We can use our parent relative path top if and only if
  // this is a subdirectory of that top.
  if(cmLocalGenerator* parent = this->GetParent())
    {
    std::string parentTop = parent->FindRelativePathTopBinary();
    if(cmSystemTools::IsSubDirectory(
         this->Makefile->GetStartOutputDirectory(), parentTop.c_str()))
      {
      return parentTop;
      }
    }

  // Otherwise this directory itself is the new top.
  return this->Makefile->GetStartOutputDirectory();
}

//----------------------------------------------------------------------------
void cmLocalGenerator::ConfigureRelativePaths()
{
  // Relative path conversion inside the source tree is not used to
  // construct relative paths passed to build tools so it is safe to
  // even when the source is a network path.
  std::string source = this->FindRelativePathTopSource();
  this->RelativePathTopSource = source;

  // The current working directory on Windows cannot be a network
  // path.  Therefore relative paths cannot work when the binary tree
  // is a network path.
  std::string binary = this->FindRelativePathTopBinary();
  if(binary.size() < 2 || binary.substr(0, 2) != "//")
    {
    this->RelativePathTopBinary = binary;
    }
  else
    {
    this->RelativePathTopBinary = "";
    }
}

//----------------------------------------------------------------------------
static bool cmLocalGeneratorNotAbove(const char* a, const char* b)
{
  return (cmSystemTools::ComparePath(a, b) ||
          cmSystemTools::IsSubDirectory(a, b));
}

//----------------------------------------------------------------------------
std::string
cmLocalGenerator::ConvertToRelativePath(const std::vector<std::string>& local,
                                        const char* in_remote)
{
  // The path should never be quoted.
  assert(in_remote[0] != '\"');

  // The local path should never have a trailing slash.
  assert(local.size() > 0 && !(local[local.size()-1] == ""));

  // If the path is already relative then just return the path.
  if(!cmSystemTools::FileIsFullPath(in_remote))
    {
    return in_remote;
    }

  // Make sure relative path conversion is configured.
  if(!this->RelativePathsConfigured)
    {
    this->ConfigureRelativePaths();
    this->RelativePathsConfigured = true;
    }

  // Skip conversion if the path and local are not both in the source
  // or both in the binary tree.
  std::string local_path = cmSystemTools::JoinPath(local);
  if(!((cmLocalGeneratorNotAbove(local_path.c_str(),
                                 this->RelativePathTopBinary.c_str()) &&
        cmLocalGeneratorNotAbove(in_remote,
                                 this->RelativePathTopBinary.c_str())) ||
       (cmLocalGeneratorNotAbove(local_path.c_str(),
                                 this->RelativePathTopSource.c_str()) &&
        cmLocalGeneratorNotAbove(in_remote,
                                 this->RelativePathTopSource.c_str()))))
    {
    return in_remote;
    }

  // Identify the longest shared path component between the remote
  // path and the local path.
  std::vector<std::string> remote;
  cmSystemTools::SplitPath(in_remote, remote);
  unsigned int common=0;
  while(common < remote.size() &&
        common < local.size() &&
        cmSystemTools::ComparePath(remote[common].c_str(),
                                   local[common].c_str()))
    {
    ++common;
    }

  // If no part of the path is in common then return the full path.
  if(common == 0)
    {
    return in_remote;
    }

  // If the entire path is in common then just return a ".".
  if(common == remote.size() &&
     common == local.size())
    {
    return ".";
    }

  // If the entire path is in common except for a trailing slash then
  // just return a "./".
  if(common+1 == remote.size() &&
     remote[common].size() == 0 &&
     common == local.size())
    {
    return "./";
    }

  // Construct the relative path.
  std::string relative;

  // First add enough ../ to get up to the level of the shared portion
  // of the path.  Leave off the trailing slash.  Note that the last
  // component of local will never be empty because local should never
  // have a trailing slash.
  for(unsigned int i=common; i < local.size(); ++i)
    {
    relative += "..";
    if(i < local.size()-1)
      {
      relative += "/";
      }
    }

  // Now add the portion of the destination path that is not included
  // in the shared portion of the path.  Add a slash the first time
  // only if there was already something in the path.  If there was a
  // trailing slash in the input then the last iteration of the loop
  // will add a slash followed by an empty string which will preserve
  // the trailing slash in the output.
  for(unsigned int i=common; i < remote.size(); ++i)
    {
    if(relative.size() > 0)
      {
      relative += "/";
      }
    relative += remote[i];
    }

  // Finally return the path.
  return relative;
}

//----------------------------------------------------------------------------
void
cmLocalGenerator
::GenerateTargetInstallRules(
  std::ostream& os, const char* config,
  std::vector<std::string> const& configurationTypes)
{
  // Convert the old-style install specification from each target to
  // an install generator and run it.
  cmTargets& tgts = this->Makefile->GetTargets();
  for(cmTargets::iterator l = tgts.begin(); l != tgts.end(); ++l)
    {
    // Include the user-specified pre-install script for this target.
    if(const char* preinstall = l->second.GetProperty("PRE_INSTALL_SCRIPT"))
      {
      cmInstallScriptGenerator g(preinstall, false, 0);
      g.Generate(os, config, configurationTypes);
      }

    // Install this target if a destination is given.
    if(l->second.GetInstallPath() != "")
      {
      // Compute the full install destination.  Note that converting
      // to unix slashes also removes any trailing slash.
      std::string destination = "${CMAKE_INSTALL_PREFIX}";
      destination += l->second.GetInstallPath();
      cmSystemTools::ConvertToUnixSlashes(destination);

      // Generate the proper install generator for this target type.
      switch(l->second.GetType())
        {
        case cmTarget::EXECUTABLE:
        case cmTarget::STATIC_LIBRARY:
        case cmTarget::MODULE_LIBRARY:
          {
          // Use a target install generator.
          cmInstallTargetGenerator g(l->second, destination.c_str(), false);
          g.Generate(os, config, configurationTypes);
          }
          break;
        case cmTarget::SHARED_LIBRARY:
          {
#if defined(_WIN32) || defined(__CYGWIN__)
          // Special code to handle DLL.  Install the import library
          // to the normal destination and the DLL to the runtime
          // destination.
          cmInstallTargetGenerator g1(l->second, destination.c_str(), true);
          g1.Generate(os, config, configurationTypes);
          destination = "${CMAKE_INSTALL_PREFIX}";
          destination += l->second.GetRuntimeInstallPath();
          cmSystemTools::ConvertToUnixSlashes(destination);
          cmInstallTargetGenerator g2(l->second, destination.c_str(), false);
          g2.Generate(os, config, configurationTypes);
#else
          // Use a target install generator.
          cmInstallTargetGenerator g(l->second, destination.c_str(), false);
          g.Generate(os, config, configurationTypes);
#endif
          }
          break;
        default:
          break;
        }
      }

    // Include the user-specified post-install script for this target.
    if(const char* postinstall = l->second.GetProperty("POST_INSTALL_SCRIPT"))
      {
      cmInstallScriptGenerator g(postinstall, false, 0);
      g.Generate(os, config, configurationTypes);
      }
    }
}

#if defined(CM_LG_ENCODE_OBJECT_NAMES)
static std::string cmLocalGeneratorMD5(const char* input)
{
  char md5out[32];
  cmsysMD5* md5 = cmsysMD5_New();
  cmsysMD5_Initialize(md5);
  cmsysMD5_Append(md5, reinterpret_cast<unsigned char const*>(input), -1);
  cmsysMD5_FinalizeHex(md5, md5out);
  cmsysMD5_Delete(md5);
  return std::string(md5out, 32);
}

static bool
cmLocalGeneratorShortenObjectName(std::string& objName,
                                  std::string::size_type max_len)
{
  // Replace the beginning of the path portion of the object name with
  // its own md5 sum.
  std::string::size_type pos = objName.find('/', objName.size()-max_len+32);
  if(pos != objName.npos)
    {
    std::string md5name = cmLocalGeneratorMD5(objName.substr(0, pos).c_str());
    md5name += objName.substr(pos);
    objName = md5name;

    // The object name is now short enough.
    return true;
    }
  else
    {
    // The object name could not be shortened enough.
    return false;
    }
}

static bool cmLocalGeneratorCheckObjectName(std::string& objName,
                                            std::string::size_type dir_len)
{
  // Choose a maximum file name length.
#if defined(_WIN32) || defined(__CYGWIN__)
  std::string::size_type const max_total_len = 250;
#else
  std::string::size_type const max_total_len = 1000;
#endif

  // Enforce the maximum file name length if possible.
  std::string::size_type max_obj_len = max_total_len;
  if(dir_len < max_total_len)
    {
    max_obj_len = max_total_len - dir_len;
    if(objName.size() > max_obj_len)
      {
      // The current object file name is too long.  Try to shorten it.
      return cmLocalGeneratorShortenObjectName(objName, max_obj_len);
      }
    else
      {
      // The object file name is short enough.
      return true;
      }
    }
  else
    {
    // The build directory in which the object will be stored is
    // already too deep.
    return false;
    }
}
#endif

//----------------------------------------------------------------------------
std::string&
cmLocalGenerator
::CreateSafeUniqueObjectFileName(const char* sin,
                                 std::string::size_type dir_len)
{
  // Look for an existing mapped name for this object file.
  std::map<cmStdString,cmStdString>::iterator it =
    this->UniqueObjectNamesMap.find(sin);

  // If no entry exists create one.
  if(it == this->UniqueObjectNamesMap.end())
    {
    // Start with the original name.
    std::string ssin = sin;

    // Avoid full paths by removing leading slashes.
    std::string::size_type pos = 0;
    for(;pos < ssin.size() && ssin[pos] == '/'; ++pos);
    ssin = ssin.substr(pos);

    // Avoid full paths by removing colons.
    cmSystemTools::ReplaceString(ssin, ":", "_");

    // Avoid relative paths that go up the tree.
    cmSystemTools::ReplaceString(ssin, "../", "__/");

    // Avoid spaces.
    cmSystemTools::ReplaceString(ssin, " ", "_");

    // Mangle the name if necessary.
    if(this->Makefile->IsOn("CMAKE_MANGLE_OBJECT_FILE_NAMES"))
      {
      bool done;
      int cc = 0;
      char rpstr[100];
      sprintf(rpstr, "_p_");
      cmSystemTools::ReplaceString(ssin, "+", rpstr);
      std::string sssin = sin;
      do
        {
        done = true;
        for ( it = this->UniqueObjectNamesMap.begin();
              it != this->UniqueObjectNamesMap.end();
              ++ it )
          {
          if ( it->second == ssin )
            {
            done = false;
            }
          }
        if ( done )
          {
          break;
          }
        sssin = ssin;
        cmSystemTools::ReplaceString(ssin, "_p_", rpstr);
        sprintf(rpstr, "_p%d_", cc++);
        }
      while ( !done );
      }

#if defined(CM_LG_ENCODE_OBJECT_NAMES)
    cmLocalGeneratorCheckObjectName(ssin, dir_len);
#else
    (void)dir_len;
#endif

    // Insert the newly mapped object file name.
    std::map<cmStdString, cmStdString>::value_type e(sin, ssin);
    it = this->UniqueObjectNamesMap.insert(e).first;
    }

  // Return the map entry.
  return it->second;
}

//----------------------------------------------------------------------------
std::string
cmLocalGenerator
::GetObjectFileNameWithoutTarget(const cmSourceFile& source,
                                 std::string::size_type dir_len,
                                 bool* hasSourceExtension)
{
  // Construct the object file name using the full path to the source
  // file which is its only unique identification.
  const char* fullPath = source.GetFullPath().c_str();

  // Try referencing the source relative to the source tree.
  std::string relFromSource = this->Convert(fullPath, START);
  assert(!relFromSource.empty());
  bool relSource = !cmSystemTools::FileIsFullPath(relFromSource.c_str());
  bool subSource = relSource && relFromSource[0] != '.';

  // Try referencing the source relative to the binary tree.
  std::string relFromBinary = this->Convert(fullPath, START_OUTPUT);
  assert(!relFromBinary.empty());
  bool relBinary = !cmSystemTools::FileIsFullPath(relFromBinary.c_str());
  bool subBinary = relBinary && relFromBinary[0] != '.';

  // Select a nice-looking reference to the source file to construct
  // the object file name.
  std::string objectName;
  if((relSource && !relBinary) || (subSource && !subBinary))
    {
    objectName = relFromSource;
    }
  else if((relBinary && !relSource) || (subBinary && !subSource))
    {
    objectName = relFromBinary;
    }
  else if(relFromBinary.length() < relFromSource.length())
    {
    objectName = relFromBinary;
    }
  else
    {
    objectName = relFromSource;
    }

  // if it is still a full path check for the try compile case
  // try compile never have in source sources, and should not
  // have conflicting source file names in the same target
  if(cmSystemTools::FileIsFullPath(objectName.c_str()))
    {
    if(this->GetGlobalGenerator()->GetCMakeInstance()->GetIsInTryCompile())
      {
      objectName = cmSystemTools::GetFilenameName(source.GetFullPath());
      }
    }

  // Replace the original source file extension with the object file
  // extension.
  bool keptSourceExtension = true;
  if(!source.GetPropertyAsBool("KEEP_EXTENSION"))
    {
    // Decide whether this language wants to replace the source
    // extension with the object extension.  For CMake 2.4
    // compatibility do this by default.
    bool replaceExt = this->NeedBackwardsCompatibility(2, 4);
    if(!replaceExt)
      {
      std::string repVar = "CMAKE_";
      repVar += source.GetLanguage();
      repVar += "_OUTPUT_EXTENSION_REPLACE";
      replaceExt = this->Makefile->IsOn(repVar.c_str());
      }

    // Remove the source extension if it is to be replaced.
    if(replaceExt)
      {
      keptSourceExtension = false;
      std::string::size_type dot_pos = objectName.rfind(".");
      if(dot_pos != std::string::npos)
        {
        objectName = objectName.substr(0, dot_pos);
        }
      }

    // Store the new extension.
    objectName +=
      this->GlobalGenerator->GetLanguageOutputExtension(source);
    }
  if(hasSourceExtension)
    {
    *hasSourceExtension = keptSourceExtension;
    }

  // Convert to a safe name.
  return this->CreateSafeUniqueObjectFileName(objectName.c_str(), dir_len);
}

//----------------------------------------------------------------------------
const char*
cmLocalGenerator
::GetSourceFileLanguage(const cmSourceFile& source)
{
  return source.GetLanguage();
}

//----------------------------------------------------------------------------
std::string cmLocalGenerator::EscapeForShellOldStyle(const char* str)
{
  std::string result;
  bool forceOn =  cmSystemTools::GetForceUnixPaths();
  if(forceOn && this->WindowsShell)
    {
    cmSystemTools::SetForceUnixPaths(false);
    }
  result = cmSystemTools::EscapeSpaces(str);
  if(forceOn && this->WindowsShell)
    {
    cmSystemTools::SetForceUnixPaths(true);
    }
  return result;
}

//----------------------------------------------------------------------------
std::string cmLocalGenerator::EscapeForShell(const char* str, bool makeVars,
                                             bool forEcho)
{
  // Compute the flags for the target shell environment.
  int flags = 0;
  if(this->WindowsVSIDE)
    {
    flags |= cmsysSystem_Shell_Flag_VSIDE;
    }
  else
    {
    flags |= cmsysSystem_Shell_Flag_Make;
    }
  if(makeVars)
    {
    flags |= cmsysSystem_Shell_Flag_AllowMakeVariables;
    }
  if(forEcho)
    {
    flags |= cmsysSystem_Shell_Flag_EchoWindows;
    }
  if(this->WatcomWMake)
    {
    flags |= cmsysSystem_Shell_Flag_WatcomWMake;
    }
  if(this->MinGWMake)
    {
    flags |= cmsysSystem_Shell_Flag_MinGWMake;
    }
  if(this->NMake)
    {
    flags |= cmsysSystem_Shell_Flag_NMake;
    }

  // Compute the buffer size needed.
  int size = (this->WindowsShell ?
              cmsysSystem_Shell_GetArgumentSizeForWindows(str, flags) :
              cmsysSystem_Shell_GetArgumentSizeForUnix(str, flags));

  // Compute the shell argument itself.
  std::vector<char> arg(size);
  if(this->WindowsShell)
    {
    cmsysSystem_Shell_GetArgumentForWindows(str, &arg[0], flags);
    }
  else
    {
    cmsysSystem_Shell_GetArgumentForUnix(str, &arg[0], flags);
    }
  return std::string(&arg[0]);
}

//----------------------------------------------------------------------------
std::string
cmLocalGenerator::GetTargetDirectory(cmTarget const&) const
{
  cmSystemTools::Error("GetTargetDirectory"
                       " called on cmLocalGenerator");
  return "";
}


//----------------------------------------------------------------------------
void 
cmLocalGenerator::GetTargetObjectFileDirectories(cmTarget* ,
                                                 std::vector<std::string>& 
                                                 )
{
  cmSystemTools::Error("GetTargetObjectFileDirectories"
                       " called on cmLocalGenerator");
}

//----------------------------------------------------------------------------
unsigned int cmLocalGenerator::GetBackwardsCompatibility()
{
  // The computed version may change until the project is fully
  // configured.
  if(!this->BackwardsCompatibilityFinal)
    {
    unsigned int major = 0;
    unsigned int minor = 0;
    unsigned int patch = 0;
    if(const char* value
       = this->Makefile->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY"))
      {
      switch(sscanf(value, "%u.%u.%u", &major, &minor, &patch))
        {
        case 2: patch = 0; break;
        case 1: minor = 0; patch = 0; break;
        default: break;
        }
      }
    this->BackwardsCompatibility = CMake_VERSION_ENCODE(major, minor, patch);
    this->BackwardsCompatibilityFinal = this->Configured;
    }

  return this->BackwardsCompatibility;
}

//----------------------------------------------------------------------------
bool cmLocalGenerator::NeedBackwardsCompatibility(unsigned int major,
                                                  unsigned int minor,
                                                  unsigned int patch)
{
  // Compatibility is needed if CMAKE_BACKWARDS_COMPATIBILITY is set
  // equal to or lower than the given version.
  unsigned int actual_compat = this->GetBackwardsCompatibility();
  return (actual_compat &&
          actual_compat <= CMake_VERSION_ENCODE(major, minor, patch));
}

//----------------------------------------------------------------------------
bool cmLocalGenerator::CheckDefinition(std::string const& define) const
{
  // Many compilers do not support -DNAME(arg)=sdf so we disable it.
  bool function_style = false;
  for(const char* c = define.c_str(); *c && *c != '='; ++c)
    {
    if(*c == '(')
      {
      function_style = true;
      break;
      }
    }
  if(function_style)
    {
    cmOStringStream e;
    e << "WARNING: Function-style preprocessor definitions may not be "
      << "passed on the compiler command line because many compilers "
      << "do not support it.\n"
      << "CMake is dropping a preprocessor definition: " << define << "\n"
      << "Consider defining the macro in a (configured) header file.\n";
    cmSystemTools::Message(e.str().c_str());
    return false;
    }

  // Many compilers do not support # in the value so we disable it.
  if(define.find_first_of("#") != define.npos)
    {
    cmOStringStream e;
    e << "WARNING: Peprocessor definitions containing '#' may not be "
      << "passed on the compiler command line because many compilers "
      << "do not support it.\n"
      << "CMake is dropping a preprocessor definition: " << define << "\n"
      << "Consider defining the macro in a (configured) header file.\n";
    cmSystemTools::Message(e.str().c_str());
    return false;
    }

  // Assume it is supported.
  return true;
}
