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

// TODO: Add "help" target.
// TODO: Identify remaining relative path violations.
// TODO: Need test for separate executable/library output path.

//----------------------------------------------------------------------------
cmLocalUnixMakefileGenerator3::cmLocalUnixMakefileGenerator3()
{
  m_WindowsShell = false;
  m_IncludeDirective = "include";
  m_MakefileVariableSize = 0;
  m_IgnoreLibPrefix = false;
  m_PassMakeflags = false;
  m_EchoNeedsQuote = true;
}

//----------------------------------------------------------------------------
cmLocalUnixMakefileGenerator3::~cmLocalUnixMakefileGenerator3()
{
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::Configure()
{
  this->ComputeHomeRelativeOutputPath();
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
    if((t->second.GetType() == cmTarget::EXECUTABLE) ||
       (t->second.GetType() == cmTarget::STATIC_LIBRARY) ||
       (t->second.GetType() == cmTarget::SHARED_LIBRARY) ||
       (t->second.GetType() == cmTarget::MODULE_LIBRARY))
      {
      t->second.TraceVSDependencies(empty, m_Makefile);
      this->WriteTargetRuleFiles(t->second);
      }
    else if(t->second.GetType() == cmTarget::UTILITY)
      {
      t->second.TraceVSDependencies(empty, m_Makefile);
      this->WriteUtilityRuleFiles(t->second);
      }
    }

  // write the local Makefile
  this->WriteLocalMakefile();
  
  // Write the cmake file with information for this directory.
  this->WriteDirectoryInformationFile();
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


void cmLocalUnixMakefileGenerator3
::WriteCustomCommands(std::ostream& ruleFileStream,
                      std::vector<std::string>& cleanFiles)
{
  // add custom commands to the clean rules?
  const char* clean_no_custom = m_Makefile->GetProperty("CLEAN_NO_CUSTOM");
  bool clean = cmSystemTools::IsOff(clean_no_custom);
  
  // Generate the rule files for each custom command.
  const std::vector<cmSourceFile*> &classes = m_Makefile->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin(); 
      i != classes.end(); i++)
    {
    if(cmCustomCommand* cc = (*i)->GetCustomCommand())
      {
      this->GenerateCustomRuleFile(*cc,ruleFileStream);
      if (clean)
        {
        cleanFiles.push_back
          (this->Convert(cc->GetOutput(),START_OUTPUT,SHELL));
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

void cmLocalUnixMakefileGenerator3::ComputeHomeRelativeOutputPath()
{
  // Include the rule file for each object.
  m_HomeRelativeOutputPath = 
    cmSystemTools::RelativePath(m_Makefile->GetHomeOutputDirectory(),
                                m_Makefile->GetStartOutputDirectory());
  if (m_HomeRelativeOutputPath.size())
    {
    m_HomeRelativeOutputPath += "/";
    }
}

const std::string &cmLocalUnixMakefileGenerator3::GetHomeRelativeOutputPath()
{
  return m_HomeRelativeOutputPath;
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteTargetRuleFiles(cmTarget& target)
{
  // Create a directory for this target.
  std::string dir = m_Makefile->GetStartOutputDirectory();
  dir += "/";
  dir += this->GetTargetDirectory(target);
  cmSystemTools::MakeDirectory(dir.c_str());

  // Generate the build-time dependencies file for this target.
  std::string depBase = dir;
  depBase += "/";
  depBase += target.GetName();

  // Construct the rule file name.
  std::string ruleFileName = dir;
  ruleFileName += "/build.make";

  // Open the rule file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  std::string ruleFileNameFull = ruleFileName;
  cmGeneratedFileStream ruleFileStream(ruleFileNameFull.c_str());
  ruleFileStream.SetCopyIfDifferent(true);
  if(!ruleFileStream)
    {
    return;
    }
  this->WriteDisclaimer(ruleFileStream);
  
  this->WriteMakeVariables(ruleFileStream);
  
  // Open the flags file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  std::string flagFileName = dir;
  flagFileName += "/flags.make";
  std::string flagFileNameFull = flagFileName;
  cmGeneratedFileStream flagFileStream(flagFileNameFull.c_str());
  flagFileStream.SetCopyIfDifferent(true);
  if(!flagFileStream)
    {
    return;
    }
  this->WriteDisclaimer(flagFileStream);

  // Include the dependencies for the target.
  std::string depPath = dir;
  depPath += "/depend.make";
  depPath = this->Convert(depPath.c_str(),HOME_OUTPUT,MAKEFILE);
  ruleFileStream
    << "# Include any dependencies generated for this target.\n"
    << m_IncludeDirective << " "
    << depPath
    << "\n\n";
  
  // Include the flags for the target.
  flagFileName = this->Convert(flagFileName.c_str(), HOME_OUTPUT, MAKEFILE);
  ruleFileStream
    << "# Include the compile flags for this target's objects.\n"
    << m_IncludeDirective << " "
    << flagFileName
    << "\n\n";
  
  // make sure the depend file exists
  depPath = dir;
  depPath += "/depend.make";
  depPath = this->Convert(depPath.c_str(),FULL,UNCHANGED);
  if (!cmSystemTools::FileExists(depPath.c_str()))
    {
    // Write an empty dependency file.
    cmGeneratedFileStream depFileStream(depPath.c_str());
    depFileStream
      << "# Empty dependencies file for " << target.GetName() << ".\n"
      << "# This may be replaced when dependencies are built." << std::endl;
    }
  
  // First generate the object rule files.  Save a list of all object
  // files for this target.
  std::vector<std::string> objects;
  std::vector<std::string> external_objects;
  const std::vector<cmSourceFile*>& sources = target.GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
      source != sources.end(); ++source)
    {
    if(!(*source)->GetPropertyAsBool("HEADER_FILE_ONLY") &&
       !(*source)->GetCustomCommand())
      {
      if(!m_GlobalGenerator->IgnoreFile((*source)->GetSourceExtension().c_str()))
        {
        // Generate this object file's rule file.
        this->WriteObjectRuleFiles(target, *(*source), objects, 
                                   ruleFileStream, flagFileStream);
        }
      else if((*source)->GetPropertyAsBool("EXTERNAL_OBJECT"))
        {
        // This is an external object file.  Just add it.
        external_objects.push_back((*source)->GetFullPath());
        }
      else
        {
        // We only get here if a source file is not an external object
        // and has an extension that is listed as an ignored file type
        // for this language.  No message or diagnosis should be
        // given.
        }
      }
    }
  
  // write language flags for target
  std::map<cmStdString,cmLocalUnixMakefileGenerator3::IntegrityCheckSet>& 
    checkSet = this->GetIntegrityCheckSet()[target.GetName()];
  for(std::map<cmStdString, 
        cmLocalUnixMakefileGenerator3::IntegrityCheckSet>::const_iterator
        l = checkSet.begin(); l != checkSet.end(); ++l)
    {
    const char *lang = l->first.c_str();
    std::string flags;
    // Add the export symbol definition for shared library objects.
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
    
    // Add language-specific flags.
    this->AddLanguageFlags(flags, lang);
    
    // Add shared-library flags if needed.
    this->AddSharedFlags(flags, lang, shared);
    
    // Add include directory flags.
    this->AppendFlags(flags, this->GetIncludeFlags(lang));
    flagFileStream << lang << "_FLAGS = " << flags
                   << "\n"
                   << "\n";
    }
  
  // write the custom commands for this target
  std::vector<std::string> cleanFiles;
  // Look for files registered for cleaning in this directory.
  if(const char* additional_clean_files =
     m_Makefile->GetProperty("ADDITIONAL_MAKE_CLEAN_FILES"))
    {
    cmSystemTools::ExpandListArgument(additional_clean_files, cleanFiles);
    }  
  this->WriteCustomCommands(ruleFileStream,cleanFiles);

  // Include the rule file for each object.
  std::string relPath = this->GetHomeRelativeOutputPath();
  std::string objTarget;

  // Write the rule for this target type.
  switch(target.GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      this->WriteStaticLibraryRule(ruleFileStream, ruleFileName.c_str(),
                                   target, objects, external_objects,
                                   cleanFiles);
      break;
    case cmTarget::SHARED_LIBRARY:
      this->WriteSharedLibraryRule(ruleFileStream, ruleFileName.c_str(),
                                   target, objects, external_objects, 
                                   cleanFiles);
      break;
    case cmTarget::MODULE_LIBRARY:
      this->WriteModuleLibraryRule(ruleFileStream, ruleFileName.c_str(),
                                   target, objects, external_objects, 
                                   cleanFiles);
      break;
    case cmTarget::EXECUTABLE:
      this->WriteExecutableRule(ruleFileStream, ruleFileName.c_str(),
                                target, objects, external_objects,
                                cleanFiles);
      break;
    default:
      break;
    }

  // Write the requires target.
  this->WriteTargetRequiresRule(ruleFileStream, target, objects);

  // Write clean target
  this->WriteTargetCleanRule(ruleFileStream, target, cleanFiles);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteObjectDependRules(cmSourceFile& source,
                         std::vector<std::string>& depends)
{
  // Create the list of dependencies known at cmake time.  These are
  // shared between the object file and dependency scanning rule.
  depends.push_back(source.GetFullPath());
  if(const char* objectDeps = source.GetProperty("OBJECT_DEPENDS"))
    {
    std::vector<std::string> deps;
    cmSystemTools::ExpandListArgument(objectDeps, deps);
    for(std::vector<std::string>::iterator i = deps.begin();
        i != deps.end(); ++i)
      {
      depends.push_back(i->c_str());
      }
    }
}


//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteObjectBuildFile(std::string &obj,
                       const char *lang, 
                       cmTarget& target, 
                       cmSourceFile& source,
                       std::vector<std::string>& depends,
                       std::ostream &ruleFileStream,
                       std::ostream &flagFileStream)
{
  // Open the rule file for writing.  This should be copy-if-different
  // because the rules may depend on this file itself.
  std::string ruleFileName = m_Makefile->GetStartOutputDirectory();
  ruleFileName += "/";
  ruleFileName += this->GetTargetDirectory(target);
  ruleFileName += "/build.make";
  std::string ruleFileNameFull = this->ConvertToFullPath(ruleFileName);

  std::string flagFileName = this->GetTargetDirectory(target);
  flagFileName += "/flags.make";
  std::string flagFileNameFull = this->ConvertToFullPath(flagFileName);
  this->AppendRuleDepend(depends, flagFileNameFull.c_str());

  // generate the depend scanning rule
  this->WriteObjectDependRules(source, depends);

  std::string relativeObj = this->GetHomeRelativeOutputPath();
  relativeObj += obj;

  // Write the build rule.
  // Build the set of compiler flags.
  std::string flags;

  // Add flags from source file properties.
  if (source.GetProperty("COMPILE_FLAGS"))
    {
    this->AppendFlags(flags, source.GetProperty("COMPILE_FLAGS"));
    flagFileStream << "# Custom flags: "
                   << relativeObj << "_FLAGS = "
                   << source.GetProperty("COMPILE_FLAGS")
                   << "\n"
                   << "\n";
    }
  
  // Add language-specific flags.
  std::string langFlags = "$(";
  langFlags += lang;
  langFlags += "_FLAGS)";
  this->AppendFlags(flags, langFlags.c_str());
  
  // Get the output paths for source and object files.
  std::string sourceFile = source.GetFullPath();
  if(m_UseRelativePaths)
    {
    sourceFile = this->Convert(sourceFile.c_str(),HOME_OUTPUT);
    }
  sourceFile = this->Convert(sourceFile.c_str(),NONE,SHELL);
  std::string objectFile = this->Convert(obj.c_str(),START_OUTPUT,SHELL);

  // Construct the build message.
  std::vector<std::string> no_commands;
  std::vector<std::string> commands;
  std::string buildEcho = "Building ";
  buildEcho += lang;
  buildEcho += " object ";
  buildEcho += relativeObj;
  this->AppendEcho(commands, buildEcho.c_str());

  // Construct the compile rules.
  std::string compileRuleVar = "CMAKE_";
  compileRuleVar += lang;
  compileRuleVar += "_COMPILE_OBJECT";
  std::string compileRule =
    m_Makefile->GetRequiredDefinition(compileRuleVar.c_str());
  cmSystemTools::ExpandListArgument(compileRule, commands);

  // Expand placeholders in the commands.
  for(std::vector<std::string>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->ExpandRuleVariables(*i,
                              lang,
                              0, // no objects
                              0, // no target
                              0, // no link libs
                              sourceFile.c_str(),
                              relativeObj.c_str(),
                              flags.c_str());
    }

  // Make the target dependency scanning rule include cmake-time-known
  // dependencies.  The others are handled by the check-build-system
  // path.
  std::string depMark = this->GetRelativeTargetDirectory(target);
  depMark += "/depend.make.mark";
  this->WriteMakeRule(ruleFileStream, 0,
                      depMark.c_str(), depends, no_commands);

  // Write the rule.
  this->WriteMakeRule(ruleFileStream, 0,
                      relativeObj.c_str(), depends, commands);

  // If the language needs provides-requires mode, create the
  // corresponding targets.
  std::string objectRequires = relativeObj;
  objectRequires += ".requires";
  std::vector<std::string> p_depends;
  // always provide an empty requires target
  this->WriteMakeRule(ruleFileStream, 0,
                      objectRequires.c_str(), p_depends, no_commands);

  // write a build rule to recursively build what this obj provides
  std::string objectProvides = relativeObj;
  objectProvides += ".provides";
  std::string temp = relativeObj;
  temp += ".provides.build";
  std::vector<std::string> r_commands;
  std::string tgtMakefileName = this->GetRelativeTargetDirectory(target);
  tgtMakefileName += "/build.make";
  r_commands.push_back(this->GetRecursiveMakeCall(tgtMakefileName.c_str(),
                                                  temp.c_str()));
  p_depends.clear();
  p_depends.push_back(objectRequires);
  this->WriteMakeRule(ruleFileStream, 0,
                      objectProvides.c_str(), p_depends, r_commands);
  
  // write the provides.build rule dependency on the obj file
  p_depends.clear();
  p_depends.push_back(relativeObj);
  this->WriteMakeRule(ruleFileStream, 0,
                      temp.c_str(), p_depends, no_commands);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteObjectRuleFiles(cmTarget& target, cmSourceFile& source,
                       std::vector<std::string>& objects, 
                       std::ostream &ruleFileStream,
                       std::ostream &flagFileStream)
{
  // Identify the language of the source file.
  const char* lang = this->GetSourceFileLanguage(source);
  if(!lang)
    {
    // If language is not known, this is an error.
    cmSystemTools::Error("Source file \"", source.GetFullPath().c_str(),
                         "\" has unknown type.");
    return;
    }

  // Get the full path name of the object file.
  std::string obj = this->GetObjectFileName(target, source);
  
  // Avoid generating duplicate rules.
  if(m_ObjectFiles.find(obj) == m_ObjectFiles.end())
    {
    m_ObjectFiles.insert(obj);
    }
  else
    {
    cmOStringStream err;
    err << "Warning: Source file \""
        << source.GetSourceName().c_str() << "."
        << source.GetSourceExtension().c_str()
        << "\" is listed multiple times for target \"" << target.GetName()
        << "\".";
    cmSystemTools::Message(err.str().c_str(), "Warning");
    return;
    }
  
  // Create the directory containing the object file.  This may be a
  // subdirectory under the target's directory.
  std::string dir = cmSystemTools::GetFilenamePath(obj.c_str());
  cmSystemTools::MakeDirectory(this->ConvertToFullPath(dir).c_str());

  // Save this in the target's list of object files.
  objects.push_back(obj);
  std::string relativeObj = this->GetHomeRelativeOutputPath();
  relativeObj += obj;
  
  // we compute some depends when writing the depend.make that we will also
  // use in the build.make, same with depMakeFile
  std::vector<std::string> depends;
  std::string depMakeFile;
  
  // generate the build rule file
  this->WriteObjectBuildFile(obj, lang, target, source, depends,
                             ruleFileStream, flagFileStream);
  
  // The object file should be checked for dependency integrity.
  m_CheckDependFiles[target.GetName()][lang].insert(&source);
     
  // add this to the list of objects for this local generator
  m_LocalObjectFiles[cmSystemTools::GetFilenameName(obj)].push_back(&target);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::GenerateCustomRuleFile(const cmCustomCommand& cc,
                         std::ostream &ruleFileStream)
{
  // Convert the output name to a relative path if possible.
  std::string output = this->Convert(cc.GetOutput(),START_OUTPUT);

  // Collect the commands.
  std::vector<std::string> commands;
  std::string preEcho = "Generating ";
  preEcho += output;
  this->AppendEcho(commands, preEcho.c_str());
  this->AppendCustomCommand(commands, cc);
  
  // Collect the dependencies.
  std::vector<std::string> depends;
  this->AppendCustomDepend(depends, cc);

  // Write the rule.
  const char* comment = 0;
  if(cc.GetComment() && *cc.GetComment())
    {
    comment = cc.GetComment();
    }
  this->WriteMakeRule(ruleFileStream, comment,
                      cc.GetOutput(), depends, commands);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteUtilityRuleFiles(cmTarget& target)
{
  // Create a directory for this target.
  std::string dir = this->GetTargetDirectory(target);
  cmSystemTools::MakeDirectory(this->ConvertToFullPath(dir).c_str());

  // Construct the name of the rule file.
  std::string ruleFileName = dir;
  ruleFileName += "/build.make";

  // Open the rule file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  std::string ruleFileNameFull = this->ConvertToFullPath(ruleFileName);
  cmGeneratedFileStream ruleFileStream(ruleFileNameFull.c_str());
  ruleFileStream.SetCopyIfDifferent(true);
  if(!ruleFileStream)
    {
    return;
    }
  this->WriteDisclaimer(ruleFileStream);
  this->WriteMakeVariables(ruleFileStream);
  ruleFileStream
    << "# Utility rule file for " << target.GetName() << ".\n\n";

  // write the custom commands for this target
  std::vector<std::string> cleanFiles;
  if(const char* additional_clean_files =
     m_Makefile->GetProperty("ADDITIONAL_MAKE_CLEAN_FILES"))
    {
    cmSystemTools::ExpandListArgument(additional_clean_files, cleanFiles);
    }  
  this->WriteCustomCommands(ruleFileStream, cleanFiles);

  // Collect the commands and dependencies.
  std::vector<std::string> commands;
  std::vector<std::string> depends;

  // Utility targets store their rules in pre- and post-build commands.
  this->AppendCustomDepends(depends, target.GetPreBuildCommands());
  this->AppendCustomDepends(depends, target.GetPostBuildCommands());
  this->AppendCustomCommands(commands, target.GetPreBuildCommands());
  this->AppendCustomCommands(commands, target.GetPostBuildCommands());

  // Add dependencies on targets that must be built first.
  this->AppendTargetDepends(depends, target);

  // Add a dependency on the rule file itself.
  std::string relPath = this->GetHomeRelativeOutputPath();
  std::string objTarget = relPath;
  objTarget += ruleFileName;
  this->AppendRuleDepend(depends, objTarget.c_str());

  // Write the rule.
  this->WriteMakeRule(ruleFileStream, 0,
                      target.GetName(), depends, commands);

  // Write convenience targets.
  dir = m_Makefile->GetStartOutputDirectory();
  dir += "/";
  dir += this->GetTargetDirectory(target);
  std::string buildTargetRuleName = dir;
  buildTargetRuleName += "/build";
  buildTargetRuleName = 
    this->Convert(buildTargetRuleName.c_str(),HOME_OUTPUT,MAKEFILE);
  this->WriteConvenienceRule(ruleFileStream, target.GetName(),
                             buildTargetRuleName.c_str());

  // Write clean target
  this->WriteTargetCleanRule(ruleFileStream, target, cleanFiles);
}

//----------------------------------------------------------------------------
bool
cmLocalUnixMakefileGenerator3
::GenerateDependsMakeFile(const std::string& lang, const char* objFile)
{
  // Construct a checker for the given language.
  std::auto_ptr<cmDepends>
    checker(this->GetDependsChecker(lang,false));
  if(checker.get())
    {
    // Check the dependencies. Ths is required because we need at least an
    // empty depends.make for make to include, so at cmake time the
    // ::Check() method will generate that if it does not exist
    checker->Check(objFile, 0);
    
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteMakeRule(std::ostream& os,
                const char* comment,
                const char* target,
                const std::vector<std::string>& depends,
                const std::vector<std::string>& commands)
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
void cmLocalUnixMakefileGenerator3::WriteDivider(std::ostream& os)
{
  os
    << "#======================================"
    << "=======================================\n";
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
  if(m_WindowsShell)
    {
    makefileStream
      << "!IF \"$(OS)\" == \"Windows_NT\"\n"
      << "NULL=\n"
      << "!ELSE\n"
      << "NULL=nul\n"
      << "!ENDIF\n";
    }
  else
    {
    makefileStream
      << "# The shell in which to execute make rules.\n"
      << "SHELL = /bin/sh\n"
      << "\n";
    }

  if(m_Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"))
    {
    makefileStream
      << "# Produce verbose output by default.\n"
      << "VERBOSE = 1\n"
      << "\n";
    }

  std::string cmakecommand =
      m_Makefile->GetRequiredDefinition("CMAKE_COMMAND");
  makefileStream
    << "# The CMake executable.\n"
    << "CMAKE_COMMAND = "
    << this->Convert(cmakecommand.c_str(), FULL, MAKEFILE).c_str() 
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

  // Write the main entry point target.  This must be the VERY first
  // target so that make with no arguments will run it.
  {
  // Just depend on the all target to drive the build.
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;
  depends.push_back("all");

  // Write the rule.
  this->WriteMakeRule(makefileStream,
                      "Default target executed when no arguments are "
                      "given to make.",
                      "default_target",
                      depends,
                      no_commands);
  }

  // Write special "test" target to run ctest.
  if(m_Makefile->IsOn("CMAKE_TESTING_ENABLED"))
    {
    std::string ctest;
    if(m_Makefile->GetDefinition("CMake_BINARY_DIR"))
      {
      // We are building CMake itself.  Use the ctest that comes with
      // this version of CMake instead of the one used to build it.
      ctest = m_ExecutableOutputPath;
      ctest += "ctest";
      ctest += cmSystemTools::GetExecutableExtension();
      ctest = this->Convert(ctest.c_str(),START_OUTPUT,SHELL);
      ctest += " --force-new-ctest-process";
      }
    else
      {
      // We are building another project.  Use the ctest that comes with
      // the CMake building it.
      ctest = m_Makefile->GetRequiredDefinition("CMAKE_COMMAND");
      ctest = cmSystemTools::GetFilenamePath(ctest.c_str());
      ctest += "/";
      ctest += "ctest";
      ctest += cmSystemTools::GetExecutableExtension();
      ctest = this->ConvertToOutputForExisting(ctest.c_str());
      }
    std::vector<std::string> no_depends;
    std::vector<std::string> commands;
    this->AppendEcho(commands, "Running tests...");
    ctest += " $(ARGS)";
    commands.push_back(ctest);
    this->WriteMakeRule(makefileStream,
                        "Special rule to drive testing with ctest.",
                        "test", no_depends, commands);
    }

  // Write special "install" target to run cmake_install.cmake script.
  {
  std::vector<std::string> depends;
  std::vector<std::string> commands;
  std::string cmd;
  if(m_Makefile->GetDefinition("CMake_BINARY_DIR"))
    {
    // We are building CMake itself.  We cannot use the original
    // executable to install over itself.
    cmd = m_ExecutableOutputPath;
    cmd += "cmake";
    cmd = this->Convert(cmd.c_str(),START_OUTPUT,SHELL);
    }
  else
    {
    cmd = "$(CMAKE_COMMAND)";
    }
  cmd += " -P cmake_install.cmake";
  commands.push_back(cmd);
  const char* noall =
    m_Makefile->GetDefinition("CMAKE_SKIP_INSTALL_ALL_DEPENDENCY");
  if(!noall || cmSystemTools::IsOff(noall))
    {
    // Drive the build before installing.
    depends.push_back("all");
    }
  this->WriteMakeRule(makefileStream,
                      "Special rule to run installation script.",
                      "install", depends, commands);
  }

  // Write special "rebuild_cache" target to re-run cmake.
  {
  std::vector<std::string> no_depends;
  std::vector<std::string> commands;
  this->AppendEcho(commands, "Running CMake to regenerate build system...");
  commands.push_back(
    "$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");
  this->WriteMakeRule(makefileStream,
                      "Special rule to re-run CMake using make.",
                      "rebuild_cache",
                      no_depends,
                      commands);
  }

  // Use CMAKE_EDIT_COMMAND for the edit_cache rule if it is defined.
  // Otherwise default to the interactive command-line interface.
  if(m_Makefile->GetDefinition("CMAKE_EDIT_COMMAND"))
    {
    std::vector<std::string> no_depends;
    std::vector<std::string> commands;
    this->AppendEcho(commands, "Running CMake cache editor...");
    commands.push_back(
      "$(CMAKE_EDIT_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)");
    this->WriteMakeRule(makefileStream,
                        "Special rule to re-run CMake cache editor using make.",
                        "edit_cache",
                        no_depends,
                        commands);
    }
  else
    {
    std::vector<std::string> no_depends;
    std::vector<std::string> commands;
    this->AppendEcho(commands,
                     "Running interactive CMake command-line interface...");
    commands.push_back(
      "$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR) -i");
    this->WriteMakeRule(makefileStream,
                        "Special rule to re-run CMake cache editor using make.",
                        "edit_cache",
                        no_depends,
                        commands);
    }

  // Write special target to silence make output.  This must be after
  // the default target in case VERBOSE is set (which changes the
  // name).  The setting of CMAKE_VERBOSE_MAKEFILE to ON will cause a
  // "VERBOSE=1" to be added as a make variable which will change the
  // name of this special target.  This gives a make-time choice to
  // the user.
  std::vector<std::string> commands;
  commands.clear();
  std::vector<std::string> no_depends;
  this->WriteMakeRule(makefileStream,
                      "Suppress display of executed commands.",
                      "$(VERBOSE).SILENT",
                      no_depends,
                      commands);

  // Special target to cleanup operation of make tool.
  std::vector<std::string> depends;
  this->WriteMakeRule(makefileStream,
                      "Disable implicit rules so canoncical targets will work.",
                      ".SUFFIXES",
                      depends,
                      commands);
  // Add a fake suffix to keep HP happy.  Must be max 32 chars for SGI make.
  depends.push_back(".hpux_make_needs_suffix_list");
  this->WriteMakeRule(makefileStream, 0,
                      ".SUFFIXES", depends, commands);

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
  this->WriteMakeRule(makefileStream,
                      "Special rule to run CMake to check the build system "
                      "integrity.\n"
                      "No rule that depends on this can have "
                      "commands that come from listfiles\n"
                      "because they might be regenerated.",
                      "cmake_check_build_system",
                      no_depends,
                      commands);
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
                        helpTarget, depends, no_commands);
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteTargetRequiresRule(std::ostream& ruleFileStream, cmTarget& target,
                          const std::vector<std::string>& objects)
{
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;

  // Construct the name of the dependency generation target.
  std::string depTarget = this->GetRelativeTargetDirectory(target);
  depTarget += "/requires";

  // This target drives dependency generation for all object files.
  std::string relPath = this->GetHomeRelativeOutputPath();
  std::string objTarget;
  for(std::vector<std::string>::const_iterator obj = objects.begin();
      obj != objects.end(); ++obj)
    {
    objTarget = relPath;
    objTarget += *obj;
    objTarget += ".requires";
    depends.push_back(objTarget);
    }

  // Write the rule.
  this->WriteMakeRule(ruleFileStream, 0,
                      depTarget.c_str(), depends, no_commands);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteExecutableRule(std::ostream& ruleFileStream,
                      const char* ruleFileName,
                      cmTarget& target,
                      const std::vector<std::string>& objects,
                      const std::vector<std::string>& external_objects,
                      std::vector<std::string>& cleanFiles)
{
  // Write the dependency generation rule.
  this->WriteTargetDependRule(ruleFileStream, target);

  std::vector<std::string> commands;

  std::string relPath = this->GetHomeRelativeOutputPath();
  std::string objTarget;

  // Build list of dependencies.
  std::vector<std::string> depends;
  for(std::vector<std::string>::const_iterator obj = objects.begin();
      obj != objects.end(); ++obj)
    {
    objTarget = relPath;
    objTarget += *obj;
    depends.push_back(objTarget);
    }

  // Add dependencies on targets that must be built first.
  this->AppendTargetDepends(depends, target);

  // Add a dependency on the rule file itself.
  this->AppendRuleDepend(depends, ruleFileName);

  for(std::vector<std::string>::const_iterator obj = external_objects.begin();
      obj != external_objects.end(); ++obj)
    {
    depends.push_back(*obj);
    }

  // from here up is the same for exe or lib

  // Get the name of the executable to generate.
  std::string targetName;
  std::string targetNameReal;
  target.GetExecutableNames(targetName, targetNameReal);

  // Construct the full path version of the names.
  std::string outpath = m_ExecutableOutputPath;
  if(outpath.length() == 0)
    {
    outpath = m_Makefile->GetStartOutputDirectory();
    outpath += "/";
    }
#ifdef __APPLE__
  if(target.GetPropertyAsBool("MACOSX_BUNDLE"))
    {
    // Make bundle directories
    outpath += target.GetName();
    outpath += ".app/Contents/MacOS/";
    }
#endif
  std::string targetFullPath = outpath + targetName;
  std::string targetFullPathReal = outpath + targetNameReal;

  // Convert to the output path to use in constructing commands.
  std::string targetOutPath =
    this->Convert(targetFullPath.c_str(),START_OUTPUT,MAKEFILE);
  std::string targetOutPathReal =
    this->Convert(targetFullPathReal.c_str(),START_OUTPUT,MAKEFILE);

  // Get the language to use for linking this executable.
  const char* linkLanguage =
    target.GetLinkerLanguage(this->GetGlobalGenerator());

  // Make sure we have a link language.
  if(!linkLanguage)
    {
    cmSystemTools::Error("Cannot determine link language for target \"",
                         target.GetName(), "\".");
    return;
    }

  // Add the link message.
  std::string buildEcho = "Linking ";
  buildEcho += linkLanguage;
  buildEcho += " executable ";
  buildEcho += targetOutPath;
  this->AppendEcho(commands, buildEcho.c_str());

  // Build a list of compiler flags and linker flags.
  std::string flags;
  std::string linkFlags;

  // Add flags to deal with shared libraries.  Any library being
  // linked in might be shared, so always use shared flags for an
  // executable.
  this->AddSharedFlags(linkFlags, linkLanguage, true);

  // Add flags to create an executable.
  this->AddConfigVariableFlags(linkFlags, "CMAKE_EXE_LINKER_FLAGS");


  if(target.GetPropertyAsBool("WIN32_EXECUTABLE"))
    {
    this->AppendFlags(linkFlags,
                      m_Makefile->GetDefinition("CMAKE_CREATE_WIN32_EXE"));
    }
  else
    {
    this->AppendFlags(linkFlags,
                      m_Makefile->GetDefinition("CMAKE_CREATE_CONSOLE_EXE"));
    }

  // Add language-specific flags.
  this->AddLanguageFlags(flags, linkLanguage);

  // Add target-specific linker flags.
  this->AppendFlags(linkFlags, target.GetProperty("LINK_FLAGS"));

  // Construct a list of files associated with this executable that
  // may need to be cleaned.
  std::vector<std::string> exeCleanFiles;
  {
  std::string cleanName;
  std::string cleanRealName;
  target.GetExecutableCleanNames(cleanName, cleanRealName);
  std::string cleanFullName = outpath + cleanName;
  std::string cleanFullRealName = outpath + cleanRealName;
  exeCleanFiles.push_back
    (this->Convert(cleanFullName.c_str(),START_OUTPUT,MAKEFILE));
  if(cleanRealName != cleanName)
    {
    exeCleanFiles.push_back
      (this->Convert(cleanFullRealName.c_str(),START_OUTPUT,MAKEFILE));
    }
  }
  // Add a command to remove any existing files for this executable.
  this->AppendCleanCommand(commands, exeCleanFiles);

  // Add the pre-build and pre-link rules.
  this->AppendCustomCommands(commands, target.GetPreBuildCommands());
  this->AppendCustomCommands(commands, target.GetPreLinkCommands());

  // Construct the main link rule.
  std::string linkRuleVar = "CMAKE_";
  linkRuleVar += linkLanguage;
  linkRuleVar += "_LINK_EXECUTABLE";
  std::string linkRule = 
    m_Makefile->GetRequiredDefinition(linkRuleVar.c_str());
  std::vector<std::string> commands1;
  cmSystemTools::ExpandListArgument(linkRule, commands1);
  this->CreateCDCommand(commands1,m_Makefile->GetStartOutputDirectory(),
                        m_Makefile->GetHomeOutputDirectory());
  commands.insert(commands.end(), commands1.begin(), commands1.end());

  // Add a rule to create necessary symlinks for the library.
  if(targetOutPath != targetOutPathReal)
    {
    std::string symlink = "$(CMAKE_COMMAND) -E cmake_symlink_executable ";
    symlink += targetOutPathReal;
    symlink += " ";
    symlink += targetOutPath;
    commands.push_back(symlink);
    }

  // Add the post-build rules.
  this->AppendCustomCommands(commands, target.GetPostBuildCommands());

  // Collect up flags to link in needed libraries.
  cmOStringStream linklibs;
  this->OutputLinkLibraries(linklibs, 0, target);

  // Construct object file lists that may be needed to expand the
  // rule.
  std::string variableName;
  std::string variableNameExternal;
  this->WriteObjectsVariable(ruleFileStream, target, objects, external_objects,
                             variableName, variableNameExternal);
  std::string buildObjs = "$(";
  buildObjs += variableName;
  buildObjs += ") $(";
  buildObjs += variableNameExternal;
  buildObjs += ")";
  std::string cleanObjs = "$(";
  cleanObjs += variableName;
  cleanObjs += ")";

  // Expand placeholders in the commands.
  for(std::vector<std::string>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->ExpandRuleVariables(*i,
                              linkLanguage,
                              buildObjs.c_str(),
                              targetOutPathReal.c_str(),
                              linklibs.str().c_str(),
                              0,
                              0,
                              flags.c_str(),
                              0,
                              0,
                              0,
                              linkFlags.c_str());
    }

  // Write the build rule.
  this->WriteMakeRule(ruleFileStream, 0,
                      targetFullPathReal.c_str(), depends, commands);

  // The symlink name for the target should depend on the real target
  // so if the target version changes it rebuilds and recreates the
  // symlink.
  if(targetFullPath != targetFullPathReal)
    {
    depends.clear();
    commands.clear();
    depends.push_back(targetFullPathReal.c_str());
    this->WriteMakeRule(ruleFileStream, 0,
                        targetFullPath.c_str(), depends, commands);
    }

  // Write convenience targets.
  std::string dir = m_Makefile->GetStartOutputDirectory();
  dir += "/";
  dir += this->GetTargetDirectory(target);
  std::string buildTargetRuleName = dir;
  buildTargetRuleName += "/build";
  buildTargetRuleName = 
    this->Convert(buildTargetRuleName.c_str(),HOME_OUTPUT,MAKEFILE);
  this->WriteConvenienceRule(ruleFileStream, targetFullPath.c_str(),
                             buildTargetRuleName.c_str());

  // Clean all the possible executable names and symlinks and object files.
  cleanFiles.insert(cleanFiles.end(),exeCleanFiles.begin(),exeCleanFiles.end());
  cleanFiles.push_back(cleanObjs);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteStaticLibraryRule(std::ostream& ruleFileStream,
                         const char* ruleFileName,
                         cmTarget& target,
                         const std::vector<std::string>& objects,
                         const std::vector<std::string>& external_objects,
                         std::vector<std::string>& cleanFiles)
{
  const char* linkLanguage =
    target.GetLinkerLanguage(this->GetGlobalGenerator());
  std::string linkRuleVar = "CMAKE_";
  if (linkLanguage)
    {
    linkRuleVar += linkLanguage;
    }
  linkRuleVar += "_CREATE_STATIC_LIBRARY";

  std::string extraFlags;
  this->AppendFlags(extraFlags, target.GetProperty("STATIC_LIBRARY_FLAGS"));
  this->WriteLibraryRule(ruleFileStream, ruleFileName, target,
                         objects, external_objects,
                         linkRuleVar.c_str(), extraFlags.c_str(),cleanFiles);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteSharedLibraryRule(std::ostream& ruleFileStream,
                         const char* ruleFileName,
                         cmTarget& target,
                         const std::vector<std::string>& objects,
                         const std::vector<std::string>& external_objects,
                         std::vector<std::string>& cleanFiles)
{
  const char* linkLanguage =
    target.GetLinkerLanguage(this->GetGlobalGenerator());
  std::string linkRuleVar = "CMAKE_";
  if (linkLanguage)
    {
    linkRuleVar += linkLanguage;
    }
  linkRuleVar += "_CREATE_SHARED_LIBRARY";

  std::string extraFlags;
  this->AppendFlags(extraFlags, target.GetProperty("LINK_FLAGS"));
  this->AddConfigVariableFlags(extraFlags, "CMAKE_SHARED_LINKER_FLAGS");
  if(m_Makefile->IsOn("WIN32") && !(m_Makefile->IsOn("CYGWIN") || m_Makefile->IsOn("MINGW")))
    {
    const std::vector<cmSourceFile*>& sources = target.GetSourceFiles();
    for(std::vector<cmSourceFile*>::const_iterator i = sources.begin();
        i != sources.end(); ++i)
      {
      if((*i)->GetSourceExtension() == "def")
        {
        extraFlags += " ";
        extraFlags += m_Makefile->GetSafeDefinition("CMAKE_LINK_DEF_FILE_FLAG");
        extraFlags += 
          this->Convert((*i)->GetFullPath().c_str(),START_OUTPUT,MAKEFILE);
        }
      }
    }
  this->WriteLibraryRule(ruleFileStream, ruleFileName, target,
                         objects, external_objects,
                         linkRuleVar.c_str(), extraFlags.c_str(), cleanFiles);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteModuleLibraryRule(std::ostream& ruleFileStream,
                         const char* ruleFileName,
                         cmTarget& target,
                         const std::vector<std::string>& objects,
                         const std::vector<std::string>& external_objects,
                         std::vector<std::string>& cleanFiles)
{
  const char* linkLanguage =
    target.GetLinkerLanguage(this->GetGlobalGenerator());
  std::string linkRuleVar = "CMAKE_";
  if (linkLanguage)
    {
    linkRuleVar += linkLanguage;
    }
  linkRuleVar += "_CREATE_SHARED_MODULE";

  std::string extraFlags;
  this->AppendFlags(extraFlags, target.GetProperty("LINK_FLAGS"));
  this->AddConfigVariableFlags(extraFlags, "CMAKE_MODULE_LINKER_FLAGS");
  // TODO: .def files should be supported here also.
  this->WriteLibraryRule(ruleFileStream, ruleFileName, target,
                         objects, external_objects,
                         linkRuleVar.c_str(), extraFlags.c_str(), cleanFiles);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteLibraryRule(std::ostream& ruleFileStream,
                   const char* ruleFileName,
                   cmTarget& target,
                   const std::vector<std::string>& objects,
                   const std::vector<std::string>& external_objects,
                   const char* linkRuleVar,
                   const char* extraFlags,
                   std::vector<std::string>& cleanFiles)
{
  // Write the dependency generation rule.
  this->WriteTargetDependRule(ruleFileStream, target);

  // TODO: Merge the methods that call this method to avoid
  // code duplication.
  std::vector<std::string> commands;

  std::string relPath = this->GetHomeRelativeOutputPath();
  std::string objTarget;

  // Build list of dependencies.
  std::vector<std::string> depends;
  for(std::vector<std::string>::const_iterator obj = objects.begin();
      obj != objects.end(); ++obj)
    {
    objTarget = relPath;
    objTarget += *obj;
    depends.push_back(objTarget);
    }

  // Add dependencies on targets that must be built first.
  this->AppendTargetDepends(depends, target);

  // Add a dependency on the rule file itself.
  this->AppendRuleDepend(depends, ruleFileName);

  for(std::vector<std::string>::const_iterator obj = external_objects.begin();
      obj != external_objects.end(); ++obj)
    {
    depends.push_back(*obj);
    }

  // from here up is the same for exe or lib

  // Get the language to use for linking this library.
  const char* linkLanguage =
    target.GetLinkerLanguage(this->GetGlobalGenerator());

  // Make sure we have a link language.
  if(!linkLanguage)
    {
    cmSystemTools::Error("Cannot determine link language for target \"",
                         target.GetName(), "\".");
    return;
    }

  // Create set of linking flags.
  std::string linkFlags;
  this->AppendFlags(linkFlags, extraFlags);

  // Construct the name of the library.
  std::string targetName;
  std::string targetNameSO;
  std::string targetNameReal;
  std::string targetNameBase;
  target.GetLibraryNames(targetName, targetNameSO,
                         targetNameReal, targetNameBase);

  // Construct the full path version of the names.
  std::string outpath = m_LibraryOutputPath;
  if(outpath.length() == 0)
    {
    outpath = m_Makefile->GetStartOutputDirectory();
    outpath += "/";
    }
  std::string targetFullPath = outpath + targetName;
  std::string targetFullPathSO = outpath + targetNameSO;
  std::string targetFullPathReal = outpath + targetNameReal;
  std::string targetFullPathBase = outpath + targetNameBase;

  // Construct the output path version of the names for use in command
  // arguments.
  std::string targetOutPath = 
    this->Convert(targetFullPath.c_str(),START_OUTPUT,MAKEFILE);
  std::string targetOutPathSO = 
    this->Convert(targetFullPathSO.c_str(),START_OUTPUT,MAKEFILE);
  std::string targetOutPathReal = 
    this->Convert(targetFullPathReal.c_str(),START_OUTPUT,MAKEFILE);
  std::string targetOutPathBase = 
    this->Convert(targetFullPathBase.c_str(),START_OUTPUT,MAKEFILE);

  // Add the link message.
  std::string buildEcho = "Linking ";
  buildEcho += linkLanguage;
  switch(target.GetType())
    {
    case cmTarget::STATIC_LIBRARY:
      buildEcho += " static library "; break;
    case cmTarget::SHARED_LIBRARY:
      buildEcho += " shared library "; break;
    case cmTarget::MODULE_LIBRARY:
      buildEcho += " shared module "; break;
    default:
      buildEcho += " library "; break;
    }
  buildEcho += targetOutPath.c_str();
  this->AppendEcho(commands, buildEcho.c_str());

  // Construct a list of files associated with this library that may
  // need to be cleaned.
  std::vector<std::string> libCleanFiles;
  {
  std::string cleanStaticName;
  std::string cleanSharedName;
  std::string cleanSharedSOName;
  std::string cleanSharedRealName;
  target.GetLibraryCleanNames(cleanStaticName,
                              cleanSharedName,
                              cleanSharedSOName,
                              cleanSharedRealName);
  std::string cleanFullStaticName = outpath + cleanStaticName;
  std::string cleanFullSharedName = outpath + cleanSharedName;
  std::string cleanFullSharedSOName = outpath + cleanSharedSOName;
  std::string cleanFullSharedRealName = outpath + cleanSharedRealName;
  libCleanFiles.push_back
    (this->Convert(cleanFullStaticName.c_str(),START_OUTPUT,MAKEFILE));
  if(cleanSharedRealName != cleanStaticName)
    {
    libCleanFiles.push_back
      (this->Convert(cleanFullSharedRealName.c_str(),START_OUTPUT,MAKEFILE));
    }
  if(cleanSharedSOName != cleanStaticName &&
     cleanSharedSOName != cleanSharedRealName)
    {
    libCleanFiles.push_back
      (this->Convert(cleanFullSharedSOName.c_str(),START_OUTPUT,MAKEFILE));
    }
  if(cleanSharedName != cleanStaticName &&
     cleanSharedName != cleanSharedSOName &&
     cleanSharedName != cleanSharedRealName)
    {
    libCleanFiles.push_back
      (this->Convert(cleanFullSharedName.c_str(),START_OUTPUT,MAKEFILE));
    }
  }
  
  // Add a command to remove any existing files for this library.
  this->AppendCleanCommand(commands, libCleanFiles);

  // Add the pre-build and pre-link rules.
  this->AppendCustomCommands(commands, target.GetPreBuildCommands());
  this->AppendCustomCommands(commands, target.GetPreLinkCommands());

  // Construct the main link rule.
  std::string linkRule = m_Makefile->GetRequiredDefinition(linkRuleVar);
  std::vector<std::string> commands1;
  cmSystemTools::ExpandListArgument(linkRule, commands1);
  this->CreateCDCommand(commands1,m_Makefile->GetStartOutputDirectory(),
                        m_Makefile->GetHomeOutputDirectory());
  commands.insert(commands.end(), commands1.begin(), commands1.end());

  // Add a rule to create necessary symlinks for the library.
  if(targetOutPath != targetOutPathReal)
    {
    std::string symlink = "$(CMAKE_COMMAND) -E cmake_symlink_library ";
    symlink += targetOutPathReal;
    symlink += " ";
    symlink += targetOutPathSO;
    symlink += " ";
    symlink += targetOutPath;
    commands1.clear();
    commands1.push_back(symlink);
    this->CreateCDCommand(commands1,m_Makefile->GetStartOutputDirectory(),
                          m_Makefile->GetHomeOutputDirectory());
    commands.insert(commands.end(), commands1.begin(), commands1.end());
    }

  // Add the post-build rules.
  this->AppendCustomCommands(commands, target.GetPostBuildCommands());

  // Collect up flags to link in needed libraries.
  cmOStringStream linklibs;
  this->OutputLinkLibraries(linklibs, target.GetName(), target);

  // Construct object file lists that may be needed to expand the
  // rule.
  std::string variableName;
  std::string variableNameExternal;
  this->WriteObjectsVariable(ruleFileStream, target, objects, external_objects,
                             variableName, variableNameExternal);
  std::string buildObjs = "$(";
  buildObjs += variableName;
  buildObjs += ") $(";
  buildObjs += variableNameExternal;
  buildObjs += ")";
  std::string cleanObjs = "$(";
  cleanObjs += variableName;
  cleanObjs += ")";

  // Expand placeholders in the commands.
  for(std::vector<std::string>::iterator i = commands.begin();
      i != commands.end(); ++i)
    {
    this->ExpandRuleVariables(*i,
                              linkLanguage,
                              buildObjs.c_str(),
                              targetOutPathReal.c_str(),
                              linklibs.str().c_str(),
                              0, 0, 0, buildObjs.c_str(),
                              targetOutPathBase.c_str(),
                              targetNameSO.c_str(),
                              linkFlags.c_str());
    }

  // Write the build rule.
  this->WriteMakeRule(ruleFileStream, 0,
                      targetFullPathReal.c_str(), depends, commands);

  // The symlink names for the target should depend on the real target
  // so if the target version changes it rebuilds and recreates the
  // symlinks.
  if(targetFullPathSO != targetFullPathReal)
    {
    depends.clear();
    commands.clear();
    depends.push_back(targetFullPathReal.c_str());
    this->WriteMakeRule(ruleFileStream, 0,
                        targetFullPathSO.c_str(), depends, commands);
    }
  if(targetFullPath != targetFullPathSO)
    {
    depends.clear();
    commands.clear();
    depends.push_back(targetFullPathSO.c_str());
    this->WriteMakeRule(ruleFileStream, 0,
                        targetFullPath.c_str(), depends, commands);
    }

  // Write convenience targets.
  std::string dir = m_Makefile->GetStartOutputDirectory();
  dir += "/";
  dir += this->GetTargetDirectory(target);
  std::string buildTargetRuleName = dir;
  buildTargetRuleName += "/build";
  buildTargetRuleName = 
    this->Convert(buildTargetRuleName.c_str(),HOME_OUTPUT,MAKEFILE);
  this->WriteConvenienceRule(ruleFileStream, targetFullPath.c_str(),
                             buildTargetRuleName.c_str());

  // Clean all the possible library names and symlinks and object files.
  cleanFiles.insert(cleanFiles.end(),libCleanFiles.begin(),libCleanFiles.end());
  cleanFiles.push_back(cleanObjs);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteObjectsVariable(std::ostream& ruleFileStream,
                       cmTarget& target,
                       const std::vector<std::string>& objects,
                       const std::vector<std::string>& external_objects,
                       std::string& variableName,
                       std::string& variableNameExternal)
{
  // Write a make variable assignment that lists all objects for the
  // target.
  variableName = this->CreateMakeVariable(target.GetName(), "_OBJECTS");
  ruleFileStream
    << "# Object files for target " << target.GetName() << "\n"
    << variableName.c_str() << " =";
  std::string object;
  for(std::vector<std::string>::const_iterator i = objects.begin();
      i != objects.end(); ++i)
    {
    ruleFileStream
      << " \\\n"
      << this->ConvertToQuotedOutputPath(i->c_str());
    }
  ruleFileStream
    << "\n";

  // Write a make variable assignment that lists all external objects
  // for the target.
  variableNameExternal = this->CreateMakeVariable(target.GetName(),
                                                  "_EXTERNAL_OBJECTS");
  ruleFileStream
    << "\n"
    << "# External object files for target " << target.GetName() << "\n"
    << variableNameExternal.c_str() << " =";
  for(std::vector<std::string>::const_iterator i = external_objects.begin();
      i != external_objects.end(); ++i)
    {
    object = this->Convert(i->c_str(),START_OUTPUT);
    ruleFileStream
      << " \\\n"
      << this->ConvertToQuotedOutputPath(object.c_str());
    }
  ruleFileStream
    << "\n"
    << "\n";
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteTargetDependRule(std::ostream& ruleFileStream, cmTarget& target)
{
  // must write the targets depend info file
  std::string dir = this->GetTargetDirectory(target);
  std::string infoFileName = dir;
  infoFileName += "/DependInfo.cmake";
  std::string ruleFileNameFull = this->ConvertToFullPath(infoFileName);
  cmGeneratedFileStream infoFileStream(ruleFileNameFull.c_str());
  infoFileStream.SetCopyIfDifferent(true);
  if(!infoFileStream)
    {
    return;
    }
  this->WriteDependLanguageInfo(infoFileStream,target);
  
  // and now write the rule to use it
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // Construct the name of the dependency generation target.
  std::string depTarget = this->GetRelativeTargetDirectory(target);
  depTarget += "/depend";
  
  std::string depMark = depTarget;
  depMark += ".make.mark";
  depends.push_back(depMark);
  
  this->WriteMakeRule(ruleFileStream, 0,
                      depTarget.c_str(), depends, commands);
  depends.clear();
  
  // Write the dependency generation rule.
  std::string depEcho = "Scanning dependencies of target ";
  depEcho += target.GetName();
  this->AppendEcho(commands, depEcho.c_str());
  
  // Add a command to call CMake to scan dependencies.  CMake will
  // touch the corresponding depends file after scanning dependencies.
  cmOStringStream depCmd;
  // TODO: Account for source file properties and directory-level
  // definitions when scanning for dependencies.
  depCmd << "$(CMAKE_COMMAND) -E cmake_depends " 
         << " \""
         << m_GlobalGenerator->GetName() << "\" "
         << this->Convert(m_Makefile->GetHomeOutputDirectory(),FULL,SHELL)
         << " "
         << this->Convert(m_Makefile->GetStartOutputDirectory(),FULL,SHELL)
         << " "
         << this->Convert(ruleFileNameFull.c_str(),FULL,SHELL);
  commands.push_back(depCmd.str());
  
  // Write the rule.
  this->WriteMakeRule(ruleFileStream, 0,
                      depMark.c_str(), depends, commands);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteTargetCleanRule(std::ostream& ruleFileStream,
                       cmTarget& target,
                       const std::vector<std::string>& files)
{
  std::vector<std::string> no_depends;
  std::vector<std::string> commands;

  // Construct the clean target name.
  std::string cleanTarget = this->GetRelativeTargetDirectory(target);
  cleanTarget += "/clean";
  
  // Construct the clean command.
  this->AppendCleanCommand(commands, files);
  this->CreateCDCommand(commands,m_Makefile->GetStartOutputDirectory(),
                        m_Makefile->GetHomeOutputDirectory());
  // Write the rule.
  this->WriteMakeRule(ruleFileStream, 0,
                      cleanTarget.c_str(), no_depends, commands);
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
cmLocalUnixMakefileGenerator3::GetTargetDirectory(cmTarget& target)
{
  std::string dir = "CMakeFiles/";
  dir += target.GetName();
  dir += ".dir";
  return dir;
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
std::string
cmLocalUnixMakefileGenerator3
::GetSubdirTargetName(const char* pass, const char* subdir)
{
  // Convert the subdirectory name to a relative path to keep it short.
  std::string reldir = this->Convert(subdir,START_OUTPUT);

  // Convert the subdirectory name to a valid make target name.
  std::string s = pass;
  s += "_";
  s += reldir;

  // Replace "../" with 3 underscores.  This allows one .. at the beginning.
  size_t pos = s.find("../");
  if(pos != std::string::npos)
    {
    s.replace(pos, 3, "___");
    }

  // Replace "/" directory separators with a single underscore.
  while((pos = s.find('/')) != std::string::npos)
    {
    s.replace(pos, 1, "_");
    }

  // Replace ":" drive specifier with a single underscore
  while((pos = s.find(':')) != std::string::npos)
    {
    s.replace(pos, 1, "_");
    }

  return s;
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3
::GetObjectFileName(cmTarget& target,
                    const cmSourceFile& source)
{
  // If the full path to the source file includes this directory,
  // we want to use the relative path for the filename of the
  // object file.  Otherwise, we will use just the filename
  // portion.
  std::string objectName;
  if((cmSystemTools::GetFilenamePath(
        source.GetFullPath()).find(
          m_Makefile->GetCurrentDirectory()) == 0)
     || (cmSystemTools::GetFilenamePath(
           source.GetFullPath()).find(
             m_Makefile->GetStartOutputDirectory()) == 0))
    {
    objectName = source.GetSourceName();
    }
  else
    {
    objectName = cmSystemTools::GetFilenameName(source.GetSourceName());
    }

  // Append the object file extension.
  objectName +=
    m_GlobalGenerator->GetLanguageOutputExtensionFromExtension(
      source.GetSourceExtension().c_str());

  // Convert to a safe name.
  objectName = this->CreateSafeUniqueObjectFileName(objectName.c_str());

  // Prepend the target directory.
  std::string obj = this->GetTargetDirectory(target);
  obj += "/";
  obj += objectName;
  return obj;
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
void
cmLocalUnixMakefileGenerator3
::AppendTargetDepends(std::vector<std::string>& depends,
                      cmTarget& target)
{
  if(target.GetType() == cmTarget::STATIC_LIBRARY)
    {
    return;
    }

  // Keep track of dependencies already listed.
  std::set<cmStdString> emitted;

  // A target should not depend on itself.
  emitted.insert(target.GetName());

  // Loop over all library dependencies.
  const cmTarget::LinkLibraries& tlibs = target.GetLinkLibraries();
  for(cmTarget::LinkLibraries::const_iterator lib = tlibs.begin();
      lib != tlibs.end(); ++lib)
    {
    // Don't emit the same library twice for this target.
    if(emitted.insert(lib->first).second)
      {
      // Add this dependency.
      this->AppendAnyDepend(depends, lib->first.c_str());
      }
    }
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendAnyDepend(std::vector<std::string>& depends, const char* name,
                  bool assume_unknown_is_file)
{
  // There are a few cases for the name of the target:
  //  - CMake target.
  //  - Full path to a file: depend on it.
  //  - Other format (like -lm): do nothing unless assume_unknown_is_file is true.

  // Look for a CMake target in the current makefile.
  cmTarget* target = m_Makefile->FindTarget(name);

  // If no target was found in the current makefile search globally.
  bool local = target?true:false;
  if(!local)
    {
    target = m_GlobalGenerator->FindTarget(0, name);
    }

  // If a target was found then depend on it.
  if(target)
    {
    switch (target->GetType())
      {
      case cmTarget::EXECUTABLE:
      case cmTarget::STATIC_LIBRARY:
      case cmTarget::SHARED_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
        {
        // Get the location of the target's output file and depend on it.
        if(const char* location = target->GetProperty("LOCATION"))
          {
          depends.push_back(location);
          }
        }
        break;
      case cmTarget::UTILITY:
        {
        if(local)
          {
          // This is a utility target in the current makefile.  Just
          // depend on it directly.
          depends.push_back(name);
          }
        }
        break;
      case cmTarget::INSTALL_FILES:
      case cmTarget::INSTALL_PROGRAMS:
        // Do not depend on install targets.
        break;
      }
    }
  else if(cmSystemTools::FileIsFullPath(name))
    {
    // This is a path to a file.  Just trust the listfile author that
    // it will be present or there is a rule to build it.
    depends.push_back(cmSystemTools::CollapseFullPath(name));
    }
  else if(assume_unknown_is_file)
    {
    // Just assume this is a file or make target that will be present.
    depends.push_back(name);
    }
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
    // Add this dependency.
    this->AppendAnyDepend(depends, d->c_str());
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
        cmd += cmSystemTools::EscapeSpaces(commandLine[j].c_str());
        }
      
      commands1.push_back(cmd);
      }
    }

  // push back the custom commands
  this->CreateCDCommand(commands1,m_Makefile->GetStartOutputDirectory(),
                        m_Makefile->GetHomeOutputDirectory());
  commands.insert(commands.end(), commands1.begin(), commands1.end());
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendCleanCommand(std::vector<std::string>& commands,
                     const std::vector<std::string>& files)
{
  if(!files.empty())
    {
    std::string remove = "$(CMAKE_COMMAND) -E remove -f";
    for(std::vector<std::string>::const_iterator f = files.begin();
        f != files.end(); ++f)
      {
      remove += " ";
      remove += this->Convert(f->c_str(),START_OUTPUT,SHELL);
      }
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

//============================================================================
//----------------------------------------------------------------------------
bool
cmLocalUnixMakefileGenerator3::SamePath(const char* path1, const char* path2)
{
  if (strcmp(path1, path2) == 0)
    {
    return true;
    }
#if defined(_WIN32) || defined(__APPLE__)
  return
    (cmSystemTools::LowerCase(this->ConvertToOutputForExisting(path1)) ==
     cmSystemTools::LowerCase(this->ConvertToOutputForExisting(path2)));
#else
  return false;
#endif
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
  if ( m_Makefile->IsOn("CMAKE_MANGLE_OBJECT_FILE_NAMES") )
    {
    std::map<cmStdString,cmStdString>::iterator it = m_UniqueObjectNamesMap.find(sin);
    if ( it == m_UniqueObjectNamesMap.end() )
      {
      std::string ssin = sin;
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
      m_UniqueObjectNamesMap[sin] = ssin;
      }
    }
  else
    {
    m_UniqueObjectNamesMap[sin] = sin;
    }
  return m_UniqueObjectNamesMap[sin];
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
  if(!m_MakefileVariableSize && unmodified.find('.') == s.npos)
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
//============================================================================

//----------------------------------------------------------------------------
cmDepends*
cmLocalUnixMakefileGenerator3::GetDependsChecker(const std::string& lang,
                                                 bool verbose)
{
  cmDepends *ret = 0;
  if(lang == "C" || lang == "CXX" || lang == "RC")
    {
    ret = new cmDependsC();
    }
#ifdef CMAKE_BUILD_WITH_CMAKE
  else if(lang == "Fortran")
    {
    ret = new cmDependsFortran();
    }
  else if(lang == "Java")
    {
    ret = new cmDependsJava();
    }
#endif
  if (ret)
    {
    ret->SetVerbose(verbose);
    ret->SetFileComparison(m_GlobalGenerator->GetCMakeInstance()->GetFileComparison());
    }
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
  this->WriteSpecialTargetsTop(ruleFileStream);
  
  std::vector<std::string> depends;
  std::vector<std::string> commands;
  
  // Write the all rule.
  std::string dir = m_Makefile->GetStartOutputDirectory();
  dir += "/directorystart";
  dir = this->Convert(dir.c_str(),HOME_OUTPUT,MAKEFILE);
  // if at the top the rule is called all
  if (!m_Parent)
    {
    dir = "all";
    depends.push_back("cmake_check_build_system");
    }
  this->CreateJumpCommand(commands,"CMakeFiles/Makefile2",dir);
  this->WriteMakeRule(ruleFileStream, "The main all target", "all", depends, commands);

  // Write the clean rule.
  dir = m_Makefile->GetStartOutputDirectory();
  dir += "/clean";
  dir = this->Convert(dir.c_str(),HOME_OUTPUT,MAKEFILE);
  commands.clear();
  depends.clear();
  this->CreateJumpCommand(commands,"CMakeFiles/Makefile2",dir);
  this->WriteMakeRule(ruleFileStream, "The main clean target", "clean", depends, commands);

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
                      depends, commands);
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
      this->CreateJumpCommand(commands,tgtMakefileName.c_str(),targetName);
      }
    this->WriteMakeRule(ruleFileStream, 
                        "target for object file", 
                        lo->first.c_str(), depends, commands);
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
                          localName.c_str(), depends, commands);
      
      // Add a target with the canonical name (no prefix, suffix or path).
      if(localName != t->second.GetName())
        {
        commands.clear();
        depends.push_back(localName);
        this->WriteMakeRule(ruleFileStream, "Convenience name for target.",
                          t->second.GetName(), depends, commands);
        }
      }
    }
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
  
  if(m_WindowsShell)
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

void cmLocalUnixMakefileGenerator3
::CreateJumpCommand(std::vector<std::string>& commands,
                    const char *MakefileName, 
                    std::string& localName)
{
  // Build the target for this pass.
  commands.push_back(this->GetRecursiveMakeCall
                     (MakefileName,localName.c_str()));
  
  this->CreateCDCommand(commands,
                        m_Makefile->GetHomeOutputDirectory(),
                        m_Makefile->GetStartOutputDirectory());
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
