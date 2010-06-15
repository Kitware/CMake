/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmLocalGenerator.h"

#include "cmComputeLinkInformation.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmInstallGenerator.h"
#include "cmInstallFilesGenerator.h"
#include "cmInstallScriptGenerator.h"
#include "cmInstallTargetGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmTest.h"
#include "cmTestGenerator.h"
#include "cmVersion.h"
#include "cmake.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
# define CM_LG_ENCODE_OBJECT_NAMES
# include <cmsys/MD5.h>
#endif

#include <cmsys/System.h>

#include <ctype.h> // for isalpha

#include <assert.h>

#if defined(__HAIKU__)
#include <StorageKit.h>
#endif

cmLocalGenerator::cmLocalGenerator()
{
  this->Makefile = 0; // moved to after set on global
  this->Parent = 0;
  this->WindowsShell = false;
  this->WindowsVSIDE = false;
  this->WatcomWMake = false;
  this->MinGWMake = false;
  this->NMake = false;
  this->MSYSShell = false;
  this->LinkScriptShell = false;
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

//----------------------------------------------------------------------------
class cmLocalGeneratorCurrent
{
  cmGlobalGenerator* GG;
  cmLocalGenerator* LG;
public:
  cmLocalGeneratorCurrent(cmLocalGenerator* lg)
    {
    this->GG = lg->GetGlobalGenerator();
    this->LG = this->GG->GetCurrentLocalGenerator();
    this->GG->SetCurrentLocalGenerator(lg);
    }
  ~cmLocalGeneratorCurrent()
    {
    this->GG->SetCurrentLocalGenerator(this->LG);
    }
};

//----------------------------------------------------------------------------
void cmLocalGenerator::Configure()
{
  // Manage the global generator's current local generator.
  cmLocalGeneratorCurrent clg(this);
  static_cast<void>(clg);

  // make sure the CMakeFiles dir is there
  std::string filesDir = this->Makefile->GetStartOutputDirectory();
  filesDir += cmake::GetCMakeFilesDirectory();
  cmSystemTools::MakeDirectory(filesDir.c_str());
  
  // find & read the list file
  this->ReadInputFile();

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

  this->ComputeObjectMaxPath();

  this->Configured = true;
}

//----------------------------------------------------------------------------
void cmLocalGenerator::ComputeObjectMaxPath()
{
  // Choose a maximum object file name length.
#if defined(_WIN32) || defined(__CYGWIN__)
  this->ObjectPathMax = 250;
#else
  this->ObjectPathMax = 1000;
#endif
  const char* plen = this->Makefile->GetDefinition("CMAKE_OBJECT_PATH_MAX");
  if(plen && *plen)
    {
    unsigned int pmax;
    if(sscanf(plen, "%u", &pmax) == 1)
      {
      if(pmax >= 128)
        {
        this->ObjectPathMax = pmax;
        }
      else
        {
        cmOStringStream w;
        w << "CMAKE_OBJECT_PATH_MAX is set to " << pmax
          << ", which is less than the minimum of 128.  "
          << "The value will be ignored.";
        this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
        }
      }
    else
      {
      cmOStringStream w;
      w << "CMAKE_OBJECT_PATH_MAX is set to \"" << plen
        << "\", which fails to parse as a positive integer.  "
        << "The value will be ignored.";
      this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, w.str());
      }
    }
  this->ObjectMaxPathViolations.clear();
}

//----------------------------------------------------------------------------
void cmLocalGenerator::ReadInputFile()
{
  // Look for the CMakeLists.txt file.
  std::string currentStart = this->Makefile->GetStartDirectory();
  currentStart += "/CMakeLists.txt";
  if(cmSystemTools::FileExists(currentStart.c_str(), true))
    {
    this->Makefile->ReadListFile(currentStart.c_str());
    return;
    }

  if(!this->Parent)
    {
    return;
    }

  // The file is missing.  Check policy CMP0014.
  cmMakefile* mf = this->Parent->GetMakefile();
  cmOStringStream e;
  e << "The source directory\n"
    << "  " << this->Makefile->GetStartDirectory() << "\n"
    << "does not contain a CMakeLists.txt file.";
  switch (mf->GetPolicyStatus(cmPolicies::CMP0014))
    {
    case cmPolicies::WARN:
      // Print the warning.
      e << "\n"
        << "CMake does not support this case but it used "
        << "to work accidentally and is being allowed for "
        << "compatibility."
        << "\n"
        << mf->GetPolicies()->GetPolicyWarning(cmPolicies::CMP0014);
      mf->IssueMessage(cmake::AUTHOR_WARNING, e.str());
    case cmPolicies::OLD:
      // OLD behavior does not warn.
      return;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      e << "\n"
        << mf->GetPolicies()->GetRequiredPolicyError(cmPolicies::CMP0014);
    case cmPolicies::NEW:
      // NEW behavior prints the error.
      mf->IssueMessage(cmake::FATAL_ERROR, e.str());
      break;
    }
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
  this->Makefile = new cmMakefile;
  this->Makefile->SetLocalGenerator(this);

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
    const char* projectFilename = 0;
    if (this->IsMakefileGenerator == false)  // only use of this variable
      {
      projectFilename = t->second.GetName();
      }
    t->second.TraceDependencies(projectFilename);
    }
}

void cmLocalGenerator::GenerateTestFiles()
{
  if ( !this->Makefile->IsOn("CMAKE_TESTING_ENABLED") )
    {
    return;
    }

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
       << "# This file includes the relevent testing commands "
       << "required for " << std::endl
       << "# testing this directory and lists subdirectories to "
       << "be tested as well." << std::endl;
  
  const char* testIncludeFile = 
    this->Makefile->GetProperty("TEST_INCLUDE_FILE");
  if ( testIncludeFile )
    {
    fout << "INCLUDE(\"" << testIncludeFile << "\")" << std::endl;
    }

  // Ask each test generator to write its code.
  std::vector<cmTestGenerator*> const&
    testers = this->Makefile->GetTestGenerators();
  for(std::vector<cmTestGenerator*>::const_iterator gi = testers.begin();
      gi != testers.end(); ++gi)
    {
    (*gi)->Generate(fout, config, configurationTypes);
    }
  if ( this->Children.size())
    {
    size_t i;
    for(i = 0; i < this->Children.size(); ++i)
      {
      fout << "SUBDIRS(";
      std::string outP = 
        this->Children[i]->GetMakefile()->GetStartOutputDirectory();
      fout << this->Convert(outP.c_str(),START_OUTPUT);
      fout << ")" << std::endl;
      }
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
#elif defined(__HAIKU__)
  if (!prefix)
    {
    BPath dir;
    if (find_directory(B_COMMON_DIRECTORY, &dir) == B_OK)
      {
      prefix = dir.Path();
      }
    else
      {
      prefix = "/boot/common";
      }
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
void cmLocalGenerator::GenerateTargetManifest()
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
    if(configNames.empty())
      {
      target.GenerateTargetManifest(0);
      }
    else
      {
      for(std::vector<std::string>::iterator ci = configNames.begin();
          ci != configNames.end(); ++ci)
        {
        const char* config = ci->c_str();
        target.GenerateTargetManifest(config);
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
      std::string dir_max;
      dir_max += this->Makefile->GetCurrentOutputDirectory();
      dir_max += "/";
      std::string obj = this->GetObjectFileNameWithoutTarget(*sf, dir_max);
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
  this->AddArchitectureFlags(langFlags, &target, llang, 0);
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
        const char* llang = target.GetLinkerLanguage();
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
      default:
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
    if(variable == "TARGET_UNQUOTED")
      {
      std::string unquoted = replaceValues.Target;
      std::string::size_type sz = unquoted.size();
      if(sz > 2 && unquoted[0] == '\"' && unquoted[sz-1] == '\"')
        {
        unquoted = unquoted.substr(1, sz-2);
        }
      return unquoted;
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
  if(replaceValues.Language)
    {
    if(variable == "LANGUAGE")
      {
      return replaceValues.Language;
      }
    }
  if(replaceValues.CMTarget)
    {
    if(variable == "TARGET_NAME")
      {
      return replaceValues.CMTarget->GetName();
      }
    if(variable == "TARGET_TYPE")
      {
      return cmTarget::TargetTypeNames[replaceValues.CMTarget->GetType()];
      }
    }
  if(replaceValues.Output)
    {
    if(variable == "OUTPUT")
      {
      return replaceValues.Output;
      }
    }
  if(variable == "CMAKE_COMMAND")
    {
    const char* cmcommand =
      this->GetMakefile()->GetDefinition("CMAKE_COMMAND");
    return this->Convert(cmcommand, FULL, SHELL);
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
  this->InsertRuleLauncher(s, replaceValues.CMTarget,
                           replaceValues.RuleLauncher);
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

//----------------------------------------------------------------------------
const char* cmLocalGenerator::GetRuleLauncher(cmTarget* target,
                                              const char* prop)
{
  if(target)
    {
    return target->GetProperty(prop);
    }
  else
    {
    return this->Makefile->GetProperty(prop);
    }
}

//----------------------------------------------------------------------------
void cmLocalGenerator::InsertRuleLauncher(std::string& s, cmTarget* target,
                                          const char* prop)
{
  if(const char* val = this->GetRuleLauncher(target, prop))
    {
    cmOStringStream wrapped;
    wrapped << val << " " << s;
    s = wrapped.str();
    }
}

//----------------------------------------------------------------------------
std::string
cmLocalGenerator::ConvertToOutputForExistingCommon(const char* remote,
                                                   std::string const& result)
{
  // If this is a windows shell, the result has a space, and the path
  // already exists, we can use a short-path to reference it without a
  // space.
  if(this->WindowsShell && result.find(' ') != result.npos &&
     cmSystemTools::FileExists(remote))
    {
    std::string tmp;
    if(cmSystemTools::GetShortPath(remote, tmp))
      {
      return this->Convert(tmp.c_str(), NONE, SHELL, true);
      }
    }

  // Otherwise, leave it unchanged.
  return result;
}

//----------------------------------------------------------------------------
std::string
cmLocalGenerator::ConvertToOutputForExisting(const char* remote,
                                             RelativeRoot local)
{
  // Perform standard conversion.
  std::string result = this->Convert(remote, local, SHELL, true);

  // Consider short-path.
  return this->ConvertToOutputForExistingCommon(remote, result);
}

//----------------------------------------------------------------------------
std::string
cmLocalGenerator::ConvertToOutputForExisting(RelativeRoot remote,
                                             const char* local)
{
  // Perform standard conversion.
  std::string result = this->Convert(remote, local, SHELL, true);

  // Consider short-path.
  const char* remotePath = this->GetRelativeRootPath(remote);
  return this->ConvertToOutputForExistingCommon(remotePath, result);
}

//----------------------------------------------------------------------------
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
  this->GetIncludeDirectories(includes, lang);
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
    if(this->Makefile->IsOn("APPLE")
       && cmSystemTools::IsPathToFramework(i->c_str()))
      {
      std::string frameworkDir = *i;
      frameworkDir += "/../";
      frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir.c_str());
      if(emitted.insert(frameworkDir).second)
        {
        includeFlags
          << "-F" << this->Convert(frameworkDir.c_str(),
                                   cmLocalGenerator::START_OUTPUT,
                                   cmLocalGenerator::SHELL, true)
          << " ";
        }
      continue;
      }

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
  if((sep[0] != ' ') && flags.size()>0 && flags[flags.size()-1] == sep[0])
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
                                             const char* lang)
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
  if(this->NeedBackwardsCompatibility(1,9))
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

  // Load implicit include directories for this language.
  std::string impDirVar = "CMAKE_";
  impDirVar += lang;
  impDirVar += "_IMPLICIT_INCLUDE_DIRECTORIES";
  if(const char* value = this->Makefile->GetDefinition(impDirVar.c_str()))
    {
    std::vector<std::string> impDirVec;
    cmSystemTools::ExpandListArgument(value, impDirVec);
    for(std::vector<std::string>::const_iterator i = impDirVec.begin();
        i != impDirVec.end(); ++i)
      {
      emitted.insert(*i);
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
      const char* linkLanguage = target.GetLinkerLanguage();
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
    default:
      break;
    }
}

std::string cmLocalGenerator::ConvertToLinkReference(std::string const& lib)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  // Work-ardound command line parsing limitations in MSVC 6.0 and
  // Watcom.
  if(this->Makefile->IsOn("MSVC60") || this->Makefile->IsOn("WATCOM"))
    {
    // Search for the last space.
    std::string::size_type pos = lib.rfind(' ');
    if(pos != lib.npos)
      {
      // Find the slash after the last space, if any.
      pos = lib.find('/', pos);

      // Convert the portion of the path with a space to a short path.
      std::string sp;
      if(cmSystemTools::GetShortPath(lib.substr(0, pos).c_str(), sp))
        {
        // Append the rest of the path with no space.
        sp += lib.substr(pos);

        // Convert to an output path.
        return this->Convert(sp.c_str(), NONE, SHELL);
        }
      }
    }
#endif

  // Normal behavior.
  return this->Convert(lib.c_str(), START_OUTPUT, SHELL);
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
  const char* config = this->Makefile->GetDefinition("CMAKE_BUILD_TYPE");
  cmComputeLinkInformation* pcli = tgt.GetLinkInformation(config);
  if(!pcli)
    {
    return;
    }
  cmComputeLinkInformation& cli = *pcli;

  // Collect library linking flags command line options.
  std::string linkLibs;

  const char* linkLanguage = cli.GetLinkLanguage();

  std::string libPathFlag = 
    this->Makefile->GetRequiredDefinition("CMAKE_LIBRARY_PATH_FLAG");
  std::string libPathTerminator = 
    this->Makefile->GetSafeDefinition("CMAKE_LIBRARY_PATH_TERMINATOR");

  // Flags to link an executable to shared libraries.
  std::string linkFlagsVar = "CMAKE_SHARED_LIBRARY_LINK_";
  linkFlagsVar += linkLanguage;
  linkFlagsVar += "_FLAGS";
  if( tgt.GetType() == cmTarget::EXECUTABLE )
    {
    linkLibs = this->Makefile->GetSafeDefinition(linkFlagsVar.c_str());
    linkLibs += " ";
    }

  // Append the framework search path flags.
  std::vector<std::string> const& fwDirs = cli.GetFrameworkPaths();
  for(std::vector<std::string>::const_iterator fdi = fwDirs.begin();
      fdi != fwDirs.end(); ++fdi)
    {
    linkLibs += "-F";
    linkLibs += this->Convert(fdi->c_str(), NONE, SHELL, false);
    linkLibs += " ";
    }

  // Append the library search path flags.
  std::vector<std::string> const& libDirs = cli.GetDirectories();
  for(std::vector<std::string>::const_iterator libDir = libDirs.begin();
      libDir != libDirs.end(); ++libDir)
    {
    std::string libpath = this->ConvertToOutputForExisting(libDir->c_str());
    linkLibs += libPathFlag;
    linkLibs += libpath;
    linkLibs += libPathTerminator;
    linkLibs += " ";
    }

  // Append the link items.
  typedef cmComputeLinkInformation::ItemVector ItemVector;
  ItemVector const& items = cli.GetItems();
  for(ItemVector::const_iterator li = items.begin(); li != items.end(); ++li)
    {
    if(li->IsPath)
      {
      linkLibs += this->ConvertToLinkReference(li->Value);
      }
    else
      {
      linkLibs += li->Value;
      }
    linkLibs += " ";
    }

  // Write the library flags to the build rule.
  fout << linkLibs;

  // Get the RPATH entries.
  std::vector<std::string> runtimeDirs;
  cli.GetRPath(runtimeDirs, relink);

  // Check what kind of rpath flags to use.
  if(cli.GetRuntimeSep().empty())
    {
    // Each rpath entry gets its own option ("-R a -R b -R c")
    std::string rpath;
    for(std::vector<std::string>::iterator ri = runtimeDirs.begin();
        ri != runtimeDirs.end(); ++ri)
      {
      rpath += cli.GetRuntimeFlag();
      rpath += this->Convert(ri->c_str(), NONE, SHELL, false);
      rpath += " ";
      }
    fout << rpath;
    }
  else
    {
    // All rpath entries are combined ("-Wl,-rpath,a:b:c").
    std::string rpath = cli.GetRPathString(relink);

    // Store the rpath option in the stream.
    if(!rpath.empty())
      {
      fout << cli.GetRuntimeFlag();
      fout << this->EscapeForShell(rpath.c_str(), true);
      fout << " ";
      }
    }

  // Add the linker runtime search path if any.
  std::string rpath_link = cli.GetRPathLinkString();
  if(!cli.GetRPathLinkFlag().empty() && !rpath_link.empty())
    {
    fout << cli.GetRPathLinkFlag();
    fout << this->EscapeForShell(rpath_link.c_str(), true);
    fout << " ";
    }

  // Add standard libraries for this language.
  std::string standardLibsVar = "CMAKE_";
  standardLibsVar += cli.GetLinkLanguage();
  standardLibsVar += "_STANDARD_LIBRARIES";
  if(const char* stdLibs =
     this->Makefile->GetDefinition(standardLibsVar.c_str()))
    {
    fout << stdLibs << " ";
    }
}


//----------------------------------------------------------------------------
void cmLocalGenerator::AddArchitectureFlags(std::string& flags,
                                            cmTarget* target,
                                            const char *lang,
                                            const char* config)
{
  // Only add Mac OS X specific flags on Darwin platforms (OSX and iphone):
  if(!this->Makefile->IsOn("APPLE"))
    {
    return;
    }

  if(this->EmitUniversalBinaryFlags)
    {
    std::vector<std::string> archs;
    target->GetAppleArchs(config, archs);
    const char* sysroot = 
      this->Makefile->GetDefinition("CMAKE_OSX_SYSROOT");
    const char* sysrootDefault = 
      this->Makefile->GetDefinition("CMAKE_OSX_SYSROOT_DEFAULT");
    const char* deploymentTarget = 
      this->Makefile->GetDefinition("CMAKE_OSX_DEPLOYMENT_TARGET");
    std::string isysrootVar = std::string("CMAKE_") + lang + "_HAS_ISYSROOT";
    bool hasIsysroot = this->Makefile->IsOn(isysrootVar.c_str());
    std::string deploymentTargetFlagVar =
      std::string("CMAKE_") + lang + "_OSX_DEPLOYMENT_TARGET_FLAG";
    const char* deploymentTargetFlag =
      this->Makefile->GetDefinition(deploymentTargetFlagVar.c_str());
    bool flagsUsed = false;
    if(!archs.empty() && sysroot && lang && (lang[0] =='C' || lang[0] == 'F'))
      {
      // if there is more than one arch add the -arch and
      // -isysroot flags, or if there is one arch flag, but
      // it is not the default -arch flag for the system, then
      // add it.  Otherwize do not add -arch and -isysroot
      if(archs[0] != "")
        {
        for( std::vector<std::string>::iterator i = archs.begin();
             i != archs.end(); ++i)
          {
          flags += " -arch ";
          flags += *i;
          }
        if(hasIsysroot)
          {
          flags += " -isysroot ";
          flags += sysroot;
          }
        flagsUsed = true;
        }
      }

    if(!flagsUsed && sysroot && sysrootDefault &&
       strcmp(sysroot, sysrootDefault) != 0 && hasIsysroot)
      {
      flags += " -isysroot ";
      flags += sysroot;
      }

    if (deploymentTargetFlag && *deploymentTargetFlag &&
        deploymentTarget && *deploymentTarget)
      {
      flags += " ";
      flags += deploymentTargetFlag;
      flags += deploymentTarget;
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
  if(cmTarget* target = this->Makefile->FindTargetToUse(name.c_str()))
    {
    // make sure it is not just a coincidence that the target name
    // found is part of the inName
    if(cmSystemTools::FileIsFullPath(inName))
      {
      std::string tLocation;
      if(target->GetType() >= cmTarget::EXECUTABLE && 
         target->GetType() <= cmTarget::MODULE_LIBRARY)
        {
        tLocation = target->GetLocation(config);
        tLocation = cmSystemTools::GetFilenamePath(tLocation);
        tLocation = cmSystemTools::CollapseFullPath(tLocation.c_str());
        }
      std::string depLocation = cmSystemTools::GetFilenamePath(
        std::string(inName));
      depLocation = cmSystemTools::CollapseFullPath(depLocation.c_str());
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
      case cmTarget::UNKNOWN_LIBRARY:
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

  // Check for a source file in this directory that matches the
  // dependency.
  if(cmSourceFile* sf = this->Makefile->GetSource(inName))
    {
    name = sf->GetFullPath();
    return name;
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
  cmTarget* target = this->Makefile->FindTargetToUse(inName);
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
      // Make the definition appear properly on the command line.  Use
      // -DNAME="value" instead of -D"NAME=value" to help VS6 parser.
      std::string::size_type eq = di->find("=");
      defines += di->substr(0, eq);
      if(eq != di->npos)
        {
        defines += "=";
        defines += this->EscapeForShell(di->c_str() + eq + 1, true);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmLocalGenerator::AppendFeatureOptions(
  std::string& flags, const char* lang, const char* feature)
{
  std::string optVar = "CMAKE_";
  optVar += lang;
  optVar += "_COMPILE_OPTIONS_";
  optVar += feature;
  if(const char* optionList = this->Makefile->GetDefinition(optVar.c_str()))
    {
    std::vector<std::string> options;
    cmSystemTools::ExpandListArgument(optionList, options);
    for(std::vector<std::string>::const_iterator oi = options.begin();
        oi != options.end(); ++oi)
      {
      this->AppendFlags(flags, this->EscapeForShell(oi->c_str()).c_str());
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
const char* cmLocalGenerator::GetRelativeRootPath(RelativeRoot relroot)
{
  switch (relroot)
    {
    case HOME:         return this->Makefile->GetHomeDirectory();
    case START:        return this->Makefile->GetStartDirectory();
    case HOME_OUTPUT:  return this->Makefile->GetHomeOutputDirectory();
    case START_OUTPUT: return this->Makefile->GetStartOutputDirectory();
    default: break;
    }
  return 0;
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
  return this->ConvertToOutputFormat(result.c_str(), output);
}

//----------------------------------------------------------------------------
std::string cmLocalGenerator::ConvertToOutputFormat(const char* source,
                                                    OutputFormat output)
{
  std::string result = source;
  // Convert it to an output path.
  if (output == MAKEFILE)
    {
    result = cmSystemTools::ConvertToOutputPath(result.c_str());
    }
  else if( output == SHELL)
    {
        // For the MSYS shell convert drive letters to posix paths, so
    // that c:/some/path becomes /c/some/path.  This is needed to
    // avoid problems with the shell path translation.
    if(this->MSYSShell && !this->LinkScriptShell)
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
  else if(output == RESPONSE)
    {
    result = this->EscapeForShell(result.c_str(), false, false);
    }
  return result;
}

//----------------------------------------------------------------------------
std::string cmLocalGenerator::Convert(RelativeRoot remote,
                                      const char* local,
                                      OutputFormat output,
                                      bool optional)
{
  const char* remotePath = this->GetRelativeRootPath(remote);

  // The relative root must have a path (i.e. not FULL or NONE)
  assert(remotePath != 0);

  if(local && (!optional || this->UseRelativePaths))
    {
    std::vector<std::string> components;
    cmSystemTools::SplitPath(local, components);
    std::string result = this->ConvertToRelativePath(components, remotePath);
    return this->ConvertToOutputFormat(result.c_str(), output);
    }
  else
    {
    return this->ConvertToOutputFormat(remotePath, output);
    }
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
                                        const char* in_remote, bool force)
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

  if(!force)
    {
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
      // We also skip over the leading slash given by the user.
      std::string destination = l->second.GetInstallPath().substr(1);
      cmSystemTools::ConvertToUnixSlashes(destination);
      if(destination.empty())
        {
        destination = ".";
        }

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
          // We also skip over the leading slash given by the user.
          destination = l->second.GetRuntimeInstallPath().substr(1);
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

static
bool cmLocalGeneratorCheckObjectName(std::string& objName,
                                     std::string::size_type dir_len,
                                     std::string::size_type max_total_len)
{
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
                                 std::string const& dir_max)
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
    for(;pos < ssin.size() && ssin[pos] == '/'; ++pos)
      {
      }
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
    if(!cmLocalGeneratorCheckObjectName(ssin, dir_max.size(),
                                        this->ObjectPathMax))
      {
      // Warn if this is the first time the path has been seen.
      if(this->ObjectMaxPathViolations.insert(dir_max).second)
        {
        cmOStringStream m;
        m << "The object file directory\n"
          << "  " << dir_max << "\n"
          << "has " << dir_max.size() << " characters.  "
          << "The maximum full path to an object file is "
          << this->ObjectPathMax << " characters "
          << "(see CMAKE_OBJECT_PATH_MAX).  "
          << "Object file\n"
          << "  " << ssin << "\n"
          << "cannot be safely placed under this directory.  "
          << "The build may not work correctly.";
        this->Makefile->IssueMessage(cmake::WARNING, m.str());
        }
      }
#else
    (void)dir_max;
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
                                 std::string const& dir_max,
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
  return this->CreateSafeUniqueObjectFileName(objectName.c_str(), dir_max);
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
static bool cmLocalGeneratorIsShellOperator(const char* str)
{
  if(strcmp(str, "<") == 0 ||
     strcmp(str, ">") == 0 ||
     strcmp(str, "<<") == 0 ||
     strcmp(str, ">>") == 0 ||
     strcmp(str, "|") == 0 ||
     strcmp(str, "||") == 0 ||
     strcmp(str, "&&") == 0 ||
     strcmp(str, "&>") == 0 ||
     strcmp(str, "1>") == 0 ||
     strcmp(str, "2>") == 0 ||
     strcmp(str, "2>&1") == 0 ||
     strcmp(str, "1>&2") == 0)
    {
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
std::string cmLocalGenerator::EscapeForShell(const char* str, bool makeVars,
                                             bool forEcho)
{
  // Do not escape shell operators.
  if(cmLocalGeneratorIsShellOperator(str))
    {
    return str;
    }

  // Compute the flags for the target shell environment.
  int flags = 0;
  if(this->WindowsVSIDE)
    {
    flags |= cmsysSystem_Shell_Flag_VSIDE;
    }
  else if(!this->LinkScriptShell)
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
std::string cmLocalGenerator::EscapeForCMake(const char* str)
{
  // Always double-quote the argument to take care of most escapes.
  std::string result = "\"";
  for(const char* c = str; *c; ++c)
    {
    if(*c == '"')
      {
      // Escape the double quote to avoid ending the argument.
      result += "\\\"";
      }
    else if(*c == '$')
      {
      // Escape the dollar to avoid expanding variables.
      result += "\\$";
      }
    else if(*c == '\\')
      {
      // Escape the backslash to avoid other escapes.
      result += "\\\\";
      }
    else
      {
      // Other characters will be parsed correctly.
      result += *c;
      }
    }
  result += "\"";
  return result;
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
  // Check the policy to decide whether to pay attention to this
  // variable.
  switch(this->Makefile->GetPolicyStatus(cmPolicies::CMP0001))
    {
    case cmPolicies::WARN:
      // WARN is just OLD without warning because user code does not
      // always affect whether this check is done.
    case cmPolicies::OLD:
      // Old behavior is to check the variable.
      break;
    case cmPolicies::NEW:
      // New behavior is to ignore the variable.
      return false;
    case cmPolicies::REQUIRED_IF_USED:
    case cmPolicies::REQUIRED_ALWAYS:
      // This will never be the case because the only way to require
      // the setting is to require the user to specify version policy
      // 2.6 or higher.  Once we add that requirement then this whole
      // method can be removed anyway.
      return false;
    }

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

//----------------------------------------------------------------------------
static void cmLGInfoProp(cmMakefile* mf, cmTarget* target, const char* prop)
{
  if(const char* val = target->GetProperty(prop))
    {
    mf->AddDefinition(prop, val);
    }
}

//----------------------------------------------------------------------------
void cmLocalGenerator::GenerateAppleInfoPList(cmTarget* target,
                                              const char* targetName,
                                              const char* fname)
{
  // Find the Info.plist template.
  const char* in = target->GetProperty("MACOSX_BUNDLE_INFO_PLIST");
  std::string inFile = (in && *in)? in : "MacOSXBundleInfo.plist.in";
  if(!cmSystemTools::FileIsFullPath(inFile.c_str()))
    {
    std::string inMod = this->Makefile->GetModulesFile(inFile.c_str());
    if(!inMod.empty())
      {
      inFile = inMod;
      }
    }
  if(!cmSystemTools::FileExists(inFile.c_str(), true))
    {
    cmOStringStream e;
    e << "Target " << target->GetName() << " Info.plist template \""
      << inFile << "\" could not be found.";
    cmSystemTools::Error(e.str().c_str());
    return;
    }

  // Convert target properties to variables in an isolated makefile
  // scope to configure the file.  If properties are set they will
  // override user make variables.  If not the configuration will fall
  // back to the directory-level values set by the user.
  cmMakefile* mf = this->Makefile;
  mf->PushScope();
  mf->AddDefinition("MACOSX_BUNDLE_EXECUTABLE_NAME", targetName);
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_INFO_STRING");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_ICON_FILE");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_GUI_IDENTIFIER");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_LONG_VERSION_STRING");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_BUNDLE_NAME");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_SHORT_VERSION_STRING");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_BUNDLE_VERSION");
  cmLGInfoProp(mf, target, "MACOSX_BUNDLE_COPYRIGHT");
  mf->ConfigureFile(inFile.c_str(), fname, false, false, false);
  mf->PopScope();
}

//----------------------------------------------------------------------------
void cmLocalGenerator::GenerateFrameworkInfoPList(cmTarget* target,
                                                  const char* targetName,
                                                  const char* fname)
{
  // Find the Info.plist template.
  const char* in = target->GetProperty("MACOSX_FRAMEWORK_INFO_PLIST");
  std::string inFile = (in && *in)? in : "MacOSXFrameworkInfo.plist.in";
  if(!cmSystemTools::FileIsFullPath(inFile.c_str()))
    {
    std::string inMod = this->Makefile->GetModulesFile(inFile.c_str());
    if(!inMod.empty())
      {
      inFile = inMod;
      }
    }
  if(!cmSystemTools::FileExists(inFile.c_str(), true))
    {
    cmOStringStream e;
    e << "Target " << target->GetName() << " Info.plist template \""
      << inFile << "\" could not be found.";
    cmSystemTools::Error(e.str().c_str());
    return;
    }

  // Convert target properties to variables in an isolated makefile
  // scope to configure the file.  If properties are set they will
  // override user make variables.  If not the configuration will fall
  // back to the directory-level values set by the user.
  cmMakefile* mf = this->Makefile;
  mf->PushScope();
  mf->AddDefinition("MACOSX_FRAMEWORK_NAME", targetName);
  cmLGInfoProp(mf, target, "MACOSX_FRAMEWORK_ICON_FILE");
  cmLGInfoProp(mf, target, "MACOSX_FRAMEWORK_IDENTIFIER");
  cmLGInfoProp(mf, target, "MACOSX_FRAMEWORK_SHORT_VERSION_STRING");
  cmLGInfoProp(mf, target, "MACOSX_FRAMEWORK_BUNDLE_VERSION");
  mf->ConfigureFile(inFile.c_str(), fname, false, false, false);
  mf->PopScope();
}
