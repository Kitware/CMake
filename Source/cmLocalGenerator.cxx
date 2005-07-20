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
#include "cmGlobalGenerator.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmGeneratedFileStream.h"
#include "cmSourceFile.h"
#include "cmOrderLinkDirectories.h"
#include "cmTest.h"
#include <ctype.h> // for isalpha

cmLocalGenerator::cmLocalGenerator()
{
  m_Makefile = new cmMakefile;
  m_Makefile->SetLocalGenerator(this);
  m_ExcludeFromAll = false;
  m_Parent = 0;
  m_WindowsShell = false;
  m_IgnoreLibPrefix = false;
  m_UseRelativePaths = false;
  this->Configured = false;
}

cmLocalGenerator::~cmLocalGenerator()
{
  delete m_Makefile;
}

void cmLocalGenerator::Configure()
{
  // find & read the list file
  std::string currentStart = m_Makefile->GetStartDirectory();
  currentStart += "/CMakeLists.txt";
  m_Makefile->ReadListFile(currentStart.c_str());

  // at the end of the ReadListFile handle any old style subdirs
  // first get all the subdirectories
  std::vector<cmLocalGenerator *> subdirs = this->GetChildren();
  
  // for each subdir recurse
  std::vector<cmLocalGenerator *>::iterator sdi = subdirs.begin();
  for (; sdi != subdirs.end(); ++sdi)
    {
    if (!(*sdi)->Configured)
      {
      m_Makefile->ConfigureSubDirectory(*sdi);
      }
    }  
  
  // Setup the current output directory components for use by
  // Convert
  std::string outdir; 
  outdir =
    cmSystemTools::CollapseFullPath(m_Makefile->GetHomeDirectory());
  cmSystemTools::SplitPath(outdir.c_str(), m_HomeDirectoryComponents);
  outdir =
    cmSystemTools::CollapseFullPath(m_Makefile->GetStartDirectory());
  cmSystemTools::SplitPath(outdir.c_str(), m_StartDirectoryComponents);
  outdir =
    cmSystemTools::CollapseFullPath(m_Makefile->GetHomeOutputDirectory());
  cmSystemTools::SplitPath(outdir.c_str(), m_HomeOutputDirectoryComponents);
  outdir =
    cmSystemTools::CollapseFullPath(m_Makefile->GetStartOutputDirectory());
  cmSystemTools::SplitPath(outdir.c_str(), m_StartOutputDirectoryComponents);

  // Check whether relative paths should be used for optionally
  // relative paths.
  m_UseRelativePaths = m_Makefile->IsOn("CMAKE_USE_RELATIVE_PATHS");

  this->Configured = true;
}

void cmLocalGenerator::SetGlobalGenerator(cmGlobalGenerator *gg)
{
  m_GlobalGenerator = gg;

  // setup the home directories
  m_Makefile->SetHomeDirectory(
    gg->GetCMakeInstance()->GetHomeDirectory());
  m_Makefile->SetHomeOutputDirectory(
    gg->GetCMakeInstance()->GetHomeOutputDirectory());
}

void cmLocalGenerator::ConfigureFinalPass()
{
  m_Makefile->ConfigureFinalPass();
}

void cmLocalGenerator::GenerateTestFiles()
{
  if ( !m_Makefile->IsOn("CMAKE_TESTING_ENABLED") )
    {
    return;
    }
  std::string file = m_Makefile->GetStartOutputDirectory();
  file += "/";
  if ( m_Makefile->IsSet("CTEST_NEW_FORMAT") )
    {
    file += "CTestTestfile.cmake";
    }
  else
    {
    file += "DartTestfile.txt";
    }
  cmGeneratedFileStream fout(file.c_str());
  fout.SetCopyIfDifferent(true);

  fout << "# CMake generated Testfile for " << std::endl
    << "# Source directory: " << m_Makefile->GetStartDirectory() << std::endl
    << "# Build directory: " << m_Makefile->GetStartOutputDirectory() << std::endl
    << "# " << std::endl
    << "# This file replicates the SUBDIRS() and ADD_TEST() commands from the source" << std::endl
    << "# tree CMakeLists.txt file, skipping any SUBDIRS() or ADD_TEST() commands" << std::endl
    << "# that are excluded by CMake control structures, i.e. IF() commands." << std::endl
    << "#" << std::endl
    << "# The next line is critical for Dart to work" << std::endl
    << "# Duh :-)" << std::endl << std::endl;

  const char* testIncludeFile = m_Makefile->GetProperty("TEST_INCLUDE_FILE");
  if ( testIncludeFile )
    {
    fout << "INCLUDE(\"" << testIncludeFile << "\")" << std::endl;
    }

  const std::vector<cmTest*> *tests = m_Makefile->GetTests();
  std::vector<cmTest*>::const_iterator it;
  for ( it = tests->begin(); it != tests->end(); ++ it )
    {
    cmTest* test = *it;
    fout << "ADD_TEST(";
    fout << test->GetName() << " \"" << test->GetCommand() << "\"";

    std::vector<cmStdString>::iterator argit;
    for (argit = test->GetArguments().begin();
      argit != test->GetArguments().end(); ++argit)
      {
      // Just double-quote all arguments so they are re-parsed
      // correctly by the test system.
      fout << " \"";
      for(std::string::iterator c = argit->begin(); c != argit->end(); ++c)
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

    }
  if ( this->Children.size())
    {
    fout << "SUBDIRS(";
    size_t i;
    std::string outDir = m_Makefile->GetStartOutputDirectory();
    outDir += "/";
    for(i = 0; i < this->Children.size(); ++i)
      {
      std::string binP = this->Children[i]->GetMakefile()->GetStartOutputDirectory();
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

void cmLocalGenerator::GenerateInstallRules()
{
  cmTargets &tgts = m_Makefile->GetTargets();
  const char* prefix
    = m_Makefile->GetDefinition("CMAKE_INSTALL_PREFIX");
#if defined(_WIN32) && !defined(__CYGWIN__)
  std::string prefix_win32;
  if(!prefix)
    {
    if(!cmSystemTools::GetEnv("SystemDrive", prefix_win32))
      {
      prefix_win32 = "C:";
      }
    const char* project_name = m_Makefile->GetDefinition("PROJECT_NAME");
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

  std::string file = m_Makefile->GetStartOutputDirectory();
  std::string homedir = m_Makefile->GetHomeOutputDirectory();
  std::string currdir = m_Makefile->GetCurrentOutputDirectory();
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

  fout << "# Install script for directory: " << m_Makefile->GetCurrentDirectory() 
    << std::endl << std::endl;

  const char* cmakeDebugPosfix = m_Makefile->GetDefinition("CMAKE_DEBUG_POSTFIX");
  if ( cmakeDebugPosfix )
    {
    fout << "SET(CMAKE_DEBUG_POSTFIX \"" << cmakeDebugPosfix << "\")"
      << std::endl << std::endl;
    }

  std::string libOutPath = "";
  if (m_Makefile->GetDefinition("LIBRARY_OUTPUT_PATH"))
    {
    libOutPath = m_Makefile->GetDefinition("LIBRARY_OUTPUT_PATH");
    if(libOutPath.size())
      {
      if(libOutPath[libOutPath.size() -1] != '/')
        {
        libOutPath += "/";
        }
      }
    }

  std::string exeOutPath = "";
  if (m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH"))
    {
    exeOutPath =
      m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH");
    if(exeOutPath.size())
      {
      if(exeOutPath[exeOutPath.size() -1] != '/')
        {
        exeOutPath += "/";
        }
      }
    }
  if ( libOutPath.size() == 0 )
    {
    // LIBRARY_OUTPUT_PATH not defined
    libOutPath = currdir + "/";
    }
  if ( exeOutPath.size() == 0 )
    {
    // EXECUTABLE_OUTPUT_PATH not defined
    exeOutPath = currdir + "/";
    }

  std::string destination;
  for(cmTargets::iterator l = tgts.begin(); 
    l != tgts.end(); l++)
    {
    const char* preinstall = l->second.GetProperty("PRE_INSTALL_SCRIPT");
    const char* postinstall = l->second.GetProperty("POST_INSTALL_SCRIPT");
    if ( preinstall )
      {
      fout << "INCLUDE(\"" << preinstall << "\")" << std::endl;
      }
    if (l->second.GetInstallPath() != "")
      {
      destination = prefix + l->second.GetInstallPath();
      cmSystemTools::ConvertToUnixSlashes(destination);
      const char* dest = destination.c_str();
      int type = l->second.GetType();


      std::string fname;
      const char* files;
      // now install the target
      switch (type)
        {
      case cmTarget::STATIC_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
        fname = libOutPath;
        fname += l->second.GetFullName(m_Makefile);
        files = fname.c_str();
        this->AddInstallRule(fout, dest, type, files);
        break;
      case cmTarget::SHARED_LIBRARY:
        {
        // Special code to handle DLL
        fname = libOutPath;
        fname += l->second.GetFullName(m_Makefile);
        std::string ext = cmSystemTools::GetFilenameExtension(fname);
        ext = cmSystemTools::LowerCase(ext);
        if ( ext == ".dll" )
          {
          std::string libname = libOutPath;
          libname += cmSystemTools::GetFilenameWithoutExtension(fname);
          libname += ".lib";
          files = libname.c_str();
          this->AddInstallRule(fout, dest, cmTarget::STATIC_LIBRARY, files, true);
          std::string dlldest = prefix + l->second.GetRuntimeInstallPath();
          files = fname.c_str();
          this->AddInstallRule(fout, dlldest.c_str(), type, files);
          }
        else
          {
          files = fname.c_str();
          std::string properties;
          const char* lib_version = l->second.GetProperty("VERSION");
          const char* lib_soversion = l->second.GetProperty("SOVERSION");
          if(!m_Makefile->GetDefinition("CMAKE_SHARED_LIBRARY_SONAME_C_FLAG"))
            {
            // Versioning is supported only for shared libraries and modules,
            // and then only when the platform supports an soname flag.
            lib_version = 0;
            lib_soversion = 0;
            }
          if ( lib_version )
            {
            properties += " VERSION ";
            properties += lib_version;
            }
          if ( lib_soversion )
            {
            properties += " SOVERSION ";
            properties += lib_soversion;
            }
          this->AddInstallRule(fout, dest, type, files, false, properties.c_str());
          }
        }
        break;
      case cmTarget::EXECUTABLE:
        if(l->second.GetPropertyAsBool("MACOSX_BUNDLE"))
          {
          fname = exeOutPath;
          fname += l->second.GetFullName(m_Makefile);
          std::string plist = fname;
          plist += ".app/Contents/Info.plist";
          fname += ".app/Contents/MacOS/";
          fname += l->second.GetName();
          files = fname.c_str();
          std::string bdest = dest;
          bdest += "/";
          bdest += l->second.GetName();
          std::string pdest = bdest;
          pdest += ".app/Contents";
          bdest += ".app/Contents/MacOS";
          // first install the actual executable
          this->AddInstallRule(fout, bdest.c_str(), type, files);
          files = plist.c_str();
          // now install the Info.plist file
          this->AddInstallRule(fout, pdest.c_str(), 
                               cmTarget::INSTALL_FILES, files);
          }
        else
          {
          fname = exeOutPath;
          fname += l->second.GetFullName(m_Makefile);
          files = fname.c_str();
          this->AddInstallRule(fout, dest, type, files);
          }
        break;
      case cmTarget::INSTALL_FILES:
          {
          std::string sourcePath = m_Makefile->GetCurrentDirectory();
          std::string binaryPath = m_Makefile->GetCurrentOutputDirectory();
          sourcePath += "/";
          binaryPath += "/";
          const std::vector<std::string> &sf = l->second.GetSourceLists();
          std::vector<std::string>::const_iterator i;
          for (i = sf.begin(); i != sf.end(); ++i)
            {
            std::string f = *i;
            if(f.substr(0, sourcePath.length()) == sourcePath)
              {
              f = f.substr(sourcePath.length());
              }
            else if(f.substr(0, binaryPath.length()) == binaryPath)
              {
              f = f.substr(binaryPath.length());
              }

            files = i->c_str();
            this->AddInstallRule(fout, dest, type, files);
            }
          }
        break;
      case cmTarget::INSTALL_PROGRAMS:
          {
          std::string sourcePath = m_Makefile->GetCurrentDirectory();
          std::string binaryPath = m_Makefile->GetCurrentOutputDirectory();
          sourcePath += "/";
          binaryPath += "/";
          const std::vector<std::string> &sf = l->second.GetSourceLists();
          std::vector<std::string>::const_iterator i;
          for (i = sf.begin(); i != sf.end(); ++i)
            {
            std::string f = *i;
            if(f.substr(0, sourcePath.length()) == sourcePath)
              {
              f = f.substr(sourcePath.length());
              }
            else if(f.substr(0, binaryPath.length()) == binaryPath)
              {
              f = f.substr(binaryPath.length());
              }
            files = i->c_str();
            this->AddInstallRule(fout, dest, type, files);
            }
          }
        break;
      case cmTarget::UTILITY:
      default:
        break;
        }
      }
    if ( postinstall )
      {
      fout << "INCLUDE(\"" << postinstall << "\")" << std::endl;
      }
    }

  if ( this->Children.size())
    {
    std::vector<cmLocalGenerator*>::const_iterator i = this->Children.begin();
    for(; i != this->Children.end(); ++i)
      {
      std::string odir = (*i)->GetMakefile()->GetStartOutputDirectory();
      cmSystemTools::ConvertToUnixSlashes(odir);
      fout << "INCLUDE(\"" <<  odir.c_str() 
           << "/cmake_install.cmake\")" << std::endl;
      }
    fout << std::endl;;
    }
  if ( toplevel_install )
    {
    fout << "FILE(WRITE \"" << homedir.c_str() << "/install_manifest.txt\" "
         << "\"\")" << std::endl;
    fout << "FOREACH(file ${CMAKE_INSTALL_MANIFEST_FILES})" << std::endl
      << "  FILE(APPEND \"" << homedir.c_str() << "/install_manifest.txt\" "
      << "\"${file}\\n\")" << std::endl
      << "ENDFOREACH(file)" << std::endl;
    }
}

void cmLocalGenerator::AddInstallRule(std::ostream& fout, const char* dest, 
  int type, const char* files, bool optional /* = false */, const char* properties /* = 0 */)
{
  std::string sfiles = files;
  std::string destination = dest;
  std::string stype;
  switch ( type )
    {
  case cmTarget::INSTALL_PROGRAMS: stype = "PROGRAM"; break;
  case cmTarget::EXECUTABLE: stype = "EXECUTABLE"; break;
  case cmTarget::STATIC_LIBRARY:   stype = "STATIC_LIBRARY"; break;
  case cmTarget::SHARED_LIBRARY:   stype = "SHARED_LIBRARY"; break;
  case cmTarget::MODULE_LIBRARY:   stype = "MODULE"; break;
  case cmTarget::INSTALL_FILES:
  default:                         stype = "FILE"; break;
    }
  std::string fname = cmSystemTools::GetFilenameName(sfiles.c_str());
  fout 
    << "MESSAGE(STATUS \"Installing " << destination.c_str() 
    << "/" << fname.c_str() << "\")\n" 
    << "FILE(INSTALL DESTINATION \"" << destination.c_str() 
    << "\" TYPE " << stype.c_str() << (optional?" OPTIONAL":"") ;
  if ( properties && *properties )
    {
    fout << " PROPERTIES" << properties;
    }
  fout
    << " FILES \"" << sfiles.c_str() << "\")\n";
}

void cmLocalGenerator::AddCustomCommandToCreateObject(const char* ofname, 
                                                      const char* lang, 
                                                      cmSourceFile& source,
                                                      cmTarget& )
{ 
  std::string objectFile = this->Convert(ofname,START_OUTPUT,SHELL);
  std::string sourceFile = 
    this->Convert(source.GetFullPath().c_str(),START_OUTPUT,SHELL,true);
  std::string varString = "CMAKE_";
  varString += lang;
  varString += "_COMPILE_OBJECT";
  std::vector<std::string> rules;
  rules.push_back(m_Makefile->GetRequiredDefinition(varString.c_str()));
  varString = "CMAKE_";
  varString += lang;
  varString += "_FLAGS";
  std::string flags;
  flags += m_Makefile->GetSafeDefinition(varString.c_str());
  flags += " ";
  flags += this->GetIncludeFlags(lang);
  std::vector<std::string> commands;
  cmSystemTools::ExpandList(rules, commands);
  for(std::vector<std::string>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->ExpandRuleVariables(*i,
                              lang,
                              0, // no objects
                              0, // no target
                              0, // no link libs
                              sourceFile.c_str(),
                              objectFile.c_str(),
                              flags.c_str());
    }
  std::vector<std::string> sourceAndDeps;
  sourceAndDeps.push_back(sourceFile);
  if(commands.size() > 1)
    {
    cmSystemTools::Error("Currently custom rules can only have one command sorry ");
    }
   // Check for extra object-file dependencies.
  std::vector<std::string> depends;
  const char* additionalDeps = source.GetProperty("OBJECT_DEPENDS");
  if(additionalDeps)
    {
    cmSystemTools::ExpandListArgument(additionalDeps, depends);
    for(std::vector<std::string>::iterator i = depends.begin();
        i != depends.end(); ++i)
      {
      sourceAndDeps.push_back(this->Convert(i->c_str(),START_OUTPUT,SHELL));
      }
    } 
#if 0
  std::string command;
  std::string args;
  cmSystemTools::SplitProgramFromArgs(commands[0].c_str(), command, args);
  std::vector<std::string> argsv;
  argsv.push_back(args);
  m_Makefile->AddCustomCommandToOutput(ofname, 
                                       command.c_str(), 
                                       argsv,
                                       source.GetFullPath().c_str(),
                                       sourceAndDeps,
                                       "build from source");
#endif
}

void cmLocalGenerator::AddBuildTargetRule(const char* llang, cmTarget& target)
{
  cmStdString objs;
  std::vector<std::string> objVector;
  // Add all the sources outputs to the depends of the target
  std::vector<cmSourceFile*>& classes = target.GetSourceFiles();
  for(std::vector<cmSourceFile*>::iterator i = classes.begin();
      i != classes.end(); ++i)
    { 
    if(!(*i)->GetPropertyAsBool("HEADER_FILE_ONLY") && 
       !(*i)->GetCustomCommand())
      {
      std::string outExt = 
        m_GlobalGenerator->GetLanguageOutputExtensionFromExtension(
          (*i)->GetSourceExtension().c_str());
      if(outExt.size() && !(*i)->GetPropertyAsBool("EXTERNAL_OBJECT") )
        {
        std::string ofname = m_Makefile->GetCurrentOutputDirectory();
        ofname += "/";
        ofname += (*i)->GetSourceName() + outExt;
        objVector.push_back(ofname);
        this->AddCustomCommandToCreateObject(ofname.c_str(), llang, *(*i), target);
        objs += this->Convert(ofname.c_str(),START_OUTPUT,MAKEFILE);
        objs += " ";
        }
      }
    }
  std::string createRule = "CMAKE_";
  createRule += llang;
  createRule += target.GetCreateRuleVariable();
  std::string targetName = target.GetFullName(m_Makefile);
  // Executable :
  // Shared Library:
  // Static Library:
  // Shared Module:
  std::string linkLibs; // should be set
  std::string flags; // should be set
  std::string linkFlags; // should be set 
  this->GetTargetFlags(linkLibs, flags, linkFlags, target);
  std::string rule = m_Makefile->GetRequiredDefinition(createRule.c_str());
  this->ExpandRuleVariables(rule, 
                            llang, // language
                            objs.c_str(), // objects
                            targetName.c_str(), // target
                            linkLibs.c_str(), // link libs
                            0, // source
                            0, // object
                            flags.c_str(), // flags
                            0, // objects quoted
                            0, // target base name
                            0, // target so name,
                            linkFlags.c_str() // link flags
    );
#if 0
  std::string command;
  std::string args;
  cmSystemTools::SplitProgramFromArgs(rule.c_str(), command, args);
  // Just like ADD_CUSTOM_TARGET(foo ALL DEPENDS a.o b.o)
  // Add a custom command for generating each .o file
  cmCustomCommand cc(command.c_str(), args.c_str(), objVector,
                     targetName.c_str(), 0);
  target.GetPostBuildCommands().push_back(cc);
#endif
}

  
void cmLocalGenerator::CreateCustomTargetsAndCommands(std::set<cmStdString> const& lang)
{ 
  cmTargets &tgts = m_Makefile->GetTargets();
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
        const char* llang = target.GetLinkerLanguage(this->GetGlobalGenerator());
        if(!llang)
          {
          cmSystemTools::Error("CMake can not determine linker language for target:",
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
      case cmTarget::INSTALL_FILES:
      case cmTarget::INSTALL_PROGRAMS:
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
  0
};

std::string
cmLocalGenerator::ExpandRuleVariable(std::string const& variable,
                                     const char* lang,
                                     const char* objects,
                                     const char* target,
                                     const char* linkLibs,
                                     const char* source,
                                     const char* object,
                                     const char* flags,
                                     const char* objectsquoted,
                                     const char* targetBase,
                                     const char* targetSOName,
                                     const char* linkFlags)
{
  if(linkFlags)
    {
    if(variable == "LINK_FLAGS")
      {
      return linkFlags;
      }
    }
  if(flags)
    {
    if(variable == "FLAGS")
      {
      return flags;
      }
    }
    
  if(source)
    {
    if(variable == "SOURCE")
      {
      return source;
      }
    }
  if(object)
    {
    if(variable == "OBJECT")
      {
      return object;
      }
    }
  if(objects)
    {
    if(variable == "OBJECTS")
      {
      return objects;
      }
    }
  if(objectsquoted)
    {
    if(variable == "OBJECTS_QUOTED")
      {
      return objectsquoted;
      }
    }
  if(target)
    { 
    if(variable == "TARGET_QUOTED")
      {
      std::string targetQuoted = target;
      if(targetQuoted.size() && targetQuoted[0] != '\"')
        {
        targetQuoted = '\"';
        targetQuoted += target;
        targetQuoted += '\"';
        return targetQuoted;
        }
      }
    if(variable == "TARGET")
      {
      return target;
      }
    }
  if(targetBase)
    {
    if(variable == "TARGET_BASE.lib" || variable == "TARGET_BASE.dll")
      {
      // special case for quoted paths with spaces 
      // if you see <TARGET_BASE>.lib then put the .lib inside
      // the quotes, same for .dll
      if((strlen(targetBase) > 1) && targetBase[0] == '\"')
        {
        std::string base = targetBase;
        base[base.size()-1] = '.';
        std::string baseLib = base + "lib\"";
        std::string baseDll = base + "dll\"";
        if(variable == "TARGET_BASE.lib" )
          {
          return baseLib;
          }
        if(variable == "TARGET_BASE.dll" )
          {
          return baseDll;
          }
        }
      }
    if(variable == "TARGET_BASE")
      {
      return targetBase;
      }
    }
  if(targetSOName)
    {
    if(variable == "TARGET_SONAME")
      {
      if(lang)
        {
        std::string name = "CMAKE_SHARED_LIBRARY_SONAME_";
        name += lang;
        name += "_FLAG";
        if(m_Makefile->GetDefinition(name.c_str()))
          {
          return targetSOName;
          }
        }
      return "";
      }
    }
  if(linkLibs)
    {
    if(variable == "LINK_LIBRARIES")
      {
      return linkLibs;
      }
    }
  std::vector<std::string> enabledLanguages;
  m_GlobalGenerator->GetEnabledLanguages(enabledLanguages);
  // loop over language specific replace variables
  int pos = 0;
  while(ruleReplaceVars[pos])
    {
    for(std::vector<std::string>::iterator i = enabledLanguages.begin();   
        i != enabledLanguages.end(); ++i)   
      { 
      lang = i->c_str();
      std::string actualReplace = ruleReplaceVars[pos];
      // If this is the compiler then look for the extra variable
      // _COMPILER_ARG1 which must be the first argument to the compiler 
      const char* compilerArg1 = 0;
      if(actualReplace == "CMAKE_${LANG}_COMPILER")
        {
        std::string arg1 = actualReplace + "_ARG1";
        cmSystemTools::ReplaceString(arg1, "${LANG}", lang);
        compilerArg1 = m_Makefile->GetDefinition(arg1.c_str());
        }
      if(actualReplace.find("${LANG}") != actualReplace.npos)
        {
        cmSystemTools::ReplaceString(actualReplace, "${LANG}", lang);
        }
      if(actualReplace == variable)
        {
        std::string replace = m_Makefile->GetSafeDefinition(variable.c_str());
        // if the variable is not a FLAG then treat it like a path
        if(variable.find("_FLAG") == variable.npos)
          {
          std::string ret = this->ConvertToOutputForExisting(replace.c_str());
          // if there is a required first argument to the compiler add it to the compiler string
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
                                      const char* lang,
                                      const char* objects,
                                      const char* target,
                                      const char* linkLibs,
                                      const char* source,
                                      const char* object,
                                      const char* flags,
                                      const char* objectsquoted,
                                      const char* targetBase,
                                      const char* targetSOName,
                                      const char* linkFlags)
{
  std::vector<std::string> enabledLanguages;
  m_GlobalGenerator->GetEnabledLanguages(enabledLanguages);
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
      std::string replace = this->ExpandRuleVariable(var, lang, objects,
                                                     target, linkLibs,
                                                     source, object, flags,
                                                     objectsquoted, 
                                                     targetBase, targetSOName,
                                                     linkFlags);
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
  std::string ret = this->Convert(p, START_OUTPUT, SHELL, true);
  // if there are spaces in the path, then get the short path version
  // if there is one
  if(ret.find(' ') != std::string::npos)
    {
    if(cmSystemTools::FileExists(p))
      {
      if(!cmSystemTools::GetShortPath(ret.c_str(), ret))
        {
        ret = this->Convert(p,START_OUTPUT,MAKEFILE,true);
        }
      }
    }
  return ret;
}

const char* cmLocalGenerator::GetIncludeFlags(const char* lang)
{
  if(!lang)
    {
    return "";
    }
  if(m_LanguageToIncludeFlags.count(lang))
    {
    return m_LanguageToIncludeFlags[lang].c_str();
    }
  cmOStringStream includeFlags;
  std::vector<std::string> includes;
  this->GetIncludeDirectories(includes);
  std::vector<std::string>::iterator i;

  std::string flagVar = "CMAKE_INCLUDE_FLAG_";
  flagVar += lang;
  const char* includeFlag = m_Makefile->GetDefinition(flagVar.c_str());
  flagVar = "CMAKE_INCLUDE_FLAG_SEP_";
  flagVar += lang;
  const char* sep = m_Makefile->GetDefinition(flagVar.c_str());

  bool repeatFlag = true; // should the include flag be repeated like ie. -IA -IB
  if(!sep)
    {
    sep = " ";
    }
  else
    {
    // if there is a separator then the flag is not repeated but is only given once
    // i.e.  -classpath a:b:c
    repeatFlag = false;
    }
  bool flagUsed = false;
  for(i = includes.begin(); i != includes.end(); ++i)
    {
    std::string include = *i;
    if(!flagUsed || repeatFlag)
      {
      includeFlags << includeFlag;
      flagUsed = true;
      }
    includeFlags << this->ConvertToOutputForExisting(i->c_str()) << sep;
    }
  std::string flags = includeFlags.str();
  // remove trailing separators
  if((sep[0] != ' ') && flags[flags.size()-1] == sep[0])
    {
    flags[flags.size()-1] = ' ';
    }
  flags += m_Makefile->GetDefineFlags();
  m_LanguageToIncludeFlags[lang] = flags;
  return m_LanguageToIncludeFlags[lang].c_str();
}

//----------------------------------------------------------------------------
void cmLocalGenerator::GetIncludeDirectories(std::vector<std::string>& dirs)
{
  // Output Include paths
  std::set<cmStdString> implicitIncludes;

  // CMake versions below 2.0 would add the source tree to the -I path
  // automatically.  Preserve compatibility.
  bool includeSourceDir = false;
  const char* versionValue =
    m_Makefile->GetDefinition("CMAKE_BACKWARDS_COMPATIBILITY");
  if(versionValue)
    {
    int major = 0;
    int minor = 0;
    if(sscanf(versionValue, "%d.%d", &major, &minor) == 2 && major < 2)
      {
      includeSourceDir = true;
      }
    }
  const char* vtkSourceDir =
    m_Makefile->GetDefinition("VTK_SOURCE_DIR");
  if(vtkSourceDir)
    {
    // Special hack for VTK 4.0 - 4.4.
    const char* vtk_major = m_Makefile->GetDefinition("VTK_MAJOR_VERSION");
    const char* vtk_minor = m_Makefile->GetDefinition("VTK_MINOR_VERSION");
    vtk_major = vtk_major? vtk_major : "4";
    vtk_minor = vtk_minor? vtk_minor : "4";
    int major = 0;
    int minor = 0;
    if(sscanf(vtk_major, "%d", &major) && sscanf(vtk_minor, "%d", &minor) &&
       major == 4 && minor <= 4)
      {
      includeSourceDir = true;
      }
    }

  if(includeSourceDir)
    {
    dirs.push_back(m_Makefile->GetStartDirectory());
    }

  // Do not explicitly add the standard include path "/usr/include".
  // This can cause problems with certain standard library
  // implementations because the wrong headers may be found first.
  implicitIncludes.insert("/usr/include");
  if(m_Makefile->GetDefinition("CMAKE_PLATFORM_IMPLICIT_INCLUDE_DIRECTORIES"))
    {
    std::string arg = m_Makefile->GetDefinition("CMAKE_PLATFORM_IMPLICIT_INCLUDE_DIRECTORIES");
    std::vector<std::string> implicitIncludeVec;
    cmSystemTools::ExpandListArgument(arg, implicitIncludeVec);
    for(unsigned int k =0; k < implicitIncludeVec.size(); k++)
      {
      implicitIncludes.insert(implicitIncludeVec[k]);
      }
    }

  // Construct the ordered list.
  std::vector<std::string>& includes = m_Makefile->GetIncludeDirectories();
  for(std::vector<std::string>::iterator i = includes.begin();
      i != includes.end(); ++i)
    {
    if(implicitIncludes.find(*i) == implicitIncludes.end())
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
  std::string buildType =  m_Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
  buildType = cmSystemTools::UpperCase(buildType); 
  const char* libraryLinkVariable = "CMAKE_SHARED_LINKER_FLAGS"; // default to shared library
  
  switch(target.GetType())
    {
    case cmTarget::STATIC_LIBRARY: 
      {
      const char* targetLinkFlags = target.GetProperty("STATIC_LIBRARY_FLAGS");
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
      linkFlags = m_Makefile->GetSafeDefinition(libraryLinkVariable);
      linkFlags += " ";
      if(buildType.size())
        {
        std::string build = libraryLinkVariable;
        build += "_";
        build += buildType;
        linkFlags += m_Makefile->GetSafeDefinition(build.c_str());
        linkFlags += " ";
        }  
      if(m_Makefile->IsOn("WIN32") && !(m_Makefile->IsOn("CYGWIN") || m_Makefile->IsOn("MINGW")))
        {
        const std::vector<cmSourceFile*>& sources = target.GetSourceFiles();
        for(std::vector<cmSourceFile*>::const_iterator i = sources.begin();
            i != sources.end(); ++i)
          {
          if((*i)->GetSourceExtension() == "def")
            {
            linkFlags += m_Makefile->GetSafeDefinition("CMAKE_LINK_DEF_FILE_FLAG");
            linkFlags += this->Convert((*i)->GetFullPath().c_str(),START_OUTPUT,MAKEFILE);
            linkFlags += " ";
            }
          }
        } 
      const char* targetLinkFlags = target.GetProperty("LINK_FLAGS");
      if(targetLinkFlags)
        {
        linkFlags += targetLinkFlags;
        linkFlags += " ";
        }  
      cmOStringStream linklibsStr;
      this->OutputLinkLibraries(linklibsStr, target.GetName(), target);
      linkLibs = linklibsStr.str();
      }
      break;
    case cmTarget::EXECUTABLE:
      {
      linkFlags += m_Makefile->GetSafeDefinition("CMAKE_EXE_LINKER_FLAGS");
      linkFlags += " ";
      if(buildType.size())
        {
        std::string build = "CMAKE_EXE_LINKER_FLAGS_";
        build += buildType;
        linkFlags += m_Makefile->GetSafeDefinition(build.c_str());
        linkFlags += " ";
        } 
      const char* linkLanguage = target.GetLinkerLanguage(this->GetGlobalGenerator());
      if(!linkLanguage)
        {
        cmSystemTools::Error("CMake can not determine linker language for target:",
                             target.GetName());
        return;
        }
      std::string langVar = "CMAKE_";
      langVar += linkLanguage;
      std::string flagsVar = langVar + "_FLAGS";
      std::string sharedFlagsVar = "CMAKE_SHARED_LIBRARY_";
      sharedFlagsVar += linkLanguage;
      sharedFlagsVar += "_FLAGS";
      flags += m_Makefile->GetSafeDefinition(flagsVar.c_str());
      flags += " ";
      flags += m_Makefile->GetSafeDefinition(sharedFlagsVar.c_str());
      flags += " ";
      cmOStringStream linklibs;
      this->OutputLinkLibraries(linklibs, 0, target);
      linkLibs = linklibs.str();
      if(cmSystemTools::IsOn(m_Makefile->GetDefinition("BUILD_SHARED_LIBS")))
        {
        std::string sFlagVar = std::string("CMAKE_SHARED_BUILD_") + linkLanguage 
          + std::string("_FLAGS");
        linkFlags += m_Makefile->GetSafeDefinition(sFlagVar.c_str());
        linkFlags += " ";
        }
      if ( target.GetPropertyAsBool("WIN32_EXECUTABLE") )
        {
        linkFlags +=  m_Makefile->GetSafeDefinition("CMAKE_CREATE_WIN32_EXE");
        linkFlags += " ";
        }
      else
        {
        linkFlags +=  m_Makefile->GetSafeDefinition("CMAKE_CREATE_CONSOLE_EXE");
        linkFlags += " ";
        }
      const char* targetLinkFlags = target.GetProperty("LINK_FLAGS");
      if(targetLinkFlags)
        {
        linkFlags += targetLinkFlags;
        linkFlags += " ";
        }
      }
      break; 
    case cmTarget::UTILITY:
    case cmTarget::INSTALL_FILES:
    case cmTarget::INSTALL_PROGRAMS:
      break;
    }
}

/**
 * Output the linking rules on a command line.  For executables,
 * targetLibrary should be a NULL pointer.  For libraries, it should point
 * to the name of the library.  This will not link a library against itself.
 */
void cmLocalGenerator::OutputLinkLibraries(std::ostream& fout,
                                           const char* targetLibrary,
                                           cmTarget &tgt)
{
  // Try to emit each search path once
  std::set<cmStdString> emitted;
  // Embed runtime search paths if possible and if required.
  bool outputRuntime = true;
  std::string runtimeFlag;
  std::string runtimeSep;
  std::vector<std::string> runtimeDirs;

  std::string buildType =  m_Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
  buildType = cmSystemTools::UpperCase(buildType);
  cmTarget::LinkLibraryType cmakeBuildType = cmTarget::GENERAL;
  if(buildType == "DEBUG")
    {
    cmakeBuildType = cmTarget::DEBUG;
    }
  else if(buildType.size())
    {
    cmakeBuildType = cmTarget::OPTIMIZED;
    }
  const char* linkLanguage = tgt.GetLinkerLanguage(this->GetGlobalGenerator());
  if(!linkLanguage)
    {
    cmSystemTools::
      Error("CMake can not determine linker language for target:",
            tgt.GetName());
    return;
    }
  std::string runTimeFlagVar = "CMAKE_SHARED_LIBRARY_RUNTIME_";
  runTimeFlagVar += linkLanguage;
  runTimeFlagVar += "_FLAG";
  std::string runTimeFlagSepVar = runTimeFlagVar + "_SEP";
  runtimeFlag = m_Makefile->GetSafeDefinition(runTimeFlagVar.c_str());
  runtimeSep = m_Makefile->GetSafeDefinition(runTimeFlagSepVar.c_str());
  
  // concatenate all paths or no?
  bool runtimeConcatenate = ( runtimeSep!="" );
  if(runtimeFlag == "" || m_Makefile->IsOn("CMAKE_SKIP_RPATH") )
    {
    outputRuntime = false;
    }

  // Some search paths should never be emitted
  emitted.insert("");
  emitted.insert("/usr/lib");
  std::string libPathFlag = m_Makefile->GetRequiredDefinition("CMAKE_LIBRARY_PATH_FLAG");
  std::string libLinkFlag = m_Makefile->GetSafeDefinition("CMAKE_LINK_LIBRARY_FLAG");
  // collect all the flags needed for linking libraries
  std::string linkLibs;
  
  // Flags to link an executable to shared libraries.
  std::string linkFlagsVar = "CMAKE_SHARED_LIBRARY_LINK_";
  linkFlagsVar += linkLanguage;
  linkFlagsVar += "_FLAGS";
  if( tgt.GetType() == cmTarget::EXECUTABLE )
    {
    linkLibs = m_Makefile->GetSafeDefinition(linkFlagsVar.c_str());
    linkLibs += " ";
    }
  
  cmOrderLinkDirectories orderLibs;
  std::string ext = 
    m_Makefile->GetSafeDefinition("CMAKE_STATIC_LIBRARY_SUFFIX");
  if(ext.size())
    {
    orderLibs.AddLinkExtension(ext.c_str());
    }
  ext = 
    m_Makefile->GetSafeDefinition("CMAKE_SHARED_LIBRARY_SUFFIX");
  if(ext.size())
    {
    orderLibs.AddLinkExtension(ext.c_str());
    }
  ext = 
    m_Makefile->GetSafeDefinition("CMAKE_LINK_LIBRARY_SUFFIX");
  if(ext.size())
    {
    orderLibs.AddLinkExtension(ext.c_str());
    }
  // compute the correct order for -L paths
  orderLibs.SetLinkInformation(tgt, cmakeBuildType, targetLibrary);
  orderLibs.DetermineLibraryPathOrder();
  std::vector<cmStdString> libdirs;
  std::vector<cmStdString> linkItems;
  orderLibs.GetLinkerInformation(libdirs, linkItems);
  for(std::vector<cmStdString>::const_iterator libDir = libdirs.begin();
      libDir != libdirs.end(); ++libDir)
    { 
    std::string libpath = this->ConvertToOutputForExisting(libDir->c_str());
    if(emitted.insert(libpath).second)
      {
      std::string fullLibPath;
      if(!m_WindowsShell && m_UseRelativePaths)
        {
        fullLibPath = "\"`cd ";
        }
      fullLibPath += libpath;
      if(!m_WindowsShell && m_UseRelativePaths)
        {
        fullLibPath += ";pwd`\"";
        }
      std::string::size_type pos = libDir->find(libPathFlag.c_str());
      if((pos == std::string::npos || pos > 0)
         && libDir->find("${") == std::string::npos)
        {
        linkLibs += libPathFlag;
        if(outputRuntime)
          {
          runtimeDirs.push_back( fullLibPath );
          }
        }
      linkLibs += fullLibPath;
      linkLibs += " ";
      }
    }

  std::string linkSuffix =
    m_Makefile->GetSafeDefinition("CMAKE_LINK_LIBRARY_SUFFIX");
  std::string regexp = ".*\\";
  regexp += linkSuffix;
  regexp += "$";
  cmsys::RegularExpression hasSuffix(regexp.c_str());
  std::string librariesLinked;
  for(std::vector<cmStdString>::iterator lib = linkItems.begin();
      lib != linkItems.end(); ++lib)
    {
    cmStdString& linkItem = *lib;
    // check to see if the link item has a -l already
    cmsys::RegularExpression reg("^([ \t]*\\-[lLWRB])|([ \t]*\\-framework)|(\\${)|([ \t]*\\-pthread)|([ \t]*`)");
    if(!reg.find(linkItem))
      {
      librariesLinked += libLinkFlag;
      }
    librariesLinked += linkItem;
    
    if(linkSuffix.size() && !hasSuffix.find(linkItem))
      {
      librariesLinked += linkSuffix;
      }
    librariesLinked += " ";
    }

  linkLibs += librariesLinked;

  fout << linkLibs;

  if(outputRuntime && runtimeDirs.size()>0)
    {
    // For the runtime search directories, do a "-Wl,-rpath,a:b:c" or
    // a "-R a -R b -R c" type link line
    fout << runtimeFlag;
    std::vector<std::string>::iterator itr = runtimeDirs.begin();
    fout << *itr;
    ++itr;
    for( ; itr != runtimeDirs.end(); ++itr )
      {
      if(runtimeConcatenate)
        {
        fout << runtimeSep << *itr;
        }
      else
        {
        fout << " " << runtimeFlag << *itr;
        }
      }
    fout << " ";
    }
  if(m_Makefile->GetDefinition("CMAKE_STANDARD_LIBRARIES"))
    {
    fout << m_Makefile->GetDefinition("CMAKE_STANDARD_LIBRARIES") << " ";
    }
}


//----------------------------------------------------------------------------
void cmLocalGenerator::AddLanguageFlags(std::string& flags,
                                                     const char* lang)
{
  // Add language-specific flags.
  std::string flagsVar = "CMAKE_";
  flagsVar += lang;
  flagsVar += "_FLAGS";
  this->AddConfigVariableFlags(flags, flagsVar.c_str());
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
    this->AppendFlags(flags, m_Makefile->GetDefinition(flagsVar.c_str()));
    }

  // Add flags specific to shared builds.
  if(cmSystemTools::IsOn(m_Makefile->GetDefinition("BUILD_SHARED_LIBS")))
    {
    flagsVar = "CMAKE_SHARED_BUILD_";
    flagsVar += lang;
    flagsVar += "_FLAGS";
    this->AppendFlags(flags, m_Makefile->GetDefinition(flagsVar.c_str()));
    }
}

//----------------------------------------------------------------------------
void cmLocalGenerator::AddConfigVariableFlags(std::string& flags,
                                                           const char* var)
{
  // Add the flags from the variable itself.
  std::string flagsVar = var;
  this->AppendFlags(flags, m_Makefile->GetDefinition(flagsVar.c_str()));

  // Add the flags from the build-type specific variable.
  const char* buildType = m_Makefile->GetDefinition("CMAKE_BUILD_TYPE");
  if(buildType && *buildType)
    {
    flagsVar += "_";
    flagsVar += cmSystemTools::UpperCase(buildType);
    this->AppendFlags(flags, m_Makefile->GetDefinition(flagsVar.c_str()));
    }
}

//----------------------------------------------------------------------------
void cmLocalGenerator::AppendFlags(std::string& flags,
                                                const char* newFlags)
{
  if(newFlags && *newFlags)
    {
    if(flags.size())
      {
      flags += " ";
      }
    flags += newFlags;
    }
}

//----------------------------------------------------------------------------
std::string
cmLocalGenerator::ConstructScript(const cmCustomCommandLines& commandLines,
                                  const char* newline)
{
  // Store the script in a string.
  std::string script;

  // Write each command on a single line.
  for(cmCustomCommandLines::const_iterator cl = commandLines.begin();
      cl != commandLines.end(); ++cl)
    {
    // Start with the command name.
    const cmCustomCommandLine& commandLine = *cl;
    script += this->Convert(commandLine[0].c_str(),START_OUTPUT,SHELL);

    // Add the arguments.
    for(unsigned int j=1;j < commandLine.size(); ++j)
      {
      script += " ";
      script += cmSystemTools::EscapeSpaces(commandLine[j].c_str());
      }

    // End the line.
    script += newline;
    }
  return script;
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
  // Convert the path to a relative path.
  std::string result = source;

  if (!optional || m_UseRelativePaths)
    {
    switch (relative)
      {
      case HOME:
        //result = cmSystemTools::CollapseFullPath(result.c_str());
        result = m_GlobalGenerator->
          ConvertToRelativePath(m_HomeDirectoryComponents, result.c_str());
        break;
      case START:
        //result = cmSystemTools::CollapseFullPath(result.c_str());
        result = m_GlobalGenerator->
          ConvertToRelativePath(m_StartDirectoryComponents, result.c_str());
        break;
      case HOME_OUTPUT:
        //result = cmSystemTools::CollapseFullPath(result.c_str());
        result = m_GlobalGenerator->
          ConvertToRelativePath(m_HomeOutputDirectoryComponents, result.c_str());
        break;
      case START_OUTPUT:
        //result = cmSystemTools::CollapseFullPath(result.c_str());
        result = m_GlobalGenerator->
          ConvertToRelativePath(m_StartOutputDirectoryComponents, result.c_str());
        break;
      case FULL:
        result = cmSystemTools::CollapseFullPath(result.c_str());
        break;
      case NONE:
        break;
      }
    }
  
  // Now convert it to an output path.
  if (output == MAKEFILE || output == SHELL)
    {
    result = cmSystemTools::ConvertToOutputPath(result.c_str());
    }
  
  return result;
}
