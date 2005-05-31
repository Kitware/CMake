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
#include "cmGlobalGenerator.h"
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
::WriteCustomCommands(const cmTarget &target,std::ostream& ruleFileStream,
                      std::vector<std::string>& cleanFiles)
{
  std::string tgtDir = m_Makefile->GetStartOutputDirectory();
  tgtDir += "/";
  tgtDir += this->GetTargetDirectory(target);

  // add custom commands to the clean rules?
  const char* clean_no_custom = m_Makefile->GetProperty("CLEAN_NO_CUSTOM");
  bool clean = cmSystemTools::IsOff(clean_no_custom);
  
  // Generate the rule files for each custom command.
  // get the classes from the source lists then add them to the groups
  const std::vector<cmSourceFile*> &classes = target.GetSourceFiles();
  std::string objTarget;
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin(); 
      i != classes.end(); i++)
    {
    if(const cmCustomCommand* cc = (*i)->GetCustomCommand())
      {
      objTarget = this->GenerateCustomRuleFile(*cc,tgtDir.c_str());
      if (clean)
        {
        cleanFiles.push_back
          (this->Convert(cc->GetOutput(),HOME_OUTPUT,SHELL));
        }
      ruleFileStream
        << m_IncludeDirective << " "
        << this->ConvertToOutputForExisting(objTarget.c_str()).c_str()
        << "\n";
      }
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteDirectoryInformationFile()
{
  std::string infoFileName = m_Makefile->GetStartOutputDirectory();
  infoFileName += "/CMakeDirectoryInformation.cmake";

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
    infoFileStream
      << "  \"" << this->Convert(i->c_str(),START_OUTPUT).c_str() << "\"\n";
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

std::string cmLocalUnixMakefileGenerator3::GetHomeRelativeOutputPath()
{
  // Include the rule file for each object.
  std::string relPath = 
    cmSystemTools::RelativePath(m_Makefile->GetHomeOutputDirectory(),
                                m_Makefile->GetStartOutputDirectory());
  if (relPath.size())
    {
    relPath += "/";
    }
  return relPath;
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteTargetRuleFiles(const cmTarget& target)
{
  // Create a directory for this target.
  std::string dir = this->GetTargetDirectory(target);
  cmSystemTools::MakeDirectory(this->ConvertToFullPath(dir).c_str());

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
        this->WriteObjectRuleFiles(target, *(*source), objects);
        }
      else if((*source)->GetPropertyAsBool("EXTERNAL_OBJECT"))
        {
        // This is an external object file.  Just add it.
        external_objects.push_back((*source)->GetFullPath());
        }
      }
    }

  // Generate the build-time dependencies file for this target.
  std::string depBase = dir;
  depBase += "/";
  depBase += target.GetName();

  // Construct the rule file name.
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
  
  // write the custom commands for this target
  std::vector<std::string> cleanFiles;
  // Look for files registered for cleaning in this directory.
  if(const char* additional_clean_files =
     m_Makefile->GetProperty("ADDITIONAL_MAKE_CLEAN_FILES"))
    {
    cmSystemTools::ExpandListArgument(additional_clean_files, cleanFiles);
    }  
  this->WriteCustomCommands(target,ruleFileStream,cleanFiles);

  // Include the rule file for each object.
  std::string relPath = this->GetHomeRelativeOutputPath();
  std::string objTarget;
  if(!objects.empty())
    {
    ruleFileStream
      << "# Include make rules for object files.\n";
    for(std::vector<std::string>::const_iterator obj = objects.begin();
        obj != objects.end(); ++obj)
      {
      objTarget = relPath;
      objTarget += *obj;
      objTarget += ".build.make";
      ruleFileStream
        << m_IncludeDirective << " "
        << this->ConvertToOutputForExisting(objTarget.c_str()).c_str()
        << "\n";
      }
    ruleFileStream
      << "\n";
    }

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
::WriteObjectDependRules(std::ostream& ruleFileStream,
                         std::string &obj,
                         const char * lang,
                         const cmSourceFile& source,
                         std::vector<std::string>& depends,
                         std::string& depMakeFile)
{
  // Generate the build-time dependencies file for this object file.
  std::string depMarkFile;
  if(!this->GenerateDependsMakeFile(lang, obj.c_str(),
                                    depMakeFile, depMarkFile))
    {
    cmSystemTools::Error("No dependency checker available for language \"",
                         lang, "\".");
    return;
    }

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
  
  // Write the dependency generation rule.
  std::string relativeObj = this->GetHomeRelativeOutputPath();
  relativeObj += obj;
  std::vector<std::string> commands;
  std::string depEcho = "Scanning ";
  depEcho += lang;
  depEcho += " dependencies of ";
  depEcho += this->Convert(relativeObj.c_str(),NONE,SHELL);
  this->AppendEcho(commands, depEcho.c_str());
  
  // Add a command to call CMake to scan dependencies.  CMake will
  // touch the corresponding depends file after scanning dependencies.
  cmOStringStream depCmd;
  // TODO: Account for source file properties and directory-level
  // definitions when scanning for dependencies.
  depCmd << "$(CMAKE_COMMAND) -E cmake_depends \"" 
         << m_GlobalGenerator->GetName() << "\" "
         << lang << " "
         << relativeObj.c_str() << " "
         << this->Convert(source.GetFullPath().c_str(),HOME_OUTPUT,SHELL);
  commands.push_back(depCmd.str());
  
  // compute the target
  std::string relPath = this->GetHomeRelativeOutputPath();
  relPath += depMarkFile;
  // Write the rule.
  this->WriteMakeRule(ruleFileStream, 0,
                      relPath.c_str(), depends, commands);
}


//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteObjectBuildFile(std::string &obj,
                       const char *lang, 
                       const cmTarget& target, 
                       const cmSourceFile& source,
                       std::vector<std::string>& depends,
                       std::string &depMakeFile)
{
  // Open the rule file for writing.  This should be copy-if-different
  // because the rules may depend on this file itself.
  std::string ruleFileName = obj;
  ruleFileName += ".build.make";
  std::string ruleFileNameFull = this->ConvertToFullPath(ruleFileName);
  cmGeneratedFileStream ruleFileStream(ruleFileNameFull.c_str());
  ruleFileStream.SetCopyIfDifferent(true);
  if(!ruleFileStream)
    {
    return;
    }
  this->WriteDisclaimer(ruleFileStream);
  ruleFileStream
    << "# Rule file for object file " << obj.c_str() << ".\n\n";

  // generate the depend scanning rule
  this->WriteObjectDependRules(ruleFileStream, obj, lang, source, 
                               depends, depMakeFile);

  this->AppendRuleDepend(depends, ruleFileNameFull.c_str());

  // Include the dependencies for the target.
  std::string depPath = this->GetHomeRelativeOutputPath();
  depPath += depMakeFile;
  depMakeFile = this->Convert(depPath.c_str(),HOME_OUTPUT,MAKEFILE);
  ruleFileStream
    << "# Include any dependencies generated for this rule.\n"
    << m_IncludeDirective << " "
    << depMakeFile
    << "\n\n";

  // Write the build rule.
  // Build the set of compiler flags.
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

  // Add flags from source file properties.
  this->AppendFlags(flags, source.GetProperty("COMPILE_FLAGS"));

  // Add language-specific flags.
  this->AddLanguageFlags(flags, lang);

  // Add shared-library flags if needed.
  this->AddSharedFlags(flags, lang, shared);

  // Add include directory flags.
  this->AppendFlags(flags, this->GetIncludeFlags(lang));

  // Get the output paths for source and object files.
  std::string sourceFile = source.GetFullPath();
  if(m_UseRelativePaths)
    {
    sourceFile = this->Convert(sourceFile.c_str(),HOME_OUTPUT);
    }
  sourceFile = this->Convert(sourceFile.c_str(),NONE,SHELL);
  std::string objectFile = this->Convert(obj.c_str(),START_OUTPUT,SHELL);

  // Construct the build message.
  std::string relativeObj = this->GetHomeRelativeOutputPath();
  relativeObj += obj;
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

  // Write the rule.
  this->WriteMakeRule(ruleFileStream, 0,
                      relativeObj.c_str(), depends, commands);

  // If the language needs provides-requires mode, create the
  // corresponding targets.
  std::string objectRequires = relativeObj;
  objectRequires += ".requires";
  std::string temp = relativeObj;
  temp += ".depend";
  std::vector<std::string> no_commands;
  std::vector<std::string> p_depends;
  p_depends.push_back(temp);
  // always provide an empty requires target
  this->WriteMakeRule(ruleFileStream, 0,
                      objectRequires.c_str(), p_depends, no_commands);

  // write a build rule to recursively build what this obj provides
  std::string objectProvides = relativeObj;
  objectProvides += ".provides";
  temp = relativeObj;
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
::WriteObjectRuleFiles(const cmTarget& target, const cmSourceFile& source,
                       std::vector<std::string>& objects)
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
  this->WriteObjectBuildFile(obj, lang, target, source, depends, depMakeFile);
  
  // The object file should be checked for dependency integrity.
  m_CheckDependFiles[lang].insert(relativeObj);

}

//----------------------------------------------------------------------------
std::string 
cmLocalUnixMakefileGenerator3
::GenerateCustomRuleFile(const cmCustomCommand& cc, const char *dir)
{
  // Convert the output name to a relative path if possible.
  std::string output = this->Convert(cc.GetOutput(),START_OUTPUT);

  // Construct the name of the rule file by transforming the output
  // name to a valid file name.  Since the output is already a file
  // everything but the path characters is valid.
  std::string customName = output;
  cmSystemTools::ReplaceString(customName, "../", "___");
  cmSystemTools::ReplaceString(customName, "/", "_");
  cmSystemTools::ReplaceString(customName, ":", "_");
  std::string ruleFileName = dir;
  ruleFileName += "/";
  ruleFileName += customName;
  ruleFileName += ".build.make";

  // what is the relative path to the rule file
  std::string relRuleFile = this->Convert(ruleFileName.c_str(),HOME_OUTPUT);
  
  // Open the rule file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  cmGeneratedFileStream ruleFileStream(ruleFileName.c_str());
  ruleFileStream.SetCopyIfDifferent(true);
  if(!ruleFileStream)
    {
    return relRuleFile;
    }
  this->WriteDisclaimer(ruleFileStream);
  ruleFileStream
    << "# Custom command rule file for " << output.c_str() << ".\n\n";

  // Collect the commands.
  std::vector<std::string> commands;
  std::string preEcho = "Generating ";
  preEcho += output;
  this->AppendEcho(commands, preEcho.c_str());
  this->AppendCustomCommand(commands, cc);
  
  // Collect the dependencies.
  std::vector<std::string> depends;
  this->AppendCustomDepend(depends, cc);

  // Add a dependency on the rule file itself.
  this->AppendRuleDepend(depends, relRuleFile.c_str());

  // Write the rule.
  const char* comment = 0;
  if(cc.GetComment() && *cc.GetComment())
    {
    comment = cc.GetComment();
    }
  this->WriteMakeRule(ruleFileStream, comment,
                      cc.GetOutput(), depends, commands);

  return relRuleFile;
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteUtilityRuleFiles(const cmTarget& target)
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
  this->WriteCustomCommands(target,ruleFileStream, cleanFiles);

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
::GenerateDependsMakeFile(const std::string& lang, const char* objFile,
                          std::string& depMakeFile, std::string& depMarkFile)
{
  // Construct a checker for the given language.
  std::auto_ptr<cmDepends>
    checker(this->GetDependsChecker(lang,
                                    m_Makefile->GetStartOutputDirectory(),
                                    objFile, false));
  if(checker.get())
    {
    // Save the make and mark file names.
    depMakeFile = checker->GetMakeFileName();
    depMarkFile = checker->GetMarkFileName();

    // Check the dependencies. Ths is required because we need at least an
    // empty foo.obj.depends.make for make to include, so at cmake time the
    // ::Check() method will generate that if it does not exist
    checker->Check();
    
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
    cmSystemTools::Error("No target for WriteMakeRule!");
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
    this->ConvertToOutputForExisting(
      m_Makefile->GetRequiredDefinition("CMAKE_COMMAND"));
  makefileStream
    << "# The CMake executable.\n"
    << "CMAKE_COMMAND = "
    << this->Convert(cmakecommand.c_str(),HOME_OUTPUT,MAKEFILE).c_str() << "\n"
    << "\n";
  makefileStream
    << "# The command to remove a file.\n"
    << "RM = "
    << this->Convert(cmakecommand.c_str(),HOME_OUTPUT,SHELL).c_str()
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
    << this->Convert(m_Makefile->GetHomeDirectory(), HOME_OUTPUT, SHELL)
    << "\n"
    << "\n";
  makefileStream
    << "# The top-level build directory on which CMake was run.\n"
    << "CMAKE_BINARY_DIR = ."
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
    std::string cmd = ctest;
    cmd += " $(ARGS)";
    commands.push_back(cmd);
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
  std::string cmakefileName = "Makefile.cmake";
  std::string runRule =
    "$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)";
  runRule += " --check-build-system ";
  runRule += this->Convert(cmakefileName.c_str(),NONE,SHELL);

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

  // Write special target to silence make output.  This must be after
  // the default target in case VERBOSE is set (which changes the
  // name).  The setting of CMAKE_VERBOSE_MAKEFILE to ON will cause a
  // "VERBOSE=1" to be added as a make variable which will change the
  // name of this special target.  This gives a make-time choice to
  // the user.
  std::vector<std::string> no_depends;
  this->WriteMakeRule(makefileStream,
                      "Suppress display of executed commands.",
                      "$(VERBOSE).SILENT",
                      no_depends,
                      no_commands);

  // Special target to cleanup operation of make tool.
  std::vector<std::string> depends;
  this->WriteMakeRule(makefileStream,
                      "Disable implicit rules so canoncical targets will work.",
                      ".SUFFIXES",
                      depends,
                      no_commands);
  // Add a fake suffix to keep HP happy.  Must be max 32 chars for SGI make.
  depends.push_back(".hpux_make_needs_suffix_list");
  this->WriteMakeRule(makefileStream, 0,
                      ".SUFFIXES", depends, no_commands);
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
::WriteTargetRequiresRule(std::ostream& ruleFileStream, const cmTarget& target,
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
                      const cmTarget& target,
                      const std::vector<std::string>& objects,
                      const std::vector<std::string>& external_objects,
                      std::vector<std::string>& cleanFiles)
{
  // Write the dependency generation rule.
  this->WriteTargetDependRule(ruleFileStream, target, objects);

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
  objTarget = relPath;
  objTarget += ruleFileName;
  this->AppendRuleDepend(depends, objTarget.c_str());

  // Construct the full path to the executable that will be generated.
  std::string targetFullPath = m_ExecutableOutputPath;
  if(targetFullPath.length() == 0)
    {
    targetFullPath = m_Makefile->GetStartOutputDirectory();
    targetFullPath += "/";
    }
#ifdef __APPLE__
  if(target.GetPropertyAsBool("MACOSX_BUNDLE"))
    {
    // Make bundle directories
    targetFullPath += target.GetName();
    targetFullPath += ".app/Contents/MacOS/";
    }
#endif
  targetFullPath += target.GetName();
  targetFullPath += cmSystemTools::GetExecutableExtension();

  // Convert to the output path to use in constructing commands.
  std::string targetOutPath = this->Convert(targetFullPath.c_str(),HOME_OUTPUT,MAKEFILE);

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

  // Add flags to deal with shared libraries.  Any library being
  // linked in might be shared, so always use shared flags for an
  // executable.
  this->AddSharedFlags(flags, linkLanguage, true);

  // Add target-specific linker flags.
  this->AppendFlags(linkFlags, target.GetProperty("LINK_FLAGS"));

  // Add the pre-build and pre-link rules.
  this->AppendCustomCommands(commands, target.GetPreBuildCommands());
  this->AppendCustomCommands(commands, target.GetPreLinkCommands());

  // Construct the main link rule.
  std::string linkRuleVar = "CMAKE_";
  linkRuleVar += linkLanguage;
  linkRuleVar += "_LINK_EXECUTABLE";
  std::string linkRule = m_Makefile->GetRequiredDefinition(linkRuleVar.c_str());
  cmSystemTools::ExpandListArgument(linkRule, commands);

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
                              targetOutPath.c_str(),
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
                      targetFullPath.c_str(), depends, commands);

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

  cleanFiles.push_back(cleanObjs);
  cleanFiles.push_back(targetOutPath.c_str());
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteStaticLibraryRule(std::ostream& ruleFileStream,
                         const char* ruleFileName,
                         const cmTarget& target,
                         const std::vector<std::string>& objects,
                         const std::vector<std::string>& external_objects,
                         std::vector<std::string>& cleanFiles)
{
  const char* linkLanguage =
    target.GetLinkerLanguage(this->GetGlobalGenerator());
  std::string linkRuleVar = "CMAKE_";
  linkRuleVar += linkLanguage;
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
                         const cmTarget& target,
                         const std::vector<std::string>& objects,
                         const std::vector<std::string>& external_objects,
                         std::vector<std::string>& cleanFiles)
{
  const char* linkLanguage =
    target.GetLinkerLanguage(this->GetGlobalGenerator());
  std::string linkRuleVar = "CMAKE_";
  linkRuleVar += linkLanguage;
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
          this->Convert((*i)->GetFullPath().c_str(),HOME_OUTPUT,MAKEFILE);
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
                         const cmTarget& target,
                         const std::vector<std::string>& objects,
                         const std::vector<std::string>& external_objects,
                         std::vector<std::string>& cleanFiles)
{
  const char* linkLanguage =
    target.GetLinkerLanguage(this->GetGlobalGenerator());
  std::string linkRuleVar = "CMAKE_";
  linkRuleVar += linkLanguage;
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
                   const cmTarget& target,
                   const std::vector<std::string>& objects,
                   const std::vector<std::string>& external_objects,
                   const char* linkRuleVar,
                   const char* extraFlags,
                   std::vector<std::string>& cleanFiles)
{
  // Write the dependency generation rule.
  this->WriteTargetDependRule(ruleFileStream, target, objects);

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
  objTarget = relPath;
  objTarget += ruleFileName;
  this->AppendRuleDepend(depends, objTarget.c_str());

  // from here up is the same for exe or lib

  for(std::vector<std::string>::const_iterator obj = external_objects.begin();
      obj != external_objects.end(); ++obj)
    {
    depends.push_back(*obj);
    }

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
  target.GetLibraryNames(m_Makefile,
                         targetName, targetNameSO,
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
    this->Convert(targetFullPath.c_str(),HOME_OUTPUT,MAKEFILE);
  std::string targetOutPathSO = 
    this->Convert(targetFullPathSO.c_str(),HOME_OUTPUT,MAKEFILE);
  std::string targetOutPathReal = 
    this->Convert(targetFullPathReal.c_str(),HOME_OUTPUT,MAKEFILE);
  std::string targetOutPathBase = 
    this->Convert(targetFullPathBase.c_str(),HOME_OUTPUT,MAKEFILE);

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
  target.GetLibraryCleanNames(m_Makefile,
                              cleanStaticName,
                              cleanSharedName,
                              cleanSharedSOName,
                              cleanSharedRealName);
  std::string cleanFullStaticName = outpath + cleanStaticName;
  std::string cleanFullSharedName = outpath + cleanSharedName;
  std::string cleanFullSharedSOName = outpath + cleanSharedSOName;
  std::string cleanFullSharedRealName = outpath + cleanSharedRealName;
  libCleanFiles.push_back(cleanFullStaticName);
  if(cleanSharedRealName != cleanStaticName)
    {
    libCleanFiles.push_back(cleanFullSharedRealName);
    }
  if(cleanSharedSOName != cleanStaticName &&
     cleanSharedSOName != cleanSharedRealName)
    {
    libCleanFiles.push_back(cleanFullSharedSOName);
    }
  if(cleanSharedName != cleanStaticName &&
     cleanSharedName != cleanSharedSOName &&
     cleanSharedName != cleanSharedRealName)
    {
    libCleanFiles.push_back(cleanFullSharedName);
    }
  }

  // Add a command to remove any existing files for this library.
  this->AppendCleanCommand(commands, libCleanFiles);

  // add the libCleanFiles to to taoal cleanFiles
  cleanFiles.insert(cleanFiles.end(),libCleanFiles.begin(),libCleanFiles.end());
    
  // Add the pre-build and pre-link rules.
  this->AppendCustomCommands(commands, target.GetPreBuildCommands());
  this->AppendCustomCommands(commands, target.GetPreLinkCommands());

  // Construct the main link rule.
  std::string linkRule = m_Makefile->GetRequiredDefinition(linkRuleVar);
  cmSystemTools::ExpandListArgument(linkRule, commands);

  // Add a rule to create necessary symlinks for the library.
  if(targetOutPath != targetOutPathReal)
    {
    std::string symlink = "$(CMAKE_COMMAND) -E cmake_symlink_library ";
    symlink += targetOutPathReal;
    symlink += " ";
    symlink += targetOutPathSO;
    symlink += " ";
    symlink += targetOutPath;
    commands.push_back(symlink);
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

  // from here down is the same for exe or lib
  
  // Write the build rule.
  this->WriteMakeRule(ruleFileStream, 0, 
                     targetFullPath.c_str(), depends, commands);

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

  cleanFiles.push_back(cleanObjs);
  cleanFiles.push_back(targetOutPath.c_str());
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteObjectsVariable(std::ostream& ruleFileStream,
                       const cmTarget& target,
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
  std::string relPath = this->GetHomeRelativeOutputPath();
  std::string object;
  for(std::vector<std::string>::const_iterator i = objects.begin();
      i != objects.end(); ++i)
    {
    object = relPath;
    object += *i;
    ruleFileStream
      << " \\\n"
      << this->ConvertToQuotedOutputPath(object.c_str());
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
    object = this->Convert(i->c_str(),HOME_OUTPUT);
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
::WriteTargetDependRule(std::ostream& ruleFileStream,
                        const cmTarget& target,
                        const std::vector<std::string>& objects)
{
  std::vector<std::string> depends;
  std::vector<std::string> no_commands;

  // Construct the name of the dependency generation target.
  std::string depTarget = this->GetRelativeTargetDirectory(target);
  depTarget += "/depend";

  // This target drives dependency generation for all object files.
  std::string relPath = this->GetHomeRelativeOutputPath();
  std::string objTarget;
  for(std::vector<std::string>::const_iterator obj = objects.begin();
      obj != objects.end(); ++obj)
    {
    objTarget = relPath;
    objTarget += *obj;
    objTarget += ".depend";
    depends.push_back(objTarget);
    }

  // Write the rule.
  this->WriteMakeRule(ruleFileStream, 0,
                      depTarget.c_str(), depends, no_commands);
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::WriteTargetCleanRule(std::ostream& ruleFileStream,
                       const cmTarget& target,
                       const std::vector<std::string>& files)
{
  std::vector<std::string> no_depends;
  std::vector<std::string> commands;

  // Construct the clean target name.
  std::string cleanTarget = this->GetRelativeTargetDirectory(target);
  cleanTarget += "/clean";
  
  // Construct the clean command.
  this->AppendCleanCommand(commands, files);

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
cmLocalUnixMakefileGenerator3::GetTargetDirectory(const cmTarget& target)
{
  std::string dir = target.GetName();
  dir += ".dir";
  return dir;
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator3::GetRelativeTargetDirectory(const cmTarget& target)
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
::GetObjectFileName(const cmTarget& target,
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
                      const cmTarget& target)
{
  // Do not bother with dependencies for static libraries.
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

  // Loop over all utility dependencies.

  // Ken --- we trust that the parent build system handled the utility
  // targets, really we trust that it also handled the libs but there is no
  // harm in listing the libs as depends, if the libs are not present they
  // cannot be built (the rules are not there) but at least it will squak
#if 0
  const std::set<cmStdString>& tutils = target.GetUtilities();
  for(std::set<cmStdString>::const_iterator util = tutils.begin();
      util != tutils.end(); ++util)
    {
    // Don't emit the same utility twice for this target.
    if(emitted.insert(*util).second)
      {
      // Add this dependency.
      this->AppendAnyDepend(depends, util->c_str());
      }
    }
#endif
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator3
::AppendAnyDepend(std::vector<std::string>& depends, const char* name,
                  bool assume_unknown_is_file)
{
  // There are a few cases for the name of the target:
  //  - CMake target in this directory: depend on it.
  //  - CMake target in another directory: depend and add jump-and-build.
  //  - Full path to a file: depend on it.
  //  - Other format (like -lm): do nothing.

  // If it is an executable or library target there will be a
  // definition for it.
  std::string dirVar = name;
  dirVar += "_CMAKE_PATH";
  const char* dir = m_Makefile->GetDefinition(dirVar.c_str());
  if(dir && *dir)
    {
    // This is a CMake target somewhere in this project.
    // Get the type of the library.  If it does not have a type then
    // it is an executable.
    std::string typeVar = name;
    typeVar += "_LIBRARY_TYPE";
    const char* libType = m_Makefile->GetDefinition(typeVar.c_str());
    
    // Get the output path for this target type.
    std::string tgtOutputPath;
    if(libType)
      {
      tgtOutputPath = m_LibraryOutputPath;
      }
    else
      {
      tgtOutputPath = m_ExecutableOutputPath;
      }

    // Get the path to the target.
    std::string tgtPath;
    if(tgtOutputPath.size())
      {
      tgtPath = tgtOutputPath;
      }
    else
      {
      tgtPath = dir;
      tgtPath += "/";
      }

    // Add the name of the targets's file.  This depends on the type
    // of the target.
    std::string prefix;
    std::string suffix;
    if(!libType)
      {
      suffix = cmSystemTools::GetExecutableExtension();
      }
    else if(strcmp(libType, "SHARED") == 0)
      {
      prefix = m_Makefile->GetSafeDefinition("CMAKE_SHARED_LIBRARY_PREFIX");
      suffix = m_Makefile->GetSafeDefinition("CMAKE_SHARED_LIBRARY_SUFFIX");
      }
    else if(strcmp(libType, "MODULE") == 0)
      {
      prefix = m_Makefile->GetSafeDefinition("CMAKE_SHARED_MODULE_PREFIX");
      suffix = m_Makefile->GetSafeDefinition("CMAKE_SHARED_MODULE_SUFFIX");
      }
    else if(strcmp(libType, "STATIC") == 0)
      {
      prefix = m_Makefile->GetSafeDefinition("CMAKE_STATIC_LIBRARY_PREFIX");
      suffix = m_Makefile->GetSafeDefinition("CMAKE_STATIC_LIBRARY_SUFFIX");
      }
    tgtPath += prefix;
    tgtPath += name;
    tgtPath += suffix;

    // Add a dependency on the target.
    depends.push_back(tgtPath.c_str());
    }
  else if(m_Makefile->GetTargets().find(name) !=
          m_Makefile->GetTargets().end())
    {
    // This is a CMake target that is not an executable or library.
    // It must be in this directory, so just depend on the name
    // directly.
    depends.push_back(name);
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
    this->AppendAnyDepend(depends, d->c_str(), true);
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
  // TODO: Convert outputs/dependencies (arguments?) to relative paths.
  
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
  
  // stick this group of commands into a cd of the proper path
  // Build the jump-and-build command list.
  if(m_WindowsShell)
    {
    // On Windows we must perform each step separately and then jump
    // back because the shell keeps the working directory between
    // commands.
    std::string cmd = "cd ";
    cmd += this->ConvertToOutputForExisting(m_Makefile->GetStartOutputDirectory());
    commands.push_back(cmd);

    // push back the custom commands
    commands.insert(commands.end(), commands1.begin(), commands1.end());
    
    // Jump back to the home directory.
    cmd = "cd ";
    cmd += this->ConvertToOutputForExisting(m_Makefile->GetHomeOutputDirectory());
    commands.push_back(cmd);
    }
  else
    {
    // On UNIX we must construct a single shell command to jump and
    // build because make resets the directory between each command.
    std::string cmd = "cd ";
    cmd += this->ConvertToOutputForExisting(m_Makefile->GetStartOutputDirectory());
    
    // add the commands
    unsigned int i;
    for (i = 0; i < commands1.size(); ++i)
      {
      cmd += " && ";
      cmd += commands1[i];
      }
    
    // Add the command as a single line.
    commands.push_back(cmd);
    }
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
                                                 const char* dir,
                                                 const char* objFile,
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
    ret->SetTargetFile(dir, objFile, ".depend",".build.depend.make");
    ret->SetCompileDirectory(m_Makefile->GetHomeOutputDirectory());
    ret->SetVerbose(verbose);
    }
  return ret;
}

//----------------------------------------------------------------------------
bool
cmLocalUnixMakefileGenerator3
::ScanDependencies(std::vector<std::string> const& args)
{
  // Format of arguments is:
  // $(CMAKE_COMMAND), cmake_depends, GeneratorName, <lang>, <obj>, <src>
  // The caller has ensured that all required arguments exist.

  // The language for which we are scanning dependencies.
  std::string const& lang = args[3];

  // The file to which to write dependencies.
  const char* objFile = args[4].c_str();

  // The source file at which to start the scan.
  const char* srcFile = args[5].c_str();

  // Read the directory information file.
  cmake cm;
  cmGlobalGenerator gg;
  gg.SetCMakeInstance(&cm);
  std::auto_ptr<cmLocalGenerator> lg(gg.CreateLocalGenerator());
  lg->SetGlobalGenerator(&gg);
  cmMakefile* mf = lg->GetMakefile();
  bool haveDirectoryInfo = false;
  if(mf->ReadListFile(0, "CMakeDirectoryInformation.cmake") &&
     !cmSystemTools::GetErrorOccuredFlag())
    {
    haveDirectoryInfo = true;
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

  // Dispatch the scan for each language.
  if(lang == "C" || lang == "CXX" || lang == "RC")
    {
    // TODO: Handle RC (resource files) dependencies correctly.
    cmDependsC scanner(srcFile, includes,
                       includeRegexScan.c_str(), includeRegexComplain.c_str());
    scanner.SetTargetFile(".",objFile,".depend",".build.depend.make");
    return scanner.Write();
    }
#ifdef CMAKE_BUILD_WITH_CMAKE
  else if(lang == "Fortran")
    {
    cmDependsFortran scanner(srcFile, includes);
    scanner.SetTargetFile(".",objFile,".depend",".build.depend.make");
    return scanner.Write();
    }
  else if(lang == "Java")
    {
    cmDependsJava scanner(srcFile);
    scanner.SetTargetFile(".",objFile,".depend",".build.depend.make");
    return scanner.Write();
    }
#endif
  return false;
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::WriteLocalMakefile()
{
  // only write the local Makefile if we are not at the top
  if (!m_Parent)
    {
    return;
    }
    
  // generate the includes
  std::string ruleFileName = "Makefile";
  
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
  
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // Write the all rule.
  std::string dir = m_Makefile->GetStartOutputDirectory();
  dir += "/directorystart";
  dir = this->Convert(dir.c_str(),HOME_OUTPUT,MAKEFILE);
  this->CreateJumpCommand(commands,dir);
  this->WriteMakeRule(ruleFileStream, "The main all target", "all", depends, commands);

  // Write the clean rule.
  dir = m_Makefile->GetStartOutputDirectory();
  dir += "/clean";
  dir = this->Convert(dir.c_str(),HOME_OUTPUT,MAKEFILE);
  commands.clear();
  this->CreateJumpCommand(commands,dir);
  this->WriteMakeRule(ruleFileStream, "The main clean target", "clean", depends, commands);

  // recursively write our targets
  this->WriteLocalMakefileTargets(ruleFileStream);
}

void cmLocalUnixMakefileGenerator3
::WriteLocalMakefileTargets(std::ostream& ruleFileStream)
{
  std::vector<std::string> depends;
  std::vector<std::string> commands;

  // for each target we just provide a rule to cd up to the top and do a make
  // on the target
  const cmTargets& targets = m_Makefile->GetTargets();
  std::string localName;
  for(cmTargets::const_iterator t = targets.begin(); t != targets.end(); ++t)
    {
    if((t->second.GetType() == cmTarget::EXECUTABLE) ||
       (t->second.GetType() == cmTarget::STATIC_LIBRARY) ||
       (t->second.GetType() == cmTarget::SHARED_LIBRARY) ||
       (t->second.GetType() == cmTarget::MODULE_LIBRARY))
      {
      // Add a rule to build the target by name.
      localName = this->GetRelativeTargetDirectory(t->second);
      commands.clear();
      depends.clear();
      
      this->CreateJumpCommand(commands,localName);
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

void cmLocalUnixMakefileGenerator3::CreateJumpCommand(std::vector<std::string>& commands,
                                                      std::string& localName)
{
  if(m_WindowsShell)
    {
    // On Windows we must perform each step separately and then change
    // back because the shell keeps the working directory between
    // commands.
    std::string cmd = "cd ";
    cmd += this->ConvertToOutputForExisting
      (m_Makefile->GetHomeOutputDirectory());
    commands.push_back(cmd);
    
    // Build the target for this pass.
    commands.push_back(this->GetRecursiveMakeCall
                       ("Makefile2",localName.c_str()));
    
    // Change back to the starting directory.  Any trailing slash must be
    // removed to avoid problems with Borland Make.
    std::string back =
      cmSystemTools::RelativePath(m_Makefile->GetHomeOutputDirectory(),
                                  m_Makefile->GetStartOutputDirectory());
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
    std::string cmd = "cd ";
    cmd += this->ConvertToOutputForExisting(m_Makefile->GetHomeOutputDirectory());
    
    // Build the target for this pass.
    cmd += " && ";
    cmd += this->GetRecursiveMakeCall("Makefile2",localName.c_str());
    
    // Add the command as a single line.
    commands.push_back(cmd);
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator3::CheckDependencies(cmMakefile* mf, 
                                                      bool verbose)
{
  // Get the list of languages that may have sources to check.
  const char* langDef = mf->GetDefinition("CMAKE_DEPENDS_LANGUAGES");
  if(!langDef)
    {
    return;
    }
  std::vector<std::string> languages;
  cmSystemTools::ExpandListArgument(langDef, languages);

  // For each language get the set of files to check.
  for(std::vector<std::string>::iterator l = languages.begin();
      l != languages.end(); ++l)
    {
    std::string depCheck = "CMAKE_DEPENDS_CHECK_";
    depCheck += *l;
    if(const char* fileDef = mf->GetDefinition(depCheck.c_str()))
      {
      // Check each file.  The current working directory is already
      // correct.
      std::vector<std::string> files;
      cmSystemTools::ExpandListArgument(fileDef, files);
      for(std::vector<std::string>::iterator f = files.begin();
          f != files.end(); ++f)
        {
        // Construct a checker for the given language.
        std::auto_ptr<cmDepends>
          checker(this->GetDependsChecker(*l, ".", f->c_str(), verbose));
        if(checker.get())
          {
          checker->Check();
          }
        }
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

