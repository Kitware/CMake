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
#include "cmLocalUnixMakefileGenerator2.h"

#include "cmGeneratedFileStream.h"
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"

#include <queue>

//----------------------------------------------------------------------------
cmLocalUnixMakefileGenerator2::cmLocalUnixMakefileGenerator2()
{
}

//----------------------------------------------------------------------------
cmLocalUnixMakefileGenerator2::~cmLocalUnixMakefileGenerator2()
{
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator2::Generate(bool fromTheTop)
{
  // TODO: Account for control-c during Makefile generation.

  // Generate old style for now.
  this->cmLocalUnixMakefileGenerator::Generate(fromTheTop);

  // Generate the rule files for each target.
  const cmTargets& targets = m_Makefile->GetTargets();
  for(cmTargets::const_iterator t = targets.begin(); t != targets.end(); ++t)
    {
    this->GenerateTargetRuleFile(t->second);
    }

  // Generate the main makefile.
  this->GenerateMakefile();

  // Generate the cmake file that keeps the makefile up to date.
  this->GenerateCMakefile();
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator2::GenerateMakefile()
{
  std::string makefileName = m_Makefile->GetStartOutputDirectory();
  makefileName += "/Makefile2";
  std::string cmakefileName = makefileName;
  cmakefileName += ".cmake";

  // Open the output files.
  std::ofstream makefileStream(makefileName.c_str());
  if(!makefileStream)
    {
    cmSystemTools::Error("Error can not open for write: ",
                         makefileName.c_str());
    cmSystemTools::ReportLastSystemError("");
    return;
    }

  // Write the do not edit header.
  this->WriteDisclaimer(makefileStream);

  // Write some rules to make things look nice.
  makefileStream
    << "# Disable some common implicit rules to speed things up.\n"
    << ".SUFFIXES:\n"
    << ".SUFFIXES:.hpuxmakemusthaverule\n\n";

  // Write standard variables to the makefile.
  this->OutputMakeVariables(makefileStream);

  // Build command to run CMake to check if anything needs regenerating.
  std::string runRule =
    "@$(CMAKE_COMMAND) -H$(CMAKE_SOURCE_DIR) -B$(CMAKE_BINARY_DIR)";
  runRule += " --check-rerun ";
  runRule += this->ConvertToRelativeOutputPath(cmakefileName.c_str());

  // Most unix makes will pass the command line flags to make down to
  // sub-invoked makes via an environment variable.  However, some
  // makes do not support that, so you have to pass the flags
  // explicitly.
  const char* depRule = "$(MAKE) -f Makefile2 $(MAKESILENT) all.depends";
  const char* allRule = "$(MAKE) -f Makefile2 $(MAKESILENT) all";
  if(m_PassMakeflags)
    {
    depRule = "$(MAKE) -f Makefile2 $(MAKESILENT) -$(MAKEFLAGS) all.depends";
    allRule = "$(MAKE) -f Makefile2 $(MAKESILENT) -$(MAKEFLAGS) all";
    }

  // Write the main entry point target.  This must be the VERY first
  // target so that make with no arguments will run it.
  {
  std::vector<std::string> depends;
  std::vector<std::string> commands;
  commands.push_back(runRule);
  commands.push_back(depRule);
  commands.push_back(allRule);
  this->OutputMakeRule(
    makefileStream,
    "Default target executed when no arguments are given to make.",
    "default_target",
    depends,
    commands);
  }

  // Write special target to silence make output.  This must be after
  // the default target in case VERBOSE is set (which changes the name).
  if(!m_Makefile->IsOn("CMAKE_VERBOSE_MAKEFILE"))
    {
    makefileStream
      << "# Suppress display of executed commands.\n"
      << "$(VERBOSE).SILENT:\n\n";
    }

  // Get the set of targets.
  const cmTargets& targets = m_Makefile->GetTargets();

  // Output top level dependency rule.
  {
  std::vector<std::string> depends;
  std::vector<std::string> commands;
  for(cmTargets::const_iterator t = targets.begin(); t != targets.end(); ++t)
    {
    if(t->second.IsInAll())
      {
      std::string dep = this->GetTargetDirectory(t->second);
      dep += "/";
      dep += t->first;
      dep += ".depends";
      depends.push_back(dep);
      }
    }
  this->OutputMakeRule(makefileStream, "all dependencies", "all.depends",
                       depends, commands);
  }

  // Output top level build rule.
  {
  std::vector<std::string> depends;
  std::vector<std::string> commands;
  for(cmTargets::const_iterator t = targets.begin(); t != targets.end(); ++t)
    {
    if(t->second.IsInAll())
      {
      depends.push_back(t->first+".requires");
      }
    }
  this->OutputMakeRule(makefileStream, "all", "all",
                       depends, commands);
  }

  // Write include statements to get rules for each target.
  makefileStream
    << "# Include target rule files.\n";
  for(cmTargets::const_iterator t = targets.begin(); t != targets.end(); ++t)
    {
    std::string ruleFileName = this->GetTargetDirectory(t->second);
    ruleFileName += "/";
    ruleFileName += t->first;
    ruleFileName += ".make";
    makefileStream
      << m_IncludeDirective << " "
      << this->ConvertToOutputForExisting(ruleFileName.c_str()).c_str()
      << "\n";
    }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator2::GenerateCMakefile()
{
  std::string makefileName = m_Makefile->GetStartOutputDirectory();
  makefileName += "/Makefile2";
  std::string cmakefileName = makefileName;
  cmakefileName += ".cmake";

  // Open the output file.
  std::ofstream cmakefileStream(cmakefileName.c_str());
  if(!cmakefileStream)
    {
    cmSystemTools::Error("Error can not open for write: ",
                         cmakefileName.c_str());
    cmSystemTools::ReportLastSystemError("");
    return;
    }

  // Write the do not edit header.
  this->WriteDisclaimer(cmakefileStream);

  // Get the list of files contributing to this generation step.
  // Sort the list and remove duplicates.
  std::vector<std::string> lfiles = m_Makefile->GetListFiles();
  std::sort(lfiles.begin(), lfiles.end(), std::less<std::string>());
  std::vector<std::string>::iterator new_end = std::unique(lfiles.begin(),
                                                           lfiles.end());
  lfiles.erase(new_end, lfiles.end());

  // Save the list to the cmake file.
  cmakefileStream
    << "# The corresponding makefile\n"
    << "# \"" << makefileName << "\"\n"
    << "# was generated from the following files:\n"
    << "SET(CMAKE_MAKEFILE_DEPENDS\n"
    << "  \"" << m_Makefile->GetHomeOutputDirectory() << "/CMakeCache.txt\"\n";
  for(std::vector<std::string>::const_iterator i = lfiles.begin();
      i !=  lfiles.end(); ++i)
    {
    cmakefileStream
      << "  \"" << i->c_str() << "\"\n";
    }
  cmakefileStream
    << "  )\n\n";

  // Set the corresponding makefile in the cmake file.
  cmakefileStream
    << "# The corresponding makefile is:\n"
    << "SET(CMAKE_MAKEFILE_OUTPUTS\n"
    << "  \"" << makefileName.c_str() << "\"\n"
    << "  )\n";
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator2
::GenerateTargetRuleFile(const cmTarget& target)
{
  // Create a directory for this target.
  std::string dir = this->GetTargetDirectory(target);
  cmSystemTools::MakeDirectory(dir.c_str());

  // First generate the object rule files.  Save a list of all object
  // files for this target.
  std::vector<std::string> objects;
  const std::vector<cmSourceFile*>& sources = target.GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator source = sources.begin();
      source != sources.end(); ++source)
    {
    if(!(*source)->GetPropertyAsBool("HEADER_FILE_ONLY") &&
       !(*source)->GetCustomCommand() &&
       !m_GlobalGenerator->IgnoreFile((*source)->GetSourceExtension().c_str()))
      {
      // Generate this object file's rule file.
      this->GenerateObjectRuleFile(target, *(*source));

      // Save the object file full path.
      std::string obj = dir;
      obj += "/";
      obj += this->GetObjectFileName(*(*source));
      objects.push_back(obj);
      }
    }

  // If there is no dependencies file, create an empty one.
  std::string depFileName = dir;
  depFileName += "/";
  depFileName += target.GetName();
  depFileName += ".depends.make";
  if(!cmSystemTools::FileExists(depFileName.c_str()))
    {
    std::ofstream depFileStream(depFileName.c_str());
    this->WriteDisclaimer(depFileStream);
    depFileStream
      << "# Empty dependencies file for target " << target.GetName() << ".\n"
      << "# This may be replaced when dependencies are built.\n";
    }

  // Open the rule file.  This should be copy-if-different because the
  // rules may depend on this file itself.
  std::string ruleFileName = dir;
  ruleFileName += "/";
  ruleFileName += target.GetName();
  ruleFileName += ".make";
  cmGeneratedFileStream ruleFile(ruleFileName.c_str());
  std::ostream& ruleFileStream = ruleFile.GetStream();
  if(!ruleFileStream)
    {
    // TODO: Produce error message that accounts for generated stream
    // .tmp.
    return;
    }
  this->WriteDisclaimer(ruleFileStream);
  ruleFileStream
    << "# Rule file for target " << target.GetName() << ".\n\n";

  // Include the dependencies for the target.
  ruleFileStream
    << "# Include any dependencies generated for this rule.\n"
    << m_IncludeDirective << " "
    << this->ConvertToOutputForExisting(depFileName.c_str()).c_str()
    << "\n\n";

  // Include the rule file for each object.
  if(!objects.empty())
    {
    ruleFileStream
      << "# Include make rules for object files.\n";
    for(std::vector<std::string>::const_iterator obj = objects.begin();
        obj != objects.end(); ++obj)
      {
      std::string objRuleFileName = *obj;
      objRuleFileName += ".make";
      ruleFileStream
        << m_IncludeDirective << " "
        << this->ConvertToOutputForExisting(objRuleFileName.c_str()).c_str()
        << "\n";
      }
    ruleFileStream
      << "\n";
    }

  // Write the dependency generation rule.
  {
  std::vector<std::string> depends;
  std::vector<std::string> commands;
  std::string depComment = "dependencies for ";
  depComment += target.GetName();
  std::string depTarget = dir;
  depTarget += "/";
  depTarget += target.GetName();
  depTarget += ".depends";
  for(std::vector<std::string>::const_iterator obj = objects.begin();
      obj != objects.end(); ++obj)
    {
    depends.push_back((*obj)+".depends");
    }
  depends.push_back(ruleFileName);
  this->OutputMakeRule(ruleFileStream, depComment.c_str(), depTarget.c_str(),
                       depends, commands);
  }

  // Write the requires rule.
  {
  std::vector<std::string> depends;
  std::vector<std::string> commands;
  std::string reqComment = "requirements for ";
  reqComment += target.GetName();
  std::string reqTarget = target.GetName();
  reqTarget += ".requires";
  for(std::vector<std::string>::const_iterator obj = objects.begin();
      obj != objects.end(); ++obj)
    {
    depends.push_back(*obj);
    }
  depends.push_back(ruleFileName);
  this->OutputMakeRule(ruleFileStream, reqComment.c_str(), reqTarget.c_str(),
                       depends, commands);
  }

#if 0
  // Write the build rule.
  {
  std::vector<std::string> depends;
  std::vector<std::string> commands;
  std::string buildComment = " target ";
  buildComment += objName;
  for(std::vector<std::string>::const_iterator obj = objects.begin();
      obj != objects.end(); ++obj)
    {
    depends.push_back(*obj);
    }
  depends.push_back(ruleFileName);
  std::string touchCmd = "@touch ";
  touchCmd += this->ConvertToRelativeOutputPath(obj.c_str());
  // TODO: Construct build rule and append command.
  commands.push_back(touchCmd);
  this->OutputMakeRule(ruleFileStream, buildComment.c_str(), obj.c_str(),
                       depends, commands);
  }
#endif
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator2
::GenerateObjectRuleFile(const cmTarget& target, const cmSourceFile& source)
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
  std::string objName = this->GetObjectFileName(source);
  std::string obj = this->GetTargetDirectory(target);
  obj += "/";
  obj += objName;

  // Create the directory containing the object file.  This may be a
  // subdirectory under the target's directory.
  std::string dir = cmSystemTools::GetFilenamePath(obj.c_str());
  cmSystemTools::MakeDirectory(dir.c_str());

  // If there is no dependencies file, create an empty one.
  std::string depFileName = obj;
  depFileName += ".depends.make";
  if(!cmSystemTools::FileExists(depFileName.c_str()))
    {
    std::ofstream depFileStream(depFileName.c_str());
    this->WriteDisclaimer(depFileStream);
    depFileStream
      << "# Empty dependencies file for object file "
      << objName.c_str() << ".\n"
      << "# This may be replaced when dependencies are built.\n";
    }

  // Open the rule file for writing.  This should be copy-if-different
  // because the rules may depend on this file itself.
  std::string ruleFileName = obj;
  ruleFileName += ".make";
  cmGeneratedFileStream ruleFile(ruleFileName.c_str());
  std::ostream& ruleFileStream = ruleFile.GetStream();
  if(!ruleFileStream)
    {
    // TODO: Produce error message that accounts for generated stream
    // .tmp.
    return;
    }
  this->WriteDisclaimer(ruleFileStream);
  ruleFileStream
    << "# Rule file for object file " << objName.c_str() << ".\n\n";

  // Include the dependencies for the target.
  ruleFileStream
    << "# Include any dependencies generated for this rule.\n"
    << m_IncludeDirective << " "
    << this->ConvertToOutputForExisting(depFileName.c_str()).c_str()
    << "\n\n";

  // Create the list of dependencies known at cmake time.  These are
  // shared between the object file and dependency scanning rule.
  std::vector<std::string> depends;
  depends.push_back(source.GetFullPath());
  if(const char* objectDeps = source.GetProperty("OBJECT_DEPENDS"))
    {
    std::vector<std::string> deps;
    cmSystemTools::ExpandListArgument(objectDeps, deps);
    for(std::vector<std::string>::iterator i = deps.begin();
        i != deps.end(); ++i)
      {
      depends.push_back(this->ConvertToRelativeOutputPath(i->c_str()));
      }
    }
  depends.push_back(ruleFileName);

  // Write the dependency generation rule.
  std::string depTarget = obj;
  depTarget += ".depends";
  {
  std::string depComment = "dependencies for ";
  depComment += objName;
  cmOStringStream depCmd;
  // TODO: Account for source file properties and directory-level
  // definitions when scanning for dependencies.
  depCmd << "$(CMAKE_COMMAND) -E cmake_depends " << lang << " "
         << this->ConvertToRelativeOutputPath(obj.c_str()) << " "
         << this->ConvertToRelativeOutputPath(source.GetFullPath().c_str());
  std::vector<std::string> includeDirs;
  this->GetIncludeDirectories(includeDirs);
  for(std::vector<std::string>::iterator i = includeDirs.begin();
      i != includeDirs.end(); ++i)
    {
    depCmd << " -I" << this->ConvertToRelativeOutputPath(i->c_str());
    }
  std::vector<std::string> commands;
  commands.push_back(depCmd.str());
  std::string touchCmd = "@touch ";
  touchCmd += this->ConvertToRelativeOutputPath(depTarget.c_str());
  commands.push_back(touchCmd);
  this->OutputMakeRule(ruleFileStream, depComment.c_str(), depTarget.c_str(),
                       depends, commands);
  }

  // Write the build rule.
  {
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
  if(const char* compileFlags = source.GetProperty("COMPILE_FLAGS"))
    {
    flags += " ";
    flags += compileFlags;
    }

  // Add language-specific flags.
  std::string langFlagsVar = "CMAKE_";
  langFlagsVar += lang;
  langFlagsVar += "_FLAGS";
  const char* langFlags = m_Makefile->GetSafeDefinition(langFlagsVar.c_str());
  if(*langFlags)
    {
    flags += " ";
    flags += langFlags;
    }
  std::string buildType =  m_Makefile->GetSafeDefinition("CMAKE_BUILD_TYPE");
  if(buildType.size())
    {
    buildType = cmSystemTools::UpperCase(buildType);
    langFlagsVar += "_";
    langFlagsVar += buildType;
    langFlags = m_Makefile->GetSafeDefinition(langFlagsVar.c_str());
    if(*langFlags)
      {
      flags += " ";
      flags += langFlags;
      }
    }

  // Add shared-library flags if needed.
  if(shared)
    {
    std::string sharedFlagsVar = "CMAKE_SHARED_LIBRARY_";
    sharedFlagsVar += lang;
    sharedFlagsVar += "_FLAGS";
    const char* sharedFlags =
      m_Makefile->GetSafeDefinition(sharedFlagsVar.c_str());
    if(*sharedFlags)
      {
      flags += " ";
      flags += sharedFlags;
      }
    }
  if(cmSystemTools::IsOn(m_Makefile->GetDefinition("BUILD_SHARED_LIBS")))
    {
    std::string sharedFlagsVar = "CMAKE_SHARED_BUILD_";
    sharedFlagsVar += lang;
    sharedFlagsVar += "_FLAGS";
    const char* sharedFlags =
      m_Makefile->GetSafeDefinition(sharedFlagsVar.c_str());
    if(*sharedFlags)
      {
      flags += " ";
      flags += sharedFlags;
      }
    }

  // Add include directory flags.
  const char* includeFlags = this->GetIncludeFlags(lang);
  if(includeFlags && *includeFlags)
    {
    flags += " ";
    flags += includeFlags;
    }

  // Get the output paths for source and object files.
  std::string sourceFile =
    this->ConvertToRelativeOutputPath(source.GetFullPath().c_str());
  std::string objectFile =
    this->ConvertToRelativeOutputPath(obj.c_str());

  // Construct the compile rules.
  std::vector<std::string> commands;
  std::string compileRuleVar = "CMAKE_";
  compileRuleVar += lang;
  compileRuleVar += "_COMPILE_OBJECT";
  std::string compileRule =
    m_Makefile->GetRequiredDefinition(compileRuleVar.c_str());
  cmSystemTools::ExpandListArgument(compileRule, commands);
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

  // Write the rule.
  std::string buildComment = lang;
  buildComment += " object";
  this->OutputMakeRule(ruleFileStream, buildComment.c_str(), obj.c_str(),
                       depends, commands);
  }
}

//----------------------------------------------------------------------------
void cmLocalUnixMakefileGenerator2::WriteDisclaimer(std::ostream& os)
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
cmLocalUnixMakefileGenerator2
::GetTargetDirectory(const cmTarget& target)
{
  std::string dir = m_Makefile->GetStartOutputDirectory();
  dir += "/";
  dir += target.GetName();
  dir += ".dir";
  return dir;
}

//----------------------------------------------------------------------------
std::string
cmLocalUnixMakefileGenerator2
::GetObjectFileName(const cmSourceFile& source)
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
             m_Makefile->GetCurrentOutputDirectory()) == 0))
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
  return this->CreateSafeUniqueObjectFileName(objectName.c_str());
}

//----------------------------------------------------------------------------
const char*
cmLocalUnixMakefileGenerator2
::GetSourceFileLanguage(const cmSourceFile& source)
{
  // Identify the language of the source file.
  return (m_GlobalGenerator
          ->GetLanguageFromExtension(source.GetSourceExtension().c_str()));
}

//----------------------------------------------------------------------------
bool
cmLocalUnixMakefileGenerator2
::ScanDependencies(std::vector<std::string> const& args)
{
  // Format of arguments is:
  // $(CMAKE_COMMAND), cmake_depends, <lang>, <obj>, <src>, [include-flags]
  // The caller has ensured that all required arguments exist.

  // The file to which to write dependencies.
  const char* objFile = args[3].c_str();

  // The source file at which to start the scan.
  const char* srcFile = args[4].c_str();

  // Convert the include flags to full paths.
  std::vector<std::string> includes;
  for(unsigned int i=5; i < args.size(); ++i)
    {
    if(args[i].substr(0, 2) == "-I")
      {
      // Get the include path without the -I flag.
      std::string inc = args[i].substr(2);
      includes.push_back(cmSystemTools::CollapseFullPath(inc.c_str()));
      }
    }

  // Dispatch the scan for each language.
  std::string const& lang = args[2];
  if(lang == "C" || lang == "CXX")
    {
    return cmLocalUnixMakefileGenerator2::ScanDependenciesC(objFile, srcFile,
                                                            includes);
    }
  return false;
}

//----------------------------------------------------------------------------
void
cmLocalUnixMakefileGenerator2ScanDependenciesC(
  std::ifstream& fin,
  std::set<cmStdString>& encountered,
  std::queue<cmStdString>& unscanned)
{
  // Regular expression to identify C preprocessor include directives.
  cmsys::RegularExpression
    includeLine("^[ \t]*#[ \t]*include[ \t]*[<\"]([^\">]+)[\">]");

  // Read one line at a time.
  std::string line;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    // Match include directives.
    if(includeLine.find(line.c_str()))
      {
      // Get the file being included.
      std::string includeFile = includeLine.match(1);

      // Queue the file if it has not yet been encountered.
      if(encountered.find(includeFile) == encountered.end())
        {
        encountered.insert(includeFile);
        unscanned.push(includeFile);
        }
      }
    }
}

//----------------------------------------------------------------------------
bool
cmLocalUnixMakefileGenerator2
::ScanDependenciesC(const char* objFile, const char* srcFile,
                    std::vector<std::string> const& includes)
{
  // Walk the dependency graph starting with the source file.
  std::set<cmStdString> dependencies;
  std::set<cmStdString> encountered;
  std::set<cmStdString> scanned;
  std::queue<cmStdString> unscanned;
  unscanned.push(srcFile);
  encountered.insert(srcFile);
  while(!unscanned.empty())
    {
    // Get the next file to scan.
    std::string fname = unscanned.front();
    unscanned.pop();

    // If not a full path, find the file in the include path.
    std::string fullName;
    if(cmSystemTools::FileIsFullPath(fname.c_str()))
      {
      fullName = fname;
      }
    else
      {
      for(std::vector<std::string>::const_iterator i = includes.begin();
          i != includes.end(); ++i)
        {
        std::string temp = *i;
        temp += "/";
        temp += fname;
        if(cmSystemTools::FileExists(temp.c_str()))
          {
          fullName = temp;
          break;
          }
        }
      }

    // Scan the file if it has not been scanned already.
    if(scanned.find(fullName) == scanned.end())
      {
      // Record scanned files.
      scanned.insert(fullName);

      // Try to scan the file.  Just leave it out if we cannot find
      // it.
      std::ifstream fin(fullName.c_str());
      if(fin)
        {
        // Add this file as a dependency.
        dependencies.insert(fullName);

        // Scan this file for new dependencies.
        cmLocalUnixMakefileGenerator2ScanDependenciesC(fin, encountered,
                                                       unscanned);
        }
      }
    }

  // Write the dependencies to the output file.
  std::string depMakeFile = objFile;
  depMakeFile += ".depends.make";
  std::ofstream fout(depMakeFile.c_str());
  fout << "# Dependencies for " << objFile << std::endl;
  for(std::set<cmStdString>::iterator i=dependencies.begin();
      i != dependencies.end(); ++i)
    {
    fout << objFile << " : " << i->c_str() << std::endl;
    }
  fout << std::endl;
  fout << "# Dependencies for " << objFile << ".depends" << std::endl;
  for(std::set<cmStdString>::iterator i=dependencies.begin();
      i != dependencies.end(); ++i)
    {
    fout << objFile << ".depends : " << i->c_str() << std::endl;
    }

  return true;
}
