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
#include "cmTarget.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include <map>
#include <set>
#include <queue>
#include <stdlib.h> // required for atof
const char* cmTarget::TargetTypeNames[] = {
  "EXECUTABLE", "STATIC_LIBRARY",
  "SHARED_LIBRARY", "MODULE_LIBRARY", "UTILITY", "GLOBAL_TARGET",
  "INSTALL_FILES", "INSTALL_PROGRAMS", "INSTALL_DIRECTORY"
};

//----------------------------------------------------------------------------
cmTarget::cmTarget()
{
  this->Makefile = 0;
  this->LinkLibrariesAnalyzed = false;
  this->LinkDirectoriesComputed = false;
  this->HaveInstallRule = false;
  this->DLLPlatform = false;

}

// define properties
void cmTarget::DefineProperties(cmake *cm)
{
  cm->DefineProperty
    ("BUILD_WITH_INSTALL_RPATH", cmProperty::TARGET, 
     "Should build tree targets have install tree rpaths.",
     "BUILD_WITH_INSTALL_RPATH is a boolean specifying whether to link "
     "the target in the build tree with the INSTALL_RPATH.  This takes "
     "precedence over SKIP_BUILD_RPATH and avoids the need for relinking "
     "before installation.");

  cm->DefineProperty
    ("CLEAN_DIRECT_OUTPUT", cmProperty::TARGET, 
     "Do not delete other varients of this target.",
     "When a library is built CMake by default generates code to remove "
     "any existing library using all possible names.  This is needed "
     "to support libraries that switch between STATIC and SHARED by "
     "a user option.  However when using OUTPUT_NAME to build a static "
     "and shared library of the same name using different logical target "
     "names the two targets will remove each other's files.  This can be "
     "prevented by setting the CLEAN_DIRECT_OUTPUT property to 1.");

  cm->DefineProperty
    ("COMPILE_FLAGS", cmProperty::TARGET, 
     "Additional flags to yse when compiling this target's sources.",
     "The COMPILE_FLAGS property sets additional compiler flags used "
     "to build sources within the target.  It may also be used to pass "
     "additional preprocessor definitions.");
     
  cm->DefineProperty
    ("DEFINE_SYMBOL", cmProperty::TARGET, 
     "Define a symbol when compiling this target's sources.",
     "DEFINE_SYMBOL sets the name of the preprocessor symbol defined when "
     "compiling sources in a shared library. "
     "If not set here then it is set to target_EXPORTS by default "
     "(with some substitutions if the target is not a valid C "
     "identifier). This is useful for headers to know whether they are "
     "being included from inside their library our outside to properly "
     "setup dllexport/dllimport decorations. ");
     
  cm->DefineProperty
    ("DEBUG_POSTFIX", cmProperty::TARGET, 
     "A postfix that will be applied to this target when build debug.",
     "A property on a target that sepcifies a postfix to add to the "
     "target name when built in debug mode. For example foo.dll "
     "versus fooD.dll");

  cm->DefineProperty
    ("EchoString", cmProperty::TARGET, 
     "A message to be displayed when the target it built.",
     "A message to display on some generaters (such as makefiles) when "
     "the target is built.");

  cm->DefineProperty
    ("HAS_CXX", cmProperty::TARGET, 
     "Force a target to use the CXX linker.",
     "Setting HAS_CXX on a target will force the target to use the "
     "C++ linker (and C++ runtime libraries) for linking even if the "
     "target has no C++ code in it.");

  cm->DefineProperty
    ("IMPORT_PREFIX", cmProperty::TARGET, 
     "What comes before the import library name.",
     "Similar to the target property PREFIX, but used for import libraries "
     "(typically corresponding to a DLL) instead of regular libraries. "
     "A target property that can be set to override the prefix "
     "(such as \"lib\") on an import library name.");

  cm->DefineProperty
    ("IMPORT_SUFFIX", cmProperty::TARGET, 
     "What comes after the import library name.",
     "Similar to the target property SUFFIX, but used for import libraries "
     "(typically corresponding to a DLL) instead of regular libraries. "
     "A target property that can be set to override the suffix "
     "(such as \".lib\") on an import library name.");

  cm->DefineProperty
    ("EXCLUDE_FROM_ALL", cmProperty::TARGET, 
     "Exclude the target from the all target.",
     "A property on a target that indicates if the target is excluded "
     "from the default build target. If it is not, then with a Makefile "
     "for example typing make will couse this target to be built as well. "
     "The same concept applies to the default build of other generators.",
     true);

  cm->DefineProperty
    ("INSTALL_NAME_DIR", cmProperty::TARGET, 
     "Mac OSX directory name for installed targets.",
     "INSTALL_NAME_DIR is a string specifying the "
     "directory portion of the \"install_name\" field of shared libraries "
     "on Mac OSX to use in the installed targets. ");

  cm->DefineProperty
    ("INSTALL_RPATH", cmProperty::TARGET, 
     "The rpath to use for installed targets.",
     "A semicolon-separated list specifying the rpath "
     "to use in installed targets (for platforms that support it).");

  cm->DefineProperty
    ("INSTALL_RPATH_USE_LINK_PATH", cmProperty::TARGET, 
     "Add paths to linker search and installed rpath.",
     "INSTALL_RPATH_USE_LINK_PATH is a boolean that if set to true will "
     "append directories in the linker search path and outside the "
     "project to the INSTALL_RPATH. ");

  cm->DefineProperty
    ("LINK_FLAGS", cmProperty::TARGET, 
     "Additional flags to use when linking this target.",
     "The LINK_FLAGS property can be used to add extra flags to the "
     "link step of a target. LINK_FLAGS_<CONFIG> will add to the "
     "configuration <CONFIG>, "
     "for example, DEBUG, RELEASE, MINSIZEREL, RELWITHDEBINFO. ");

  cm->DefineProperty
    ("LINKER_LANGUAGE", cmProperty::TARGET, 
     "What tool to use for linking, based on language.",
     "The LINKER_LANGUAGE property is used to change the tool "
     "used to link an executable or shared library. The default is "
     "set the language to match the files in the library. CXX and C "
     "are common values for this property.");

  cm->DefineProperty
    ("LOCATION", cmProperty::TARGET, 
     "Where a target will be written on disk.",
     "A read only property on a target that indicates where that target "
     "will be written. For libraries and execuatables this will be where "
     "the file is written on disk. This property is computed based on a "
     "number of other settings.");

  cm->DefineProperty
    ("OUTPUT_NAME", cmProperty::TARGET, 
     "Sets the real name of a target when it is built.",
     "Sets the real name of a target when it is built and "
     "can be used to help create two targets of the same name even though "
     "CMake requires unique logical target names.  There is also a "
     "<CONFIG>_OUTPUT_NAME that can set the output name on a "
     "per-configuration basis.");

  cm->DefineProperty
    ("PRE_INSTALL_SCRIPT", cmProperty::TARGET, 
     "Deprecated install support.",
     "The PRE_INSTALL_SCRIPT and POST_INSTALL_SCRIPT properties are the "
     "old way to specify CMake scripts to run before and after "
     "installing a target.  They are used only when the old "
     "INSTALL_TARGETS command is used to install the target.  Use the "
     "INSTALL command instead.");

  cm->DefineProperty
    ("PREFIX", cmProperty::TARGET, 
     "What comes before the library name.",
     "A target property that can be set to override the prefix "
     "(such as \"lib\") on a library name.");

  cm->DefineProperty
    ("POST_INSTALL_SCRIPT", cmProperty::TARGET, 
     "Deprecated install support.",
     "The PRE_INSTALL_SCRIPT and POST_INSTALL_SCRIPT properties are the "
     "old way to specify CMake scripts to run before and after "
     "installing a target.  They are used only when the old "
     "INSTALL_TARGETS command is used to install the target.  Use the "
     "INSTALL command instead.");

  cm->DefineProperty
    ("SKIP_BUILD_RPATH", cmProperty::TARGET, 
     "Should rpaths be used for the build tree.",
     "SKIP_BUILD_RPATH is a boolean specifying whether to skip automatic "
     "generation of an rpath allowing the target to run from the "
     "build tree. ");

  cm->DefineProperty
    ("SOVERSION", cmProperty::TARGET, 
     "What version number is this target.",
     "For shared libraries VERSION and SOVERSION can be used to specify "
     "the build version and api version respectively. When building or "
     "installing appropriate symlinks are created if the platform "
     "supports symlinks and the linker supports so-names. "
     "If only one of both is specified the missing is assumed to have "
     "the same version number. "
     "For shared libraries and executables on Windows the VERSION "
     "attribute is parsed to extract a \"major.minor\" version number. "
     "These numbers are used as the image version of the binary. ");

  cm->DefineProperty
    ("STATIC_LIBRARY_FLAGS", cmProperty::TARGET, 
     "Extra flags to use when linking static libraries.",
     "Extra flags to use when linking a static library.");

  cm->DefineProperty
    ("SUFFIX", cmProperty::TARGET, 
     "What comes after the library name.",
     "A target property that can be set to override the suffix "
     "(such as \".so\") on a library name.");

  cm->DefineProperty
    ("VERSION", cmProperty::TARGET, 
     "What version number is this target.",
     "For shared libraries VERSION and SOVERSION can be used to specify "
     "the build version and api version respectively. When building or "
     "installing appropriate symlinks are created if the platform "
     "supports symlinks and the linker supports so-names. "
     "If only one of both is specified the missing is assumed to have "
     "the same version number. "
     "For executables VERSION can be used to specify the build version. "
     "When building or installing appropriate symlinks are created if "
     "the platform supports symlinks. "
     "For shared libraries and executables on Windows the VERSION "
     "attribute is parsed to extract a \"major.minor\" version number. "
     "These numbers are used as the image version of the binary. ");


  cm->DefineProperty
    ("WIN32_EXECUTABLE", cmProperty::TARGET, 
     "Used to specify Windows executable with a WinMain entry point.",
     "This can be set to indicate that a target is a Windows executable "
     "in contrast to a console application for example. This changes "
     "how the executable will be linked.");

  cm->DefineProperty
    ("ENABLE_EXPORTS", cmProperty::TARGET,
     "Specify whether an executable exports symbols for loadable modules.",
     "Normally an executable does not export any symbols because it is "
     "the final program.  It is possible for an executable to export "
     "symbols to be used by loadable modules.  When this property is "
     "set to true CMake will allow other targets to \"link\" to the "
     "executable with the TARGET_LINK_LIBRARIES command.  "
     "On all platforms a target-level dependency on the executable is "
     "created for targets that link to it.  "
     "For non-DLL platforms the link rule is simply ignored since "
     "the dynamic loader will automatically bind symbols when the "
     "module is loaded.  "
     "For DLL platforms an import library will be created for the "
     "exported symbols and then used for linking.  "
     "All Windows-based systems including Cygwin are DLL platforms.");

  cm->DefineProperty
    ("GENERATOR_FILE_NAME", cmProperty::TARGET, 
     "Generator's file for this target.",
     "An internal property used by some generators to record the name of "
     "project or dsp file associated with this target.");

  cm->DefineProperty
    ("OBJECT_FILES", cmProperty::TARGET, 
     "Used to get the resulting list of object files that make up a "
     "target.",
     "This can be used to put object files from one library "
     "into another library. It is a read only property.  It "
     "converts the source list for the target into a list of full "
     "paths to object names that will be produced by the target.");

#define CM_TARGET_FILE_TYPES_DOC                                            \
     "There are three kinds of target files that may be built: "            \
     "archive, library, and runtime.  "                                     \
     "Executables are always treated as runtime targets. "                  \
     "Static libraries are always treated as archive targets. "             \
     "Module libraries are always treated as library targets. "             \
     "For non-DLL platforms shared libraries are treated as library "       \
     "targets. "                                                            \
     "For DLL platforms the DLL part of a shared library is treated as "    \
     "a runtime target and the corresponding import library is treated as " \
     "an archive target. "                                                  \
     "All Windows-based systems including Cygwin are DLL platforms."

  cm->DefineProperty
    ("ARCHIVE_OUTPUT_DIRECTORY", cmProperty::TARGET,
     "Output directory in which to build ARCHIVE target files.",
     "This property specifies the directory into which archive target files "
     "should be built. "
     CM_TARGET_FILE_TYPES_DOC " "
     "This property is initialized by the value of the variable "
     "CMAKE_ARCHIVE_OUTPUT_DIRECTORY if it is set when a target is created.");
  cm->DefineProperty
    ("LIBRARY_OUTPUT_DIRECTORY", cmProperty::TARGET,
     "Output directory in which to build LIBRARY target files.",
     "This property specifies the directory into which library target files "
     "should be built. "
     CM_TARGET_FILE_TYPES_DOC " "
     "This property is initialized by the value of the variable "
     "CMAKE_LIBRARY_OUTPUT_DIRECTORY if it is set when a target is created.");
  cm->DefineProperty
    ("RUNTIME_OUTPUT_DIRECTORY", cmProperty::TARGET,
     "Output directory in which to build RUNTIME target files.",
     "This property specifies the directory into which runtime target files "
     "should be built. "
     CM_TARGET_FILE_TYPES_DOC " "
     "This property is initialized by the value of the variable "
     "CMAKE_RUNTIME_OUTPUT_DIRECTORY if it is set when a target is created.");

  // define some properties without documentation
  cm->DefineProperty("DEBUG_OUTPUT_NAME", cmProperty::TARGET,0,0);
  cm->DefineProperty("RELEASE_OUTPUT_NAME", cmProperty::TARGET,0,0);
  cm->DefineProperty("LINK_FLAGS_DEBUG", cmProperty::TARGET,0,0);
}

void cmTarget::SetType(TargetType type, const char* name)
{
  this->Name = name;
  // only add dependency information for library targets
  this->TargetTypeValue = type;
  if(this->TargetTypeValue >= STATIC_LIBRARY 
     && this->TargetTypeValue <= MODULE_LIBRARY) 
    {
    this->RecordDependencies = true;
    } 
  else 
    {
    this->RecordDependencies = false;
    }
}

//----------------------------------------------------------------------------
void cmTarget::SetMakefile(cmMakefile* mf)
{
  // Set our makefile.
  this->Makefile = mf;

  // set the cmake instance of the properties
  this->Properties.SetCMakeInstance(mf->GetCMakeInstance());

  // Check whether this is a DLL platform.
  this->DLLPlatform = (this->Makefile->IsOn("WIN32") ||
                       this->Makefile->IsOn("CYGWIN") ||
                       this->Makefile->IsOn("MINGW"));

  // Setup default property values.
  this->SetPropertyDefault("INSTALL_NAME_DIR", "");
  this->SetPropertyDefault("INSTALL_RPATH", "");
  this->SetPropertyDefault("INSTALL_RPATH_USE_LINK_PATH", "OFF");
  this->SetPropertyDefault("SKIP_BUILD_RPATH", "OFF");
  this->SetPropertyDefault("BUILD_WITH_INSTALL_RPATH", "OFF");
  this->SetPropertyDefault("ARCHIVE_OUTPUT_DIRECTORY", 0);
  this->SetPropertyDefault("LIBRARY_OUTPUT_DIRECTORY", 0);
  this->SetPropertyDefault("RUNTIME_OUTPUT_DIRECTORY", 0);

  // Collect the set of configuration types.
  std::vector<std::string> configNames;
  if(const char* configurationTypes =
     mf->GetDefinition("CMAKE_CONFIGURATION_TYPES"))
    {
    cmSystemTools::ExpandListArgument(configurationTypes, configNames);
    }
  else if(const char* buildType = mf->GetDefinition("CMAKE_BUILD_TYPE"))
    {
    if(*buildType)
      {
      configNames.push_back(buildType);
      }
    }

  // Setup per-configuration property default values.
  for(std::vector<std::string>::iterator ci = configNames.begin();
      ci != configNames.end(); ++ci)
    {
    // Initialize per-configuration name postfix property from the
    // variable only for non-executable targets.  This preserves
    // compatibility with previous CMake versions in which executables
    // did not support this variable.  Projects may still specify the
    // property directly.  TODO: Make this depend on backwards
    // compatibility setting.
    if(this->TargetTypeValue != cmTarget::EXECUTABLE)
      {
      std::string property = cmSystemTools::UpperCase(*ci);
      property += "_POSTFIX";
      this->SetPropertyDefault(property.c_str(), 0);
      }
    }
}

void cmTarget::TraceVSDependencies(std::string projFile, 
                                   cmMakefile *makefile)
{ 
  // get the classes from the source lists then add them to the groups
  std::vector<cmSourceFile*> & classes = this->GetSourceFiles();
  // use a deck to keep track of processed source files
  std::queue<std::string> srcFilesToProcess;
  std::set<cmStdString> srcFilesQueued;
  std::string name;
  std::vector<cmSourceFile*> newClasses;
  for(std::vector<cmSourceFile*>::const_iterator i = classes.begin(); 
      i != classes.end(); ++i)
    {
    name = (*i)->GetSourceName();
    if ((*i)->GetSourceExtension() != "rule")
      {
      name += ".";
      name += (*i)->GetSourceExtension();
      }
    srcFilesToProcess.push(name);
    srcFilesQueued.insert(name);
    // does this sourcefile have object depends on it?
    // If so then add them as well
    const char* additionalDeps = (*i)->GetProperty("OBJECT_DEPENDS");
    std::vector<std::string> depends = (*i)->GetDepends();
    if (additionalDeps || depends.size())
      {
      if(additionalDeps)
        {
        cmSystemTools::ExpandListArgument(additionalDeps, depends);
        }
      for(std::vector<std::string>::iterator id = depends.begin();
          id != depends.end(); ++id)
        {
        // if there is a custom rule to generate that dependency
        // then add it to the list
        cmSourceFile* outsf = 
          makefile->GetSourceFileWithOutput(id->c_str());
        // if a source file was found then add it
        if (outsf && 
            (std::find(classes.begin(),classes.end(),outsf) == classes.end())
            &&
            (std::find(newClasses.begin(),newClasses.end(),outsf)
             == newClasses.end()))
          {
          // then add the source to this target and add it to the queue
          newClasses.push_back(outsf);
          name = outsf->GetSourceName();
          if (outsf->GetSourceExtension() != "rule")
            {
            name += ".";
            name += outsf->GetSourceExtension();
            }
          std::string temp = 
            cmSystemTools::GetFilenamePath(outsf->GetFullPath());
          temp += "/";
          temp += name;
          // if it hasn't been processed
          if (srcFilesQueued.find(temp) == srcFilesQueued.end())
            {
            srcFilesToProcess.push(temp);
            srcFilesQueued.insert(temp);
            }
          }
        }
      }
    }
  for(std::vector<cmSourceFile*>::const_iterator i = newClasses.begin(); 
      i != newClasses.end(); ++i)
    {
    classes.push_back(*i);
    }
  
  // add in the project file itself
  if (projFile.size())
    {
    srcFilesToProcess.push(projFile);
    srcFilesQueued.insert(projFile);
    }
  // add in the library depends for custom targets
  if (this->GetType() == cmTarget::UTILITY || 
      this->GetType() == cmTarget::GLOBAL_TARGET)
    {
    for (std::vector<cmCustomCommand>::iterator ic = 
           this->GetPostBuildCommands().begin();
         ic != this->GetPostBuildCommands().end(); ++ic)
      {
      cmCustomCommand &c = *ic;
      for (std::vector<std::string>::const_iterator i 
             = c.GetDepends().begin(); i != c.GetDepends().end(); ++i)
        {
        srcFilesToProcess.push(*i);
        srcFilesQueued.insert(*i);
        }
      }
    }
  while (!srcFilesToProcess.empty())
    {
    // is this source the output of a custom command
    cmSourceFile* outsf = 
      makefile->GetSourceFileWithOutput(srcFilesToProcess.front().c_str());
    if (outsf)
      {
      // is it not already in the target?
      if (std::find(classes.begin(),classes.end(),outsf) == classes.end())
        {
        // then add the source to this target and add it to the queue
        classes.push_back(outsf);
        name = outsf->GetSourceName();
        if (outsf->GetSourceExtension() != "rule")
          {
          name += ".";
          name += outsf->GetSourceExtension();
          }
        std::string temp = 
          cmSystemTools::GetFilenamePath(outsf->GetFullPath());
        temp += "/";
        temp += name;
        // if it hasn't been processed
        if (srcFilesQueued.find(temp) == srcFilesQueued.end())
          {
          srcFilesToProcess.push(temp);
          srcFilesQueued.insert(temp);
          }
        }
      // add its dependencies to the list to check
      unsigned int i;
      for (i = 0; i < outsf->GetCustomCommand()->GetDepends().size(); ++i)
        {
        const std::string& fullName 
          = outsf->GetCustomCommand()->GetDepends()[i];
        std::string dep = cmSystemTools::GetFilenameName(fullName);
        if (cmSystemTools::GetFilenameLastExtension(dep) == ".exe")
          {
          dep = cmSystemTools::GetFilenameWithoutLastExtension(dep);
          }
        bool isUtility = false;
        // see if we can find a target with this name
        cmTarget* t =  this->Makefile->GetLocalGenerator()->
          GetGlobalGenerator()->FindTarget(0, dep.c_str());
        if(t)
          {
          // if we find the target and the dep was given as a full
          // path, then make sure it was not a full path to something
          // else, and the fact that the name matched a target was 
          // just a coincident 
          if(cmSystemTools::FileIsFullPath(fullName.c_str()))
            {
            std::string tLocation = t->GetLocation(0);
            tLocation = cmSystemTools::GetFilenamePath(tLocation);
            std::string depLocation = cmSystemTools::GetFilenamePath(
              std::string(fullName));
            depLocation = 
              cmSystemTools::CollapseFullPath(depLocation.c_str());
            tLocation = cmSystemTools::CollapseFullPath(tLocation.c_str());
            if(depLocation == tLocation)
              {
              isUtility = true;
              }
            }
          // if it was not a full path then it must be a target
          else
            {
            isUtility = true;
            }
          }
        if(isUtility)
          {
          // add the depend as a utility on the target
          this->AddUtility(dep.c_str());
          }
        else
          {
          if (srcFilesQueued.find(outsf->GetCustomCommand()->GetDepends()[i]) 
          == srcFilesQueued.end())
            {
            srcFilesToProcess.push
              (outsf->GetCustomCommand()->GetDepends()[i]);
            srcFilesQueued.insert(outsf->GetCustomCommand()->GetDepends()[i]);
            }
          }
        }
      }
    // finished with this SF move to the next
    srcFilesToProcess.pop();
    }
  // mark all custom commands in the targets list of source files as used.
  for(std::vector<cmSourceFile*>::iterator i =  this->SourceFiles.begin();
      i != this->SourceFiles.end(); ++i)
    {
    cmCustomCommand* cc = (*i)->GetCustomCommand();
    if(cc)
      {
      cc->SetUsed();
      }
    }
}

void cmTarget::GenerateSourceFilesFromSourceLists( cmMakefile &mf)
{
  // only allow this to be called once
  // there is a lazy evaluation of this in ComputeObjectFiles,
  // that could break backwards compatibility with projects that
  // use old style source lists.  
  if(this->SourceFiles.size() != 0)
    {
    return;
    }
  // this is only done for non install targets
  if ((this->TargetTypeValue == cmTarget::INSTALL_FILES)
      || (this->TargetTypeValue == cmTarget::INSTALL_PROGRAMS))
    {
    return;
    }
  // for each src lists add the classes
  for (std::vector<std::string>::const_iterator s = this->SourceLists.begin();
       s != this->SourceLists.end(); ++s)
    {
    int done = 0;
    // replace any variables
    std::string temps = *s;
    mf.ExpandVariablesInString(temps);

    // Next if one wasn't found then assume it is a single class
    // check to see if it is an existing source file
    if (!done)
      {
      cmSourceFile* sourceFile = mf.GetSource(temps.c_str());
      if ( sourceFile )
        {
        this->SourceFiles.push_back(sourceFile);
        done = 1;
        }
      }
      
    // if we still are not done, try to create the SourceFile structure
    if (!done)
      {
      cmSourceFile file;
      file.GetProperties().
        SetCMakeInstance(this->Makefile->GetCMakeInstance());
      file.SetProperty("ABSTRACT","0");
      file.SetName(temps.c_str(), mf.GetCurrentDirectory(),
                   mf.GetSourceExtensions(),
                   mf.GetHeaderExtensions(), this->Name.c_str());
      this->SourceFiles.push_back(mf.AddSource(file));
      }
    }
  
  // expand any link library variables whle we are at it
  LinkLibraryVectorType::iterator p = this->LinkLibraries.begin();
  for (;p != this->LinkLibraries.end(); ++p)
    {
    mf.ExpandVariablesInString(p->first, true, true);
    }
}


void cmTarget::MergeLinkLibraries( cmMakefile& mf,
                                   const char *selfname,
                                   const LinkLibraryVectorType& libs )
{
  // Only add on libraries we haven't added on before.
  // Assumption: the global link libraries could only grow, never shrink
  LinkLibraryVectorType::const_iterator i = libs.begin();
  i += this->PrevLinkedLibraries.size();
  for( ; i != libs.end(); ++i )
    {
    // We call this so that the dependencies get written to the cache
    this->AddLinkLibrary( mf, selfname, i->first.c_str(), i->second );
    }
  this->PrevLinkedLibraries = libs;
}

//----------------------------------------------------------------------------
void cmTarget::AddLinkDirectory(const char* d)
{
  // Make sure we don't add unnecessary search directories.
  if(std::find(this->ExplicitLinkDirectories.begin(),
               this->ExplicitLinkDirectories.end(), d)
     == this->ExplicitLinkDirectories.end() )
    {
    this->ExplicitLinkDirectories.push_back( d );
    this->LinkDirectoriesComputed = false;
    }
}

//----------------------------------------------------------------------------
const std::vector<std::string>& cmTarget::GetLinkDirectories()
{
  // Make sure all library dependencies have been analyzed.
  if(!this->LinkLibrariesAnalyzed && !this->LinkLibraries.empty())
    {
    cmSystemTools::Error(
      "cmTarget::GetLinkDirectories called before "
      "cmTarget::AnalyzeLibDependencies on target ",
      this->Name.c_str());
    }

  // Make sure the complete set of link directories has been computed.
  if(!this->LinkDirectoriesComputed)
    {
    // Check whether we should use an import library for linking a target.
    bool implib =
      this->Makefile->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX") != 0;

    // Compute the full set of link directories including the
    // locations of targets that have been linked in.  Start with the
    // link directories given explicitly.
    this->LinkDirectories = this->ExplicitLinkDirectories;
    for(LinkLibraryVectorType::iterator ll = this->LinkLibraries.begin();
        ll != this->LinkLibraries.end(); ++ll)
      {
      // If this library is a CMake target then add its location as a
      // link directory.
      std::string lib = ll->first;
      cmTarget* tgt = 0;
      if(this->Makefile && this->Makefile->GetLocalGenerator() &&
         this->Makefile->GetLocalGenerator()->GetGlobalGenerator())
        {
        tgt = (this->Makefile->GetLocalGenerator()->GetGlobalGenerator()
               ->FindTarget(0, lib.c_str()));
        }
      if(tgt)
        {
        // Add the directory only if it is not already present.  This
        // is an N^2 algorithm for adding the directories, but N
        // should not get very big.
        const char* libpath = tgt->GetDirectory(0, implib);
        if(std::find(this->LinkDirectories.begin(), 
                     this->LinkDirectories.end(),
                     libpath) == this->LinkDirectories.end())
          {
          this->LinkDirectories.push_back(libpath);
          }
        }
      }

    // The complete set of link directories has now been computed.
    this->LinkDirectoriesComputed = true;
    }

  // Return the complete set of link directories.
  return this->LinkDirectories;
}

void cmTarget::ClearDependencyInformation( cmMakefile& mf, 
                                           const char* target )
{
  // Clear the dependencies. The cache variable must exist iff we are
  // recording dependency information for this target.
  std::string depname = target;
  depname += "_LIB_DEPENDS";
  if (this->RecordDependencies)
    {
    mf.AddCacheDefinition(depname.c_str(), "",
                          "Dependencies for target", cmCacheManager::STATIC);
    }
  else
    {
    if (mf.GetDefinition( depname.c_str() ))
      {
      std::string message = "Target ";
      message += target;
      message += " has dependency information when it shouldn't.\n";
      message += "Your cache is probably stale. Please remove the entry\n  ";
      message += depname;
      message += "\nfrom the cache.";
      cmSystemTools::Error( message.c_str() );  
      }
    }
}



void cmTarget::AddLinkLibrary(const std::string& lib, 
                              LinkLibraryType llt)
{
  this->AddFramework(lib.c_str(), llt);
  cmTarget::LibraryID tmp;
  tmp.first = lib;
  tmp.second = llt;
  this->LinkLibraries.push_back(tmp);
}

bool cmTarget::AddFramework(const std::string& libname, LinkLibraryType llt)
{
  (void)llt; // TODO: What is this?
  if(cmSystemTools::IsPathToFramework(libname.c_str()))
    {
    std::string frameworkDir = libname;
    frameworkDir += "/../";
    frameworkDir = cmSystemTools::CollapseFullPath(frameworkDir.c_str());
    std::vector<std::string>::iterator i = 
      std::find(this->Frameworks.begin(),
                this->Frameworks.end(), frameworkDir);
    if(i == this->Frameworks.end())
      {
      this->Frameworks.push_back(frameworkDir);
      }
    return true;
    }
  return false;
}
void cmTarget::AddLinkLibrary(cmMakefile& mf,
                              const char *target, const char* lib, 
                              LinkLibraryType llt)
{
  // Never add a self dependency, even if the user asks for it.
  if(strcmp( target, lib ) == 0)
    {
    return;
    }
  this->AddFramework(lib, llt);
  cmTarget::LibraryID tmp;
  tmp.first = lib;
  tmp.second = llt;
  this->LinkLibraries.push_back( tmp );

  // Add the explicit dependency information for this target. This is
  // simply a set of libraries separated by ";". There should always
  // be a trailing ";". These library names are not canonical, in that
  // they may be "-framework x", "-ly", "/path/libz.a", etc.
  // We shouldn't remove duplicates here because external libraries
  // may be purposefully duplicated to handle recursive dependencies,
  // and we removing one instance will break the link line. Duplicates
  // will be appropriately eliminated at emit time.
  if(this->RecordDependencies)
    {
    std::string targetEntry = target;
    targetEntry += "_LIB_DEPENDS";
    std::string dependencies;
    const char* old_val = mf.GetDefinition( targetEntry.c_str() );
    if( old_val )
      {
      dependencies += old_val;
      }
    switch (llt)
      {
      case cmTarget::GENERAL:
        dependencies += "general";
        break;
      case cmTarget::DEBUG:
        dependencies += "debug";
        break;
      case cmTarget::OPTIMIZED:
        dependencies += "optimized";
        break;
      }
    dependencies += ";";
    dependencies += lib;
    dependencies += ";";
    mf.AddCacheDefinition( targetEntry.c_str(), dependencies.c_str(),
                           "Dependencies for the target", 
                           cmCacheManager::STATIC );
    }
  
}

void
cmTarget::AnalyzeLibDependencies( const cmMakefile& mf )
{
  // There are two key parts of the dependency analysis: (1)
  // determining the libraries in the link line, and (2) constructing
  // the dependency graph for those libraries.
  //
  // The latter is done using the cache entries that record the
  // dependencies of each library.
  //
  // The former is a more thorny issue, since it is not clear how to
  // determine if two libraries listed on the link line refer to the a
  // single library or not. For example, consider the link "libraries"
  //    /usr/lib/libtiff.so -ltiff 
  // Is this one library or two? The solution implemented here is the
  // simplest (and probably the only practical) one: two libraries are
  // the same if their "link strings" are identical. Thus, the two
  // libraries above are considered distinct. This also means that for
  // dependency analysis to be effective, the CMake user must specify
  // libraries build by his project without using any linker flags or
  // file extensions. That is,
  //    LINK_LIBRARIES( One Two )
  // instead of
  //    LINK_LIBRARIES( -lOne ${binarypath}/libTwo.a )
  // The former is probably what most users would do, but it never
  // hurts to document the assumptions. :-) Therefore, in the analysis
  // code, the "canonical name" of a library is simply its name as
  // given to a LINK_LIBRARIES command.
  //
  // Also, we will leave the original link line intact; we will just add any
  // dependencies that were missing.
  //
  // There is a problem with recursive external libraries
  // (i.e. libraries with no dependency information that are
  // recursively dependent). We must make sure that the we emit one of
  // the libraries twice to satisfy the recursion, but we shouldn't
  // emit it more times than necessary. In particular, we must make
  // sure that handling this improbable case doesn't cost us when
  // dealing with the common case of non-recursive libraries. The
  // solution is to assume that the recursion is satisfied at one node
  // of the dependency tree. To illustrate, assume libA and libB are
  // extrenal and mutually dependent. Suppose libX depends on
  // libA, and libY on libA and libX. Then
  //   TARGET_LINK_LIBRARIES( Y X A B A )
  //   TARGET_LINK_LIBRARIES( X A B A )
  //   TARGET_LINK_LIBRARIES( Exec Y )
  // would result in "-lY -lX -lA -lB -lA". This is the correct way to
  // specify the dependencies, since the mutual dependency of A and B
  // is resolved *every time libA is specified*.
  //
  // Something like
  //   TARGET_LINK_LIBRARIES( Y X A B A )
  //   TARGET_LINK_LIBRARIES( X A B )
  //   TARGET_LINK_LIBRARIES( Exec Y )
  // would result in "-lY -lX -lA -lB", and the mutual dependency
  // information is lost. This is because in some case (Y), the mutual
  // dependency of A and B is listed, while in another other case (X),
  // it is not. Depending on which line actually emits A, the mutual
  // dependency may or may not be on the final link line.  We can't
  // handle this pathalogical case cleanly without emitting extra
  // libraries for the normal cases. Besides, the dependency
  // information for X is wrong anyway: if we build an executable
  // depending on X alone, we would not have the mutual dependency on
  // A and B resolved.
  //
  // IMPROVEMENTS:
  // -- The current algorithm will not always pick the "optimal" link line
  //    when recursive dependencies are present. It will instead break the
  //    cycles at an aribtrary point. The majority of projects won't have
  //    cyclic dependencies, so this is probably not a big deal. Note that
  //    the link line is always correct, just not necessary optimal.

  typedef std::vector< std::string > LinkLine;

  // The dependency map.
  DependencyMap dep_map;

  if ( this->OriginalLinkLibraries.size() == 0 )
    {
    this->OriginalLinkLibraries = this->LinkLibraries;
    }

  // 1. Build the dependency graph
  //
  for(LinkLibraryVectorType::reverse_iterator lib 
        = this->LinkLibraries.rbegin();
      lib != this->LinkLibraries.rend(); ++lib)
    {
    this->GatherDependencies( mf, *lib, dep_map);
    }

  // 2. Remove any dependencies that are already satisfied in the original
  // link line.
  //
  for(LinkLibraryVectorType::iterator lib = this->LinkLibraries.begin();
      lib != this->LinkLibraries.end(); ++lib)
    {
    for( LinkLibraryVectorType::iterator lib2 = lib;
      lib2 != this->LinkLibraries.end(); ++lib2)
      {
      this->DeleteDependency( dep_map, *lib, *lib2);
      }
    }

  
  // 3. Create the new link line by simply emitting any dependencies that are
  // missing.  Start from the back and keep adding.
  //
  std::set<DependencyMap::key_type> done, visited;
  std::vector<DependencyMap::key_type> newLinkLibraries;
  for(LinkLibraryVectorType::reverse_iterator lib = 
        this->LinkLibraries.rbegin();
      lib != this->LinkLibraries.rend(); ++lib)
    {
    // skip zero size library entries, this may happen
    // if a variable expands to nothing.
    if (lib->first.size() != 0)
      {
      this->Emit( *lib, dep_map, done, visited, newLinkLibraries );
      }
    }

  // 4. Add the new libraries to the link line.
  //
  for( std::vector<DependencyMap::key_type>::reverse_iterator k = 
         newLinkLibraries.rbegin();
       k != newLinkLibraries.rend(); ++k )
    {
    // get the llt from the dep_map    
    this->LinkLibraries.push_back( std::make_pair(k->first,k->second) );
    }
  this->LinkLibrariesAnalyzed = true;
}


void cmTarget::InsertDependency( DependencyMap& depMap,
                                 const LibraryID& lib,
                                 const LibraryID& dep) 
{
  depMap[lib].push_back(dep);
}

void cmTarget::DeleteDependency( DependencyMap& depMap,
                                 const LibraryID& lib,
                                 const LibraryID& dep) 
{
  // Make sure there is an entry in the map for lib. If so, delete all
  // dependencies to dep. There may be repeated entries because of
  // external libraries that are specified multiple times.
  DependencyMap::iterator map_itr = depMap.find( lib );
  if( map_itr != depMap.end() )
    {
    DependencyList& depList = map_itr->second;
    DependencyList::iterator itr;
    while( (itr = std::find(depList.begin(), depList.end(), dep)) != 
           depList.end() )
      {
      depList.erase( itr );
      }
    }
}

void cmTarget::Emit(const LibraryID lib,
                    const DependencyMap& dep_map,
                    std::set<LibraryID>& emitted,
                    std::set<LibraryID>& visited,
                    DependencyList& link_line )
{
  // It's already been emitted
  if( emitted.find(lib) != emitted.end() )
    {
    return;
    }

  // Emit the dependencies only if this library node hasn't been
  // visited before. If it has, then we have a cycle. The recursion
  // that got us here should take care of everything.

  if( visited.insert(lib).second )
    {
    if( dep_map.find(lib) != dep_map.end() ) // does it have dependencies?
      {
      const DependencyList& dep_on = dep_map.find( lib )->second;
      DependencyList::const_reverse_iterator i;

      // To cater for recursive external libraries, we must emit
      // duplicates on this link line *unless* they were emitted by
      // some other node, in which case we assume that the recursion
      // was resolved then. We making the simplifying assumption that
      // any duplicates on a single link line are on purpose, and must
      // be preserved.

      // This variable will keep track of the libraries that were
      // emitted directory from the current node, and not from a
      // recursive call. This way, if we come across a library that
      // has already been emitted, we repeat it iff it has been
      // emitted here.
      std::set<DependencyMap::key_type> emitted_here;
      for( i = dep_on.rbegin(); i != dep_on.rend(); ++i )
        {
        if( emitted_here.find(*i) != emitted_here.end() )
          {
          // a repeat. Must emit.
          emitted.insert(*i);
          link_line.push_back( *i );
          }
        else
          {
          // Emit only if no-one else has
          if( emitted.find(*i) == emitted.end() )
            {
            // emit dependencies
            Emit( *i, dep_map, emitted, visited, link_line );
            // emit self
            emitted.insert(*i);
            emitted_here.insert(*i);
            link_line.push_back( *i );
            }
          }
        }
      }
    }
}


void cmTarget::GatherDependencies( const cmMakefile& mf,
                                   const LibraryID& lib,
                                   DependencyMap& dep_map)
{
  // If the library is already in the dependency map, then it has
  // already been fully processed.
  if( dep_map.find(lib) != dep_map.end() )
    {
    return;
    }

  const char* deps = mf.GetDefinition( (lib.first+"_LIB_DEPENDS").c_str() );
  if( deps && strcmp(deps,"") != 0 )
    {
    // Make sure this library is in the map, even if it has an empty
    // set of dependencies. This distinguishes the case of explicitly
    // no dependencies with that of unspecified dependencies.
    dep_map[lib];

    // Parse the dependency information, which is a set of
    // type, library pairs separated by ";". There is always a trailing ";".
    cmTarget::LinkLibraryType llt = cmTarget::GENERAL;
    std::string depline = deps;
    std::string::size_type start = 0;
    std::string::size_type end;
    end = depline.find( ";", start );
    while( end != std::string::npos )
      {
      std::string l = depline.substr( start, end-start );
      if( l.size() != 0 )
        {
        if (l == "debug")
          {
          llt = cmTarget::DEBUG;
          }
        else if (l == "optimized")
          {
          llt = cmTarget::OPTIMIZED;
          }
        else if (l == "general")
          {
          llt = cmTarget::GENERAL;
          }
        else
          {
          LibraryID lib2(l,llt);
          this->InsertDependency( dep_map, lib, lib2);
          this->GatherDependencies( mf, lib2, dep_map);
          llt = cmTarget::GENERAL;
          }
        }
      start = end+1; // skip the ;
      end = depline.find( ";", start );
      }
    // cannot depend on itself
    this->DeleteDependency( dep_map, lib, lib); 
    }
}


void cmTarget::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }
  if (!value)
    {
    value = "NOTFOUND";
    }

  this->Properties.SetProperty(prop, value, cmProperty::TARGET);
}

const char* cmTarget::GetDirectory(const char* config, bool implib)
{
  if(config)
    {
    this->Directory = this->GetOutputDir(implib);
    // Add the configuration's subdirectory.
    this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
      AppendDirectoryForConfig("/", config, "", this->Directory);
    return this->Directory.c_str();
    }
  else
    {
    return this->GetOutputDir(implib);
    }
}

const char* cmTarget::GetLocation(const char* config)
{
  this->Location = this->GetDirectory();
  if(!this->Location.empty())
    {
    this->Location += "/";
    }
  const char* cfgid = this->Makefile->GetDefinition("CMAKE_CFG_INTDIR");
  if(cfgid && strcmp(cfgid, ".") != 0)
    {
    this->Location += cfgid;
    this->Location += "/";
    }
  this->Location += this->GetFullName(config, false);
  return this->Location.c_str();
}

//----------------------------------------------------------------------------
void cmTarget::GetTargetVersion(int& major, int& minor)
{
  // Set the default values.
  major = 0;
  minor = 0;

  // Look for a VERSION property.
  if(const char* version = this->GetProperty("VERSION"))
    {
    // Try to parse the version number and store the results that were
    // successfully parsed.
    int parsed_major;
    int parsed_minor;
    switch(sscanf(version, "%d.%d", &parsed_major, &parsed_minor))
      {
      case 2: minor = parsed_minor; // no break!
      case 1: major = parsed_major; // no break!
      default: break;
      }
    }
}

const char *cmTarget::GetProperty(const char* prop)
{
  return this->GetProperty(prop, cmProperty::TARGET);
}

void cmTarget::ComputeObjectFiles()
{
  // Force the SourceFiles vector to be populated
  this->GenerateSourceFilesFromSourceLists(*this->Makefile);
  std::vector<std::string> dirs;
  this->Makefile->GetLocalGenerator()->
    GetTargetObjectFileDirectories(this,
                                   dirs);
  std::string objectFiles;
  std::string objExtensionLookup1 = "CMAKE_";
  std::string objExtensionLookup2 = "_OUTPUT_EXTENSION";
  
  for(std::vector<std::string>::iterator d = dirs.begin();
      d != dirs.end(); ++d)
    {
    for(std::vector<cmSourceFile*>::iterator s = this->SourceFiles.begin();
        s != this->SourceFiles.end(); ++s)
      {
      cmSourceFile* sf = *s;
      const char* lang = this->Makefile->GetLocalGenerator()->
        GetGlobalGenerator()->
        GetLanguageFromExtension(sf->GetSourceExtension().c_str());
      std::string lookupObj = objExtensionLookup1 + lang;
      lookupObj += objExtensionLookup2;
      const char* obj = this->Makefile->GetDefinition(lookupObj.c_str());
      if(obj)
        {
        if(objectFiles.size())
          {
          objectFiles += ";";
          }
        std::string objFile = *d;
        objFile += "/";
        objFile += this->Makefile->GetLocalGenerator()->
          GetSourceObjectName(*sf);
        objFile += obj;
        objectFiles += objFile;
        }
      }
    }
  this->SetProperty("OBJECT_FILES", objectFiles.c_str());
}


const char *cmTarget::GetProperty(const char* prop,
                                  cmProperty::ScopeType scope)
{
  if(!prop)
    {
    return 0;
    }

  // watch for special "computed" properties that are dependent on other
  // properties or variables, always recompute them
  if (!strcmp(prop,"LOCATION"))
    {
    // Set the LOCATION property of the target.  Note that this cannot take
    // into account the per-configuration name of the target because the
    // configuration type may not be known at CMake time.  We should
    // deprecate this feature and instead support transforming an executable
    // target name given as the command part of custom commands into the
    // proper path at build time.  Alternatively we could put environment
    // variable settings in all custom commands that hold the name of the
    // target for each configuration and then give a reference to the
    // variable in the location.
    this->SetProperty("LOCATION", this->GetLocation(0));
    }

  // Per-configuration location can be computed.
  int len = static_cast<int>(strlen(prop));
  if(len > 9 && strcmp(prop+len-9, "_LOCATION") == 0)
    {
    std::string configName(prop, len-9);
    this->SetProperty(prop, this->GetLocation(configName.c_str()));
    }
  
  if(strcmp(prop, "OBJECT_FILES") == 0)
    {
    this->ComputeObjectFiles();
    }

  // the type property returns what type the target is
  if (!strcmp(prop,"TYPE"))
    {
    switch( this->GetType() )
      {
      case cmTarget::STATIC_LIBRARY:
        return "STATIC_LIBRARY";
        // break; /* unreachable */
      case cmTarget::MODULE_LIBRARY:
        return "MODULE_LIBRARY";
        // break; /* unreachable */
      case cmTarget::SHARED_LIBRARY:
        return "SHARED_LIBRARY";
        // break; /* unreachable */
      case cmTarget::EXECUTABLE:
        return "EXECUTABLE";
        // break; /* unreachable */
      case cmTarget::UTILITY:
        return "UTILITY";
        // break; /* unreachable */
      case cmTarget::GLOBAL_TARGET:
        return "GLOBAL_TARGET";
        // break; /* unreachable */
      case cmTarget::INSTALL_FILES:
        return "INSTALL_FILES";
        // break; /* unreachable */
      case cmTarget::INSTALL_PROGRAMS:
        return "INSTALL_PROGRAMS";
        // break; /* unreachable */
      case cmTarget::INSTALL_DIRECTORY:
        return "INSTALL_DIRECTORY";
        // break; /* unreachable */
      }
    return 0;
    }
  bool chain = false;
  const char *retVal = 
    this->Properties.GetPropertyValue(prop, scope, chain);
  if (chain)
    {
    return this->Makefile->GetProperty(prop,scope);
    }
  return retVal;    
}

bool cmTarget::GetPropertyAsBool(const char* prop)
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

const char* cmTarget::GetLinkerLanguage(cmGlobalGenerator* gg)
{
  if(this->GetProperty("HAS_CXX"))
    {
    const_cast<cmTarget*>(this)->SetProperty("LINKER_LANGUAGE", "CXX");
    }
  const char* linkerLang = this->GetProperty("LINKER_LANGUAGE");
  if(linkerLang)
    {
    return linkerLang;
    }
  std::set<cmStdString> languages;
  for(std::vector<cmSourceFile*>::const_iterator i 
        = this->SourceFiles.begin();
      i != this->SourceFiles.end(); ++i)
    {
    const char* lang = 
      gg->GetLanguageFromExtension((*i)->GetSourceExtension().c_str());
    if(lang)
      {
      languages.insert(lang);
      }
    }
  if(languages.size() == 0)
    {
    return 0;
    }
  if(languages.size() == 1)
    {
    const_cast<cmTarget*>(this)->SetProperty("LINKER_LANGUAGE", 
                                             languages.begin()->c_str());
    return this->GetProperty("LINKER_LANGUAGE");
    }
  const char* prefLang = 0;
  for(std::set<cmStdString>::const_iterator s = languages.begin(); 
      s != languages.end(); ++s)
    {
    const char* lpref = gg->GetLinkerPreference(s->c_str());
    if(lpref[0] == 'P')
      {
      if(prefLang && !(*s == prefLang))
        {
        std::string m = "Error Target: ";
        m += this->Name + " Contains more than one Prefered language: ";
        m += *s;
        m += " and ";
        m += prefLang;
        m += "\nYou must set the LINKER_LANGUAGE property for this target.";
        cmSystemTools::Error(m.c_str());
        }
      else
        {
        prefLang = s->c_str();
        }
      }
    }
  if(!prefLang)
    {
    prefLang = languages.begin()->c_str();
    }
  const_cast<cmTarget*>(this)->SetProperty("LINKER_LANGUAGE", prefLang);
  return this->GetProperty("LINKER_LANGUAGE");
}

const char* cmTarget::GetCreateRuleVariable()
{
  switch(this->GetType())
    { 
    case cmTarget::STATIC_LIBRARY:
      return "_CREATE_STATIC_LIBRARY";
    case cmTarget::SHARED_LIBRARY:
      return "_CREATE_SHARED_LIBRARY";
    case cmTarget::MODULE_LIBRARY:
      return "_CREATE_SHARED_MODULE";
    case cmTarget::EXECUTABLE:
      return "_LINK_EXECUTABLE";
    case cmTarget::UTILITY:
    case cmTarget::GLOBAL_TARGET:
    case cmTarget::INSTALL_FILES:
    case cmTarget::INSTALL_PROGRAMS:
    case cmTarget::INSTALL_DIRECTORY:
      break;
    }
  return "";
}

const char* cmTarget::GetSuffixVariableInternal(TargetType type,
                                                bool implib)
{
  switch(type)
    {
    case cmTarget::STATIC_LIBRARY:
      return "CMAKE_STATIC_LIBRARY_SUFFIX";
    case cmTarget::SHARED_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_SUFFIX"
              : "CMAKE_SHARED_LIBRARY_SUFFIX");
    case cmTarget::MODULE_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_SUFFIX"
              : "CMAKE_SHARED_MODULE_SUFFIX");
    case cmTarget::EXECUTABLE:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_SUFFIX"
              : "CMAKE_EXECUTABLE_SUFFIX");
    case cmTarget::UTILITY:
    case cmTarget::GLOBAL_TARGET:
    case cmTarget::INSTALL_FILES:
    case cmTarget::INSTALL_PROGRAMS:
    case cmTarget::INSTALL_DIRECTORY:
      break;
    }
  return "";
}


const char* cmTarget::GetPrefixVariableInternal(TargetType type,
                                                bool implib)
{
  switch(type)
    {
    case cmTarget::STATIC_LIBRARY:
      return "CMAKE_STATIC_LIBRARY_PREFIX";
    case cmTarget::SHARED_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_PREFIX"
              : "CMAKE_SHARED_LIBRARY_PREFIX");
    case cmTarget::MODULE_LIBRARY:
      return (implib
              ? "CMAKE_IMPORT_LIBRARY_PREFIX"
              : "CMAKE_SHARED_MODULE_PREFIX");
    case cmTarget::EXECUTABLE:
      return (implib? "CMAKE_IMPORT_LIBRARY_PREFIX" : "");
    case cmTarget::UTILITY:
    case cmTarget::GLOBAL_TARGET:
    case cmTarget::INSTALL_FILES:
    case cmTarget::INSTALL_PROGRAMS:
    case cmTarget::INSTALL_DIRECTORY:
      break;
    }
  return "";
}

//----------------------------------------------------------------------------
std::string cmTarget::GetPDBName(const char* config)
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(this->GetType(), config, false,
                            prefix, base, suffix);
  return prefix+base+".pdb";
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullName(const char* config, bool implib)
{
  return this->GetFullNameInternal(this->GetType(), config, implib);
}

//----------------------------------------------------------------------------
void cmTarget::GetFullName(std::string& prefix, std::string& base,
                           std::string& suffix, const char* config,
                           bool implib)
{
  this->GetFullNameInternal(this->GetType(), config, implib,
                            prefix, base, suffix);
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullPath(const char* config, bool implib)
{
  // Start with the output directory for the target.
  std::string fpath = this->GetDirectory(config, implib);
  fpath += "/";

  // Add the full name of the target.
  fpath += this->GetFullName(config, implib);
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullNameInternal(TargetType type, const char* config,
                                          bool implib)
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(type, config, implib, prefix, base, suffix);
  return prefix+base+suffix;
}

//----------------------------------------------------------------------------
void cmTarget::GetFullNameInternal(TargetType type,
                                   const char* config,
                                   bool implib,
                                   std::string& outPrefix,
                                   std::string& outBase,
                                   std::string& outSuffix)
{
  // Use just the target name for non-main target types.
  if(type != cmTarget::STATIC_LIBRARY &&
     type != cmTarget::SHARED_LIBRARY &&
     type != cmTarget::MODULE_LIBRARY &&
     type != cmTarget::EXECUTABLE)
    {
    outPrefix = "";
    outBase = this->GetName();
    outSuffix = "";
    return;
    }

  // Return an empty name for the import library if this platform
  // does not support import libraries.
  if(implib &&
     !this->Makefile->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX"))
    {
    outPrefix = "";
    outBase = "";
    outSuffix = "";
    return;
    }

  // The implib option is only allowed for shared libraries, module
  // libraries, and executables.
  if(type != cmTarget::SHARED_LIBRARY &&
     type != cmTarget::MODULE_LIBRARY &&
     type != cmTarget::EXECUTABLE)
    {
    implib = false;
    }

  // Compute the full name for main target types.
  const char* targetPrefix = (implib
                              ? this->GetProperty("IMPORT_PREFIX")
                              : this->GetProperty("PREFIX"));
  const char* targetSuffix = (implib
                              ? this->GetProperty("IMPORT_SUFFIX")
                              : this->GetProperty("SUFFIX"));
  const char* configPostfix = 0;
  if(config && *config)
    {
    std::string configProp = cmSystemTools::UpperCase(config);
    configProp += "_POSTFIX";
    configPostfix = this->GetProperty(configProp.c_str());
    }
  const char* prefixVar = this->GetPrefixVariableInternal(type, implib);
  const char* suffixVar = this->GetSuffixVariableInternal(type, implib);
  const char* ll =
    this->GetLinkerLanguage(
      this->Makefile->GetLocalGenerator()->GetGlobalGenerator());
  // first try language specific suffix
  if(ll)
    {
    if(!targetSuffix && suffixVar && *suffixVar)
      {
      std::string langSuff = suffixVar + std::string("_") + ll;
      targetSuffix = this->Makefile->GetDefinition(langSuff.c_str());
      }
    if(!targetPrefix && prefixVar && *prefixVar)
      {
      std::string langPrefix = prefixVar + std::string("_") + ll;
      targetPrefix = this->Makefile->GetDefinition(langPrefix.c_str());
      }
    }

  // if there is no prefix on the target use the cmake definition
  if(!targetPrefix && prefixVar)
    {
    targetPrefix = this->Makefile->GetSafeDefinition(prefixVar);
    }
  // if there is no suffix on the target use the cmake definition
  if(!targetSuffix && suffixVar)
    {
    targetSuffix = this->Makefile->GetSafeDefinition(suffixVar);
    }

  // Begin the final name with the prefix.
  outPrefix = targetPrefix?targetPrefix:"";

  // Append the target name or property-specified name.
  const char* outName = 0;
  if(config && *config)
    {
    std::string configProp = cmSystemTools::UpperCase(config);
    configProp += "_OUTPUT_NAME";
    outName = this->GetProperty(configProp.c_str());
    }
  if(!outName)
    {
    outName = this->GetProperty("OUTPUT_NAME");
    }
  if(outName)
    {
    outBase = outName;
    }
  else
    {
    outBase = this->GetName();
    }

  // Append the per-configuration postfix.
  outBase += configPostfix?configPostfix:"";

  // Name shared libraries with their version number on some platforms.
  if(const char* version = this->GetProperty("VERSION"))
    {
    if(type == cmTarget::SHARED_LIBRARY && !implib &&
       this->Makefile->IsOn("CMAKE_SHARED_LIBRARY_NAME_WITH_VERSION"))
      {
      outBase += "-";
      outBase += version;
      }
    }

  // Append the suffix.
  outSuffix = targetSuffix?targetSuffix:"";
}

void cmTarget::GetLibraryNames(std::string& name,
                               std::string& soName,
                               std::string& realName,
                               std::string& impName,
                               std::string& pdbName,
                               const char* config)
{
  // Get the names based on the real type of the library.
  this->GetLibraryNamesInternal(name, soName, realName, impName, pdbName,
                                this->GetType(), config);
}

void cmTarget::GetLibraryCleanNames(std::string& staticName,
                                    std::string& sharedName,
                                    std::string& sharedSOName,
                                    std::string& sharedRealName,
                                    std::string& importName,
                                    std::string& pdbName,
                                    const char* config)
{
  // Get the name as if this were a static library.
  std::string soName;
  std::string realName;
  std::string impName;
  this->GetLibraryNamesInternal(staticName, soName, realName, impName,
                                pdbName, cmTarget::STATIC_LIBRARY, config);

  // Get the names as if this were a shared library.
  if(this->GetType() == cmTarget::STATIC_LIBRARY)
    {
    // Since the real type is static then the user either specified
    // STATIC or did not specify a type.  In the former case the
    // shared library will never be present.  In the latter case the
    // type will never be MODULE.  Either way the only names that
    // might have to be cleaned are the shared library names.
    this->GetLibraryNamesInternal(sharedName, sharedSOName, sharedRealName,
                                  importName, pdbName,
                                  cmTarget::SHARED_LIBRARY, config);
    }
  else
    {
    // Use the name of the real type of the library (shared or module).
    this->GetLibraryNamesInternal(sharedName, sharedSOName, sharedRealName,
                                  importName, pdbName, this->GetType(),
                                  config);
    }
}

void cmTarget::GetLibraryNamesInternal(std::string& name,
                                       std::string& soName,
                                       std::string& realName,
                                       std::string& impName,
                                       std::string& pdbName,
                                       TargetType type,
                                       const char* config)
{
  // Construct the name of the soname flag variable for this language.
  const char* ll =
    this->GetLinkerLanguage(
      this->Makefile->GetLocalGenerator()->GetGlobalGenerator());
  std::string sonameFlag = "CMAKE_SHARED_LIBRARY_SONAME";
  if(ll)
    {
    sonameFlag += "_";
    sonameFlag += ll;
    }
  sonameFlag += "_FLAG";

  // Check for library version properties.
  const char* version = this->GetProperty("VERSION");
  const char* soversion = this->GetProperty("SOVERSION");
  if((type != cmTarget::SHARED_LIBRARY && type != cmTarget::MODULE_LIBRARY) ||
     !this->Makefile->GetDefinition(sonameFlag.c_str()))
    {
    // Versioning is supported only for shared libraries and modules,
    // and then only when the platform supports an soname flag.
    version = 0;
    soversion = 0;
    }
  if(version && !soversion)
    {
    // The soversion must be set if the library version is set.  Use
    // the library version as the soversion.
    soversion = version;
    }

  // Get the components of the library name.
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(type, config, false, prefix, base, suffix);

  // The library name.
  name = prefix+base+suffix;

  // The library's soname.
#if defined(__APPLE__)
  soName = prefix+base;
#else
  soName = name;
#endif
  if(soversion)
    {
    soName += ".";
    soName += soversion;
    }
#if defined(__APPLE__)
  soName += suffix;
#endif

  // The library's real name on disk.
#if defined(__APPLE__)
  realName = prefix+base;
#else
  realName = name;
#endif
  if(version)
    {
    realName += ".";
    realName += version;
    }
  else if(soversion)
    {
    realName += ".";
    realName += soversion;
    }
#if defined(__APPLE__)
  realName += suffix;
#endif

  // The import library name.
  if(type == cmTarget::SHARED_LIBRARY ||
     type == cmTarget::MODULE_LIBRARY)
    {
    impName = this->GetFullNameInternal(type, config, true);
    }
  else
    {
    impName = "";
    }

  // The program database file name.
  pdbName = prefix+base+".pdb";
}

void cmTarget::GetExecutableNames(std::string& name,
                                  std::string& realName,
                                  std::string& impName,
                                  std::string& pdbName,
                                  const char* config)
{
  // Get the names based on the real type of the executable.
  this->GetExecutableNamesInternal(name, realName, impName, pdbName,
                                   this->GetType(), config);
}

void cmTarget::GetExecutableCleanNames(std::string& name,
                                       std::string& realName,
                                       std::string& impName,
                                       std::string& pdbName,
                                       const char* config)
{
  // Get the name and versioned name of this executable.
  this->GetExecutableNamesInternal(name, realName, impName, pdbName,
                                   cmTarget::EXECUTABLE, config);
}

void cmTarget::GetExecutableNamesInternal(std::string& name,
                                          std::string& realName,
                                          std::string& impName,
                                          std::string& pdbName,
                                          TargetType type,
                                          const char* config)
{
  // This versioning is supported only for executables and then only
  // when the platform supports symbolic links.
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* version = 0;
#else
  // Check for executable version properties.
  const char* version = this->GetProperty("VERSION");
  if(type != cmTarget::EXECUTABLE)
    {
    version = 0;
    }
#endif

  // Get the components of the executable name.
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(type, config, false, prefix, base, suffix);

  // The executable name.
  name = prefix+base+suffix;

  // The executable's real name on disk.
#if defined(__CYGWIN__)
  realName = prefix+base;
#else
  realName = name;
#endif
  if(version)
    {
    realName += "-";
    realName += version;
    }
#if defined(__CYGWIN__)
  realName += suffix;
#endif

  // The import library name.
  impName = this->GetFullNameInternal(type, config, true);

  // The program database file name.
  pdbName = prefix+base+".pdb";
}

//----------------------------------------------------------------------------
void cmTarget::SetPropertyDefault(const char* property,
                                  const char* default_value)
{
  // Compute the name of the variable holding the default value.
  std::string var = "CMAKE_";
  var += property;

  if(const char* value = this->Makefile->GetDefinition(var.c_str()))
    {
    this->SetProperty(property, value);
    }
  else if(default_value)
    {
    this->SetProperty(property, default_value);
    }
}

//----------------------------------------------------------------------------
bool cmTarget::HaveBuildTreeRPATH()
{
  return (!this->GetPropertyAsBool("SKIP_BUILD_RPATH") &&
          !this->LinkLibraries.empty());
}

//----------------------------------------------------------------------------
bool cmTarget::HaveInstallTreeRPATH()
{
  const char* install_rpath = this->GetProperty("INSTALL_RPATH");
  return install_rpath && *install_rpath;
}

//----------------------------------------------------------------------------
bool cmTarget::NeedRelinkBeforeInstall()
{
  // Only executables and shared libraries can have an rpath and may
  // need relinking.
  if(this->TargetTypeValue != cmTarget::EXECUTABLE &&
     this->TargetTypeValue != cmTarget::SHARED_LIBRARY &&
     this->TargetTypeValue != cmTarget::MODULE_LIBRARY)
    {
    return false;
    }

  // If there is no install location this target will not be installed
  // and therefore does not need relinking.
  if(!this->GetHaveInstallRule())
    {
    return false;
    }

  // If skipping all rpaths completely then no relinking is needed.
  if(this->Makefile->IsOn("CMAKE_SKIP_RPATH"))
    {
    return false;
    }

  // If building with the install-tree rpath no relinking is needed.
  if(this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"))
    {
    return false;
    }

  // Check for rpath support on this platform.
  if(const char* ll = this->GetLinkerLanguage(
       this->Makefile->GetLocalGenerator()->GetGlobalGenerator()))
    {
    std::string flagVar = "CMAKE_SHARED_LIBRARY_RUNTIME_";
    flagVar += ll;
    flagVar += "_FLAG";
    if(!this->Makefile->IsSet(flagVar.c_str()))
      {
      // There is no rpath support on this platform so nothing needs
      // relinking.
      return false;
      }
    }
  else
    {
    // No linker language is known.  This error will be reported by
    // other code.
    return false;
    }

  // If either a build or install tree rpath is set then the rpath
  // will likely change between the build tree and install tree and
  // this target must be relinked.
  return this->HaveBuildTreeRPATH() || this->HaveInstallTreeRPATH();
}

//----------------------------------------------------------------------------
std::string cmTarget::GetInstallNameDirForBuildTree(const char* config)
{
  // If building directly for installation then the build tree install_name
  // is the same as the install tree.
  if(this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"))
    {
    return GetInstallNameDirForInstallTree(config);
    }

  // Use the build tree directory for the target.
  if(this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME") &&
     !this->Makefile->IsOn("CMAKE_SKIP_RPATH") &&
     !this->GetPropertyAsBool("SKIP_BUILD_RPATH"))
    {
    std::string dir = this->GetDirectory(config);
    dir += "/";
    return dir;
    }
  else
    {
    return "";
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::GetInstallNameDirForInstallTree(const char*)
{
  // Lookup the target property.
  const char* install_name_dir = this->GetProperty("INSTALL_NAME_DIR");
  if(this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME") &&
     !this->Makefile->IsOn("CMAKE_SKIP_RPATH") &&
     install_name_dir && *install_name_dir)
    {
    std::string dir = install_name_dir;
    dir += "/";
    return dir;
    }
  else
    {
    return "";
    }
}

//----------------------------------------------------------------------------
const char* cmTarget::GetOutputDir(bool implib)
{
  // The implib option is only allowed for shared libraries, module
  // libraries, and executables.
  if(this->GetType() != cmTarget::SHARED_LIBRARY &&
     this->GetType() != cmTarget::MODULE_LIBRARY &&
     this->GetType() != cmTarget::EXECUTABLE)
    {
    implib = false;
    }

  // Sanity check.  Only generators on platforms supporting import
  // libraries should be asking for the import library output
  // directory.
  if(implib &&
     !this->Makefile->GetDefinition("CMAKE_IMPORT_LIBRARY_SUFFIX"))
    {
    abort();
    }
  if(implib && !this->DLLPlatform)
    {
    abort();
    }

  // Select whether we are constructing the directory for the main
  // target or the import library.
  std::string& out = implib? this->OutputDirImplib : this->OutputDir;

  if(out.empty())
    {
    // Look for a target property defining the target output directory
    // based on the target type.
    const char* propertyName = 0;
    switch(this->GetType())
      {
      case cmTarget::SHARED_LIBRARY:
        {
        // For non-DLL platforms shared libraries are treated as
        // library targets.  For DLL platforms the DLL part of a
        // shared library is treated as a runtime target and the
        // corresponding import library is treated as an archive
        // target.
        if(this->DLLPlatform)
          {
          if(implib)
            {
            propertyName = "ARCHIVE_OUTPUT_DIRECTORY";
            }
          else
            {
            propertyName = "RUNTIME_OUTPUT_DIRECTORY";
            }
          }
        else
          {
          propertyName = "LIBRARY_OUTPUT_DIRECTORY";
          }
        } break;
      case cmTarget::STATIC_LIBRARY:
        {
        // Static libraries are always treated as archive targets.
        propertyName = "ARCHIVE_OUTPUT_DIRECTORY";
        } break;
      case cmTarget::MODULE_LIBRARY:
        {
        // Module libraries are always treated as library targets.
        // Module import libraries are treated as archive targets.
        if(implib)
          {
          propertyName = "ARCHIVE_OUTPUT_DIRECTORY";
          }
        else
          {
          propertyName = "LIBRARY_OUTPUT_DIRECTORY";
          }
        } break;
      case cmTarget::EXECUTABLE:
        {
        // Executables are always treated as runtime targets.
        // Executable import libraries are treated as archive targets.
        if(implib)
          {
          propertyName = "ARCHIVE_OUTPUT_DIRECTORY";
          }
        else
          {
          propertyName = "RUNTIME_OUTPUT_DIRECTORY";
          }
        } break;
      default: break;
      }

    // Select an output directory.
    if(const char* outdir = this->GetProperty(propertyName))
      {
      // Use the user-specified output directory.
      out = outdir;
      }
    else if(this->GetType() == cmTarget::EXECUTABLE)
      {
      // Lookup the output path for executables.
      out = this->Makefile->GetSafeDefinition("EXECUTABLE_OUTPUT_PATH");
      }
    else if(this->GetType() == cmTarget::STATIC_LIBRARY ||
            this->GetType() == cmTarget::SHARED_LIBRARY ||
            this->GetType() == cmTarget::MODULE_LIBRARY)
      {
      // Lookup the output path for libraries.
      out = this->Makefile->GetSafeDefinition("LIBRARY_OUTPUT_PATH");
      }
    if(out.empty())
      {
      // Default to the current output directory.
      out = ".";
      }

    // Convert the output path to a full path in case it is
    // specified as a relative path.  Treat a relative path as
    // relative to the current output directory for this makefile.
    out =
      cmSystemTools::CollapseFullPath
      (out.c_str(), this->Makefile->GetStartOutputDirectory());

    // Make sure the output path exists on disk.
    if(!cmSystemTools::MakeDirectory(out.c_str()))
      {
      cmSystemTools::Error("Error failed to create output directory:",
                           out.c_str());
      }
    }

  return out.c_str();
}

//----------------------------------------------------------------------------
const char* cmTarget::GetExportMacro()
{
  // Define the symbol for targets that export symbols.
  if(this->GetType() == cmTarget::SHARED_LIBRARY ||
     this->GetType() == cmTarget::MODULE_LIBRARY ||
     this->GetType() == cmTarget::EXECUTABLE &&
     this->GetPropertyAsBool("ENABLE_EXPORTS"))
    {
    if(const char* custom_export_name = this->GetProperty("DEFINE_SYMBOL"))
      {
      this->ExportMacro = custom_export_name;
      }
    else
      {
      std::string in = this->GetName();
      in += "_EXPORTS";
      this->ExportMacro = cmSystemTools::MakeCindentifier(in.c_str());
      }
    return this->ExportMacro.c_str();
    }
  else
    {
    return 0;
    }
}
