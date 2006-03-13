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
#include "cmLocalUnixMakefileGenerator3.h"

#include "cmDepends.h"
#include "cmGeneratedFileStream.h"
#include "cmGlobalUnixMakefileGenerator3.h"
#include "cmMakefile.h"
#include "cmMakefileTargetGenerator.h"
#include "cmSourceFile.h"
#include "cmake.h"

// Include dependency scanners for supported languages.  Only the
// C/C++ scanner is needed for bootstrapping CMake.
#include "cmDependsC.h"
#ifdef CMAKE_BUILD_WITH_CMAKE
# include "cmDependsFortran.h"
# include "cmDependsJava.h"
#endif

#include <memory> // auto_ptr
#include <queue>

//----------------------------------------------------------------------------
cmLocalUnixMakefileGenerator3::cmLocalUnixMakefileGenerator3()
{
  m_SilentNoColon = false;
  m_WindowsShell = false;
  m_IncludeDirective = "include";
  m_MakefileVariableSize = 0;
  m_IgnoreLibPrefix = false;
  m_PassMakeflags = false;
  m_EchoNeedsQuote = true;
  m_DefineWindowsNULL = false;
  m_UnixCD = true;
}

//----------------------------------------------------------------------------
cmLocalUnixMakefileGenerator3::~cmLocalUnixMakefileGenerator3()
{
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::Configure()
{
  // Include the rule file for each object.
  m_HomeRelativeOutputPath = 
    cmSystemTools::RelativePath(m_Makefile->GetHomeOutputDirectory(),
                                m_Makefile->GetStartOutputDirectory());
  if (m_HomeRelativeOutputPath.size())
    {
    m_HomeRelativeOutputPath += "/";
    }
  this->cmLocalGenerator::Configure();
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::Generate()
{
  // Setup our configuration variables for this directory.
  this->ConfigureOutputPaths();

  // Generate the rule files for each target.
  cmTargets& targets = m_Makefile->GetTargets();
  std::string empty;
  for(cmTargets::iterator t = targets.begin(); t != targets.end(); ++t)
    {
    cmMakefileTargetGenerator *tg = 
      cmMakefileTargetGenerator::New(this, t->first, &(t->second));
    if (tg)
      {
      this->TargetGenerators.push_back(tg);
      t->second.TraceVSDependencies(empty, m_Makefile);
      tg->WriteRuleFiles();
      }
    }

  // write the local Makefile
  this->WriteLocalMakefile();
  
  // Write the cmake file with information for this directory.
  this->WriteDirectoryInformationFile();
  
  // delete the makefile target generator objects
  for (std::vector<cmMakefileTargetGenerator *>::iterator mtgIter = 
         this->TargetGenerators.begin();
       mtgIter != this->TargetGenerators.end(); ++mtgIter)
    {
    delete *mtgIter;
    }  
  this->TargetGenerators.clear();
}


//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::ConfigureOutputPaths()
{
  // Format the library and executable output paths.
  if(const char* libOut = m_Makefile->GetDefinition("LIBRARY_OUTPUT_PATH"))
    {
    m_LibraryOutputPath = libOut;
    this->FormatOutputPath(m_LibraryOutputPath, "LIBRARY");
    }
  if(const char* exeOut = m_Makefile->GetDefinition("EXECUTABLE_OUTPUT_PATH"))
    {
    m_ExecutableOutputPath = exeOut;
    this->FormatOutputPath(m_ExecutableOutputPath, "EXECUTABLE");
    }

  // Store the configuration name that will be generated.
  if(const char* config = m_Makefile->GetDefinition("CMAKE_BUILD_TYPE"))
    {
    // Use the build type given by the user.
    m_ConfigurationName = config;
    }
  else
    {
    // No configuration type given.
    m_ConfigurationName = "";
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::FormatOutputPath(std::string& path,
                                                     const char* name)
{
  if(!path.empty())
    {
    // Convert the output path to a full path in case it is
    // specified as a relative path.  Treat a relative path as
    // relative to the current output directory for this makefile.
    path =
      cmSystemTools::CollapseFullPath(path.c_str(),
                                      m_Makefile->GetStartOutputDirectory());

    // Add a trailing slash for easy appending later.
    if(path.empty() || path[path.size()-1] != '/')
      {
      path += "/";
      }

    // Make sure the output path exists on disk.
    if(!cmSystemTools::MakeDirectory(path.c_str()))
      {
      cmSystemTools::Error("Error failed to create ",
                           name, "_OUTPUT_PATH directory:", path.c_str());
      }

    // Add this as a link directory automatically.
    m_Makefile->AddLinkDirectory(path.c_str());
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteLocalMakefile()
{
  // generate the includes
  std::string ruleFileName = "Makefile";

  // Open the rule file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  std::string ruleFileNameFull = this->ConvertToFullPath(ruleFileName);
  cmGeneratedFileStream ruleFileStream(ruleFileNameFull.c_str());
  if(!ruleFileStream)
    {
    return;
    }
  // always write the top makefile
  if (m_Parent)
    {
    ruleFileStream.SetCopyIfDifferent(true);
    }
  
  // write the all rules
  this->WriteLocalAllRules(ruleFileStream);
  
  // only write local targets unless at the top Keep track of targets already
  // listed.
  std::set<cmStdString> emittedTargets;
  if (m_Parent)
    {
    // write our targets, and while doing it collect up the object
    // file rules
    this->WriteLocalMakefileTargets(ruleFileStream,emittedTargets);
    }
  else
    {
    cmGlobalUnixMakefileGenerator3 *gg = 
      static_cast<cmGlobalUnixMakefileGenerator3*>(m_GlobalGenerator);
    gg->WriteConvenienceRules(ruleFileStream,emittedTargets);
    }
  
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // now write out the object rules
  // for each object file name
  for (std::map<cmStdString,std::vector<cmTarget *> >::iterator lo = 
         m_LocalObjectFiles.begin();
       lo != m_LocalObjectFiles.end(); ++lo)
    {
    commands.clear();
    // for each target using the object file
    for (std::vector<cmTarget *>::iterator to = 
           lo->second.begin(); to != lo->second.end(); ++to)
      {
      std::string tgtMakefileName = this->GetRelativeTargetDirectory(**to);
      std::string targetName = tgtMakefileName;
      tgtMakefileName += "/build.make";
      targetName += "/";
      targetName += lo->first.c_str();
      commands.push_back(this->GetRecursiveMakeCall
                         (tgtMakefileName.c_str(),targetName.c_str()));  
      this->CreateCDCommand(commands,
                                    m_Makefile->GetHomeOutputDirectory(),
                                    m_Makefile->GetStartOutputDirectory());
      }
    this->WriteMakeRule(ruleFileStream, 
                        "target for object file", 
                        lo->first.c_str(), depends, commands, false);
    }

  // add a help target as long as there isn;t a real target named help
  if(emittedTargets.insert("help").second)
    {
    cmGlobalUnixMakefileGenerator3 *gg = 
      static_cast<cmGlobalUnixMakefileGenerator3*>(m_GlobalGenerator);
    gg->WriteHelpRule(ruleFileStream,this);
    }

  this->WriteSpecialTargetsBottom(ruleFileStream);
}

void cmLocalUnixMakefileGenerator3
::WriteLocalMakefileTargets(std::ostream& ruleFileStream,
                            std::set<cmStdString> &emitted)
{
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // for each target we just provide a rule to cd up to the top and do a make
  // on the target
  cmTargets& targets = m_Makefile->GetTargets();
  std::string localName;
  for(cmTargets::iterator t = targets.begin(); t != targets.end(); ++t)
    {
    if((t->second.GetType() == cmTarget::EXECUTABLE) ||
       (t->second.GetType() == cmTarget::STATIC_LIBRARY) ||
       (t->second.GetType() == cmTarget::SHARED_LIBRARY) ||
       (t->second.GetType() == cmTarget::MODULE_LIBRARY) ||
       (t->second.GetType() == cmTarget::UTILITY))
      {
      emitted.insert(t->second.GetName());

      // for subdirs add a rule to build this specific target by name.
      localName = this->GetRelativeTargetDirectory(t->second);
      localName += "/rule";
      commands.clear();
      depends.clear();
      
      // Build the target for this pass.
      commands.push_back(this->GetRecursiveMakeCall
                         ("CMakeFiles/Makefile2",localName.c_str()));
      this->CreateCDCommand(commands,
                            m_Makefile->GetHomeOutputDirectory(),
                            m_Makefile->GetStartOutputDirectory());
      this->WriteMakeRule(ruleFileStream, "Convenience name for target.",
                          localName.c_str(), depends, commands, true);
      
      // Add a target with the canonical name (no prefix, suffix or path).
      if(localName != t->second.GetName())
        {
        commands.clear();
        depends.push_back(localName);
        this->WriteMakeRule(ruleFileStream, "Convenience name for target.",
                            t->second.GetName(), depends, commands, true);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteDirectoryInformationFile()
{
  std::string infoFileName = m_Makefile->GetStartOutputDirectory();
  infoFileName += "/CMakeFiles/CMakeDirectoryInformation.cmake";

  // Open the output file.
  cmGeneratedFileStream infoFileStream(infoFileName.c_str());
  if(!infoFileStream)
    {
    return;
    }

  // Write the do not edit header.
  this->WriteDisclaimer(infoFileStream);

  // Tell the dependency scanner to use unix paths if necessary.
  if(cmSystemTools::GetForceUnixPaths())
    {
    infoFileStream
      << "# Force unix paths in dependencies.\n"
      << "SET(CMAKE_FORCE_UNIX_PATHS 1)\n"
      << "\n";
    }

  // Store the include search path for this directory.
  infoFileStream
    << "# The C and CXX include file search paths:\n";
  infoFileStream
    << "SET(CMAKE_C_INCLUDE_PATH\n";
  std::vector<std::string> includeDirs;
  this->GetIncludeDirectories(includeDirs);
  for(std::vector<std::string>::iterator i = includeDirs.begin();
      i != includeDirs.end(); ++i)
    {
    // Note: This path conversion must match that used for
    // CMAKE_GENERATED_FILES so that the file names match.
    infoFileStream
      << "  \"" << this->Convert(i->c_str(),HOME_OUTPUT).c_str() << "\"\n";
    }
  infoFileStream
    << "  )\n";
  infoFileStream
    << "SET(CMAKE_CXX_INCLUDE_PATH ${CMAKE_C_INCLUDE_PATH})\n";

  // Store the include regular expressions for this directory.
  infoFileStream
    << "\n"
    << "# The C and CXX include file regular expressions for this directory.\n";
  infoFileStream
    << "SET(CMAKE_C_INCLUDE_REGEX_SCAN ";
  this->WriteCMakeArgument(infoFileStream,
                           m_Makefile->GetIncludeRegularExpression());
  infoFileStream
    << ")\n";
  infoFileStream
    << "SET(CMAKE_C_INCLUDE_REGEX_COMPLAIN ";
  this->WriteCMakeArgument(infoFileStream,
                           m_Makefile->GetComplainRegularExpression());
  infoFileStream
    << ")\n";
  infoFileStream
    << "SET(CMAKE_CXX_INCLUDE_REGEX_SCAN ${CMAKE_C_INCLUDE_REGEX_SCAN})\n";
  infoFileStream
    << "SET(CMAKE_CXX_INCLUDE_REGEX_COMPLAIN ${CMAKE_C_INCLUDE_REGEX_COMPLAIN})\n";

  // Store the set of available generated files.
  infoFileStream
    << "\n"
    << "# The set of files generated by rules in this directory:\n";
  infoFileStream
    << "SET(CMAKE_GENERATED_FILES\n";
  for(std::vector<cmSourceFile*>::const_iterator
        i = m_Makefile->GetSourceFiles().begin();
      i != m_Makefile->GetSourceFiles().end(); ++i)
    {
    cmSourceFile* src = *i;
    if(src->GetPropertyAsBool("GENERATED"))
      {
      // Note: This path conversion must match that used for
      // CMAKE_C_INCLUDE_PATH so that the file names match.
      infoFileStream
        << "  \""
        << this->Convert(src->GetFullPath().c_str(), HOME_OUTPUT)
        << "\"\n";
      }
    }
  infoFileStream
    << ")\n";
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::ConvertToFullPath(const std::string& localPath)
{
  std::string dir = m_Makefile->GetStartOutputDirectory();
  dir += "/";
  dir += localPath;
  return dir;
}


const std::string &cmLocalUnixMakefileGenerator3::GetHomeRelativeOutputPath()
{
  return m_HomeRelativeOutputPath;
}


//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteMakeRule(std::ostream& os,
                const char* comment,
                const char* target,
                const std::vector<std::string>& depends,
                const std::vector<std::string>& commands,
                bool symbolic)
{
  // Make sure there is a target.
  if(!target || !*target)
    {
    cmSystemTools::Error("No target for WriteMakeRule! called with comment: ",
                         comment);
    return;
    }

  std::string replace;

  // Write the comment describing the rule in the makefile.
  if(comment)
    {
    replace = comment;
    std::string::size_type lpos = 0;
    std::string::size_type rpos;
    while((rpos = replace.find('\n', lpos)) != std::string::npos)
      {
      os << "# " << replace.substr(lpos, rpos-lpos) << "\n";
      lpos = rpos+1;
      }
    os << "# " << replace.substr(lpos) << "\n";
    }

  // Construct the left hand side of the rule.
  replace = target;
  std::string tgt = this->Convert(replace.c_str(),HOME_OUTPUT,MAKEFILE);
  tgt = this->ConvertToMakeTarget(tgt.c_str());
  const char* space = "";
  if(tgt.size() == 1)
    {
    // Add a space before the ":" to avoid drive letter confusion on
    // Windows.
    space = " ";
    }

  // Mark the rule as symbolic if requested.
  if(symbolic)
    {
    if(const char* sym =
       m_Makefile->GetDefinition("CMAKE_MAKE_SYMBOLIC_RULE"))
      {
      os << tgt.c_str() << space << ": " << sym << "\n";
      }
    }

  // Write the rule.
  if(depends.empty())
    {
    // No dependencies.  The commands will always run.
    os << tgt.c_str() << space << ":\n";
    }
  else
    {
    // Split dependencies into multiple rule lines.  This allows for
    // very long dependency lists even on older make implementations.
    for(std::vector<std::string>::const_iterator dep = depends.begin();
        dep != depends.end(); ++dep)
      {
      replace = *dep;
      replace = this->Convert(replace.c_str(),HOME_OUTPUT,MAKEFILE);
      replace = this->ConvertToMakeTarget(replace.c_str());
      os << tgt.c_str() << space << ": " << replace.c_str() << "\n";
      }
    }

  // Write the list of commands.
  for(std::vector<std::string>::const_iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    replace = *i;
    os << "\t" << replace.c_str() << "\n";
    }
  os << "\n";
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteMakeVariables(std::ostream& makefileStream)
{
  this->WriteDivider(makefileStream);
  makefileStream
    << "# Set environment variables for the build.\n"
    << "\n";
  if(m_DefineWindowsNULL)
    {
    makefileStream
      << "!IF \"$(OS)\" == \"Windows_NT\"\n"
      << "NULL=\n"
      << "!ELSE\n"
      << "NULL=nul\n"
      << "!ENDIF\n";
    }
  if(m_WindowsShell)
    {
     makefileStream
       << "SHELL = C:\\WINDOWS\\system32\\cmd.exe\n";
    }
  else
    {
      makefileStream
        << "# The shell in which to execute make rules.\n"
        << "SHELL = /bin/sh\n"
        << "\n";
    }

  std::string cmakecommand =
      m_Makefile->GetRequiredDefinition("CMAKE_COMMAND");
  makefileStream
    << "# The CMake executable.\n"
    << "CMAKE_COMMAND = "
    << this->Convert(cmakecommand.c_str(), FULL, SHELL).c_str() 
    << "\n"
    << "\n";
  makefileStream
    << "# The command to remove a file.\n"
    << "RM = "
    << this->Convert(cmakecommand.c_str(),FULL,SHELL).c_str()
    << " -E remove -f\n"
    << "\n";
  
  if(m_Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))
    {
    makefileStream
      << "# The program to use to edit the cache.\n"
      << "CMAKE_EDIT_COMMAND = "
      << (this->ConvertToOutputForExisting(
            m_Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))) << "\n"
      << "\n";
    }

  makefileStream
    << "# The top-level source directory on which CMake was run.\n"
    << "CMAKE_SOURCE_DIR = "
    << this->Convert(m_Makefile->GetHomeDirectory(), FULL, SHELL)
    << "\n"
    << "\n";
  makefileStream
    << "# The top-level build directory on which CMake was run.\n"
    << "CMAKE_BINARY_DIR = "
    << this->Convert(m_Makefile->GetHomeOutputDirectory(), FULL, SHELL)
    << "\n"
    << "\n";
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteSpecialTargetsTop(std::ostream& makefileStream)
{
  this->WriteDivider(makefileStream);
  makefileStream
    << "# Special targets provided by cmake.\n"
    << "\n";

  // Write special target to silence make output.  This must be after
  // the default target in case VERBOSE is set (which changes the
  // name).  The setting of CMAKE_VERBOSE_MAKEFILE to ON will cause a
  // "VERBOSE=1" to be added as a make variable which will change the
  // name of this special target.  This gives a make-time choice to
  // the user.
  std::vector<std::string> commands;
  std::vector<std::string> no_depends;
  commands.clear();
  if(m_Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"))
    {
    makefileStream
      << "# Produce verbose output by default.\n"
      << "VERBOSE = 1\n"
      << "\n";
    }
  if(m_SilentNoColon)
    {
    makefileStream << "$(VERBOSE).SILENT\n";
    }
  else
    {
    this->WriteMakeRule(makefileStream,
                        "Suppress display of executed commands.",
                        "$(VERBOSE).SILENT",
                        no_depends,
                        commands, false);
    }

  // Special target to cleanup operation of make tool.
  std::vector<std::string> depends;
  this->WriteMakeRule(makefileStream,
                      "Disable implicit rules so canoncical targets will work.",
                      ".SUFFIXES",
                      depends,
                      commands, false);
  // Add a fake suffix to keep HP happy.  Must be max 32 chars for SGI make.
  depends.push_back(".hpux_make_needs_suffix_list");
  this->WriteMakeRule(makefileStream, 0,
                      ".SUFFIXES", depends, commands, false);

}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteSpecialTargetsBottom(std::ostream& makefileStream)
{
  this->WriteDivider(makefileStream);
  makefileStream
    << "# Special targets to cleanup operation of make.\n"
    << "\n";

  // Write special "cmake_check_build_system" target to run cmake with
  // the --check-build-system flag.
  {
  // Build command to run CMake to check if anything needs regenerating.
  std::string cmakefileName = "CMakeFiles/Makefile.cmake";
  std::string runRule =
    "$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)";
  runRule += " --check-build-system ";
  runRule += this->Convert(cmakefileName.c_str(),NONE,SHELL);
  runRule += " 0";

  std::vector<std::string> no_depends;
  std::vector<std::string> commands;
  commands.push_back(runRule);
  if(m_Parent)
    {
    this->CreateCDCommand(commands,
                          m_Makefile->GetHomeOutputDirectory(),
                          m_Makefile->GetStartOutputDirectory());
    }
  this->WriteMakeRule(makefileStream,
                      "Special rule to run CMake to check the build system "
                      "integrity.\n"
                      "No rule that depends on this can have "
                      "commands that come from listfiles\n"
                      "because they might be regenerated.",
                      "cmake_check_build_system",
                      no_depends,
                      commands, true);
  }

  std::vector<std::string> no_commands;

}



//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteConvenienceRule(std::ostream& ruleFileStream,
                       const char* realTarget,
                       const char* helpTarget)
{
  // A rule is only needed if the names are different.
  if(strcmp(realTarget, helpTarget) != 0)
    {
    // The helper target depends on the real target.
    std::vector<std::string> depends;
    depends.push_back(realTarget);

    // There are no commands.
    std::vector<std::string> no_commands;

    // Write the rule.
    this->WriteMakeRule(ruleFileStream, "Convenience name for target.",
                        helpTarget, depends, no_commands, true);
    }
}


//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3::GetRelativeTargetDirectory(cmTarget& target)
{
  std::string dir = m_Makefile->GetStartOutputDirectory();
  dir += "/";
  dir += this->GetTargetDirectory(target);
  dir = cmSystemTools::RelativePath(m_Makefile->GetHomeOutputDirectory(), dir.c_str());
  return this->Convert(dir.c_str(),NONE,MAKEFILE);
}



//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendRuleDepend(std::vector<std::string>& depends,
                   const char* ruleFileName)
{
  // Add a dependency on the rule file itself unless an option to skip
  // it is specifically enabled by the user or project.
  const char* nodep = m_Makefile->GetDefinition("CMAKE_SKIP_RULE_DEPENDENCY");
  if(!nodep || cmSystemTools::IsOff(nodep))
    {
    depends.push_back(ruleFileName);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCustomDepends(std::vector<std::string>& depends,
                      const std::vector<cmCustomCommand>& ccs)
{
  for(std::vector<cmCustomCommand>::const_iterator i = ccs.begin();
      i != ccs.end(); ++i)
    {
    this->AppendCustomDepend(depends, *i);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCustomDepend(std::vector<std::string>& depends,
                     const cmCustomCommand& cc)
{
  for(std::vector<std::string>::const_iterator d = cc.GetDepends().begin();
      d != cc.GetDepends().end(); ++d)
    {
    // Lookup the real name of the dependency in case it is a CMake target.
    std::string dep = this->GetRealDependency(d->c_str(),
                                              m_ConfigurationName.c_str());
    depends.push_back(dep);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCustomCommands(std::vector<std::string>& commands,
                       const std::vector<cmCustomCommand>& ccs)
{
  for(std::vector<cmCustomCommand>::const_iterator i = ccs.begin();
      i != ccs.end(); ++i)
    {
    this->AppendCustomCommand(commands, *i);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCustomCommand(std::vector<std::string>& commands,
                      const cmCustomCommand& cc)
{
  std::vector<std::string> commands1;

  // Add each command line to the set of commands.
  for(cmCustomCommandLines::const_iterator cl = cc.GetCommandLines().begin();
      cl != cc.GetCommandLines().end(); ++cl)
    {
    // Build the command line in a single string.
    const cmCustomCommandLine& commandLine = *cl;
    std::string cmd = commandLine[0];
    if (cmd.size())
      {
      cmSystemTools::ReplaceString(cmd, "/./", "/");
      cmd = this->Convert(cmd.c_str(),START_OUTPUT);
      if(cmd.find("/") == cmd.npos &&
         commandLine[0].find("/") != cmd.npos)
        {
        // Add a leading "./" for executables in the current directory.
        cmd = "./" + cmd;
        }
      cmd = this->Convert(cmd.c_str(),NONE,SHELL);
      for(unsigned int j=1; j < commandLine.size(); ++j)
        {
        cmd += " ";
        bool forceOn =  cmSystemTools::GetForceUnixPaths();
        if(forceOn && m_WindowsShell)
          {
          cmSystemTools::SetForceUnixPaths(false);
          }
        cmd += cmSystemTools::EscapeSpaces(commandLine[j].c_str());
        if(forceOn && m_WindowsShell)
          {
          cmSystemTools::SetForceUnixPaths(true);
          }
        }
      commands1.push_back(cmd);
      }
    }

  // push back the custom commands
  const char* dir  = m_Makefile->GetStartOutputDirectory();
  // if the command specified a working directory use it.
  if(cc.GetWorkingDirectory())
    {
    dir = cc.GetWorkingDirectory();
    }
  this->CreateCDCommand(commands1, dir,
                        m_Makefile->GetHomeOutputDirectory());

  commands.insert(commands.end(), commands1.begin(), commands1.end());
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCleanCommand(std::vector<std::string>& commands,
                     const std::vector<std::string>& files,
                     cmTarget& target, const char* filename)
{
  if(!files.empty())
    {
    std::string cleanfile = m_Makefile->GetCurrentOutputDirectory();
    cleanfile += "/";
    cleanfile += this->GetTargetDirectory(target);
    cleanfile += "/cmake_clean";
    if(filename)
      {
      cleanfile += "_";
      cleanfile += filename;
      }
    cleanfile += ".cmake";
    std::string cleanfilePath = this->Convert(cleanfile.c_str(), FULL);
    std::ofstream fout(cleanfilePath.c_str());
    if(!fout)
      {
      cmSystemTools::Error("Could not create ", cleanfilePath.c_str());
      }
    fout << "FILE(REMOVE\n";
    std::string remove = "$(CMAKE_COMMAND) -P ";
    remove += this->Convert(cleanfile.c_str(), START_OUTPUT, SHELL);
    for(std::vector<std::string>::const_iterator f = files.begin();
        f != files.end(); ++f)
      {
      fout << "\"" << this->Convert(f->c_str(),START_OUTPUT,UNCHANGED) 
           << "\"\n";
      }
    fout << ")\n";
    commands.push_back(remove);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3::AppendEcho(std::vector<std::string>& commands,
                                          const char* text)
{
  // Echo one line at a time.
  std::string line;
  for(const char* c = text;; ++c)
    {
    if(*c == '\n' || *c == '\0')
      {
      // Avoid writing a blank last line on end-of-string.
      if(*c != '\0' || !line.empty())
        {
        // Add a command to echo this line.
        std::string cmd = "@echo ";
        if(m_EchoNeedsQuote)
          {
          cmd += "\"";
          }
        cmd += line;
        if(m_EchoNeedsQuote)
          {
          cmd += "\"";
          }
        commands.push_back(cmd);
        }

      // Reset the line to emtpy.
      line = "";

      // Terminate on end-of-string.
      if(*c == '\0')
        {
        return;
        }
      }
    else if(*c != '\r')
      {
      // Append this character to the current line.
      line += *c;
      }
    }
}

//----------------------------------------------------------------------------
// take a tgt path and convert it into a make target, it could be full, or relative
std::string
cmLocalUnixMakefileGenerator3
::ConvertToMakeTarget(const char* tgt)
{
  // Make targets should not have a leading './' for a file in the
  // directory containing the makefile.
  std::string ret = tgt;
  if(ret.size() > 2 && (ret[0] == '.') &&
     ( (ret[1] == '/') || ret[1] == '\\'))
    {
    std::string upath = ret;
    cmSystemTools::ConvertToUnixSlashes(upath);
    if(upath.find(2, '/') == upath.npos)
      {
      ret = ret.substr(2, ret.size()-2);
      }
    }
  return ret;
}

//----------------------------------------------------------------------------
std::string&
cmLocalUnixMakefileGenerator3::CreateSafeUniqueObjectFileName(const char* sin)
{
  // Look for an existing mapped name for this object file.
  std::map<cmStdString,cmStdString>::iterator it =
    m_UniqueObjectNamesMap.find(sin);

  // If no entry exists create one.
  if(it == m_UniqueObjectNamesMap.end())
    {
    // Start with the original name.
    std::string ssin = sin;

    // Avoid relative paths that go up the tree.
    cmSystemTools::ReplaceString(ssin, "../", "__/");

    // Mangle the name if necessary.
    if(m_Makefile->IsOn("CMAKE_MANGLE_OBJECT_FILE_NAMES"))
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
        for ( it = m_UniqueObjectNamesMap.begin();
              it != m_UniqueObjectNamesMap.end();
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

    // Insert the newly mapped object file name.
    std::map<cmStdString, cmStdString>::value_type e(sin, ssin);
    it = m_UniqueObjectNamesMap.insert(e).first;
    }

  // Return the map entry.
  return it->second;
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::CreateMakeVariable(const char* sin, const char* s2in)
{
  std::string s = sin;
  std::string s2 = s2in;
  std::string unmodified = s;
  unmodified += s2;
  // if there is no restriction on the length of make variables
  // and there are no "." charactors in the string, then return the
  // unmodified combination.
  if((!m_MakefileVariableSize && unmodified.find('.') == s.npos)
     && (!m_MakefileVariableSize && unmodified.find('-') == s.npos))
    {
    return unmodified;
    }

  // see if the variable has been defined before and return
  // the modified version of the variable
  std::map<cmStdString, cmStdString>::iterator i = m_MakeVariableMap.find(unmodified);
  if(i != m_MakeVariableMap.end())
    {
    return i->second;
    }
  // start with the unmodified variable
  std::string ret = unmodified;
  // if this there is no value for m_MakefileVariableSize then
  // the string must have bad characters in it
  if(!m_MakefileVariableSize)
    {
    cmSystemTools::ReplaceString(ret, ".", "_");
    cmSystemTools::ReplaceString(ret, "-", "__");
    int ni = 0;
    char buffer[5];
    // make sure the _ version is not already used, if
    // it is used then add number to the end of the variable
    while(m_ShortMakeVariableMap.count(ret) && ni < 1000)
      {
      ++ni;
      sprintf(buffer, "%04d", ni);
      ret = unmodified + buffer;
      }
    m_ShortMakeVariableMap[ret] = "1";
    m_MakeVariableMap[unmodified] = ret;
    return ret;
    }

  // if the string is greater the 32 chars it is an invalid vairable name
  // for borland make
  if(static_cast<int>(ret.size()) > m_MakefileVariableSize)
    {
    int keep = m_MakefileVariableSize - 8;
    int size = keep + 3;
    std::string str1 = s;
    std::string str2 = s2;
    // we must shorten the combined string by 4 charactors
    // keep no more than 24 charactors from the second string
    if(static_cast<int>(str2.size()) > keep)
      {
      str2 = str2.substr(0, keep);
      }
    if(static_cast<int>(str1.size()) + static_cast<int>(str2.size()) > size)
      {
      str1 = str1.substr(0, size - str2.size());
      }
    char buffer[5];
    int ni = 0;
    sprintf(buffer, "%04d", ni);
    ret = str1 + str2 + buffer;
    while(m_ShortMakeVariableMap.count(ret) && ni < 1000)
      {
      ++ni;
      sprintf(buffer, "%04d", ni);
      ret = str1 + str2 + buffer;
      }
    if(ni == 1000)
      {
      cmSystemTools::Error("Borland makefile variable length too long");
      return unmodified;
      }
    // once an unused variable is found
    m_ShortMakeVariableMap[ret] = "1";
    }
  // always make an entry into the unmodified to variable map
  m_MakeVariableMap[unmodified] = ret;
  return ret;
}

//----------------------------------------------------------------------------
bool
cmLocalUnixMakefileGenerator3
::ScanDependencies(std::vector<std::string> const& args)
{
  // Format of arguments is:
  // $(CMAKE_COMMAND), cmake_depends, GeneratorName, home_output_dir, start_output_dir, info file
  // The caller has ensured that all required arguments exist.

  // The info file for this target
  std::string const& infoFile = args[5];

  // Read the directory information file.
  cmake cm;
  cmGlobalGenerator gg;
  gg.SetCMakeInstance(&cm);
  std::auto_ptr<cmLocalGenerator> lg(gg.CreateLocalGenerator());
  lg->SetGlobalGenerator(&gg);
  cmMakefile* mf = lg->GetMakefile();
  mf->SetHomeOutputDirectory(args[3].c_str());
  mf->SetStartOutputDirectory(args[4].c_str());
  lg->SetupPathConversions();
  
  bool haveDirectoryInfo = false;
  std::string dirInfoFile = args[4];
  dirInfoFile += "/CMakeFiles/CMakeDirectoryInformation.cmake";
  if(mf->ReadListFile(0, dirInfoFile.c_str()) &&
     !cmSystemTools::GetErrorOccuredFlag())
    {
    haveDirectoryInfo = true;
    }
  
  // read in the target info file
  if(!mf->ReadListFile(0, infoFile.c_str()) ||
     cmSystemTools::GetErrorOccuredFlag())
    {
    cmSystemTools::Error("Target DependInfo.cmake file not found");    
    }
  
  // Test whether we need to force Unix paths.
  if(haveDirectoryInfo)
    {
    if(const char* force = mf->GetDefinition("CMAKE_FORCE_UNIX_PATHS"))
      {
      if(!cmSystemTools::IsOff(force))
        {
        cmSystemTools::SetForceUnixPaths(true);
        }
      }
    }
  else
    {
    cmSystemTools::Error("Directory Information file not found");
    }

  // create the file stream for the depends file
  std::string dir = cmSystemTools::GetFilenamePath(infoFile);
  
  // Open the rule file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  std::string ruleFileNameFull = dir;
  ruleFileNameFull += "/depend.make";
  cmGeneratedFileStream ruleFileStream(ruleFileNameFull.c_str());
  ruleFileStream.SetCopyIfDifferent(true);
  if(!ruleFileStream)
    {
    return false;
    }
  std::string internalRuleFileNameFull = dir;
  internalRuleFileNameFull += "/depend.internal";
  cmGeneratedFileStream internalRuleFileStream(internalRuleFileNameFull.c_str());
  internalRuleFileStream.SetCopyIfDifferent(true);
  if(!internalRuleFileStream)
    {
    return false;
    }

  this->WriteDisclaimer(ruleFileStream);
  this->WriteDisclaimer(internalRuleFileStream);

  // Get the set of generated files.
  std::vector<std::string> generatedFilesVec;
  if(haveDirectoryInfo)
    {
    if(const char* generated = mf->GetDefinition("CMAKE_GENERATED_FILES"))
      {
      cmSystemTools::ExpandListArgument(generated, generatedFilesVec);
      }
    }

  // Sort for efficient lookup.
  std::set<cmStdString> generatedFiles;
  for(std::vector<std::string>::iterator gfi = generatedFilesVec.begin();
      gfi != generatedFilesVec.end(); ++gfi)
    {
    generatedFiles.insert(*gfi);
    }

  // for each language we need to scan, scan it 
  const char *langStr = mf->GetSafeDefinition("CMAKE_DEPENDS_LANGUAGES");
  std::vector<std::string> langs;
  cmSystemTools::ExpandListArgument(langStr, langs);
  for (std::vector<std::string>::iterator li = 
         langs.begin(); li != langs.end(); ++li)
    {
    // construct the checker
    std::string lang = li->c_str();
    
    // Get the set of include directories.
    std::vector<std::string> includes;
    if(haveDirectoryInfo)
      {
      std::string includePathVar = "CMAKE_";
      includePathVar += lang;
      includePathVar += "_INCLUDE_PATH";
      if(const char* includePath = mf->GetDefinition(includePathVar.c_str()))
        {
        cmSystemTools::ExpandListArgument(includePath, includes);
        }
      }
    
    // Get the include file regular expression.
    std::string includeRegexScan = "^.*$";
    std::string includeRegexComplain = "^$";
    if(haveDirectoryInfo)
      {
      std::string scanRegexVar = "CMAKE_";
      scanRegexVar += lang;
      scanRegexVar += "_INCLUDE_REGEX_SCAN";
      if(const char* scanRegex = mf->GetDefinition(scanRegexVar.c_str()))
        {
        includeRegexScan = scanRegex;
        }
      std::string complainRegexVar = "CMAKE_";
      complainRegexVar += lang;
      complainRegexVar += "_INCLUDE_REGEX_COMPLAIN";
      if(const char* complainRegex = mf->GetDefinition(complainRegexVar.c_str()))
        {
        includeRegexComplain = complainRegex;
        }
      }

    // Create the scanner for this language
    cmDepends *scanner = 0;
    if(lang == "C" || lang == "CXX" || lang == "RC")
      {
      std::string includeCacheFileName = dir;
      includeCacheFileName += "/includecache.";
      includeCacheFileName += lang;
      
      // TODO: Handle RC (resource files) dependencies correctly.
      scanner = new cmDependsC(includes,
                               includeRegexScan.c_str(),
                               includeRegexComplain.c_str(),
                               generatedFiles, includeCacheFileName);
      }
#ifdef CMAKE_BUILD_WITH_CMAKE
    else if(lang == "Fortran")
      {
      scanner = new cmDependsFortran(includes);
      }
    else if(lang == "Java")
      {
      scanner = new cmDependsJava();
      }
#endif
    
    if (scanner)
      {
      scanner->SetFileComparison(m_GlobalGenerator->GetCMakeInstance()->GetFileComparison());
      // for each file we need to scan
      std::string srcLang = "CMAKE_DEPENDS_CHECK_";
      srcLang += lang;
      const char *srcStr = mf->GetSafeDefinition(srcLang.c_str());
      std::vector<std::string> srcs;
      cmSystemTools::ExpandListArgument(srcStr, srcs);
      for (std::vector<std::string>::iterator si = 
        srcs.begin(); si != srcs.end(); ++si)
        {
        std::string &src = *si;
        ++si;
        // make sure the object file is relative to home output
        std::string obj = *si;
        obj = lg->Convert(obj.c_str(),HOME_OUTPUT,MAKEFILE);
        scanner->Write(src.c_str(),obj.c_str(),ruleFileStream, internalRuleFileStream);
        }

      // free the scanner for this language
      delete scanner;
      }
    }

  // dependencies were generated, so touch the mark file
  ruleFileNameFull += ".mark";
  std::ofstream fmark(ruleFileNameFull.c_str());
  fmark << "Dependencies updated>" << std::endl;
  
  return true;
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3
::WriteLocalAllRules(std::ostream& ruleFileStream)
{
  this->WriteDisclaimer(ruleFileStream);
  this->WriteMakeVariables(ruleFileStream);

  // Write the main entry point target.  This must be the VERY first
  // target so that make with no arguments will run it.
  {
  // Just depend on the all target to drive the build.
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;
  depends.push_back("all");

  // Write the rule.
  this->WriteMakeRule(ruleFileStream,
                      "Default target executed when no arguments are "
                      "given to make.",
                      "default_target",
                      depends,
                      no_commands, true);
  }

  // Write all global targets
  cmTargets* targets = &(m_Makefile->GetTargets());
  cmTargets::iterator glIt;
  for ( glIt = targets->begin(); glIt != targets->end(); ++ glIt )
    {
    if ( glIt->second.GetType() == cmTarget::GLOBAL_TARGET )
      {
      std::string targetString = "Special rule for the target " + glIt->first;
      std::vector<std::string> commands;
      std::vector<std::string> depends;

      const char* text = glIt->second.GetProperty("EchoString");
      if ( !text )
        {
        text = "Running external command ...";
        }
      std::set<cmStdString>::const_iterator dit;
      for ( dit = glIt->second.GetUtilities().begin();
         dit != glIt->second.GetUtilities().end();
        ++ dit )
        {
        depends.push_back(dit->c_str());
        }
      this->AppendEcho(commands, text);

      // Utility targets store their rules in pre- and post-build commands.
      this->AppendCustomDepends(depends,   glIt->second.GetPreBuildCommands());
      this->AppendCustomDepends(depends,   glIt->second.GetPostBuildCommands());
      this->AppendCustomCommands(commands, glIt->second.GetPreBuildCommands());
      this->AppendCustomCommands(commands, glIt->second.GetPostBuildCommands());
      this->WriteMakeRule(ruleFileStream, targetString.c_str(), glIt->first.c_str(), depends, commands, true);
      }
    }

  this->WriteSpecialTargetsTop(ruleFileStream);

  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // Write the all rule.
  std::string dir = m_Makefile->GetStartOutputDirectory();
  dir += "/all";
  dir = this->Convert(dir.c_str(),HOME_OUTPUT,MAKEFILE);
  depends.push_back("cmake_check_build_system");
  commands.push_back
    (this->GetRecursiveMakeCall("CMakeFiles/Makefile2",dir.c_str()));  
  this->CreateCDCommand(commands,
                                m_Makefile->GetHomeOutputDirectory(),
                                m_Makefile->GetStartOutputDirectory());
  this->WriteMakeRule(ruleFileStream, "The main all target", "all",
                      depends, commands, true);

  // Write the clean rule.
  dir = m_Makefile->GetStartOutputDirectory();
  dir += "/clean";
  dir = this->Convert(dir.c_str(),HOME_OUTPUT,MAKEFILE);
  commands.clear();
  depends.clear();
  commands.push_back
    (this->GetRecursiveMakeCall("CMakeFiles/Makefile2",dir.c_str()));  
  this->CreateCDCommand(commands,
                                m_Makefile->GetHomeOutputDirectory(),
                                m_Makefile->GetStartOutputDirectory());
  this->WriteMakeRule(ruleFileStream, "The main clean target", "clean",
                      depends, commands, true);

  // Write the preinstall rule.
  dir = m_Makefile->GetStartOutputDirectory();
  dir += "/preinstall";
  dir = this->Convert(dir.c_str(), HOME_OUTPUT,MAKEFILE);
  commands.clear();
  depends.clear();
  const char* noall =
    m_Makefile->GetDefinition("CMAKE_SKIP_INSTALL_ALL_DEPENDENCY");
  if(!noall || cmSystemTools::IsOff(noall))
    {
    // Drive the build before installing.
    depends.push_back("all");
    }
  else
    {
    // At least make sure the build system is up to date.
    depends.push_back("cmake_check_build_system");
    }
  commands.push_back
    (this->GetRecursiveMakeCall("CMakeFiles/Makefile2", dir.c_str()));
  this->CreateCDCommand(commands,
                        m_Makefile->GetHomeOutputDirectory(),
                        m_Makefile->GetStartOutputDirectory());
  this->WriteMakeRule(ruleFileStream, "Prepare targets for installation.",
                      "preinstall", depends, commands, true);

  // write the depend rule, really a recompute depends rule
  depends.clear();
  commands.clear();
  std::string cmakefileName = "CMakeFiles/Makefile.cmake";
  this->Convert(cmakefileName.c_str(),HOME_OUTPUT,
                cmLocalGenerator::MAKEFILE);  
  std::string runRule =
    "$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)";
  runRule += " --check-build-system ";
  runRule += this->Convert(cmakefileName.c_str(),cmLocalGenerator::NONE,
                           cmLocalGenerator::SHELL);
  runRule += " 1";
  commands.push_back(runRule);
  this->WriteMakeRule(ruleFileStream, "clear depends", 
                      "depend", 
                      depends, commands, true);
}


//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::CheckDependencies(cmMakefile* mf, 
                                                      bool verbose,
                                                      bool clear)
{
  // Get the list of target files to check
  const char* infoDef = mf->GetDefinition("CMAKE_DEPEND_INFO_FILES");
  if(!infoDef)
    {
    return;
    }
  std::vector<std::string> files;
  cmSystemTools::ExpandListArgument(infoDef, files);

  // For each info file run the check
  cmDependsC checker;
  checker.SetVerbose(verbose);
  checker.SetFileComparison(m_GlobalGenerator->GetCMakeInstance()->GetFileComparison());
  for(std::vector<std::string>::iterator l = files.begin();
      l != files.end(); ++l)
    {
    // either clear or check the files
    std::string dir = cmSystemTools::GetFilenamePath(l->c_str());
    std::string internalDependFile = dir + "/depend.internal";
    std::string dependFile = dir + "/depend.make";
    if (clear)
      {
      checker.Clear(internalDependFile.c_str());
      checker.Clear(dependFile.c_str());
      }
    else
      {
      checker.Check(dependFile.c_str(), internalDependFile.c_str());
      }
    }
}


void cmLocalUnixMakefileGenerator3
::WriteDependLanguageInfo(std::ostream& cmakefileStream, cmTarget &target)
{
  // now write all the language stuff
  // Set the set of files to check for dependency integrity.
  std::set<cmStdString> checkSetLangs;
  std::map<cmStdString,cmLocalUnixMakefileGenerator3::IntegrityCheckSet>& 
    checkSet = this->GetIntegrityCheckSet()[target.GetName()];
  for(std::map<cmStdString, 
        cmLocalUnixMakefileGenerator3::IntegrityCheckSet>::const_iterator
        l = checkSet.begin(); l != checkSet.end(); ++l)
    {
    checkSetLangs.insert(l->first);
    }
  
  // list the languages
  cmakefileStream
    << "# The set of files whose dependency integrity should be checked:\n";
  cmakefileStream
    << "SET(CMAKE_DEPENDS_LANGUAGES\n";
  for(std::set<cmStdString>::iterator
        l = checkSetLangs.begin(); l != checkSetLangs.end(); ++l)
    {
    cmakefileStream << "  \"" << l->c_str() << "\"\n";
    }
  cmakefileStream << "  )\n";
  
  // now list the files for each language
  for(std::set<cmStdString>::iterator
        l = checkSetLangs.begin(); l != checkSetLangs.end(); ++l)
    {
    cmakefileStream
      << "SET(CMAKE_DEPENDS_CHECK_" << l->c_str() << "\n";
    // get the check set for this local gen and language
    cmLocalUnixMakefileGenerator3::IntegrityCheckSet iCheckSet = 
      checkSet[*l];
    // for each file
    for(cmLocalUnixMakefileGenerator3::IntegrityCheckSet::const_iterator 
          csIter = iCheckSet.begin();
        csIter != iCheckSet.end(); ++csIter)
      {
      cmakefileStream << "  \"" << (*csIter)->GetFullPath() << "\"\n";
      // Get the full path name of the object file.
      std::string obj = m_Makefile->GetStartOutputDirectory();
      obj += "/";
      obj += this->GetObjectFileName(target, **csIter);
      cmakefileStream << "  \"" << 
        this->Convert(obj.c_str(),
                      cmLocalGenerator::FULL).c_str() << "\"\n";
      }
    cmakefileStream << "  )\n";
    }
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::GetObjectFileName(cmTarget& target,
                    const cmSourceFile& source,
                    std::string* nameWithoutTargetDir)
{
  // If the source file is located below the current binary directory
  // then use that relative path for the object file name.
  std::string objectName =
    cmSystemTools::RelativePath(m_Makefile->GetCurrentOutputDirectory(),
                                source.GetFullPath().c_str());
  if(objectName.empty() || objectName[0] == '.')
    {
    // If the source file is located below the current source
    // directory then use that relative path for the object file name.
    // Otherwise just use the relative path from the current binary
    // directory.
    std::string relFromSource =
      cmSystemTools::RelativePath(m_Makefile->GetCurrentDirectory(),
                                  source.GetFullPath().c_str());
    if(!relFromSource.empty() && relFromSource[0] != '.')
      {
      objectName = relFromSource;
      }
    }

  // Replace the original source file extension with the object file
  // extension.
  std::string::size_type dot_pos = objectName.rfind(".");
  if(dot_pos != std::string::npos)
    {
    objectName = objectName.substr(0, dot_pos);
    }
  objectName +=
    m_GlobalGenerator->GetLanguageOutputExtensionFromExtension(
      source.GetSourceExtension().c_str());

  // Convert to a safe name.
  objectName = this->CreateSafeUniqueObjectFileName(objectName.c_str());

  // Prepend the target directory.
  std::string obj = this->GetTargetDirectory(target);
  obj += "/";
  obj += objectName;
  if(nameWithoutTargetDir)
    {
    *nameWithoutTargetDir = objectName;
    }
  return obj;
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteDisclaimer(std::ostream& os)
{
  os
    << "# CMAKE generated file: DO NOT EDIT!\n"
    << "# Generated by \"" << m_GlobalGenerator->GetName() << "\""
    << " Generator, CMake Version "
    << cmMakefile::GetMajorVersion() << "."
    << cmMakefile::GetMinorVersion() << "\n\n";
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::GetRecursiveMakeCall(const char *Makefile, const char* tgt)
{
  // Call make on the given file.
  std::string cmd;
  cmd += "$(MAKE) -f ";
  cmd += Makefile;
  cmd += " ";
  
  // Passg down verbosity level.
  if(this->GetMakeSilentFlag().size())
    {
    cmd += this->GetMakeSilentFlag();
    cmd += " ";
    }

  // Most unix makes will pass the command line flags to make down to
  // sub-invoked makes via an environment variable.  However, some
  // makes do not support that, so you have to pass the flags
  // explicitly.
  if(this->GetPassMakeflags())
    {
    cmd += "-$(MAKEFLAGS) ";
    }

  // Add the target.
  if (tgt && tgt[0] != '\0')
    {
    std::string tgt2 = this->Convert(tgt,HOME_OUTPUT,MAKEFILE);
    tgt2 = this->ConvertToMakeTarget(tgt2.c_str());
    cmd += tgt2;
    }
  return cmd;
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteDivider(std::ostream& os)
{
  os
    << "#======================================"
    << "=======================================\n";
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteCMakeArgument(std::ostream& os, const char* s)
{
  // Write the given string to the stream with escaping to get it back
  // into CMake through the lexical scanner.
  os << "\"";
  for(const char* c = s; *c; ++c)
    {
    if(*c == '\\')
      {
      os << "\\\\";
      }
    else if(*c == '"')
      {
      os << "\\\"";
      }
    else
      {
      os << *c;
      }
    }
  os << "\"";
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3::ConvertToQuotedOutputPath(const char* p)
{
  
  // Split the path into its components.
  std::vector<std::string> components;
  cmSystemTools::SplitPath(p, components);

  // Return an empty path if there are no components.
  if(components.empty())
    {
    return "\"\"";
    }

  // Choose a slash direction and fix root component.
  const char* slash = "/";
#if defined(_WIN32) && !defined(__CYGWIN__)
   if(!cmSystemTools::GetForceUnixPaths())
     {
     slash = "\\";
     for(std::string::iterator i = components[0].begin();
       i != components[0].end(); ++i)
       {
       if(*i == '/')
         {
         *i = '\\';
         }
       }
     }
#endif

  // Begin the quoted result with the root component.
  std::string result = "\"";
  result += components[0];

  // Now add the rest of the components separated by the proper slash
  // direction for this platform.
  bool first = true;
  for(unsigned int i=1; i < components.size(); ++i)
    {
    // Only the last component can be empty to avoid double slashes.
    if(components[i].length() > 0 || (i == (components.size()-1)))
      {
      if(!first)
        {
        result += slash;
        }
      result += components[i];
      first = false;
      }
    }

  // Close the quoted result.
  result += "\"";

  return result;
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3::GetTargetDirectory(cmTarget& target)
{
  std::string dir = "CMakeFiles/";
  dir += target.GetName();
  dir += ".dir";
  return dir;
}

void cmLocalUnixMakefileGenerator3
::CreateCDCommand(std::vector<std::string>& commands, const char *tgtDir,
                  const char *retDir)
{
  // do we need to cd?
  if (!strcmp(tgtDir,retDir))
    {
    return;
    }
  
  if(!m_UnixCD)
    {
    // On Windows we must perform each step separately and then change
    // back because the shell keeps the working directory between
    // commands.
    std::string cmd = "cd ";
    cmd += this->ConvertToOutputForExisting(tgtDir);
    commands.insert(commands.begin(),cmd);
    
    // Change back to the starting directory.  Any trailing slash must be
    // removed to avoid problems with Borland Make.
    std::string back = retDir;
    if(back.size() && back[back.size()-1] == '/')
      {
      back = back.substr(0, back.size()-1);
      }
    cmd = "cd ";
    cmd += this->ConvertToOutputForExisting(back.c_str());
    commands.push_back(cmd);
    }
  else
    {
    // On UNIX we must construct a single shell command to change
    // directory and build because make resets the directory between
    // each command.
    std::vector<std::string>::iterator i = commands.begin();
    for (; i != commands.end(); ++i)
      {
      std::string cmd = "cd ";
      cmd += this->ConvertToOutputForExisting(tgtDir);
      cmd += " && ";
      cmd += *i;
      *i = cmd;
      }
    }
}

//----------------------------------------------------------------------------
const char*
cmLocalUnixMakefileGenerator3
::GetSourceFileLanguage(const cmSourceFile& source)
{
  // Identify the language of the source file.
  return (m_GlobalGenerator
          ->GetLanguageFromExtension(source.GetSourceExtension().c_str()));
}

