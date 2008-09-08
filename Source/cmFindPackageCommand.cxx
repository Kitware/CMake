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
#include "cmFindPackageCommand.h"
#include <cmsys/RegularExpression.hxx>

#ifdef CMAKE_BUILD_WITH_CMAKE
#include "cmVariableWatch.h"
#endif

void cmFindPackageNeedBackwardsCompatibility(const std::string& variable,
  int access_type, void*, const char* newValue,
  const cmMakefile*)
{
  (void)newValue;
#ifdef CMAKE_BUILD_WITH_CMAKE
  if(access_type == cmVariableWatch::UNKNOWN_VARIABLE_READ_ACCESS)
    {
    std::string message = "An attempt was made to access a variable: ";
    message += variable;
    message +=
      " that has not been defined. This variable is created by the "
      "FIND_PACKAGE command. CMake version 1.6 always converted the "
      "variable name to upper-case, but this behavior is no longer the "
      "case.  To fix this you might need to set the cache value of "
      "CMAKE_BACKWARDS_COMPATIBILITY to 1.6 or less.  If you are writing a "
      "CMake listfile, you should change the variable reference to use "
      "the case of the argument to FIND_PACKAGE.";
    cmSystemTools::Error(message.c_str());
    }
#else
  (void)variable;
  (void)access_type;
#endif
}

//----------------------------------------------------------------------------
cmFindPackageCommand::cmFindPackageCommand()
{
  cmSystemTools::ReplaceString(this->GenericDocumentationRootPath,
                               "CMAKE_FIND_ROOT_PATH_MODE_XXX",
                               "CMAKE_FIND_ROOT_PATH_MODE_PACKAGE");
  cmSystemTools::ReplaceString(this->GenericDocumentationPathsOrder,
                               "FIND_ARGS_XXX", "<package>");
  cmSystemTools::ReplaceString(this->GenericDocumentationPathsOrder,
                               "FIND_XXX", "find_package");
  this->CMakePathName = "PACKAGE";
  this->Quiet = false;
  this->Required = false;
  this->NoBuilds = false;
  this->NoModule = false;
  this->DebugMode = false;
  this->UseLib64Paths = false;
  this->VersionMajor = 0;
  this->VersionMinor = 0;
  this->VersionPatch = 0;
  this->VersionCount = 0;
  this->VersionExact = false;
  this->VersionFoundMajor = 0;
  this->VersionFoundMinor = 0;
  this->VersionFoundPatch = 0;
  this->VersionFoundCount = 0;
  this->CommandDocumentation =
    "  find_package(<package> [major[.minor[.patch]]] [EXACT] [QUIET]\n"
    "               [[REQUIRED|COMPONENTS] [components...]])\n"
    "Finds and loads settings from an external project.  "
    "<package>_FOUND will be set to indicate whether the package was found.  "
    "When the package is found package-specific information is provided "
    "through variables documented by the package itself.  "
    "The QUIET option disables messages if the package cannot be found.  "
    "The REQUIRED option stops processing with an error message if the "
    "package cannot be found.  "
    "A package-specific list of components may be listed after the "
    "REQUIRED option or after the COMPONENTS option if no REQUIRED "
    "option is given.  "
    "The \"[major[.minor[.patch]]]\" version argument specifies a desired "
    "version with which the package found should be compatible.  "
    "The EXACT option requests that the version be matched exactly.  "
    "Version support is currently provided only on a package-by-package "
    "basis (details below).\n"
    "User code should generally look for packages using the above simple "
    "signature.  The remainder of this command documentation specifies the "
    "full command signature and details of the search process.  Project "
    "maintainers wishing to provide a package to be found by this command "
    "are encouraged to read on.\n"
    "The command has two modes by which it searches for packages: "
    "\"Module\" mode and \"Config\" mode.  "
    "Module mode is available when the command is invoked with the above "
    "reduced signature.  "
    "CMake searches for a file called \"Find<package>.cmake\" in "
    "the CMAKE_MODULE_PATH followed by the CMake installation.  "
    "If the file is found, it is read and processed by CMake.  "
    "It is responsible for finding the package, checking the version, "
    "and producing any needed messages.  "
    "Many find-modules provide limited or no support for versioning; "
    "check the module documentation.  "
    "If no module is found the command proceeds to Config mode.\n"
    "The complete Config mode command signature is:\n"
    "  find_package(<package> [major[.minor[.patch]]] [EXACT] [QUIET]\n"
    "               [[REQUIRED|COMPONENTS] [components...]] [NO_MODULE]\n"
    "               [NAMES name1 [name2 ...]]\n"
    "               [CONFIGS config1 [config2 ...]]\n"
    "               [HINTS path1 [path2 ... ]]\n"
    "               [PATHS path1 [path2 ... ]]\n"
    "               [PATH_SUFFIXES suffix1 [suffix2 ...]]\n"
    "               [NO_DEFAULT_PATH]\n"
    "               [NO_CMAKE_ENVIRONMENT_PATH]\n"
    "               [NO_CMAKE_PATH]\n"
    "               [NO_SYSTEM_ENVIRONMENT_PATH]\n"
    "               [NO_CMAKE_BUILDS_PATH]\n"
    "               [NO_CMAKE_SYSTEM_PATH]\n"
    "               [CMAKE_FIND_ROOT_PATH_BOTH |\n"
    "                ONLY_CMAKE_FIND_ROOT_PATH |\n"
    "                NO_CMAKE_FIND_ROOT_PATH])\n"
    "The NO_MODULE option may be used to skip Module mode explicitly.  "
    "It is also implied by use of options not specified in the reduced "
    "signature.  "
    "\n"
    "Config mode attempts to locate a configuration file provided by the "
    "package to be found.  A cache entry called <package>_DIR is created to "
    "hold the directory containing the file.  "
    "By default the command searches for a package with the name <package>.  "
    "If the NAMES option is given the names following it are used instead "
    "of <package>.  "
    "The command searches for a file called \"<name>Config.cmake\" or "
    "\"<lower-case-name>-config.cmake\" for each name specified.  "
    "A replacement set of possible configuration file names may be given "
    "using the CONFIGS option.  "
    "The search procedure is specified below.  Once found, the configuration "
    "file is read and processed by CMake.  Since the file is provided by the "
    "package it already knows the location of package contents.  "
    "The full path to the configuration file is stored in the cmake "
    "variable <package>_CONFIG."
    "\n"
    "If the package configuration file cannot be found CMake "
    "will generate an error describing the problem unless the QUIET "
    "argument is specified.  If REQUIRED is specified and the package "
    "is not found a fatal error is generated and the configure step stops "
    "executing.  If <package>_DIR has been set to a directory not containing "
    "a configuration file a fatal error is always generated because user "
    "intervention is required."
    "\n"
    "When the \"[major[.minor[.patch]]]\" version argument is specified "
    "Config mode will only find a version of the package that claims "
    "compatibility with the requested version.  "
    "If the EXACT option is given only a version of the package claiming "
    "an exact match of the requested version may be found.  "
    "CMake does not establish any convention for the meaning of version "
    "numbers.  "
    "Package version numbers are checked by \"version\" files provided by "
    "the packages themselves.  "
    "For a candidate package confguration file \"<config-file>.cmake\" the "
    "corresponding version file is located next to it and named either "
    "\"<config-file>-version.cmake\" or \"<config-file>Version.cmake\".  "
    "If no such version file is available then the configuration file "
    "is assumed to not be compatible with any requested version.  "
    "When a version file is found it is loaded to check the requested "
    "version number.  "
    "The version file is loaded in a nested scope in which the following "
    "variables have been defined:\n"
    "  PACKAGE_FIND_NAME          = the <package> name\n"
    "  PACKAGE_FIND_VERSION       = full requested version string\n"
    "  PACKAGE_FIND_VERSION_MAJOR = requested major version, if any\n"
    "  PACKAGE_FIND_VERSION_MINOR = requested minor version, if any\n"
    "  PACKAGE_FIND_VERSION_PATCH = requested patch version, if any\n"
    "The version file checks whether it satisfies the requested version "
    "and sets these variables:\n"
    "  PACKAGE_VERSION            = package version (major[.minor[.patch]])\n"
    "  PACKAGE_VERSION_EXACT      = true if version is exact match\n"
    "  PACKAGE_VERSION_COMPATIBLE = true if version is compatible\n"
    "These variables are checked by the find_package command to determine "
    "whether the configuration file provides an acceptable version.  "
    "They are not available after the find_package call returns.  "
    "If the version is acceptable the following variables are set:\n"
    "  <package>_VERSION       = package version (major[.minor[.patch]])\n"
    "  <package>_VERSION_MAJOR = major from major[.minor[.patch]], if any\n"
    "  <package>_VERSION_MINOR = minor from major[.minor[.patch]], if any\n"
    "  <package>_VERSION_PATCH = patch from major[.minor[.patch]], if any\n"
    "and the corresponding package configuration file is loaded.  "
    "When multiple package configuration files are available whose version "
    "files claim compatibility with the version requested it is unspecified "
    "which one is chosen.  "
    "No attempt is made to choose a highest or closest version number."
    "\n"
    "Config mode provides an elaborate interface and search procedure.  "
    "Much of the interface is provided for completeness and for use "
    "internally by find-modules loaded by Module mode.  "
    "Most user code should simply call\n"
    "  find_package(<package> [major[.minor]] [EXACT] [REQUIRED|QUIET])\n"
    "in order to find a package.  Package maintainers providing CMake "
    "package configuration files are encouraged to name and install "
    "them such that the procedure outlined below will find them "
    "without requiring use of additional options."
    "\n"
    "CMake constructs a set of possible installation prefixes for the "
    "package.  Under each prefix several directories are searched for a "
    "configuration file.  The tables below show the directories searched.  "
    "Each entry is meant for installation trees following Windows (W), "
    "UNIX (U), or Apple (A) conventions.\n"
    "  <prefix>/                                               (W)\n"
    "  <prefix>/(cmake|CMake)/                                 (W)\n"
    "  <prefix>/(share|lib)/<name>*/                           (U)\n"
    "  <prefix>/(share|lib)/<name>*/(cmake|CMake)/             (U)\n"
    "On systems supporting OS X Frameworks and Application Bundles "
    "the following directories are searched for frameworks or bundles "
    "containing a configuration file:\n"
    "  <prefix>/<name>.framework/Resources/                    (A)\n"
    "  <prefix>/<name>.framework/Resources/CMake/              (A)\n"
    "  <prefix>/<name>.framework/Versions/*/Resources/         (A)\n"
    "  <prefix>/<name>.framework/Versions/*/Resources/CMake/   (A)\n"
    "  <prefix>/<name>.app/Contents/Resources/                 (A)\n"
    "  <prefix>/<name>.app/Contents/Resources/CMake/           (A)\n"
    "In all cases the <name> is treated as case-insensitive and corresponds "
    "to any of the names specified (<package> or names given by NAMES).  "
    "If PATH_SUFFIXES is specified the suffixes are appended to each "
    "(W) or (U) directory entry one-by-one.\n"
    "This set of directories is intended to work in cooperation with "
    "projects that provide configuration files in their installation trees.  "
    "Directories above marked with (W) are intended for installations on "
    "Windows where the prefix may point at the top of an application's "
    "installation directory.  Those marked with (U) are intended for "
    "installations on UNIX platforms where the prefix is shared by "
    "multiple packages.  This is merely a convention, so all (W) and (U) "
    "directories are still searched on all platforms.  "
    "Directories marked with (A) are intended for installations on "
    "Apple platforms.  The cmake variables CMAKE_FIND_FRAMEWORK and "
    "CMAKE_FIND_APPBUNDLE determine the order of preference "
    "as specified below.\n"
    "The set of installation prefixes is constructed using the following "
    "steps.  If NO_DEFAULT_PATH is specified all NO_* options are enabled.\n"
    "1. Search paths specified in cmake-specific cache variables.  "
    "These are intended to be used on the command line with a -DVAR=value.  "
    "This can be skipped if NO_CMAKE_PATH is passed.\n"
    "   CMAKE_PREFIX_PATH\n"
    "   CMAKE_FRAMEWORK_PATH\n"
    "   CMAKE_APPBUNDLE_PATH\n"
    "2. Search paths specified in cmake-specific environment variables.  "
    "These are intended to be set in the user's shell configuration.  "
    "This can be skipped if NO_CMAKE_ENVIRONMENT_PATH is passed.\n"
    "   CMAKE_PREFIX_PATH\n"
    "   CMAKE_FRAMEWORK_PATH\n"
    "   CMAKE_APPBUNDLE_PATH\n"
    "3. Search paths specified by the HINTS option.  "
    "These should be paths computed by system introspection, such as a "
    "hint provided by the location of another item already found.  "
    "Hard-coded guesses should be specified with the PATHS option.\n"
    "4. Search the standard system environment variables. "
    "This can be skipped if NO_SYSTEM_ENVIRONMENT_PATH is passed.  "
    "Path entries ending in \"/bin\" or \"/sbin\" are automatically "
    "converted to their parent directories.\n"
    "   PATH\n"
    "5. Search project build trees recently configured in a CMake GUI.  "
    "This can be skipped if NO_CMAKE_BUILDS_PATH is passed.  "
    "It is intended for the case when a user is building multiple "
    "dependent projects one after another.\n"
    "6. Search cmake variables defined in the Platform files "
    "for the current system.  This can be skipped if NO_CMAKE_SYSTEM_PATH "
    "is passed.\n"
    "   CMAKE_SYSTEM_PREFIX_PATH\n"
    "   CMAKE_SYSTEM_FRAMEWORK_PATH\n"
    "   CMAKE_SYSTEM_APPBUNDLE_PATH\n"
    "7. Search paths specified by the PATHS option.  "
    "These are typically hard-coded guesses.\n"
    ;
  this->CommandDocumentation += this->GenericDocumentationMacPolicy;
  this->CommandDocumentation += this->GenericDocumentationRootPath;
  this->CommandDocumentation += this->GenericDocumentationPathsOrder;
}

//----------------------------------------------------------------------------
const char* cmFindPackageCommand::GetFullDocumentation()
{
  return this->CommandDocumentation.c_str();
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 1)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Check for debug mode.
  this->DebugMode = this->Makefile->IsOn("CMAKE_FIND_DEBUG_MODE");

  // Lookup whether lib64 paths should be used.
  if(const char* sizeof_dptr =
     this->Makefile->GetDefinition("CMAKE_SIZEOF_VOID_P"))
    {
    if(atoi(sizeof_dptr) == 8 &&
       this->Makefile->GetCMakeInstance()
       ->GetPropertyAsBool("FIND_LIBRARY_USE_LIB64_PATHS"))
      {
      this->UseLib64Paths = true;
      }
    }

  // Find the current root path mode.
  this->SelectDefaultRootPathMode();

  // Find the current bundle/framework search policy.
  this->SelectDefaultMacMode();

  // Record options.
  this->Name = args[0];
  std::string components;
  const char* components_sep = "";

  // Check ancient compatibility.
  this->Compatibility_1_6 =
    this->Makefile->GetLocalGenerator()
    ->NeedBackwardsCompatibility(1, 6);

  // Always search directly in a generated path.
  this->SearchPathSuffixes.push_back("");

  // Parse the arguments.
  enum Doing { DoingNone, DoingComponents, DoingNames, DoingPaths,
               DoingPathSuffixes, DoingConfigs, DoingHints };
  Doing doing = DoingNone;
  cmsys::RegularExpression version("^[0-9.]+$");
  bool haveVersion = false;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "QUIET")
      {
      this->Quiet = true;
      doing = DoingNone;
      }
    else if(args[i] == "EXACT")
      {
      this->VersionExact = true;
      this->Compatibility_1_6 = false;
      doing = DoingNone;
      }
    else if(args[i] == "NO_MODULE")
      {
      this->NoModule = true;
      doing = DoingNone;
      }
    else if(args[i] == "REQUIRED")
      {
      this->Required = true;
      doing = DoingComponents;
      }
    else if(args[i] == "COMPONENTS")
      {
      this->Compatibility_1_6 = false;
      doing = DoingComponents;
      }
    else if(args[i] == "NAMES")
      {
      this->NoModule = true;
      this->Compatibility_1_6 = false;
      doing = DoingNames;
      }
    else if(args[i] == "PATHS")
      {
      this->NoModule = true;
      this->Compatibility_1_6 = false;
      doing = DoingPaths;
      }
    else if(args[i] == "HINTS")
      {
      this->NoModule = true;
      this->Compatibility_1_6 = false;
      doing = DoingHints;
      }
    else if(args[i] == "PATH_SUFFIXES")
      {
      this->NoModule = true;
      this->Compatibility_1_6 = false;
      doing = DoingPathSuffixes;
      }
    else if(args[i] == "CONFIGS")
      {
      this->NoModule = true;
      this->Compatibility_1_6 = false;
      doing = DoingConfigs;
      }
    else if(args[i] == "NO_CMAKE_BUILDS_PATH")
      {
      this->NoBuilds = true;
      this->NoModule = true;
      this->Compatibility_1_6 = false;
      doing = DoingNone;
      }
    else if(this->CheckCommonArgument(args[i]))
      {
      this->NoModule = true;
      this->Compatibility_1_6 = false;
      doing = DoingNone;
      }
    else if(doing == DoingComponents)
      {
      // Set a variable telling the find script this component
      // is required.
      std::string req_var = this->Name + "_FIND_REQUIRED_" + args[i];
      this->Makefile->AddDefinition(req_var.c_str(), "1");

      // Append to the list of required components.
      components += components_sep;
      components += args[i];
      components_sep = ";";
      }
    else if(doing == DoingNames)
      {
      this->Names.push_back(args[i]);
      }
    else if(doing == DoingPaths)
      {
      this->AddUserPath(args[i], this->UserPaths);
      }
    else if(doing == DoingHints)
      {
      this->AddUserPath(args[i], this->UserHints);
      }
    else if(doing == DoingPathSuffixes)
      {
      this->AddPathSuffix(args[i]);
      }
    else if(doing == DoingConfigs)
      {
      if(args[i].find_first_of(":/\\") != args[i].npos ||
         cmSystemTools::GetFilenameLastExtension(args[i]) != ".cmake")
        {
        cmOStringStream e;
        e << "given CONFIGS option followed by invalid file name \""
          << args[i] << "\".  The names given must be file names without "
          << "a path and with a \".cmake\" extension.";
        return false;
        }
      this->Configs.push_back(args[i]);
      }
    else if(!haveVersion && version.find(args[i].c_str()))
      {
      haveVersion = true;
      this->Version = args[i];
      }
    else
      {
      cmOStringStream e;
      e << "called with invalid argument \"" << args[i].c_str() << "\"";
      this->SetError(e.str().c_str());
      return false;
      }
    }

  if(!this->Version.empty())
    {
    // Try to parse the version number and store the results that were
    // successfully parsed.
    unsigned int parsed_major;
    unsigned int parsed_minor;
    unsigned int parsed_patch;
    this->VersionCount = sscanf(this->Version.c_str(), "%u.%u.%u",
                                &parsed_major, &parsed_minor, &parsed_patch);
    switch(this->VersionCount)
      {
      case 3: this->VersionPatch = parsed_patch; // no break!
      case 2: this->VersionMinor = parsed_minor; // no break!
      case 1: this->VersionMajor = parsed_major; // no break!
      default: break;
      }
    }

  this->SetModuleVariables(components);

  // See if there is a Find<package>.cmake module.
  if(!this->NoModule)
    {
    bool foundModule = false;
    if(!this->FindModule(foundModule))
      {
      this->AppendSuccessInformation();
      return false;
      }
    if(foundModule)
      {
      this->AppendSuccessInformation();
      return true;
      }
    }

  // No find module.  Assume the project has a CMake config file.  Use
  // a <package>_DIR cache variable to locate it.
  this->Variable = this->Name;
  this->Variable += "_DIR";

  // Add the default name.
  if(this->Names.empty())
    {
    this->Names.push_back(this->Name);
    }

  // Add the default configs.
  if(this->Configs.empty())
    {
    for(std::vector<std::string>::const_iterator ni = this->Names.begin();
        ni != this->Names.end(); ++ni)
      {
      std::string config = *ni;
      config += "Config.cmake";
      this->Configs.push_back(config);

      config = cmSystemTools::LowerCase(*ni);
      config += "-config.cmake";
      this->Configs.push_back(config);
      }
    }

  // Find and load the package.
  bool result = this->HandlePackageMode();
  this->AppendSuccessInformation();
  return result;
}


//----------------------------------------------------------------------------
void cmFindPackageCommand::SetModuleVariables(const std::string& components)
{
  // Store the list of components.
  std::string components_var = this->Name + "_FIND_COMPONENTS";
  this->Makefile->AddDefinition(components_var.c_str(), components.c_str());
   
  if(this->Quiet)
    {
    // Tell the module that is about to be read that it should find
    // quietly.
    std::string quietly = this->Name;
    quietly += "_FIND_QUIETLY";
    this->Makefile->AddDefinition(quietly.c_str(), "1");
    }

  if(this->Required)
    {
    // Tell the module that is about to be read that it should report
    // a fatal error if the package is not found.
    std::string req = this->Name;
    req += "_FIND_REQUIRED";
    this->Makefile->AddDefinition(req.c_str(), "1");
    }

  if(!this->Version.empty())
    {
    // Tell the module that is about to be read what version of the
    // package has been requested.
    std::string ver = this->Name;
    ver += "_FIND_VERSION";
    this->Makefile->AddDefinition(ver.c_str(), this->Version.c_str());
    char buf[64];
    switch(this->VersionCount)
      {
      case 3:
        {
        sprintf(buf, "%u", this->VersionPatch);
        this->Makefile->AddDefinition((ver+"_PATCH").c_str(), buf);
        } // no break
      case 2:
        {
        sprintf(buf, "%u", this->VersionMinor);
        this->Makefile->AddDefinition((ver+"_MINOR").c_str(), buf);
        } // no break
      case 1:
        {
        sprintf(buf, "%u", this->VersionMajor);
        this->Makefile->AddDefinition((ver+"_MAJOR").c_str(), buf);
        } // no break
      default: break;
      }

    // Tell the module whether an exact version has been requested.
    std::string exact = this->Name;
    exact += "_FIND_VERSION_EXACT";
    this->Makefile->AddDefinition(exact.c_str(),
                                  this->VersionExact? "1":"0");
   }
}


//----------------------------------------------------------------------------
bool cmFindPackageCommand::FindModule(bool& found)
{
  std::string module = "Find";
  module += this->Name;
  module += ".cmake";
  std::string mfile = this->Makefile->GetModulesFile(module.c_str());
  if ( mfile.size() )
    {
    // Load the module we found.
    found = true;
    return this->ReadListFile(mfile.c_str());
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::HandlePackageMode()
{
  // Support old capitalization behavior.
  std::string upperDir = cmSystemTools::UpperCase(this->Name);
  std::string upperFound = cmSystemTools::UpperCase(this->Name);
  upperDir += "_DIR";
  upperFound += "_FOUND";
  if(upperDir == this->Variable)
    {
    this->Compatibility_1_6 = false;
    }

  // Try to find the config file.
  const char* def = this->Makefile->GetDefinition(this->Variable.c_str());
  if(this->Compatibility_1_6 && cmSystemTools::IsOff(def))
    {
    // Use the setting of the old name of the variable to provide the
    // value of the new.
    const char* oldDef = this->Makefile->GetDefinition(upperDir.c_str());
    if(!cmSystemTools::IsOff(oldDef))
      {
      this->Makefile->AddDefinition(this->Variable.c_str(), oldDef);
      def = this->Makefile->GetDefinition(this->Variable.c_str());
      }
    }

  // Search for the config file if it is not already found.
  if(cmSystemTools::IsOff(def))
    {
    this->FindConfig();
    def = this->Makefile->GetDefinition(this->Variable.c_str());
    }

  // If the config file was found, load it.
  std::string file;
  bool result = true;
  bool found = false;
  if(!cmSystemTools::IsOff(def))
    {
    // Get the directory from the variable value.
    std::string dir = def;
    cmSystemTools::ConvertToUnixSlashes(dir);

    // Treat relative paths with respect to the current source dir.
    if(!cmSystemTools::FileIsFullPath(dir.c_str()))
      {
      dir = "/" + dir;
      dir = this->Makefile->GetCurrentDirectory() + dir;
      }

    // Find the configuration file.
    if(this->FindConfigFileToLoad(dir, file))
      {
      // Set the version variables before loading the config file.
      // It may override them.
      this->StoreVersionFound();

      // Parse the configuration file.
      if(this->ReadListFile(file.c_str()))
        {
        // The package has been found.
        found = true;
        }
      else
        {
        // The configuration file is invalid.
        result = false;
        }
      }
    else
      {
      // The variable setting is wrong.
      cmOStringStream e;
      e << "cannot find package " << this->Name << " because "
        << this->Variable << " is set to \"" << def << "\" "
        << "which is not a directory containing a package configuration "
        << "file (or it is not for the requested version).  "
        << "Please set the cache entry " << this->Variable << " "
        << "to the correct directory, or delete it to ask CMake "
        << "to search.";
      this->SetError(e.str().c_str());
      result = false;
      }
    }
  else if(!this->Quiet || this->Required)
    {
    // The variable is not set.
    cmOStringStream e;
    e << "Could not find ";
    if(!this->NoModule)
      {
      e << "module Find" << this->Name << ".cmake or ";
      }
    e << "a configuration file for package " << this->Name << ".\n";
    if(!this->NoModule)
      {
      e << "Adjust CMAKE_MODULE_PATH to find Find"
        << this->Name << ".cmake or set ";
      }
    else
      {
      e << "Set ";
      }
    e << this->Variable << " to the directory containing a CMake "
      << "configuration file for " << this->Name << ".  ";
    if(this->Configs.size() == 1)
      {
      e << "The file will be called " << this->Configs[0];
      }
    else
      {
      e << "The file will have one of the following names:\n";
      for(std::vector<std::string>::const_iterator ci = this->Configs.begin();
          ci != this->Configs.end(); ++ci)
        {
        e << "  " << *ci << "\n";
        }
      }
    this->Makefile->IssueMessage(
      this->Required? cmake::FATAL_ERROR : cmake::WARNING, e.str());
    }

  // Set a variable marking whether the package was found.
  std::string foundVar = this->Name;
  foundVar += "_FOUND";
  this->Makefile->AddDefinition(foundVar.c_str(), found? "1":"0");

  // Set a variable naming the configuration file that was found.
  std::string fileVar = this->Name;
  fileVar += "_CONFIG";
  if(found)
    {
    this->Makefile->AddDefinition(fileVar.c_str(), file.c_str());
    }
  else
    {
    this->Makefile->RemoveDefinition(fileVar.c_str());
    }

  // Handle some ancient compatibility stuff.
  if(this->Compatibility_1_6)
    {
    // Listfiles will be looking for the capitalized version of the
    // name.  Provide it.
    this->Makefile->AddDefinition
      (upperDir.c_str(),
       this->Makefile->GetDefinition(this->Variable.c_str()));
    this->Makefile->AddDefinition
      (upperFound.c_str(),
       this->Makefile->GetDefinition(foundVar.c_str()));
    }

#ifdef CMAKE_BUILD_WITH_CMAKE
  if(!(upperDir == this->Variable))
    {
    if(this->Compatibility_1_6)
      {
      // Listfiles may use the capitalized version of the name.
      // Remove any previously added watch.
      this->Makefile->GetVariableWatch()->RemoveWatch(
        upperDir.c_str(),
        cmFindPackageNeedBackwardsCompatibility
        );
      }
    else
      {
      // Listfiles should not be using the capitalized version of the
      // name.  Add a watch to warn the user.
      this->Makefile->GetVariableWatch()->AddWatch(
        upperDir.c_str(),
        cmFindPackageNeedBackwardsCompatibility
        );
      }
    }
#endif

  return result;
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::FindConfig()
{
  // Compute the set of search prefixes.
  this->ComputePrefixes();

  // Look for the project's configuration file.
  bool found = false;

  // Search for frameworks.
  if(!found && this->SearchFrameworkFirst || this->SearchFrameworkOnly)
    {
    found = this->FindFrameworkConfig();
    }

  // Search for apps.
  if(!found && this->SearchAppBundleFirst || this->SearchAppBundleOnly)
    {
    found = this->FindAppBundleConfig();
    }

  // Search prefixes.
  if(!found && !(this->SearchFrameworkOnly || this->SearchAppBundleOnly))
    {
    found = this->FindPrefixedConfig();
    }

  // Search for frameworks.
  if(!found && this->SearchFrameworkLast)
    {
    found = this->FindFrameworkConfig();
    }

  // Search for apps.
  if(!found && this->SearchAppBundleLast)
    {
    found = this->FindAppBundleConfig();
    }

  // Store the entry in the cache so it can be set by the user.
  std::string init;
  if(found)
    {
    init = cmSystemTools::GetFilenamePath(this->FileFound);
    }
  else
    {
    init = this->Variable + "-NOTFOUND";
    }
  std::string help =
    "The directory containing a CMake configuration file for ";
  help += this->Name;
  help += ".";
  this->Makefile->AddCacheDefinition(this->Variable.c_str(),
                                     init.c_str(), help.c_str(),
                                     cmCacheManager::PATH);
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::FindPrefixedConfig()
{
  std::vector<std::string>& prefixes = this->SearchPaths;
  for(std::vector<std::string>::const_iterator pi = prefixes.begin();
      pi != prefixes.end(); ++pi)
    {
    if(this->SearchPrefix(*pi))
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::FindFrameworkConfig()
{
  std::vector<std::string>& prefixes = this->SearchPaths;
  for(std::vector<std::string>::const_iterator i = prefixes.begin();
      i != prefixes.end(); ++i)
    {
    if(this->SearchFrameworkPrefix(*i))
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::FindAppBundleConfig()
{
  std::vector<std::string>& prefixes = this->SearchPaths;
  for(std::vector<std::string>::const_iterator i = prefixes.begin();
      i != prefixes.end(); ++i)
    {
    if(this->SearchAppBundlePrefix(*i))
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::ReadListFile(const char* f)
{
  if(this->Makefile->ReadListFile(this->Makefile->GetCurrentListFile(),f))
    {
    return true;
    }
  std::string e = "Error reading CMake code from \"";
  e += f;
  e += "\".";
  this->SetError(e.c_str());
  return false;
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AppendToProperty(const char* propertyName)
{
  std::string propertyValue;
  const char *prop =
      this->Makefile->GetCMakeInstance()->GetProperty(propertyName);
  if (prop && *prop)
    {
    propertyValue = prop;

    std::vector<std::string> contents;
    cmSystemTools::ExpandListArgument(propertyValue, contents, false);

    bool alreadyInserted = false;
    for(std::vector<std::string>::const_iterator it = contents.begin();
      it != contents.end(); ++ it )
      {
      if (*it == this->Name)
        {
        alreadyInserted = true;
        break;
        }
      }
    if (!alreadyInserted)
      {
      propertyValue += ";";
      propertyValue += this->Name;
      }
    }
  else
    {
    propertyValue = this->Name;
    }
  this->Makefile->GetCMakeInstance()->SetProperty(propertyName,
                                                  propertyValue.c_str());
 }

//----------------------------------------------------------------------------
void cmFindPackageCommand::AppendSuccessInformation()
{
  std::string found = this->Name;
  found += "_FOUND";
  std::string upperFound = cmSystemTools::UpperCase(found);

  const char* upperResult = this->Makefile->GetDefinition(upperFound.c_str());
  const char* result = this->Makefile->GetDefinition(found.c_str());
  if ((cmSystemTools::IsOn(result)) || (cmSystemTools::IsOn(upperResult)))
    {
    this->AppendToProperty("PACKAGES_FOUND");
    if (!this->Quiet)
      {
      this->AppendToProperty("ENABLED_FEATURES");
      }
    }
  else
    {
    this->AppendToProperty("PACKAGES_NOT_FOUND");
    if (!this->Quiet)
      {
      this->AppendToProperty("DISABLED_FEATURES");
      }
    }
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::ComputePrefixes()
{
  this->AddPrefixesCMakeVariable();
  this->AddPrefixesCMakeEnvironment();
  this->AddPrefixesUserHints();
  this->AddPrefixesSystemEnvironment();
  this->AddPrefixesBuilds();
  this->AddPrefixesCMakeSystemVariable();
  this->AddPrefixesUserGuess();
  this->ComputeFinalPrefixes();
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AddPrefixesCMakeEnvironment()
{
  if(!this->NoCMakeEnvironmentPath && !this->NoDefaultPath)
    {
    // Check the environment variable with the same name as the cache
    // entry.
    std::string env;
    if(cmSystemTools::GetEnv(this->Variable.c_str(), env) && env.length() > 0)
      {
      cmSystemTools::ConvertToUnixSlashes(env);
      this->AddPathInternal(env, EnvPath);
      }

    this->AddEnvPath("CMAKE_PREFIX_PATH");
    this->AddEnvPath("CMAKE_FRAMEWORK_PATH");
    this->AddEnvPath("CMAKE_APPBUNDLE_PATH");
    }
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AddPrefixesCMakeVariable()
{
  if(!this->NoCMakePath && !this->NoDefaultPath)
    {
    this->AddCMakePath("CMAKE_PREFIX_PATH");
    this->AddCMakePath("CMAKE_FRAMEWORK_PATH");
    this->AddCMakePath("CMAKE_APPBUNDLE_PATH");
    }
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AddPrefixesSystemEnvironment()
{
  if(!this->NoSystemEnvironmentPath && !this->NoDefaultPath)
    {
    // Use the system search path to generate prefixes.
    // Relative paths are interpreted with respect to the current
    // working directory.
    std::vector<std::string> tmp;
    cmSystemTools::GetPath(tmp);
    for(std::vector<std::string>::iterator i = tmp.begin();
        i != tmp.end(); ++i)
      {
      std::string const& d = *i;

      // If the path is a PREFIX/bin case then add its parent instead.
      if(d.size() >= 4 && strcmp(d.c_str()+d.size()-4, "/bin") == 0 ||
         d.size() >= 5 && strcmp(d.c_str()+d.size()-5, "/sbin") == 0)
        {
        this->AddPathInternal(cmSystemTools::GetFilenamePath(d), EnvPath);
        }
      else
        {
        this->AddPathInternal(d, EnvPath);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AddPrefixesBuilds()
{
  if(!this->NoBuilds && !this->NoDefaultPath)
    {
    // It is likely that CMake will have recently built the project.
    for(int i=1; i <= 10; ++i)
      {
      cmOStringStream r;
      r <<
        "[HKEY_CURRENT_USER\\Software\\Kitware\\CMakeSetup\\"
        "Settings\\StartPath;WhereBuild" << i << "]";
      std::string f = r.str();
      cmSystemTools::ExpandRegistryValues(f);
      cmSystemTools::ConvertToUnixSlashes(f);
      if(cmSystemTools::FileIsFullPath(f.c_str()) &&
         cmSystemTools::FileIsDirectory(f.c_str()))
        {
        this->AddPathInternal(f, FullPath);
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AddPrefixesCMakeSystemVariable()
{
  if(!this->NoCMakeSystemPath && !this->NoDefaultPath)
    {
    this->AddCMakePath("CMAKE_SYSTEM_PREFIX_PATH");
    this->AddCMakePath("CMAKE_SYSTEM_FRAMEWORK_PATH");
    this->AddCMakePath("CMAKE_SYSTEM_APPBUNDLE_PATH");
    }
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AddPrefixesUserGuess()
{
  // Add guesses specified by the caller.
  this->AddPathsInternal(this->UserPaths, CMakePath);
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AddPrefixesUserHints()
{
  // Add hints specified by the caller.
  this->AddPathsInternal(this->UserHints, CMakePath);
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::ComputeFinalPrefixes()
{
  std::vector<std::string>& prefixes = this->SearchPaths;

  // Construct the final set of prefixes.
  this->RerootPaths(prefixes);

  // Add a trailing slash to all prefixes to aid the search process.
  this->AddTrailingSlashes(prefixes);
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::SearchDirectory(std::string const& dir)
{
  assert(!dir.empty() && dir[dir.size()-1] == '/');

  // Check each path suffix on this directory.
  for(std::vector<std::string>::const_iterator
        si = this->SearchPathSuffixes.begin();
      si != this->SearchPathSuffixes.end(); ++si)
    {
    std::string d = dir;
    if(!si->empty())
      {
      d += *si;
      d += "/";
      }
    if(this->CheckDirectory(d))
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::CheckDirectory(std::string const& dir)
{
  assert(!dir.empty() && dir[dir.size()-1] == '/');

  // Look for the file in this directory.
  std::string d = dir.substr(0, dir.size()-1);
  if(this->FindConfigFile(d, this->FileFound))
    {
    // Remove duplicate slashes.
    cmSystemTools::ConvertToUnixSlashes(this->FileFound);
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::FindConfigFile(std::string const& dir,
                                          std::string& file)
{
  for(std::vector<std::string>::const_iterator ci = this->Configs.begin();
      ci != this->Configs.end(); ++ci)
    {
    file = dir;
    file += "/";
    file += *ci;
    if(this->DebugMode)
      {
      fprintf(stderr, "Checking file [%s]\n", file.c_str());
      }
    if(cmSystemTools::FileExists(file.c_str(), true) &&
       this->CheckVersion(file))
      {
      return true;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::FindConfigFileToLoad(std::string const& dir,
                                                std::string& file)
{
  if(this->FileFound.empty())
    {
    // The file location was cached.  Look for the correct file.
    return this->FindConfigFile(dir, file);
    }
  else
    {
    // The file location was just found during this call.
    // Use the file found without searching again.
    file = this->FileFound;
    return true;
    }
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::CheckVersion(std::string const& config_file)
{
  // Get the filename without the .cmake extension.
  std::string::size_type pos = config_file.rfind('.');
  std::string version_file_base = config_file.substr(0, pos);

  // Look for foo-config-version.cmake
  std::string version_file = version_file_base;
  version_file += "-version.cmake";
  if(cmSystemTools::FileExists(version_file.c_str(), true))
    {
    return this->CheckVersionFile(version_file);
    }

  // Look for fooConfigVersion.cmake
  version_file = version_file_base;
  version_file += "Version.cmake";
  if(cmSystemTools::FileExists(version_file.c_str(), true))
    {
    return this->CheckVersionFile(version_file);
    }

  // If no version was requested a versionless package is acceptable.
  if(this->Version.empty())
    {
    return true;
    }

  // No version file found.  Assume the version is incompatible.
  return false;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::CheckVersionFile(std::string const& version_file)
{
  // The version file will be loaded in an isolated scope.
  this->Makefile->PushScope();

  // Clear the output variables.
  this->Makefile->RemoveDefinition("PACKAGE_VERSION");
  this->Makefile->RemoveDefinition("PACKAGE_VERSION_COMPATIBLE");
  this->Makefile->RemoveDefinition("PACKAGE_VERSION_EXACT");

  // Set the input variables.
  this->Makefile->AddDefinition("PACKAGE_FIND_NAME", this->Name.c_str());
  this->Makefile->AddDefinition("PACKAGE_FIND_VERSION",
                                this->Version.c_str());
  if(this->VersionCount >= 3)
    {
    char buf[64];
    sprintf(buf, "%u", this->VersionPatch);
    this->Makefile->AddDefinition("PACKAGE_FIND_VERSION_PATCH", buf);
    }
  else
    {
    this->Makefile->RemoveDefinition("PACKAGE_FIND_VERSION_PATCH");
    }
  if(this->VersionCount >= 2)
    {
    char buf[64];
    sprintf(buf, "%u", this->VersionMinor);
    this->Makefile->AddDefinition("PACKAGE_FIND_VERSION_MINOR", buf);
    }
  else
    {
    this->Makefile->RemoveDefinition("PACKAGE_FIND_VERSION_MINOR");
    }
  if(this->VersionCount >= 1)
    {
    char buf[64];
    sprintf(buf, "%u", this->VersionMajor);
    this->Makefile->AddDefinition("PACKAGE_FIND_VERSION_MAJOR", buf);
    }
  else
    {
    this->Makefile->RemoveDefinition("PACKAGE_FIND_VERSION_MAJOR");
    }

  // Load the version check file.
  bool found = false;
  if(this->ReadListFile(version_file.c_str()))
    {
    // Check the output variables.
    found = this->Makefile->IsOn("PACKAGE_VERSION_EXACT");
    if(!found && !this->VersionExact)
      {
      found = this->Makefile->IsOn("PACKAGE_VERSION_COMPATIBLE");
      }
    if(found || this->Version.empty())
      {
      // Get the version found.
      this->VersionFound =
        this->Makefile->GetSafeDefinition("PACKAGE_VERSION");

      // Try to parse the version number and store the results that were
      // successfully parsed.
      unsigned int parsed_major;
      unsigned int parsed_minor;
      unsigned int parsed_patch;
      this->VersionFoundCount =
        sscanf(this->VersionFound.c_str(), "%u.%u.%u",
               &parsed_major, &parsed_minor, &parsed_patch);
      switch(this->VersionFoundCount)
        {
        case 3: this->VersionFoundPatch = parsed_patch; // no break!
        case 2: this->VersionFoundMinor = parsed_minor; // no break!
        case 1: this->VersionFoundMajor = parsed_major; // no break!
        default: break;
        }
      }
    }

  // Restore the original scope.
  this->Makefile->PopScope();

  // Succeed if the version was found or no version was requested.
  return found || this->Version.empty();
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::StoreVersionFound()
{
  // Store the whole version string.
  std::string ver = this->Name;
  ver += "_VERSION";
  if(this->VersionFound.empty())
    {
    this->Makefile->RemoveDefinition(ver.c_str());
    }
  else
    {
    this->Makefile->AddDefinition(ver.c_str(), this->VersionFound.c_str());
    }

  // Store the portions that could be parsed.
  char buf[64];
  switch(this->VersionFoundCount)
    {
    case 3:
      {
      sprintf(buf, "%u", this->VersionFoundPatch);
      this->Makefile->AddDefinition((ver+"_PATCH").c_str(), buf);
      } // no break
    case 2:
      {
      sprintf(buf, "%u", this->VersionFoundMinor);
      this->Makefile->AddDefinition((ver+"_MINOR").c_str(), buf);
      } // no break
    case 1:
      {
      sprintf(buf, "%u", this->VersionFoundMajor);
      this->Makefile->AddDefinition((ver+"_MAJOR").c_str(), buf);
      } // no break
    default: break;
    }
}

//----------------------------------------------------------------------------
#include <cmsys/Directory.hxx>
#include <cmsys/Glob.hxx>
#include <cmsys/String.h>
#include <cmsys/auto_ptr.hxx>

class cmFileList;
class cmFileListGeneratorBase
{
public:
  virtual ~cmFileListGeneratorBase() {}
protected:
  bool Consider(std::string const& fullPath, cmFileList& listing);
private:
  bool Search(cmFileList&);
  virtual bool Search(std::string const& parent, cmFileList&) = 0;
  virtual cmsys::auto_ptr<cmFileListGeneratorBase> Clone() const = 0;
  friend class cmFileList;
  cmFileListGeneratorBase* SetNext(cmFileListGeneratorBase const& next);
  cmsys::auto_ptr<cmFileListGeneratorBase> Next;
};

class cmFileList
{
public:
  cmFileList(): First(), Last(0) {}
  virtual ~cmFileList() {}
  cmFileList& operator/(cmFileListGeneratorBase const& rhs)
    {
    if(this->Last)
      {
      this->Last = this->Last->SetNext(rhs);
      }
    else
      {
      this->First = rhs.Clone();
      this->Last = this->First.get();
      }
    return *this;
    }
  bool Search()
    {
    if(this->First.get())
      {
      return this->First->Search(*this);
      }
    return false;
    }
private:
  virtual bool Visit(std::string const& fullPath) = 0;
  friend class cmFileListGeneratorBase;
  cmsys::auto_ptr<cmFileListGeneratorBase> First;
  cmFileListGeneratorBase* Last;
};

class cmFindPackageFileList: public cmFileList
{
public:
  cmFindPackageFileList(cmFindPackageCommand* fpc,
                        bool use_suffixes = true):
    cmFileList(), FPC(fpc), UseSuffixes(use_suffixes) {}
private:
  bool Visit(std::string const& fullPath)
    {
    if(this->UseSuffixes)
      {
      return this->FPC->SearchDirectory(fullPath);
      }
    else
      {
      return this->FPC->CheckDirectory(fullPath);
      }
    }
  cmFindPackageCommand* FPC;
  bool UseSuffixes;
};

bool cmFileListGeneratorBase::Search(cmFileList& listing)
{
  return this->Search("", listing);
}

cmFileListGeneratorBase*
cmFileListGeneratorBase::SetNext(cmFileListGeneratorBase const& next)
{
  this->Next = next.Clone();
  return this->Next.get();
}

bool cmFileListGeneratorBase::Consider(std::string const& fullPath,
                                       cmFileList& listing)
{
  if(this->Next.get())
    {
    return this->Next->Search(fullPath + "/", listing);
    }
  else
    {
    return listing.Visit(fullPath + "/");
    }
}

class cmFileListGeneratorFixed: public cmFileListGeneratorBase
{
public:
  cmFileListGeneratorFixed(std::string const& str):
    cmFileListGeneratorBase(), String(str) {}
  cmFileListGeneratorFixed(cmFileListGeneratorFixed const& r):
    cmFileListGeneratorBase(), String(r.String) {}
private:
  std::string String;
  virtual bool Search(std::string const& parent, cmFileList& lister)
    {
    std::string fullPath = parent + this->String;
    return this->Consider(fullPath, lister);
    }
  virtual cmsys::auto_ptr<cmFileListGeneratorBase> Clone() const
    {
    cmsys::auto_ptr<cmFileListGeneratorBase>
      g(new cmFileListGeneratorFixed(*this));
    return g;
    }
};

class cmFileListGeneratorEnumerate: public cmFileListGeneratorBase
{
public:
  cmFileListGeneratorEnumerate(std::vector<std::string> const& v):
    cmFileListGeneratorBase(), Vector(v) {}
  cmFileListGeneratorEnumerate(cmFileListGeneratorEnumerate const& r):
    cmFileListGeneratorBase(), Vector(r.Vector) {}
private:
  std::vector<std::string> const& Vector;
  virtual bool Search(std::string const& parent, cmFileList& lister)
    {
    for(std::vector<std::string>::const_iterator i = this->Vector.begin();
        i != this->Vector.end(); ++i)
      {
      if(this->Consider(parent + *i, lister))
        {
        return true;
        }
      }
    return false;
    }
  virtual cmsys::auto_ptr<cmFileListGeneratorBase> Clone() const
    {
    cmsys::auto_ptr<cmFileListGeneratorBase>
      g(new cmFileListGeneratorEnumerate(*this));
    return g;
    }
};

class cmFileListGeneratorProject: public cmFileListGeneratorBase
{
public:
  cmFileListGeneratorProject(std::vector<std::string> const& names):
    cmFileListGeneratorBase(), Names(names) {}
  cmFileListGeneratorProject(cmFileListGeneratorProject const& r):
    cmFileListGeneratorBase(), Names(r.Names) {}
private:
  std::vector<std::string> const& Names;
  virtual bool Search(std::string const& parent, cmFileList& lister)
    {
    // Construct a list of matches.
    std::vector<std::string> matches;
    cmsys::Directory d;
    d.Load(parent.c_str());
    for(unsigned long i=0; i < d.GetNumberOfFiles(); ++i)
      {
      const char* fname = d.GetFile(i);
      if(strcmp(fname, ".") == 0 ||
         strcmp(fname, "..") == 0)
        {
        continue;
        }
      for(std::vector<std::string>::const_iterator ni = this->Names.begin();
          ni != this->Names.end(); ++ni)
        {
        if(cmsysString_strncasecmp(fname, ni->c_str(),
                                   ni->length()) == 0)
          {
          matches.push_back(fname);
          }
        }
      }

    for(std::vector<std::string>::const_iterator i = matches.begin();
        i != matches.end(); ++i)
      {
      if(this->Consider(parent + *i, lister))
        {
        return true;
        }
      }
    return false;
    }
  virtual cmsys::auto_ptr<cmFileListGeneratorBase> Clone() const
    {
    cmsys::auto_ptr<cmFileListGeneratorBase>
      g(new cmFileListGeneratorProject(*this));
    return g;
    }
};

class cmFileListGeneratorMacProject: public cmFileListGeneratorBase
{
public:
  cmFileListGeneratorMacProject(std::vector<std::string> const& names,
                                const char* ext):
    cmFileListGeneratorBase(), Names(names), Extension(ext) {}
  cmFileListGeneratorMacProject(cmFileListGeneratorMacProject const& r):
    cmFileListGeneratorBase(), Names(r.Names), Extension(r.Extension) {}
private:
  std::vector<std::string> const& Names;
  std::string Extension;
  virtual bool Search(std::string const& parent, cmFileList& lister)
    {
    // Construct a list of matches.
    std::vector<std::string> matches;
    cmsys::Directory d;
    d.Load(parent.c_str());
    for(unsigned long i=0; i < d.GetNumberOfFiles(); ++i)
      {
      const char* fname = d.GetFile(i);
      if(strcmp(fname, ".") == 0 ||
         strcmp(fname, "..") == 0)
        {
        continue;
        }
      for(std::vector<std::string>::const_iterator ni = this->Names.begin();
          ni != this->Names.end(); ++ni)
        {
        std::string name = *ni;
        name += this->Extension;
        if(cmsysString_strcasecmp(fname, name.c_str()) == 0)
          {
          matches.push_back(fname);
          }
        }
      }

    for(std::vector<std::string>::const_iterator i = matches.begin();
        i != matches.end(); ++i)
      {
      if(this->Consider(parent + *i, lister))
        {
        return true;
        }
      }
    return false;
    }
  virtual cmsys::auto_ptr<cmFileListGeneratorBase> Clone() const
    {
    cmsys::auto_ptr<cmFileListGeneratorBase>
      g(new cmFileListGeneratorMacProject(*this));
    return g;
    }
};

class cmFileListGeneratorCaseInsensitive: public cmFileListGeneratorBase
{
public:
  cmFileListGeneratorCaseInsensitive(std::string const& str):
    cmFileListGeneratorBase(), String(str) {}
  cmFileListGeneratorCaseInsensitive(
    cmFileListGeneratorCaseInsensitive const& r):
    cmFileListGeneratorBase(), String(r.String) {}
private:
  std::string String;
  virtual bool Search(std::string const& parent, cmFileList& lister)
    {
    // Look for matching files.
    std::vector<std::string> matches;
    cmsys::Directory d;
    d.Load(parent.c_str());
    for(unsigned long i=0; i < d.GetNumberOfFiles(); ++i)
      {
      const char* fname = d.GetFile(i);
      if(strcmp(fname, ".") == 0 ||
         strcmp(fname, "..") == 0)
        {
        continue;
        }
      if(cmsysString_strcasecmp(fname, this->String.c_str()) == 0)
        {
        if(this->Consider(parent + fname, lister))
          {
          return true;
          }
        }
      }
    return false;
    }
  virtual cmsys::auto_ptr<cmFileListGeneratorBase> Clone() const
    {
    cmsys::auto_ptr<cmFileListGeneratorBase>
      g(new cmFileListGeneratorCaseInsensitive(*this));
    return g;
    }
};

class cmFileListGeneratorGlob: public cmFileListGeneratorBase
{
public:
  cmFileListGeneratorGlob(std::string const& str):
    cmFileListGeneratorBase(), Pattern(str) {}
  cmFileListGeneratorGlob(cmFileListGeneratorGlob const& r):
    cmFileListGeneratorBase(), Pattern(r.Pattern) {}
private:
  std::string Pattern;
  virtual bool Search(std::string const& parent, cmFileList& lister)
    {
    // Glob the set of matching files.
    std::string expr = parent;
    expr += this->Pattern;
    cmsys::Glob g;
    if(!g.FindFiles(expr))
      {
      return false;
      }
    std::vector<std::string> const& files = g.GetFiles();

    // Look for directories among the matches.
    for(std::vector<std::string>::const_iterator fi = files.begin();
        fi != files.end(); ++fi)
      {
      if(cmSystemTools::FileIsDirectory(fi->c_str()))
        {
        if(this->Consider(*fi, lister))
          {
          return true;
          }
        }
      }
    return false;
    }
  virtual cmsys::auto_ptr<cmFileListGeneratorBase> Clone() const
    {
    cmsys::auto_ptr<cmFileListGeneratorBase>
      g(new cmFileListGeneratorGlob(*this));
    return g;
    }
};

//----------------------------------------------------------------------------
bool cmFindPackageCommand::SearchPrefix(std::string const& prefix_in)
{
  assert(!prefix_in.empty() && prefix_in[prefix_in.size()-1] == '/');
  if(this->DebugMode)
    {
    fprintf(stderr, "Checking prefix [%s]\n", prefix_in.c_str());
    }

  // Skip this if the prefix does not exist.
  if(!cmSystemTools::FileIsDirectory(prefix_in.c_str()))
    {
    return false;
    }

  //  PREFIX/ (useful on windows or in build trees)
  if(this->SearchDirectory(prefix_in))
    {
    return true;
    }

  // Strip the trailing slash because the path generator is about to
  // add one.
  std::string prefix = prefix_in.substr(0, prefix_in.size()-1);

  //  PREFIX/(cmake|CMake)/ (useful on windows or in build trees)
  {
  cmFindPackageFileList lister(this);
  lister
    / cmFileListGeneratorFixed(prefix)
    / cmFileListGeneratorCaseInsensitive("cmake");
  if(lister.Search())
    {
    return true;
    }
  }

  // Construct list of common install locations (lib and share).
  std::vector<std::string> common;
  if(this->UseLib64Paths)
    {
    common.push_back("lib64");
    }
  common.push_back("lib");
  common.push_back("share");

  //  PREFIX/(share|lib)/(Foo|foo|FOO).*/
  {
  cmFindPackageFileList lister(this);
  lister
    / cmFileListGeneratorFixed(prefix)
    / cmFileListGeneratorEnumerate(common)
    / cmFileListGeneratorProject(this->Names);
  if(lister.Search())
    {
    return true;
    }
  }

  //  PREFIX/(share|lib)/(Foo|foo|FOO).*/(cmake|CMake)/
  {
  cmFindPackageFileList lister(this);
  lister
    / cmFileListGeneratorFixed(prefix)
    / cmFileListGeneratorEnumerate(common)
    / cmFileListGeneratorProject(this->Names)
    / cmFileListGeneratorCaseInsensitive("cmake");
  if(lister.Search())
    {
    return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::SearchFrameworkPrefix(std::string const& prefix_in)
{
  assert(!prefix_in.empty() && prefix_in[prefix_in.size()-1] == '/');
  if(this->DebugMode)
    {
    fprintf(stderr, "Checking framework prefix [%s]\n", prefix_in.c_str());
    }

  // Strip the trailing slash because the path generator is about to
  // add one.
  std::string prefix = prefix_in.substr(0, prefix_in.size()-1);

  // <prefix>/Foo.framework/Resources/
  {
  cmFindPackageFileList lister(this);
  lister
    / cmFileListGeneratorFixed(prefix)
    / cmFileListGeneratorMacProject(this->Names, ".framework")
    / cmFileListGeneratorFixed("Resources");
  if(lister.Search())
    {
    return true;
    }
  }
  // <prefix>/Foo.framework/Resources/CMake/
  {
  cmFindPackageFileList lister(this);
  lister
    / cmFileListGeneratorFixed(prefix)
    / cmFileListGeneratorMacProject(this->Names, ".framework")
    / cmFileListGeneratorFixed("Resources")
    / cmFileListGeneratorCaseInsensitive("cmake");
  if(lister.Search())
    {
    return true;
    }
  }

  // <prefix>/Foo.framework/Versions/*/Resources/
  {
  cmFindPackageFileList lister(this);
  lister
    / cmFileListGeneratorFixed(prefix)
    / cmFileListGeneratorMacProject(this->Names, ".framework")
    / cmFileListGeneratorFixed("Versions")
    / cmFileListGeneratorGlob("*/Resources");
  if(lister.Search())
    {
    return true;
    }
  }

  // <prefix>/Foo.framework/Versions/*/Resources/CMake/
  {
  cmFindPackageFileList lister(this);
  lister
    / cmFileListGeneratorFixed(prefix)
    / cmFileListGeneratorMacProject(this->Names, ".framework")
    / cmFileListGeneratorFixed("Versions")
    / cmFileListGeneratorGlob("*/Resources")
    / cmFileListGeneratorCaseInsensitive("cmake");
  if(lister.Search())
    {
    return true;
    }
  }

  return false;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::SearchAppBundlePrefix(std::string const& prefix_in)
{
  assert(!prefix_in.empty() && prefix_in[prefix_in.size()-1] == '/');
  if(this->DebugMode)
    {
    fprintf(stderr, "Checking bundle prefix [%s]\n", prefix_in.c_str());
    }

  // Strip the trailing slash because the path generator is about to
  // add one.
  std::string prefix = prefix_in.substr(0, prefix_in.size()-1);

  // <prefix>/Foo.app/Contents/Resources
  {
  cmFindPackageFileList lister(this);
  lister
    / cmFileListGeneratorFixed(prefix)
    / cmFileListGeneratorMacProject(this->Names, ".app")
    / cmFileListGeneratorFixed("Contents/Resources");
  if(lister.Search())
    {
    return true;
    }
  }

  // <prefix>/Foo.app/Contents/Resources/CMake
  {
  cmFindPackageFileList lister(this);
  lister
    / cmFileListGeneratorFixed(prefix)
    / cmFileListGeneratorMacProject(this->Names, ".app")
    / cmFileListGeneratorFixed("Contents/Resources")
    / cmFileListGeneratorCaseInsensitive("cmake");
  if(lister.Search())
    {
    return true;
    }
  }

  return false;
}

// TODO: Version numbers?  Perhaps have a listing component class that
// sorts by lexicographic and numerical ordering.  Also try to match
// some command argument for the version.  Alternatively provide an
// API that just returns a list of valid directories?  Perhaps push a
// scope and try loading the target file just to get its version
// number?  Could add a foo-version.cmake or FooVersion.cmake file
// in the projects that contains just version information.

// TODO: Debug cmsys::Glob double slash problem.

// TODO: Add registry entries after cmake system search path?
// Currently the user must specify them with the PATHS option.
//
//  [HKEY_CURRENT_USER\Software\*\Foo*;InstallDir]
//  [HKEY_CURRENT_USER\Software\*\*\Foo*;InstallDir]
//  [HKEY_LOCAL_MACHINE\Software\*\Foo*;InstallDir]
//  [HKEY_LOCAL_MACHINE\Software\*\*\Foo*;InstallDir]
