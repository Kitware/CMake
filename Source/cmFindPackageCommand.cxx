/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmFindPackageCommand.h"

#include <cmsys/Directory.hxx>
#include <cmsys/RegularExpression.hxx>

#ifdef CMAKE_BUILD_WITH_CMAKE
#include "cmVariableWatch.h"
#endif

#if defined(__HAIKU__)
#include <StorageKit.h>
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
  this->CMakePathName = "PACKAGE";
  this->Quiet = false;
  this->Required = false;
  this->NoUserRegistry = false;
  this->NoSystemRegistry = false;
  this->NoBuilds = false;
  this->UseConfigFiles = true;
  this->UseFindModules = true;
  this->DebugMode = false;
  this->UseLib64Paths = false;
  this->PolicyScope = true;
  this->VersionMajor = 0;
  this->VersionMinor = 0;
  this->VersionPatch = 0;
  this->VersionTweak = 0;
  this->VersionCount = 0;
  this->VersionExact = false;
  this->VersionFoundMajor = 0;
  this->VersionFoundMinor = 0;
  this->VersionFoundPatch = 0;
  this->VersionFoundTweak = 0;
  this->VersionFoundCount = 0;
  this->RequiredCMakeVersion = 0;
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::GenerateDocumentation()
{
  this->cmFindCommon::GenerateDocumentation();
  cmSystemTools::ReplaceString(this->GenericDocumentationRootPath,
                               "CMAKE_FIND_ROOT_PATH_MODE_XXX",
                               "CMAKE_FIND_ROOT_PATH_MODE_PACKAGE");
  cmSystemTools::ReplaceString(this->GenericDocumentationPathsOrder,
                               "FIND_ARGS_XXX", "<package>");
  cmSystemTools::ReplaceString(this->GenericDocumentationPathsOrder,
                               "FIND_XXX", "find_package");
  this->CommandDocumentation =
    "  find_package(<package> [version] [EXACT] [QUIET] [MODULE]\n"
    "               [REQUIRED] [[COMPONENTS] [components...]]\n"
    "               [OPTIONAL_COMPONENTS components...]\n"
    "               [NO_POLICY_SCOPE])\n"
    "Finds and loads settings from an external project.  "
    "<package>_FOUND will be set to indicate whether the package was found.  "
    "When the package is found package-specific information is provided "
    "through variables and imported targets documented by the package"
    "itself.  "
    "The QUIET option disables messages if the package cannot be found.  "
    "The MODULE option disables the second signature documented below.  "
    "The REQUIRED option stops processing with an error message if the "
    "package cannot be found."
    "\n"
    "A package-specific list of required components may be listed after the "
    "COMPONENTS option (or after the REQUIRED option if present).  "
    "Additional optional components may be listed after OPTIONAL_COMPONENTS.  "
    "Available components and their influence on whether a package is "
    "considered to be found are defined by the target package."
    "\n"
    "The [version] argument requests a version with which the package found "
    "should be compatible (format is major[.minor[.patch[.tweak]]]).  "
    "The EXACT option requests that the version be matched exactly.  "
    "If no [version] and/or component list is given to a recursive "
    "invocation inside a find-module, the corresponding arguments "
    "are forwarded automatically from the outer call (including the "
    "EXACT flag for [version]).  "
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
    "If no module is found and the MODULE option is not given the command "
    "proceeds to Config mode.\n"
    "The complete Config mode command signature is:\n"
    "  find_package(<package> [version] [EXACT] [QUIET]\n"
    "               [REQUIRED] [[COMPONENTS] [components...]]\n"
    "               [CONFIG|NO_MODULE]\n"
    "               [NO_POLICY_SCOPE]\n"
    "               [NAMES name1 [name2 ...]]\n"
    "               [CONFIGS config1 [config2 ...]]\n"
    "               [HINTS path1 [path2 ... ]]\n"
    "               [PATHS path1 [path2 ... ]]\n"
    "               [PATH_SUFFIXES suffix1 [suffix2 ...]]\n"
    "               [NO_DEFAULT_PATH]\n"
    "               [NO_CMAKE_ENVIRONMENT_PATH]\n"
    "               [NO_CMAKE_PATH]\n"
    "               [NO_SYSTEM_ENVIRONMENT_PATH]\n"
    "               [NO_CMAKE_PACKAGE_REGISTRY]\n"
    "               [NO_CMAKE_BUILDS_PATH]\n"
    "               [NO_CMAKE_SYSTEM_PATH]\n"
    "               [NO_CMAKE_SYSTEM_PACKAGE_REGISTRY]\n"
    "               [CMAKE_FIND_ROOT_PATH_BOTH |\n"
    "                ONLY_CMAKE_FIND_ROOT_PATH |\n"
    "                NO_CMAKE_FIND_ROOT_PATH])\n"
    "The CONFIG option may be used to skip Module mode explicitly and "
    "switch to Config mode.  It is synonymous to using NO_MODULE.  "
    "Config mode is also implied by use of options not specified in the "
    "reduced signature.  "
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
    "All configuration files which have been considered by CMake while "
    "searching for an installation of the package with an appropriate "
    "version are stored in the cmake variable <package>_CONSIDERED_CONFIGS, "
    "the associated versions in <package>_CONSIDERED_VERSIONS. "
    "\n"
    "If the package configuration file cannot be found CMake "
    "will generate an error describing the problem unless the QUIET "
    "argument is specified.  If REQUIRED is specified and the package "
    "is not found a fatal error is generated and the configure step stops "
    "executing.  If <package>_DIR has been set to a directory not containing "
    "a configuration file CMake will ignore it and search from scratch."
    "\n"
    "When the [version] argument is given Config mode will only find a "
    "version of the package that claims compatibility with the requested "
    "version (format is major[.minor[.patch[.tweak]]]).  "
    "If the EXACT option is given only a version of the package claiming "
    "an exact match of the requested version may be found.  "
    "CMake does not establish any convention for the meaning of version "
    "numbers.  "
    "Package version numbers are checked by \"version\" files provided by "
    "the packages themselves.  "
    "For a candidate package configuration file \"<config-file>.cmake\" the "
    "corresponding version file is located next to it and named either "
    "\"<config-file>-version.cmake\" or \"<config-file>Version.cmake\".  "
    "If no such version file is available then the configuration file "
    "is assumed to not be compatible with any requested version.  "
    "A basic version file containing generic version matching code can be "
    "created using the macro write_basic_package_version_file(), see its "
    "documentation for more details.  "
    "When a version file is found it is loaded to check the requested "
    "version number.  "
    "The version file is loaded in a nested scope in which the following "
    "variables have been defined:\n"
    "  PACKAGE_FIND_NAME          = the <package> name\n"
    "  PACKAGE_FIND_VERSION       = full requested version string\n"
    "  PACKAGE_FIND_VERSION_MAJOR = major version if requested, else 0\n"
    "  PACKAGE_FIND_VERSION_MINOR = minor version if requested, else 0\n"
    "  PACKAGE_FIND_VERSION_PATCH = patch version if requested, else 0\n"
    "  PACKAGE_FIND_VERSION_TWEAK = tweak version if requested, else 0\n"
    "  PACKAGE_FIND_VERSION_COUNT = number of version components, 0 to 4\n"
    "The version file checks whether it satisfies the requested version "
    "and sets these variables:\n"
    "  PACKAGE_VERSION            = full provided version string\n"
    "  PACKAGE_VERSION_EXACT      = true if version is exact match\n"
    "  PACKAGE_VERSION_COMPATIBLE = true if version is compatible\n"
    "  PACKAGE_VERSION_UNSUITABLE = true if unsuitable as any version\n"
    "These variables are checked by the find_package command to determine "
    "whether the configuration file provides an acceptable version.  "
    "They are not available after the find_package call returns.  "
    "If the version is acceptable the following variables are set:\n"
    "  <package>_VERSION       = full provided version string\n"
    "  <package>_VERSION_MAJOR = major version if provided, else 0\n"
    "  <package>_VERSION_MINOR = minor version if provided, else 0\n"
    "  <package>_VERSION_PATCH = patch version if provided, else 0\n"
    "  <package>_VERSION_TWEAK = tweak version if provided, else 0\n"
    "  <package>_VERSION_COUNT = number of version components, 0 to 4\n"
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
    "  <prefix>/<name>*/                                       (W)\n"
    "  <prefix>/<name>*/(cmake|CMake)/                         (W)\n"
    "  <prefix>/(lib/<arch>|lib|share)/cmake/<name>*/          (U)\n"
    "  <prefix>/(lib/<arch>|lib|share)/<name>*/                (U)\n"
    "  <prefix>/(lib/<arch>|lib|share)/<name>*/(cmake|CMake)/  (U)\n"
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
    "Paths with lib/<arch> are enabled if CMAKE_LIBRARY_ARCHITECTURE is set.  "
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
    "   <package>_DIR\n"
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
    "6. Search paths stored in the CMake user package registry.  "
    "This can be skipped if NO_CMAKE_PACKAGE_REGISTRY is passed.  "
    "On Windows a <package> may appear under registry key\n"
    "  HKEY_CURRENT_USER\\Software\\Kitware\\CMake\\Packages\\<package>\n"
    "as a REG_SZ value, with arbitrary name, that specifies the directory "
    "containing the package configuration file.  "
    "On UNIX platforms a <package> may appear under the directory\n"
    "  ~/.cmake/packages/<package>\n"
    "as a file, with arbitrary name, whose content specifies the directory "
    "containing the package configuration file.  "
    "See the export(PACKAGE) command to create user package registry entries "
    "for project build trees."
    "\n"
    "7. Search cmake variables defined in the Platform files "
    "for the current system.  This can be skipped if NO_CMAKE_SYSTEM_PATH "
    "is passed.\n"
    "   CMAKE_SYSTEM_PREFIX_PATH\n"
    "   CMAKE_SYSTEM_FRAMEWORK_PATH\n"
    "   CMAKE_SYSTEM_APPBUNDLE_PATH\n"
    "8. Search paths stored in the CMake system package registry.  "
    "This can be skipped if NO_CMAKE_SYSTEM_PACKAGE_REGISTRY is passed.  "
    "On Windows a <package> may appear under registry key\n"
    "  HKEY_LOCAL_MACHINE\\Software\\Kitware\\CMake\\Packages\\<package>\n"
    "as a REG_SZ value, with arbitrary name, that specifies the directory "
    "containing the package configuration file.  "
    "There is no system package registry on non-Windows platforms."
    "\n"
    "9. Search paths specified by the PATHS option.  "
    "These are typically hard-coded guesses.\n"
    ;
  this->CommandDocumentation += this->GenericDocumentationMacPolicy;
  this->CommandDocumentation += this->GenericDocumentationRootPath;
  this->CommandDocumentation += this->GenericDocumentationPathsOrder;
  this->CommandDocumentation +=
    "\n"
    "Every non-REQUIRED find_package() call can be disabled by setting the "
    "variable CMAKE_DISABLE_FIND_PACKAGE_<package> to TRUE. See the "
    "documentation for the CMAKE_DISABLE_FIND_PACKAGE_<package> variable for "
    "more information.\n"
    "When loading a find module or package configuration file find_package "
    "defines variables to provide information about the call arguments "
    "(and restores their original state before returning):\n"
    " <package>_FIND_REQUIRED      = true if REQUIRED option was given\n"
    " <package>_FIND_QUIETLY       = true if QUIET option was given\n"
    " <package>_FIND_VERSION       = full requested version string\n"
    " <package>_FIND_VERSION_MAJOR = major version if requested, else 0\n"
    " <package>_FIND_VERSION_MINOR = minor version if requested, else 0\n"
    " <package>_FIND_VERSION_PATCH = patch version if requested, else 0\n"
    " <package>_FIND_VERSION_TWEAK = tweak version if requested, else 0\n"
    " <package>_FIND_VERSION_COUNT = number of version components, 0 to 4\n"
    " <package>_FIND_VERSION_EXACT = true if EXACT option was given\n"
    " <package>_FIND_COMPONENTS    = list of requested components\n"
    " <package>_FIND_REQUIRED_<c>  = true if component <c> is required\n"
    "                                false if component <c> is optional\n"
    "In Module mode the loaded find module is responsible to honor the "
    "request detailed by these variables; see the find module for details.  "
    "In Config mode find_package handles REQUIRED, QUIET, and version "
    "options automatically but leaves it to the package configuration file "
    "to handle components in a way that makes sense for the package.  "
    "The package configuration file may set <package>_FOUND to false "
    "to tell find_package that component requirements are not satisfied."
    "\n"
    "See the cmake_policy() command documentation for discussion of the "
    "NO_POLICY_SCOPE option."
    ;
}

//----------------------------------------------------------------------------
const char* cmFindPackageCommand::GetFullDocumentation() const
{
  if(this->CommandDocumentation.empty())
    {
    const_cast<cmFindPackageCommand *>(this)->GenerateDocumentation();
    }
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

  // Lookup required version of CMake.
  if(const char* rv =
     this->Makefile->GetDefinition("CMAKE_MINIMUM_REQUIRED_VERSION"))
    {
    unsigned int v[3] = {0,0,0};
    sscanf(rv, "%u.%u.%u", &v[0], &v[1], &v[2]);
    this->RequiredCMakeVersion = CMake_VERSION_ENCODE(v[0],v[1],v[2]);
    }

  // Check for debug mode.
  this->DebugMode = this->Makefile->IsOn("CMAKE_FIND_DEBUG_MODE");

  // Lookup target architecture, if any.
  if(const char* arch =
     this->Makefile->GetDefinition("CMAKE_LIBRARY_ARCHITECTURE"))
    {
    this->LibraryArchitecture = arch;
    }

  // Lookup whether lib64 paths should be used.
  if(this->Makefile->PlatformIs64Bit() &&
     this->Makefile->GetCMakeInstance()
     ->GetPropertyAsBool("FIND_LIBRARY_USE_LIB64_PATHS"))
    {
    this->UseLib64Paths = true;
    }

  // Find the current root path mode.
  this->SelectDefaultRootPathMode();

  // Find the current bundle/framework search policy.
  this->SelectDefaultMacMode();

  // Record options.
  this->Name = args[0];
  std::string components;
  const char* components_sep = "";
  std::set<std::string> requiredComponents;
  std::set<std::string> optionalComponents;

  // Check ancient compatibility.
  this->Compatibility_1_6 =
    this->Makefile->GetLocalGenerator()
    ->NeedBackwardsCompatibility(1, 6);

  // Always search directly in a generated path.
  this->SearchPathSuffixes.push_back("");

  // Parse the arguments.
  enum Doing { DoingNone, DoingComponents, DoingOptionalComponents, DoingNames,
               DoingPaths, DoingPathSuffixes, DoingConfigs, DoingHints };
  Doing doing = DoingNone;
  cmsys::RegularExpression version("^[0-9.]+$");
  bool haveVersion = false;
  std::set<unsigned int> configArgs;
  std::set<unsigned int> moduleArgs;
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
    else if(args[i] == "MODULE")
      {
      moduleArgs.insert(i);
      doing = DoingNone;
      }
    else if(args[i] == "CONFIG")
      {
      configArgs.insert(i);
      doing = DoingNone;
      }
    else if(args[i] == "NO_MODULE")
      {
      configArgs.insert(i);
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
    else if(args[i] == "OPTIONAL_COMPONENTS")
      {
      this->Compatibility_1_6 = false;
      doing = DoingOptionalComponents;
      }
    else if(args[i] == "NAMES")
      {
      configArgs.insert(i);
      this->Compatibility_1_6 = false;
      doing = DoingNames;
      }
    else if(args[i] == "PATHS")
      {
      configArgs.insert(i);
      this->Compatibility_1_6 = false;
      doing = DoingPaths;
      }
    else if(args[i] == "HINTS")
      {
      configArgs.insert(i);
      this->Compatibility_1_6 = false;
      doing = DoingHints;
      }
    else if(args[i] == "PATH_SUFFIXES")
      {
      configArgs.insert(i);
      this->Compatibility_1_6 = false;
      doing = DoingPathSuffixes;
      }
    else if(args[i] == "CONFIGS")
      {
      configArgs.insert(i);
      this->Compatibility_1_6 = false;
      doing = DoingConfigs;
      }
    else if(args[i] == "NO_POLICY_SCOPE")
      {
      this->PolicyScope = false;
      this->Compatibility_1_6 = false;
      doing = DoingNone;
      }
    else if(args[i] == "NO_CMAKE_PACKAGE_REGISTRY")
      {
      this->NoUserRegistry = true;
      configArgs.insert(i);
      this->Compatibility_1_6 = false;
      doing = DoingNone;
      }
    else if(args[i] == "NO_CMAKE_SYSTEM_PACKAGE_REGISTRY")
      {
      this->NoSystemRegistry = true;
      configArgs.insert(i);
      this->Compatibility_1_6 = false;
      doing = DoingNone;
      }
    else if(args[i] == "NO_CMAKE_BUILDS_PATH")
      {
      this->NoBuilds = true;
      configArgs.insert(i);
      this->Compatibility_1_6 = false;
      doing = DoingNone;
      }
    else if(this->CheckCommonArgument(args[i]))
      {
      configArgs.insert(i);
      this->Compatibility_1_6 = false;
      doing = DoingNone;
      }
    else if((doing == DoingComponents) || (doing == DoingOptionalComponents))
      {
      // Set a variable telling the find script whether this component
      // is required.
      const char* isRequired = "1";
      if (doing == DoingOptionalComponents)
        {
        isRequired = "0";
        optionalComponents.insert(args[i]);
        }
      else
        {
        requiredComponents.insert(args[i]);
        }

      std::string req_var = this->Name + "_FIND_REQUIRED_" + args[i];
      this->AddFindDefinition(req_var.c_str(), isRequired);

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
        this->SetError(e.str().c_str());
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

  std::vector<std::string> doubledComponents;
  std::set_intersection(requiredComponents.begin(), requiredComponents.end(),
                        optionalComponents.begin(), optionalComponents.end(),
                        std::back_inserter(doubledComponents));
  if(!doubledComponents.empty())
    {
    cmOStringStream e;
    e << "called with components that are both required and optional:\n";
    for(unsigned int i=0; i<doubledComponents.size(); ++i)
      {
      e << "  " << doubledComponents[i] << "\n";
      }
    this->SetError(e.str().c_str());
    return false;
    }

  // Maybe choose one mode exclusively.
  this->UseFindModules = configArgs.empty();
  this->UseConfigFiles = moduleArgs.empty();
  if(!this->UseFindModules && !this->UseConfigFiles)
    {
    cmOStringStream e;
    e << "given options exclusive to Module mode:\n";
    for(std::set<unsigned int>::const_iterator si = moduleArgs.begin();
        si != moduleArgs.end(); ++si)
      {
      e << "  " << args[*si] << "\n";
      }
    e << "and options exclusive to Config mode:\n";
    for(std::set<unsigned int>::const_iterator si = configArgs.begin();
        si != configArgs.end(); ++si)
      {
      e << "  " << args[*si] << "\n";
      }
    e << "The options are incompatible.";
    this->SetError(e.str().c_str());
    return false;
    }

  // Ignore EXACT with no version.
  if(this->Version.empty() && this->VersionExact)
    {
    this->VersionExact = false;
    this->Makefile->IssueMessage(
      cmake::AUTHOR_WARNING, "Ignoring EXACT since no version is requested.");
    }

  if(this->Version.empty() || components.empty())
    {
    // Check whether we are recursing inside "Find<name>.cmake" within
    // another find_package(<name>) call.
    std::string mod = this->Name;
    mod += "_FIND_MODULE";
    if(this->Makefile->IsOn(mod.c_str()))
      {
      if(this->Version.empty())
        {
        // Get version information from the outer call if necessary.
        // Requested version string.
        std::string ver = this->Name;
        ver += "_FIND_VERSION";
        this->Version = this->Makefile->GetSafeDefinition(ver.c_str());

        // Whether an exact version is required.
        std::string exact = this->Name;
        exact += "_FIND_VERSION_EXACT";
        this->VersionExact = this->Makefile->IsOn(exact.c_str());
        }
      if(components.empty())
        {
        std::string components_var = this->Name + "_FIND_COMPONENTS";
        components = this->Makefile->GetSafeDefinition(components_var.c_str());
        }
      }
    }

  if(!this->Version.empty())
    {
    // Try to parse the version number and store the results that were
    // successfully parsed.
    unsigned int parsed_major;
    unsigned int parsed_minor;
    unsigned int parsed_patch;
    unsigned int parsed_tweak;
    this->VersionCount = sscanf(this->Version.c_str(), "%u.%u.%u.%u",
                                &parsed_major, &parsed_minor,
                                &parsed_patch, &parsed_tweak);
    switch(this->VersionCount)
      {
      case 4: this->VersionTweak = parsed_tweak; // no break!
      case 3: this->VersionPatch = parsed_patch; // no break!
      case 2: this->VersionMinor = parsed_minor; // no break!
      case 1: this->VersionMajor = parsed_major; // no break!
      default: break;
      }
    }

  std::string disableFindPackageVar = "CMAKE_DISABLE_FIND_PACKAGE_";
  disableFindPackageVar += this->Name;
  if(this->Makefile->IsOn(disableFindPackageVar.c_str()))
    {
    if (this->Required)
      {
      cmOStringStream e;
      e << "for module " << this->Name << " called with REQUIRED, but "
        << disableFindPackageVar
        << " is enabled. A REQUIRED package cannot be disabled.";
      this->SetError(e.str().c_str());
      return false;
      }

    return true;
    }


  this->SetModuleVariables(components);

  // See if there is a Find<package>.cmake module.
  if(this->UseFindModules)
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

  if(this->UseFindModules && this->UseConfigFiles &&
     this->Makefile->IsOn("CMAKE_FIND_PACKAGE_WARN_NO_MODULE"))
    {
    cmOStringStream aw;
    if(this->RequiredCMakeVersion >= CMake_VERSION_ENCODE(2,8,8))
      {
      aw << "find_package called without either MODULE or CONFIG option and "
        "no Find" << this->Name << ".cmake module is in CMAKE_MODULE_PATH.  "
        "Add MODULE to exclusively request Module mode and fail if "
        "Find" << this->Name << ".cmake is missing.  "
        "Add CONFIG to exclusively request Config mode and search for a "
        "package configuration file provided by " << this->Name <<
        " (" << this->Name << "Config.cmake or " <<
        cmSystemTools::LowerCase(this->Name) << "-config.cmake).  ";
      }
    else
      {
      aw << "find_package called without NO_MODULE option and no "
        "Find" << this->Name << ".cmake module is in CMAKE_MODULE_PATH.  "
        "Add NO_MODULE to exclusively request Config mode and search for a "
        "package configuration file provided by " << this->Name <<
        " (" << this->Name << "Config.cmake or " <<
        cmSystemTools::LowerCase(this->Name) << "-config.cmake).  "
        "Otherwise make Find" << this->Name << ".cmake available in "
        "CMAKE_MODULE_PATH.";
      }
    aw << "\n"
      "(Variable CMAKE_FIND_PACKAGE_WARN_NO_MODULE enabled this warning.)";
    this->Makefile->IssueMessage(cmake::AUTHOR_WARNING, aw.str());
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

  // get igonored paths from vars and reroot them.
  std::vector<std::string> ignored;
  this->GetIgnoredPaths(ignored);
  this->RerootPaths(ignored);

  // Construct a set of ignored paths
  this->IgnoredPaths.clear();
  this->IgnoredPaths.insert(ignored.begin(), ignored.end());

  // Find and load the package.
  bool result = this->HandlePackageMode();
  this->AppendSuccessInformation();
  return result;
}


//----------------------------------------------------------------------------
void cmFindPackageCommand::SetModuleVariables(const std::string& components)
{
  this->AddFindDefinition("CMAKE_FIND_PACKAGE_NAME", this->Name.c_str());

  // Store the list of components.
  std::string components_var = this->Name + "_FIND_COMPONENTS";
  this->AddFindDefinition(components_var.c_str(), components.c_str());

  if(this->Quiet)
    {
    // Tell the module that is about to be read that it should find
    // quietly.
    std::string quietly = this->Name;
    quietly += "_FIND_QUIETLY";
    this->AddFindDefinition(quietly.c_str(), "1");
    }

  if(this->Required)
    {
    // Tell the module that is about to be read that it should report
    // a fatal error if the package is not found.
    std::string req = this->Name;
    req += "_FIND_REQUIRED";
    this->AddFindDefinition(req.c_str(), "1");
    }

  if(!this->Version.empty())
    {
    // Tell the module that is about to be read what version of the
    // package has been requested.
    std::string ver = this->Name;
    ver += "_FIND_VERSION";
    this->AddFindDefinition(ver.c_str(), this->Version.c_str());
    char buf[64];
    sprintf(buf, "%u", this->VersionMajor);
    this->AddFindDefinition((ver+"_MAJOR").c_str(), buf);
    sprintf(buf, "%u", this->VersionMinor);
    this->AddFindDefinition((ver+"_MINOR").c_str(), buf);
    sprintf(buf, "%u", this->VersionPatch);
    this->AddFindDefinition((ver+"_PATCH").c_str(), buf);
    sprintf(buf, "%u", this->VersionTweak);
    this->AddFindDefinition((ver+"_TWEAK").c_str(), buf);
    sprintf(buf, "%u", this->VersionCount);
    this->AddFindDefinition((ver+"_COUNT").c_str(), buf);

    // Tell the module whether an exact version has been requested.
    std::string exact = this->Name;
    exact += "_FIND_VERSION_EXACT";
    this->AddFindDefinition(exact.c_str(), this->VersionExact? "1":"0");
   }
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AddFindDefinition(const char* var, const char* val)
{
  if(const char* old = this->Makefile->GetDefinition(var))
    {
    this->OriginalDefs[var].exists = true;
    this->OriginalDefs[var].value = old;
    }
  else
    {
    this->OriginalDefs[var].exists = false;
    }
  this->Makefile->AddDefinition(var, val);
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::RestoreFindDefinitions()
{
  for(std::map<cmStdString, OriginalDef>::iterator
        i = this->OriginalDefs.begin(); i != this->OriginalDefs.end(); ++i)
    {
    OriginalDef const& od = i->second;
    if(od.exists)
      {
      this->Makefile->AddDefinition(i->first.c_str(), od.value.c_str());
      }
    else
      {
      this->Makefile->RemoveDefinition(i->first.c_str());
      }
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
    // Load the module we found, and set "<name>_FIND_MODULE" to true
    // while inside it.
    found = true;
    std::string var = this->Name;
    var += "_FIND_MODULE";
    this->Makefile->AddDefinition(var.c_str(), "1");
    bool result = this->ReadListFile(mfile.c_str(), DoPolicyScope);
    this->Makefile->RemoveDefinition(var.c_str());
    return result;
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::HandlePackageMode()
{
  this->ConsideredConfigs.clear();

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

  // Try to load the config file if the directory is known
  bool fileFound = false;
  if (this->UseConfigFiles)
    {
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
      // The file location was cached.  Look for the correct file.
      std::string file;
      if (this->FindConfigFile(dir, file))
        {
        this->FileFound = file;
        fileFound = true;
        }
      def = this->Makefile->GetDefinition(this->Variable.c_str());
      }

    // Search for the config file if it is not already found.
    if(cmSystemTools::IsOff(def) || !fileFound)
      {
      fileFound = this->FindConfig();
      def = this->Makefile->GetDefinition(this->Variable.c_str());
      }

    // Sanity check.
    if(fileFound && this->FileFound.empty())
      {
      this->Makefile->IssueMessage(
        cmake::INTERNAL_ERROR, "fileFound is true but FileFound is empty!");
      fileFound = false;
      }
    }

  std::string foundVar = this->Name;
  foundVar += "_FOUND";
  std::string notFoundMessageVar = this->Name;
  notFoundMessageVar += "_NOT_FOUND_MESSAGE";
  std::string notFoundMessage;

  // If the directory for the config file was found, try to read the file.
  bool result = true;
  bool found = false;
  bool configFileSetFOUNDFalse = false;

  if(fileFound)
    {
    if ((this->Makefile->IsDefinitionSet(foundVar.c_str()))
      && (this->Makefile->IsOn(foundVar.c_str()) == false))
      {
      // by removing Foo_FOUND here if it is FALSE, we don't really change
      // the situation for the Config file which is about to be included,
      // but we make it possible to detect later on whether the Config file
      // has set Foo_FOUND to FALSE itself:
      this->Makefile->RemoveDefinition(foundVar.c_str());
      }
    this->Makefile->RemoveDefinition(notFoundMessageVar.c_str());

    // Set the version variables before loading the config file.
    // It may override them.
    this->StoreVersionFound();

    // Parse the configuration file.
    if(this->ReadListFile(this->FileFound.c_str(), DoPolicyScope))
      {
      // The package has been found.
      found = true;

      // Check whether the Config file has set Foo_FOUND to FALSE:
      if ((this->Makefile->IsDefinitionSet(foundVar.c_str()))
           && (this->Makefile->IsOn(foundVar.c_str()) == false))
        {
        // we get here if the Config file has set Foo_FOUND actively to FALSE
        found = false;
        configFileSetFOUNDFalse = true;
        notFoundMessage = this->Makefile->GetSafeDefinition(
                                                   notFoundMessageVar.c_str());
        }
      }
    else
      {
      // The configuration file is invalid.
      result = false;
      }
    }

  if (result && !found && (!this->Quiet || this->Required))
    {
    // The variable is not set.
    cmOStringStream e;
    cmOStringStream aw;
    if (configFileSetFOUNDFalse)
      {
      e << "Found package configuration file:\n"
        "  " << this->FileFound << "\n"
        "but it set " << foundVar << " to FALSE so package \"" <<
        this->Name << "\" is considered to be NOT FOUND.";
      if (!notFoundMessage.empty())
        {
        e << " Reason given by package: \n" << notFoundMessage << "\n";
        }
      }
    // If there are files in ConsideredConfigs, it means that FooConfig.cmake
    // have been found, but they didn't have appropriate versions.
    else if (this->ConsideredConfigs.size() > 0)
      {
      e << "Could not find a configuration file for package \""
        << this->Name << "\" that "
        << (this->VersionExact? "exactly matches" : "is compatible with")
        << " requested version \"" << this->Version << "\".\n"
        << "The following configuration files were considered but not "
           "accepted:\n";
      for(std::vector<ConfigFileInfo>::size_type i=0;
          i<this->ConsideredConfigs.size(); i++)
        {
        e << "  " << this->ConsideredConfigs[i].filename
          << ", version: " << this->ConsideredConfigs[i].version << "\n";
        }
      }
    else
      {
      std::string requestedVersionString;
      if(!this->Version.empty())
        {
        requestedVersionString = " (requested version ";
        requestedVersionString += this->Version;
        requestedVersionString += ")";
        }

      if (this->UseConfigFiles)
        {
        if(this->UseFindModules)
          {
          e << "By not providing \"Find" << this->Name << ".cmake\" in "
               "CMAKE_MODULE_PATH this project has asked CMake to find a "
               "package configuration file provided by \""<<this->Name<< "\", "
               "but CMake did not find one.\n";
          }

        if(this->Configs.size() == 1)
          {
          e << "Could not find a package configuration file named \""
            << this->Configs[0] << "\" provided by package \""
            << this->Name << "\"" << requestedVersionString <<".\n";
          }
        else
          {
          e << "Could not find a package configuration file provided by \""
            << this->Name << "\"" << requestedVersionString
            << " with any of the following names:\n";
          for(std::vector<std::string>::const_iterator ci =
                this->Configs.begin();
              ci != this->Configs.end(); ++ci)
            {
            e << "  " << *ci << "\n";
            }
          }

        e << "Add the installation prefix of \"" << this->Name << "\" to "
          "CMAKE_PREFIX_PATH or set \"" << this->Variable << "\" to a "
          "directory containing one of the above files. "
          "If \"" << this->Name << "\" provides a separate development "
          "package or SDK, be sure it has been installed.";
        }
      else // if(!this->UseFindModules && !this->UseConfigFiles)
        {
        e << "No \"Find" << this->Name << ".cmake\" found in "
          << "CMAKE_MODULE_PATH.";

        aw<< "Find"<< this->Name <<".cmake must either be part of this "
             "project itself, in this case adjust CMAKE_MODULE_PATH so that "
             "it points to the correct location inside its source tree.\n"
             "Or it must be installed by a package which has already been "
             "found via find_package().  In this case make sure that "
             "package has indeed been found and adjust CMAKE_MODULE_PATH to "
             "contain the location where that package has installed "
             "Find" << this->Name << ".cmake.  This must be a location "
             "provided by that package.  This error in general means that "
             "the buildsystem of this project is relying on a Find-module "
             "without ensuring that it is actually available.\n";
        }
      }


    this->Makefile->IssueMessage(
      this->Required? cmake::FATAL_ERROR : cmake::WARNING, e.str());
    if (this->Required)
      {
      cmSystemTools::SetFatalErrorOccured();
      }

    if (!aw.str().empty())
      {
      this->Makefile->IssueMessage(cmake::AUTHOR_WARNING,aw.str());
      }
    }

  // Set a variable marking whether the package was found.
  this->Makefile->AddDefinition(foundVar.c_str(), found? "1":"0");

  // Set a variable naming the configuration file that was found.
  std::string fileVar = this->Name;
  fileVar += "_CONFIG";
  if(found)
    {
    this->Makefile->AddDefinition(fileVar.c_str(), this->FileFound.c_str());
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

  std::string consideredConfigsVar = this->Name;
  consideredConfigsVar += "_CONSIDERED_CONFIGS";
  std::string consideredVersionsVar = this->Name;
  consideredVersionsVar += "_CONSIDERED_VERSIONS";

  std::string consideredConfigFiles;
  std::string consideredVersions;

  const char* sep = "";
  for(std::vector<ConfigFileInfo>::size_type i=0;
      i<this->ConsideredConfigs.size(); i++)
    {
    consideredConfigFiles += sep;
    consideredVersions += sep;
    consideredConfigFiles += this->ConsideredConfigs[i].filename;
    consideredVersions += this->ConsideredConfigs[i].version;
    sep = ";";
    }

  this->Makefile->AddDefinition(consideredConfigsVar.c_str(),
                                consideredConfigFiles.c_str());

  this->Makefile->AddDefinition(consideredVersionsVar.c_str(),
                                consideredVersions.c_str());

  return result;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::FindConfig()
{
  // Compute the set of search prefixes.
  this->ComputePrefixes();

  // Look for the project's configuration file.
  bool found = false;

  // Search for frameworks.
  if(!found && (this->SearchFrameworkFirst || this->SearchFrameworkOnly))
    {
    found = this->FindFrameworkConfig();
    }

  // Search for apps.
  if(!found && (this->SearchAppBundleFirst || this->SearchAppBundleOnly))
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
  // We force the value since we do not get here if it was already set.
  this->Makefile->AddCacheDefinition(this->Variable.c_str(),
                                     init.c_str(), help.c_str(),
                                     cmCacheManager::PATH, true);
  return found;
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
bool cmFindPackageCommand::ReadListFile(const char* f, PolicyScopeRule psr)
{
  if(this->Makefile->ReadListFile(this->Makefile->GetCurrentListFile(), f, 0,
                                  !this->PolicyScope || psr == NoPolicyScope))
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
void cmFindPackageCommand::AppendToFoundProperty(bool found)
{
  std::vector<std::string> foundContents;
  const char *foundProp =
             this->Makefile->GetCMakeInstance()->GetProperty("PACKAGES_FOUND");
  if (foundProp && *foundProp)
    {
    std::string tmp = foundProp;

    cmSystemTools::ExpandListArgument(tmp, foundContents, false);
    std::vector<std::string>::iterator nameIt = std::find(
                       foundContents.begin(), foundContents.end(), this->Name);
    if(nameIt != foundContents.end())
      {
      foundContents.erase(nameIt);
      }
    }

  std::vector<std::string> notFoundContents;
  const char *notFoundProp =
         this->Makefile->GetCMakeInstance()->GetProperty("PACKAGES_NOT_FOUND");
  if (notFoundProp && *notFoundProp)
    {
    std::string tmp = notFoundProp;

    cmSystemTools::ExpandListArgument(tmp, notFoundContents, false);
    std::vector<std::string>::iterator nameIt = std::find(
                 notFoundContents.begin(), notFoundContents.end(), this->Name);
    if(nameIt != notFoundContents.end())
      {
      notFoundContents.erase(nameIt);
      }
    }

  if(found)
    {
    foundContents.push_back(this->Name);
    }
  else
    {
    notFoundContents.push_back(this->Name);
    }


  std::string tmp;
  const char* sep ="";
  for(size_t i=0; i<foundContents.size(); i++)
    {
    tmp += sep;
    tmp += foundContents[i];
    sep = ";";
    }

  this->Makefile->GetCMakeInstance()->SetProperty("PACKAGES_FOUND",
                                                  tmp.c_str());

  tmp = "";
  sep = "";
  for(size_t i=0; i<notFoundContents.size(); i++)
    {
    tmp += sep;
    tmp += notFoundContents[i];
    sep = ";";
    }
  this->Makefile->GetCMakeInstance()->SetProperty("PACKAGES_NOT_FOUND",
                                                  tmp.c_str());
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AppendSuccessInformation()
{
  std::string found = this->Name;
  found += "_FOUND";
  std::string upperFound = cmSystemTools::UpperCase(found);

  const char* upperResult = this->Makefile->GetDefinition(upperFound.c_str());
  const char* result = this->Makefile->GetDefinition(found.c_str());
  bool packageFound = ((cmSystemTools::IsOn(result))
                                        || (cmSystemTools::IsOn(upperResult)));

  this->AppendToFoundProperty(packageFound);

  // Record whether the find was quiet or not, so this can be used
  // e.g. in FeatureSummary.cmake
  std::string quietInfoPropName = "_CMAKE_";
  quietInfoPropName += this->Name;
  quietInfoPropName += "_QUIET";
  this->Makefile->GetCMakeInstance()->SetProperty(quietInfoPropName.c_str(),
                                               this->Quiet ? "TRUE" : "FALSE");

  // set a global property to record the required version of this package
  std::string versionInfoPropName = "_CMAKE_";
  versionInfoPropName += this->Name;
  versionInfoPropName += "_REQUIRED_VERSION";
  std::string versionInfo;
  if(!this->Version.empty())
    {
    versionInfo = this->VersionExact ? "==" : ">=";
    versionInfo += " ";
    versionInfo += this->Version;
    }
  this->Makefile->GetCMakeInstance()->SetProperty(versionInfoPropName.c_str(),
                                                  versionInfo.c_str());
  if (this->Required)
    {
    std::string requiredInfoPropName = "_CMAKE_";
    requiredInfoPropName += this->Name;
    requiredInfoPropName += "_TYPE";
    this->Makefile->GetCMakeInstance()->SetProperty(
                                     requiredInfoPropName.c_str(), "REQUIRED");
    }


  // Restore original state of "_FIND_" variables we set.
  this->RestoreFindDefinitions();
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::ComputePrefixes()
{
  this->AddPrefixesCMakeVariable();
  this->AddPrefixesCMakeEnvironment();
  this->AddPrefixesUserHints();
  this->AddPrefixesSystemEnvironment();
  this->AddPrefixesUserRegistry();
  this->AddPrefixesBuilds();
  this->AddPrefixesCMakeSystemVariable();
  this->AddPrefixesSystemRegistry();
  this->AddPrefixesUserGuess();
  this->ComputeFinalPaths();
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
      if((d.size() >= 4 && strcmp(d.c_str()+d.size()-4, "/bin") == 0) ||
         (d.size() >= 5 && strcmp(d.c_str()+d.size()-5, "/sbin") == 0))
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
void cmFindPackageCommand::AddPrefixesUserRegistry()
{
  if(this->NoUserRegistry || this->NoDefaultPath)
    {
    return;
    }

#if defined(_WIN32) && !defined(__CYGWIN__)
  this->LoadPackageRegistryWinUser();
#elif defined(__HAIKU__)
  BPath dir;
  if (find_directory(B_USER_SETTINGS_DIRECTORY, &dir) == B_OK)
    {
    dir.Append("cmake/packages");
    dir.Append(this->Name.c_str());
    this->LoadPackageRegistryDir(dir.Path());
    }
#else
  if(const char* home = cmSystemTools::GetEnv("HOME"))
    {
    std::string dir = home;
    dir += "/.cmake/packages/";
    dir += this->Name;
    this->LoadPackageRegistryDir(dir);
    }
#endif
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AddPrefixesSystemRegistry()
{
  if(this->NoSystemRegistry || this->NoDefaultPath)
    {
    return;
    }

#if defined(_WIN32) && !defined(__CYGWIN__)
  this->LoadPackageRegistryWinSystem();
#endif
}

#if defined(_WIN32) && !defined(__CYGWIN__)
# include <windows.h>
# undef GetCurrentDirectory
  // http://msdn.microsoft.com/en-us/library/aa384253%28v=vs.85%29.aspx
# if !defined(KEY_WOW64_32KEY)
#  define KEY_WOW64_32KEY 0x0200
# endif
# if !defined(KEY_WOW64_64KEY)
#  define KEY_WOW64_64KEY 0x0100
# endif
//----------------------------------------------------------------------------
void cmFindPackageCommand::LoadPackageRegistryWinUser()
{
  // HKEY_CURRENT_USER\\Software shares 32-bit and 64-bit views.
  this->LoadPackageRegistryWin(true, 0);
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::LoadPackageRegistryWinSystem()
{
  // HKEY_LOCAL_MACHINE\\SOFTWARE has separate 32-bit and 64-bit views.
  // Prefer the target platform view first.
  if(this->Makefile->PlatformIs64Bit())
    {
    this->LoadPackageRegistryWin(false, KEY_WOW64_64KEY);
    this->LoadPackageRegistryWin(false, KEY_WOW64_32KEY);
    }
  else
    {
    this->LoadPackageRegistryWin(false, KEY_WOW64_32KEY);
    this->LoadPackageRegistryWin(false, KEY_WOW64_64KEY);
    }
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::LoadPackageRegistryWin(bool user,
                                                  unsigned int view)
{
  std::string key = "Software\\Kitware\\CMake\\Packages\\";
  key += this->Name;
  std::set<cmStdString> bad;
  HKEY hKey;
  if(RegOpenKeyEx(user? HKEY_CURRENT_USER : HKEY_LOCAL_MACHINE, key.c_str(),
                  0, KEY_QUERY_VALUE|view, &hKey) == ERROR_SUCCESS)
    {
    DWORD valueType = REG_NONE;
    char name[16384];
    std::vector<char> data(512);
    bool done = false;
    DWORD index = 0;
    while(!done)
      {
      DWORD nameSize = static_cast<DWORD>(sizeof(name));
      DWORD dataSize = static_cast<DWORD>(data.size()-1);
      switch(RegEnumValue(hKey, index, name, &nameSize,
                          0, &valueType, (BYTE*)&data[0], &dataSize))
        {
        case ERROR_SUCCESS:
          ++index;
          if(valueType == REG_SZ)
            {
            data[dataSize] = 0;
            cmsys_ios::stringstream ss(&data[0]);
            if(!this->CheckPackageRegistryEntry(ss))
              {
              // The entry is invalid.
              bad.insert(name);
              }
            }
          break;
        case ERROR_MORE_DATA:
          data.resize(dataSize+1);
          break;
        case ERROR_NO_MORE_ITEMS: default: done = true; break;
        }
      }
    RegCloseKey(hKey);
    }

  // Remove bad values if possible.
  if(user && !bad.empty() &&
     RegOpenKeyEx(HKEY_CURRENT_USER, key.c_str(),
                  0, KEY_SET_VALUE|view, &hKey) == ERROR_SUCCESS)
    {
    for(std::set<cmStdString>::const_iterator vi = bad.begin();
        vi != bad.end(); ++vi)
      {
      RegDeleteValue(hKey, vi->c_str());
      }
    RegCloseKey(hKey);
    }
}
#else
//----------------------------------------------------------------------------
class cmFindPackageCommandHoldFile
{
  const char* File;
public:
  cmFindPackageCommandHoldFile(const char* f): File(f) {}
  ~cmFindPackageCommandHoldFile()
    { if(this->File) { cmSystemTools::RemoveFile(this->File); } }
  void Release() { this->File = 0; }
};

//----------------------------------------------------------------------------
void cmFindPackageCommand::LoadPackageRegistryDir(std::string const& dir)
{
  cmsys::Directory files;
  if(!files.Load(dir.c_str()))
    {
    return;
    }

  std::string fname;
  for(unsigned long i=0; i < files.GetNumberOfFiles(); ++i)
    {
    fname = dir;
    fname += "/";
    fname += files.GetFile(i);

    if(!cmSystemTools::FileIsDirectory(fname.c_str()))
      {
      // Hold this file hostage until it behaves.
      cmFindPackageCommandHoldFile holdFile(fname.c_str());

      // Load the file.
      std::ifstream fin(fname.c_str(), std::ios::in | cmsys_ios_binary);
      if(fin && this->CheckPackageRegistryEntry(fin))
        {
        // The file references an existing package, so release it.
        holdFile.Release();
        }
      }
    }

  // TODO: Wipe out the directory if it is empty.
}
#endif

//----------------------------------------------------------------------------
bool cmFindPackageCommand::CheckPackageRegistryEntry(std::istream& is)
{
  // Parse the content of one package registry entry.
  std::string fname;
  if(cmSystemTools::GetLineFromStream(is, fname) &&
     cmSystemTools::FileIsFullPath(fname.c_str()))
    {
    // The first line in the stream is the full path to a file or
    // directory containing the package.
    if(cmSystemTools::FileExists(fname.c_str()))
      {
      // The path exists.  Look for the package here.
      if(!cmSystemTools::FileIsDirectory(fname.c_str()))
        {
        fname = cmSystemTools::GetFilenamePath(fname);
        }
      this->AddPathInternal(fname, FullPath);
      return true;
      }
    else
      {
      // The path does not exist.  Assume the stream content is
      // associated with an old package that no longer exists, and
      // delete it to keep the package registry clean.
      return false;
      }
    }
  else
    {
    // The first line in the stream is not the full path to a file or
    // directory.  Assume the stream content was created by a future
    // version of CMake that uses a different format, and leave it.
    return true;
    }
}

//----------------------------------------------------------------------------
void cmFindPackageCommand::AddPrefixesBuilds()
{
  if(!this->NoBuilds && !this->NoDefaultPath)
    {
    // It is likely that CMake will have recently built the project.
    for(int i=0; i <= 10; ++i)
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
  if (this->IgnoredPaths.count(dir))
    {
    return false;
    }

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
bool cmFindPackageCommand::CheckVersion(std::string const& config_file)
{
  bool result = false; // by default, assume the version is not ok.
  bool haveResult = false;
  std::string version = "unknown";

  // Get the filename without the .cmake extension.
  std::string::size_type pos = config_file.rfind('.');
  std::string version_file_base = config_file.substr(0, pos);

  // Look for foo-config-version.cmake
  std::string version_file = version_file_base;
  version_file += "-version.cmake";
  if ((haveResult == false)
       && (cmSystemTools::FileExists(version_file.c_str(), true)))
    {
    result = this->CheckVersionFile(version_file, version);
    haveResult = true;
    }

  // Look for fooConfigVersion.cmake
  version_file = version_file_base;
  version_file += "Version.cmake";
  if ((haveResult == false)
       && (cmSystemTools::FileExists(version_file.c_str(), true)))
    {
    result = this->CheckVersionFile(version_file, version);
    haveResult = true;
    }


  // If no version was requested a versionless package is acceptable.
  if ((haveResult == false) && (this->Version.empty()))
    {
    result = true;
    haveResult = true;
    }

  ConfigFileInfo configFileInfo;
  configFileInfo.filename = config_file;
  configFileInfo.version = version;
  this->ConsideredConfigs.push_back(configFileInfo);

  return result;
}

//----------------------------------------------------------------------------
bool cmFindPackageCommand::CheckVersionFile(std::string const& version_file,
                                            std::string& result_version)
{
  // The version file will be loaded in an isolated scope.
  cmMakefile::ScopePushPop varScope(this->Makefile);
  cmMakefile::PolicyPushPop polScope(this->Makefile);
  static_cast<void>(varScope);
  static_cast<void>(polScope);

  // Clear the output variables.
  this->Makefile->RemoveDefinition("PACKAGE_VERSION");
  this->Makefile->RemoveDefinition("PACKAGE_VERSION_UNSUITABLE");
  this->Makefile->RemoveDefinition("PACKAGE_VERSION_COMPATIBLE");
  this->Makefile->RemoveDefinition("PACKAGE_VERSION_EXACT");

  // Set the input variables.
  this->Makefile->AddDefinition("PACKAGE_FIND_NAME", this->Name.c_str());
  this->Makefile->AddDefinition("PACKAGE_FIND_VERSION",
                                this->Version.c_str());
  char buf[64];
  sprintf(buf, "%u", this->VersionMajor);
  this->Makefile->AddDefinition("PACKAGE_FIND_VERSION_MAJOR", buf);
  sprintf(buf, "%u", this->VersionMinor);
  this->Makefile->AddDefinition("PACKAGE_FIND_VERSION_MINOR", buf);
  sprintf(buf, "%u", this->VersionPatch);
  this->Makefile->AddDefinition("PACKAGE_FIND_VERSION_PATCH", buf);
  sprintf(buf, "%u", this->VersionTweak);
  this->Makefile->AddDefinition("PACKAGE_FIND_VERSION_TWEAK", buf);
  sprintf(buf, "%u", this->VersionCount);
  this->Makefile->AddDefinition("PACKAGE_FIND_VERSION_COUNT", buf);

  // Load the version check file.  Pass NoPolicyScope because we do
  // our own policy push/pop independent of CMP0011.
  bool suitable = false;
  if(this->ReadListFile(version_file.c_str(), NoPolicyScope))
    {
    // Check the output variables.
    bool okay = this->Makefile->IsOn("PACKAGE_VERSION_EXACT");
    bool unsuitable = this->Makefile->IsOn("PACKAGE_VERSION_UNSUITABLE");
    if(!okay && !this->VersionExact)
      {
      okay = this->Makefile->IsOn("PACKAGE_VERSION_COMPATIBLE");
      }

    // The package is suitable if the version is okay and not
    // explicitly unsuitable.
    suitable = !unsuitable && (okay || this->Version.empty());
    if(suitable)
      {
      // Get the version found.
      this->VersionFound =
        this->Makefile->GetSafeDefinition("PACKAGE_VERSION");

      // Try to parse the version number and store the results that were
      // successfully parsed.
      unsigned int parsed_major;
      unsigned int parsed_minor;
      unsigned int parsed_patch;
      unsigned int parsed_tweak;
      this->VersionFoundCount =
        sscanf(this->VersionFound.c_str(), "%u.%u.%u.%u",
               &parsed_major, &parsed_minor,
               &parsed_patch, &parsed_tweak);
      switch(this->VersionFoundCount)
        {
        case 4: this->VersionFoundTweak = parsed_tweak; // no break!
        case 3: this->VersionFoundPatch = parsed_patch; // no break!
        case 2: this->VersionFoundMinor = parsed_minor; // no break!
        case 1: this->VersionFoundMajor = parsed_major; // no break!
        default: break;
        }
      }
    }

  result_version = this->Makefile->GetSafeDefinition("PACKAGE_VERSION");
  if (result_version.empty())
    {
    result_version = "unknown";
    }

  // Succeed if the version is suitable.
  return suitable;
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

  // Store the version components.
  char buf[64];
  sprintf(buf, "%u", this->VersionFoundMajor);
  this->Makefile->AddDefinition((ver+"_MAJOR").c_str(), buf);
  sprintf(buf, "%u", this->VersionFoundMinor);
  this->Makefile->AddDefinition((ver+"_MINOR").c_str(), buf);
  sprintf(buf, "%u", this->VersionFoundPatch);
  this->Makefile->AddDefinition((ver+"_PATCH").c_str(), buf);
  sprintf(buf, "%u", this->VersionFoundTweak);
  this->Makefile->AddDefinition((ver+"_TWEAK").c_str(), buf);
  sprintf(buf, "%u", this->VersionFoundCount);
  this->Makefile->AddDefinition((ver+"_COUNT").c_str(), buf);
}

//----------------------------------------------------------------------------
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

  //  PREFIX/(Foo|foo|FOO).*/
  {
  cmFindPackageFileList lister(this);
  lister
    / cmFileListGeneratorFixed(prefix)
    / cmFileListGeneratorProject(this->Names);
  if(lister.Search())
    {
    return true;
    }
  }

  //  PREFIX/(Foo|foo|FOO).*/(cmake|CMake)/
  {
  cmFindPackageFileList lister(this);
  lister
    / cmFileListGeneratorFixed(prefix)
    / cmFileListGeneratorProject(this->Names)
    / cmFileListGeneratorCaseInsensitive("cmake");
  if(lister.Search())
    {
    return true;
    }
  }

  // Construct list of common install locations (lib and share).
  std::vector<std::string> common;
  if(!this->LibraryArchitecture.empty())
    {
    common.push_back("lib/"+this->LibraryArchitecture);
    }
  if(this->UseLib64Paths)
    {
    common.push_back("lib64");
    }
  common.push_back("lib");
  common.push_back("share");

  //  PREFIX/(lib/ARCH|lib|share)/cmake/(Foo|foo|FOO).*/
  {
  cmFindPackageFileList lister(this);
  lister
    / cmFileListGeneratorFixed(prefix)
    / cmFileListGeneratorEnumerate(common)
    / cmFileListGeneratorFixed("cmake")
    / cmFileListGeneratorProject(this->Names);
  if(lister.Search())
    {
    return true;
    }
  }

  //  PREFIX/(lib/ARCH|lib|share)/(Foo|foo|FOO).*/
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

  //  PREFIX/(lib/ARCH|lib|share)/(Foo|foo|FOO).*/(cmake|CMake)/
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

// TODO: Debug cmsys::Glob double slash problem.
