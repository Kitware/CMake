/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmTarget.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmComputeLinkInformation.h"
#include "cmDocumentCompileDefinitions.h"
#include "cmDocumentGeneratorExpressions.h"
#include "cmDocumentLocationUndefined.h"
#include "cmListFileCache.h"
#include "cmGeneratorExpression.h"
#include "cmGeneratorExpressionDAGChecker.h"
#include <cmsys/RegularExpression.hxx>
#include <map>
#include <set>
#include <queue>
#include <stdlib.h> // required for atof
#include <assert.h>

const char* cmTarget::GetTargetTypeName(TargetType targetType)
{
  switch( targetType )
    {
      case cmTarget::STATIC_LIBRARY:
        return "STATIC_LIBRARY";
      case cmTarget::MODULE_LIBRARY:
        return "MODULE_LIBRARY";
      case cmTarget::SHARED_LIBRARY:
        return "SHARED_LIBRARY";
      case cmTarget::OBJECT_LIBRARY:
        return "OBJECT_LIBRARY";
      case cmTarget::EXECUTABLE:
        return "EXECUTABLE";
      case cmTarget::UTILITY:
        return "UTILITY";
      case cmTarget::GLOBAL_TARGET:
        return "GLOBAL_TARGET";
      case cmTarget::UNKNOWN_LIBRARY:
        return "UNKNOWN_LIBRARY";
    }
  assert(0 && "Unexpected target type");
  return 0;
}

//----------------------------------------------------------------------------
struct cmTarget::OutputInfo
{
  std::string OutDir;
  std::string ImpDir;
  std::string PdbDir;
};

//----------------------------------------------------------------------------
struct cmTarget::ImportInfo
{
  bool NoSOName;
  std::string Location;
  std::string SOName;
  std::string ImportLibrary;
  cmTarget::LinkInterface LinkInterface;
};

struct TargetConfigPair : public std::pair<cmTarget*, std::string> {
  TargetConfigPair(cmTarget* tgt, const std::string &config)
    : std::pair<cmTarget*, std::string>(tgt, config) {}
};

//----------------------------------------------------------------------------
class cmTargetInternals
{
public:
  cmTargetInternals()
    {
    this->SourceFileFlagsConstructed = false;
    }
  cmTargetInternals(cmTargetInternals const& r)
    {
    this->SourceFileFlagsConstructed = false;
    // Only some of these entries are part of the object state.
    // Others not copied here are result caches.
    this->SourceEntries = r.SourceEntries;
    }
  ~cmTargetInternals();
  typedef cmTarget::SourceFileFlags SourceFileFlags;
  std::map<cmSourceFile const*, SourceFileFlags> SourceFlagsMap;
  bool SourceFileFlagsConstructed;

  // The backtrace when the target was created.
  cmListFileBacktrace Backtrace;

  // Cache link interface computation from each configuration.
  struct OptionalLinkInterface: public cmTarget::LinkInterface
  {
    OptionalLinkInterface(): Exists(false) {}
    bool Exists;
  };
  typedef std::map<TargetConfigPair, OptionalLinkInterface>
                                                          LinkInterfaceMapType;
  LinkInterfaceMapType LinkInterfaceMap;

  typedef std::map<cmStdString, cmTarget::OutputInfo> OutputInfoMapType;
  OutputInfoMapType OutputInfoMap;

  typedef std::map<TargetConfigPair, cmTarget::ImportInfo>
                                                            ImportInfoMapType;
  ImportInfoMapType ImportInfoMap;

  // Cache link implementation computation from each configuration.
  typedef std::map<TargetConfigPair,
                   cmTarget::LinkImplementation> LinkImplMapType;
  LinkImplMapType LinkImplMap;

  typedef std::map<TargetConfigPair, cmTarget::LinkClosure>
                                                          LinkClosureMapType;
  LinkClosureMapType LinkClosureMap;

  struct SourceEntry { std::vector<cmSourceFile*> Depends; };
  typedef std::map<cmSourceFile*, SourceEntry> SourceEntriesType;
  SourceEntriesType SourceEntries;

  struct IncludeDirectoriesEntry {
    IncludeDirectoriesEntry(cmsys::auto_ptr<cmCompiledGeneratorExpression> cge,
      const std::string &targetName = std::string())
      : ge(cge), TargetName(targetName)
    {}
    const cmsys::auto_ptr<cmCompiledGeneratorExpression> ge;
    std::vector<std::string> CachedIncludes;
    const std::string TargetName;
  };
  std::vector<IncludeDirectoriesEntry*> IncludeDirectoriesEntries;
  std::vector<cmValueWithOrigin> LinkInterfaceIncludeDirectoriesEntries;

  std::vector<IncludeDirectoriesEntry*>
                                CachedLinkInterfaceIncludeDirectoriesEntries;
  std::map<std::string, std::string> CachedLinkInterfaceCompileDefinitions;

  std::map<std::string, bool> CacheLinkInterfaceIncludeDirectoriesDone;
  std::map<std::string, bool> CacheLinkInterfaceCompileDefinitionsDone;
};

//----------------------------------------------------------------------------
void deleteAndClear(
      std::vector<cmTargetInternals::IncludeDirectoriesEntry*> &entries)
{
  for (std::vector<cmTargetInternals::IncludeDirectoriesEntry*>::const_iterator
      it = entries.begin(),
      end = entries.end();
      it != end; ++it)
    {
      delete *it;
    }
  entries.clear();
}

//----------------------------------------------------------------------------
cmTargetInternals::~cmTargetInternals()
{
  deleteAndClear(CachedLinkInterfaceIncludeDirectoriesEntries);
}

//----------------------------------------------------------------------------
cmTarget::cmTarget()
{
  this->Makefile = 0;
  this->PolicyStatusCMP0003 = cmPolicies::WARN;
  this->PolicyStatusCMP0004 = cmPolicies::WARN;
  this->PolicyStatusCMP0008 = cmPolicies::WARN;
  this->PolicyStatusCMP0020 = cmPolicies::WARN;
  this->LinkLibrariesAnalyzed = false;
  this->HaveInstallRule = false;
  this->DLLPlatform = false;
  this->IsApple = false;
  this->IsImportedTarget = false;
  this->BuildInterfaceIncludesAppended = false;
  this->DebugIncludesDone = false;
}

//----------------------------------------------------------------------------
void cmTarget::DefineProperties(cmake *cm)
{
  cm->DefineProperty
    ("AUTOMOC", cmProperty::TARGET,
     "Should the target be processed with automoc (for Qt projects).",
     "AUTOMOC is a boolean specifying whether CMake will handle "
     "the Qt moc preprocessor automatically, i.e. without having to use "
     "the QT4_WRAP_CPP() or QT5_WRAP_CPP() macro. Currently Qt4 and Qt5 are "
     "supported.  "
     "When this property is set to TRUE, CMake will scan the source files "
     "at build time and invoke moc accordingly. "
     "If an #include statement like #include \"moc_foo.cpp\" is found, "
     "the Q_OBJECT class declaration is expected in the header, and moc is "
     "run on the header file. "
     "If an #include statement like #include \"foo.moc\" is found, "
     "then a Q_OBJECT is expected in the current source file and moc "
     "is run on the file itself. "
     "Additionally, all header files are parsed for Q_OBJECT macros, "
     "and if found, moc is also executed on those files. The resulting "
     "moc files, which are not included as shown above in any of the source "
     "files are included in a generated <targetname>_automoc.cpp file, "
     "which is compiled as part of the target."
     "This property is initialized by the value of the variable "
     "CMAKE_AUTOMOC if it is set when a target is created.\n"
     "Additional command line options for moc can be set via the "
     "AUTOMOC_MOC_OPTIONS property.\n"
     "By setting the CMAKE_AUTOMOC_RELAXED_MODE variable to TRUE the rules "
     "for searching the files which will be processed by moc can be relaxed. "
     "See the documentation for this variable for more details.");

  cm->DefineProperty
    ("AUTOMOC_MOC_OPTIONS", cmProperty::TARGET,
    "Additional options for moc when using automoc (see the AUTOMOC property)",
     "This property is only used if the AUTOMOC property is set to TRUE for "
     "this target. In this case, it holds additional command line options "
     "which will be used when moc is executed during the build, i.e. it is "
     "equivalent to the optional OPTIONS argument of the qt4_wrap_cpp() "
     "macro.\n"
     "By default it is empty.");

  cm->DefineProperty
    ("BUILD_WITH_INSTALL_RPATH", cmProperty::TARGET,
     "Should build tree targets have install tree rpaths.",
     "BUILD_WITH_INSTALL_RPATH is a boolean specifying whether to link "
     "the target in the build tree with the INSTALL_RPATH.  This takes "
     "precedence over SKIP_BUILD_RPATH and avoids the need for relinking "
     "before installation.  "
     "This property is initialized by the value of the variable "
     "CMAKE_BUILD_WITH_INSTALL_RPATH if it is set when a target is created.");

  cm->DefineProperty
    ("COMPILE_FLAGS", cmProperty::TARGET,
     "Additional flags to use when compiling this target's sources.",
     "The COMPILE_FLAGS property sets additional compiler flags used "
     "to build sources within the target.  Use COMPILE_DEFINITIONS "
     "to pass additional preprocessor definitions.");

  cm->DefineProperty
    ("COMPILE_DEFINITIONS", cmProperty::TARGET,
     "Preprocessor definitions for compiling a target's sources.",
     "The COMPILE_DEFINITIONS property may be set to a "
     "semicolon-separated list of preprocessor "
     "definitions using the syntax VAR or VAR=value.  Function-style "
     "definitions are not supported.  CMake will automatically escape "
     "the value correctly for the native build system (note that CMake "
     "language syntax may require escapes to specify some values).  "
     "This property may be set on a per-configuration basis using the name "
     "COMPILE_DEFINITIONS_<CONFIG> where <CONFIG> is an upper-case name "
     "(ex. \"COMPILE_DEFINITIONS_DEBUG\").\n"
     "CMake will automatically drop some definitions that "
     "are not supported by the native build tool.  "
     "The VS6 IDE does not support definition values with spaces "
     "(but NMake does).\n"
     "Contents of COMPILE_DEFINITIONS may use \"generator expressions\" with "
     "the syntax \"$<...>\".  "
     CM_DOCUMENT_COMMAND_GENERATOR_EXPRESSIONS
     CM_DOCUMENT_COMPILE_DEFINITIONS_DISCLAIMER);

  cm->DefineProperty
    ("COMPILE_DEFINITIONS_<CONFIG>", cmProperty::TARGET,
     "Per-configuration preprocessor definitions on a target.",
     "This is the configuration-specific version of COMPILE_DEFINITIONS.");

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
     "See target property <CONFIG>_POSTFIX.",
     "This property is a special case of the more-general <CONFIG>_POSTFIX "
     "property for the DEBUG configuration.");

  cm->DefineProperty
    ("<CONFIG>_POSTFIX", cmProperty::TARGET,
     "Postfix to append to the target file name for configuration <CONFIG>.",
     "When building with configuration <CONFIG> the value of this property "
     "is appended to the target file name built on disk.  "
     "For non-executable targets, this property is initialized by the value "
     "of the variable CMAKE_<CONFIG>_POSTFIX if it is set when a target is "
     "created.  "
     "This property is ignored on the Mac for Frameworks and App Bundles.");

  cm->DefineProperty
    ("EchoString", cmProperty::TARGET,
     "A message to be displayed when the target is built.",
     "A message to display on some generators (such as makefiles) when "
     "the target is built.");

  cm->DefineProperty
    ("BUNDLE", cmProperty::TARGET,
     "This target is a CFBundle on the Mac.",
     "If a module library target has this property set to true it will "
     "be built as a CFBundle when built on the mac. It will have the "
     "directory structure required for a CFBundle and will be suitable "
     "to be used for creating Browser Plugins or other application "
     "resources.");

  cm->DefineProperty
    ("BUNDLE_EXTENSION", cmProperty::TARGET,
     "The file extension used to name a BUNDLE target on the Mac.",
     "The default value is \"bundle\" - you can also use \"plugin\" or "
     "whatever file extension is required by the host app for your "
     "bundle.");

  cm->DefineProperty
    ("EXCLUDE_FROM_DEFAULT_BUILD", cmProperty::TARGET,
     "Exclude target from \"Build Solution\".",
     "This property is only used by Visual Studio generators 7 and above. "
     "When set to TRUE, the target will not be built when you press "
     "\"Build Solution\".");

  cm->DefineProperty
    ("EXCLUDE_FROM_DEFAULT_BUILD_<CONFIG>", cmProperty::TARGET,
     "Per-configuration version of target exclusion from \"Build Solution\". ",
     "This is the configuration-specific version of "
     "EXCLUDE_FROM_DEFAULT_BUILD. If the generic EXCLUDE_FROM_DEFAULT_BUILD "
     "is also set on a target, EXCLUDE_FROM_DEFAULT_BUILD_<CONFIG> takes "
     "precedence in configurations for which it has a value.");

  cm->DefineProperty
    ("FRAMEWORK", cmProperty::TARGET,
     "This target is a framework on the Mac.",
     "If a shared library target has this property set to true it will "
     "be built as a framework when built on the mac. It will have the "
     "directory structure required for a framework and will be suitable "
     "to be used with the -framework option");

  cm->DefineProperty
    ("HAS_CXX", cmProperty::TARGET,
     "Link the target using the C++ linker tool (obsolete).",
     "This is equivalent to setting the LINKER_LANGUAGE property to CXX.  "
     "See that property's documentation for details.");

  cm->DefineProperty
    ("IMPLICIT_DEPENDS_INCLUDE_TRANSFORM", cmProperty::TARGET,
     "Specify #include line transforms for dependencies in a target.",
     "This property specifies rules to transform macro-like #include lines "
     "during implicit dependency scanning of C and C++ source files.  "
     "The list of rules must be semicolon-separated with each entry of "
     "the form \"A_MACRO(%)=value-with-%\" (the % must be literal).  "
     "During dependency scanning occurrences of A_MACRO(...) on #include "
     "lines will be replaced by the value given with the macro argument "
     "substituted for '%'.  For example, the entry\n"
     "  MYDIR(%)=<mydir/%>\n"
     "will convert lines of the form\n"
     "  #include MYDIR(myheader.h)\n"
     "to\n"
     "  #include <mydir/myheader.h>\n"
     "allowing the dependency to be followed.\n"
     "This property applies to sources in the target on which it is set.");

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
    ("IMPORTED", cmProperty::TARGET,
     "Read-only indication of whether a target is IMPORTED.",
     "The boolean value of this property is true for targets created with "
     "the IMPORTED option to add_executable or add_library.  "
     "It is false for targets built within the project.");

  cm->DefineProperty
    ("IMPORTED_CONFIGURATIONS", cmProperty::TARGET,
     "Configurations provided for an IMPORTED target.",
     "Set this to the list of configuration names available for an "
     "IMPORTED target.  "
     "The names correspond to configurations defined in the project from "
     "which the target is imported.  "
     "If the importing project uses a different set of configurations "
     "the names may be mapped using the MAP_IMPORTED_CONFIG_<CONFIG> "
     "property.  "
     "Ignored for non-imported targets.");

  cm->DefineProperty
    ("IMPORTED_IMPLIB", cmProperty::TARGET,
     "Full path to the import library for an IMPORTED target.",
     "Set this to the location of the \".lib\" part of a windows DLL.  "
     "Ignored for non-imported targets.");

  cm->DefineProperty
    ("IMPORTED_IMPLIB_<CONFIG>", cmProperty::TARGET,
     "<CONFIG>-specific version of IMPORTED_IMPLIB property.",
     "Configuration names correspond to those provided by the project "
     "from which the target is imported.");

  cm->DefineProperty
    ("IMPORTED_LINK_DEPENDENT_LIBRARIES", cmProperty::TARGET,
     "Dependent shared libraries of an imported shared library.",
     "Shared libraries may be linked to other shared libraries as part "
     "of their implementation.  On some platforms the linker searches "
     "for the dependent libraries of shared libraries they are including "
     "in the link.  "
     "Set this property to the list of dependent shared libraries of an "
     "imported library.  "
     "The list "
     "should be disjoint from the list of interface libraries in the "
     "IMPORTED_LINK_INTERFACE_LIBRARIES property.  On platforms requiring "
     "dependent shared libraries to be found at link time CMake uses this "
     "list to add appropriate files or paths to the link command line.  "
     "Ignored for non-imported targets.");

  cm->DefineProperty
    ("IMPORTED_LINK_DEPENDENT_LIBRARIES_<CONFIG>", cmProperty::TARGET,
     "<CONFIG>-specific version of IMPORTED_LINK_DEPENDENT_LIBRARIES.",
     "Configuration names correspond to those provided by the project "
     "from which the target is imported.  "
     "If set, this property completely overrides the generic property "
     "for the named configuration.");

  cm->DefineProperty
    ("IMPORTED_LINK_INTERFACE_LIBRARIES", cmProperty::TARGET,
     "Transitive link interface of an IMPORTED target.",
     "Set this to the list of libraries whose interface is included when "
     "an IMPORTED library target is linked to another target.  "
     "The libraries will be included on the link line for the target.  "
     "Unlike the LINK_INTERFACE_LIBRARIES property, this property applies "
     "to all imported target types, including STATIC libraries.  "
     "This property is ignored for non-imported targets.");

  cm->DefineProperty
    ("IMPORTED_LINK_INTERFACE_LIBRARIES_<CONFIG>", cmProperty::TARGET,
     "<CONFIG>-specific version of IMPORTED_LINK_INTERFACE_LIBRARIES.",
     "Configuration names correspond to those provided by the project "
     "from which the target is imported.  "
     "If set, this property completely overrides the generic property "
     "for the named configuration.");

  cm->DefineProperty
    ("IMPORTED_LINK_INTERFACE_LANGUAGES", cmProperty::TARGET,
     "Languages compiled into an IMPORTED static library.",
     "Set this to the list of languages of source files compiled to "
     "produce a STATIC IMPORTED library (such as \"C\" or \"CXX\").  "
     "CMake accounts for these languages when computing how to link a "
     "target to the imported library.  "
     "For example, when a C executable links to an imported C++ static "
     "library CMake chooses the C++ linker to satisfy language runtime "
     "dependencies of the static library.  "
     "\n"
     "This property is ignored for targets that are not STATIC libraries.  "
     "This property is ignored for non-imported targets.");

  cm->DefineProperty
    ("IMPORTED_LINK_INTERFACE_LANGUAGES_<CONFIG>", cmProperty::TARGET,
     "<CONFIG>-specific version of IMPORTED_LINK_INTERFACE_LANGUAGES.",
     "Configuration names correspond to those provided by the project "
     "from which the target is imported.  "
     "If set, this property completely overrides the generic property "
     "for the named configuration.");

  cm->DefineProperty
    ("IMPORTED_LINK_INTERFACE_MULTIPLICITY", cmProperty::TARGET,
     "Repetition count for cycles of IMPORTED static libraries.",
     "This is LINK_INTERFACE_MULTIPLICITY for IMPORTED targets.");
  cm->DefineProperty
    ("IMPORTED_LINK_INTERFACE_MULTIPLICITY_<CONFIG>", cmProperty::TARGET,
     "<CONFIG>-specific version of IMPORTED_LINK_INTERFACE_MULTIPLICITY.",
     "If set, this property completely overrides the generic property "
     "for the named configuration.");

  cm->DefineProperty
    ("IMPORTED_LOCATION", cmProperty::TARGET,
     "Full path to the main file on disk for an IMPORTED target.",
     "Set this to the location of an IMPORTED target file on disk.  "
     "For executables this is the location of the executable file.  "
     "For bundles on OS X this is the location of the executable file "
     "inside Contents/MacOS under the application bundle folder.  "
     "For static libraries and modules this is the location of the "
     "library or module.  "
     "For shared libraries on non-DLL platforms this is the location of "
     "the shared library.  "
     "For frameworks on OS X this is the location of the library file "
     "symlink just inside the framework folder.  "
     "For DLLs this is the location of the \".dll\" part of the library.  "
     "For UNKNOWN libraries this is the location of the file to be linked.  "
     "Ignored for non-imported targets."
     "\n"
     "Projects may skip IMPORTED_LOCATION if the configuration-specific "
     "property IMPORTED_LOCATION_<CONFIG> is set.  "
     "To get the location of an imported target read one of the "
     "LOCATION or LOCATION_<CONFIG> properties.");

  cm->DefineProperty
    ("IMPORTED_LOCATION_<CONFIG>", cmProperty::TARGET,
     "<CONFIG>-specific version of IMPORTED_LOCATION property.",
     "Configuration names correspond to those provided by the project "
     "from which the target is imported.");

  cm->DefineProperty
    ("IMPORTED_SONAME", cmProperty::TARGET,
     "The \"soname\" of an IMPORTED target of shared library type.",
     "Set this to the \"soname\" embedded in an imported shared library.  "
     "This is meaningful only on platforms supporting the feature.  "
     "Ignored for non-imported targets.");

  cm->DefineProperty
    ("IMPORTED_SONAME_<CONFIG>", cmProperty::TARGET,
     "<CONFIG>-specific version of IMPORTED_SONAME property.",
     "Configuration names correspond to those provided by the project "
     "from which the target is imported.");

  cm->DefineProperty
    ("IMPORTED_NO_SONAME", cmProperty::TARGET,
     "Specifies that an IMPORTED shared library target has no \"soname\".  ",
     "Set this property to true for an imported shared library file that "
     "has no \"soname\" field.  "
     "CMake may adjust generated link commands for some platforms to prevent "
     "the linker from using the path to the library in place of its missing "
     "soname.  "
     "Ignored for non-imported targets.");

  cm->DefineProperty
    ("IMPORTED_NO_SONAME_<CONFIG>", cmProperty::TARGET,
     "<CONFIG>-specific version of IMPORTED_NO_SONAME property.",
     "Configuration names correspond to those provided by the project "
     "from which the target is imported.");

  cm->DefineProperty
    ("EXCLUDE_FROM_ALL", cmProperty::TARGET,
     "Exclude the target from the all target.",
     "A property on a target that indicates if the target is excluded "
     "from the default build target. If it is not, then with a Makefile "
     "for example typing make will cause this target to be built. "
     "The same concept applies to the default build of other generators. "
     "Installing a target with EXCLUDE_FROM_ALL set to true has "
     "undefined behavior.");

  cm->DefineProperty
    ("LINK_LIBRARIES", cmProperty::TARGET,
     "List of direct link dependencies.",
     "This property specifies the list of libraries or targets which will be "
     "used for linking. "
     "In addition to accepting values from the target_link_libraries "
     "command, values may be set directly on any target using the "
     "set_property command. "
     "\n"
     "The target property values are used by the generators to set "
     "the link libraries for the compiler.  "
     "See also the target_link_libraries command.\n"
     "Contents of LINK_LIBRARIES may use \"generator expressions\" with "
     "the syntax \"$<...>\".  "
     CM_DOCUMENT_COMMAND_GENERATOR_EXPRESSIONS);

  cm->DefineProperty
    ("INCLUDE_DIRECTORIES", cmProperty::TARGET,
     "List of preprocessor include file search directories.",
     "This property specifies the list of directories given "
     "so far to the include_directories command. "
     "This property exists on directories and targets. "
     "In addition to accepting values from the include_directories "
     "command, values may be set directly on any directory or any "
     "target using the set_property command. "
     "A target gets its initial value for this property from the value "
     "of the directory property. "
     "A directory gets its initial value from its parent directory if "
     "it has one. "
     "Both directory and target property values are adjusted by calls "
     "to the include_directories command."
     "\n"
     "The target property values are used by the generators to set "
     "the include paths for the compiler.  "
     "See also the include_directories command.\n"
     "Contents of INCLUDE_DIRECTORIES may use \"generator expressions\" with "
     "the syntax \"$<...>\".  "
     CM_DOCUMENT_COMMAND_GENERATOR_EXPRESSIONS);

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
     "to use in installed targets (for platforms that support it).  "
     "This property is initialized by the value of the variable "
     "CMAKE_INSTALL_RPATH if it is set when a target is created.");

  cm->DefineProperty
    ("INSTALL_RPATH_USE_LINK_PATH", cmProperty::TARGET,
     "Add paths to linker search and installed rpath.",
     "INSTALL_RPATH_USE_LINK_PATH is a boolean that if set to true will "
     "append directories in the linker search path and outside the "
     "project to the INSTALL_RPATH.  "
     "This property is initialized by the value of the variable "
     "CMAKE_INSTALL_RPATH_USE_LINK_PATH if it is set when a target is "
     "created.");

  cm->DefineProperty
    ("INTERPROCEDURAL_OPTIMIZATION", cmProperty::TARGET,
     "Enable interprocedural optimization for a target.",
     "If set to true, enables interprocedural optimizations "
     "if they are known to be supported by the compiler.");

  cm->DefineProperty
    ("INTERPROCEDURAL_OPTIMIZATION_<CONFIG>", cmProperty::TARGET,
     "Per-configuration interprocedural optimization for a target.",
     "This is a per-configuration version of INTERPROCEDURAL_OPTIMIZATION.  "
     "If set, this property overrides the generic property "
     "for the named configuration.");

  cm->DefineProperty
    ("LABELS", cmProperty::TARGET,
     "Specify a list of text labels associated with a target.",
     "Target label semantics are currently unspecified.");

  cm->DefineProperty
    ("LINK_FLAGS", cmProperty::TARGET,
     "Additional flags to use when linking this target.",
     "The LINK_FLAGS property can be used to add extra flags to the "
     "link step of a target. LINK_FLAGS_<CONFIG> will add to the "
     "configuration <CONFIG>, "
     "for example, DEBUG, RELEASE, MINSIZEREL, RELWITHDEBINFO. ");

  cm->DefineProperty
    ("LINK_FLAGS_<CONFIG>", cmProperty::TARGET,
     "Per-configuration linker flags for a target.",
     "This is the configuration-specific version of LINK_FLAGS.");

#define CM_LINK_SEARCH_SUMMARY \
  "Some linkers support switches such as -Bstatic and -Bdynamic " \
  "to determine whether to use static or shared libraries for -lXXX " \
  "options.  CMake uses these options to set the link type for " \
  "libraries whose full paths are not known or (in some cases) are in " \
  "implicit link directories for the platform.  "

  cm->DefineProperty
    ("LINK_SEARCH_START_STATIC", cmProperty::TARGET,
     "Assume the linker looks for static libraries by default.",
     CM_LINK_SEARCH_SUMMARY
     "By default the linker search type is assumed to be -Bdynamic at "
     "the beginning of the library list.  This property switches the "
     "assumption to -Bstatic.  It is intended for use when linking an "
     "executable statically (e.g. with the GNU -static option).  "
     "See also LINK_SEARCH_END_STATIC.");

  cm->DefineProperty
    ("LINK_SEARCH_END_STATIC", cmProperty::TARGET,
     "End a link line such that static system libraries are used.",
     CM_LINK_SEARCH_SUMMARY
     "By default CMake adds an option at the end of the library list (if "
     "necessary) to set the linker search type back to its starting type.  "
     "This property switches the final linker search type to -Bstatic "
     "regardless of how it started.  "
     "See also LINK_SEARCH_START_STATIC.");

  cm->DefineProperty
    ("LINKER_LANGUAGE", cmProperty::TARGET,
     "Specifies language whose compiler will invoke the linker.",
     "For executables, shared libraries, and modules, this sets the "
     "language whose compiler is used to link the target "
     "(such as \"C\" or \"CXX\").  "
     "A typical value for an executable is the language of the source "
     "file providing the program entry point (main).  "
     "If not set, the language with the highest linker preference "
     "value is the default.  "
     "See documentation of CMAKE_<LANG>_LINKER_PREFERENCE variables.");

  cm->DefineProperty
    ("LOCATION", cmProperty::TARGET,
     "Read-only location of a target on disk.",
     "For an imported target, this read-only property returns the value of "
     "the LOCATION_<CONFIG> property for an unspecified configuration "
     "<CONFIG> provided by the target.\n"
     "For a non-imported target, this property is provided for compatibility "
     "with CMake 2.4 and below.  "
     "It was meant to get the location of an executable target's output file "
     "for use in add_custom_command.  "
     "The path may contain a build-system-specific portion that "
     "is replaced at build time with the configuration getting built "
     "(such as \"$(ConfigurationName)\" in VS). "
     "In CMake 2.6 and above add_custom_command automatically recognizes a "
     "target name in its COMMAND and DEPENDS options and computes the "
     "target location.  "
     "In CMake 2.8.4 and above add_custom_command recognizes generator "
     "expressions to refer to target locations anywhere in the command.  "
     "Therefore this property is not needed for creating custom commands."
     CM_LOCATION_UNDEFINED_BEHAVIOR("reading this property"));

  cm->DefineProperty
    ("LOCATION_<CONFIG>", cmProperty::TARGET,
     "Read-only property providing a target location on disk.",
     "A read-only property that indicates where a target's main file is "
     "located on disk for the configuration <CONFIG>.  "
     "The property is defined only for library and executable targets.  "
     "An imported target may provide a set of configurations different "
     "from that of the importing project.  "
     "By default CMake looks for an exact-match but otherwise uses an "
     "arbitrary available configuration.  "
     "Use the MAP_IMPORTED_CONFIG_<CONFIG> property to map imported "
     "configurations explicitly."
     CM_LOCATION_UNDEFINED_BEHAVIOR("reading this property"));

  cm->DefineProperty
    ("LINK_DEPENDS", cmProperty::TARGET,
     "Additional files on which a target binary depends for linking.",
     "Specifies a semicolon-separated list of full-paths to files on which "
     "the link rule for this target depends.  "
     "The target binary will be linked if any of the named files is newer "
     "than it."
     "\n"
     "This property is ignored by non-Makefile generators.  "
     "It is intended to specify dependencies on \"linker scripts\" for "
     "custom Makefile link rules.");

  cm->DefineProperty
    ("LINK_DEPENDS_NO_SHARED", cmProperty::TARGET,
     "Do not depend on linked shared library files.",
     "Set this property to true to tell CMake generators not to add "
     "file-level dependencies on the shared library files linked by "
     "this target.  "
     "Modification to the shared libraries will not be sufficient to "
     "re-link this target.  "
     "Logical target-level dependencies will not be affected so the "
     "linked shared libraries will still be brought up to date before "
     "this target is built."
     "\n"
     "This property is initialized by the value of the variable "
     "CMAKE_LINK_DEPENDS_NO_SHARED if it is set when a target is "
     "created.");

  cm->DefineProperty
    ("LINK_INTERFACE_LIBRARIES", cmProperty::TARGET,
     "List public interface libraries for a shared library or executable.",
     "By default linking to a shared library target transitively "
     "links to targets with which the library itself was linked.  "
     "For an executable with exports (see the ENABLE_EXPORTS property) "
     "no default transitive link dependencies are used.  "
     "This property replaces the default transitive link dependencies with "
     "an explicit list.  "
     "When the target is linked into another target the libraries "
     "listed (and recursively their link interface libraries) will be "
     "provided to the other target also.  "
     "If the list is empty then no transitive link dependencies will be "
     "incorporated when this target is linked into another target even if "
     "the default set is non-empty.  "
     "This property is initialized by the value of the variable "
     "CMAKE_LINK_INTERFACE_LIBRARIES if it is set when a target is "
     "created.  "
     "This property is ignored for STATIC libraries.");

  cm->DefineProperty
    ("LINK_INTERFACE_LIBRARIES_<CONFIG>", cmProperty::TARGET,
     "Per-configuration list of public interface libraries for a target.",
     "This is the configuration-specific version of "
     "LINK_INTERFACE_LIBRARIES.  "
     "If set, this property completely overrides the generic property "
     "for the named configuration.");

  cm->DefineProperty
    ("INTERFACE_INCLUDE_DIRECTORIES", cmProperty::TARGET,
     "List of public include directories for a library.",
     "Targets may populate this property to publish the include directories "
     "required to compile against the headers for the target.  Consuming "
     "targets can add entries to their own INCLUDE_DIRECTORIES property such "
     "as $<TARGET_PROPERTY:foo,INTERFACE_INCLUDE_DIRECTORIES> to use the "
     "include directories specified in the interface of 'foo'."
     "\n"
     CM_DOCUMENT_COMMAND_GENERATOR_EXPRESSIONS);

  cm->DefineProperty
    ("INTERFACE_COMPILE_DEFINITIONS", cmProperty::TARGET,
     "List of public compile definitions for a library.",
     "Targets may populate this property to publish the compile definitions "
     "required to compile against the headers for the target.  Consuming "
     "targets can add entries to their own COMPILE_DEFINITIONS property such "
     "as $<TARGET_PROPERTY:foo,INTERFACE_COMPILE_DEFINITIONS> to use the "
     "compile definitions specified in the interface of 'foo'."
     "\n"
     CM_DOCUMENT_COMMAND_GENERATOR_EXPRESSIONS);

  cm->DefineProperty
    ("LINK_INTERFACE_MULTIPLICITY", cmProperty::TARGET,
     "Repetition count for STATIC libraries with cyclic dependencies.",
     "When linking to a STATIC library target with cyclic dependencies the "
     "linker may need to scan more than once through the archives in the "
     "strongly connected component of the dependency graph.  "
     "CMake by default constructs the link line so that the linker will "
     "scan through the component at least twice.  "
     "This property specifies the minimum number of scans if it is larger "
     "than the default.  "
     "CMake uses the largest value specified by any target in a component.");
  cm->DefineProperty
    ("LINK_INTERFACE_MULTIPLICITY_<CONFIG>", cmProperty::TARGET,
     "Per-configuration repetition count for cycles of STATIC libraries.",
     "This is the configuration-specific version of "
     "LINK_INTERFACE_MULTIPLICITY.  "
     "If set, this property completely overrides the generic property "
     "for the named configuration.");

  cm->DefineProperty
    ("MAP_IMPORTED_CONFIG_<CONFIG>", cmProperty::TARGET,
     "Map from project configuration to IMPORTED target's configuration.",
     "Set this to the list of configurations of an imported target that "
     "may be used for the current project's <CONFIG> configuration.  "
     "Targets imported from another project may not provide the same set "
     "of configuration names available in the current project.  "
     "Setting this property tells CMake what imported configurations are "
     "suitable for use when building the <CONFIG> configuration.  "
     "The first configuration in the list found to be provided by the "
     "imported target is selected.  If this property is set and no matching "
     "configurations are available, then the imported target is considered "
     "to be not found.  This property is ignored for non-imported targets.",
     false /* TODO: make this chained */ );

  cm->DefineProperty
    ("OSX_ARCHITECTURES", cmProperty::TARGET,
     "Target specific architectures for OS X.",
     "The OSX_ARCHITECTURES property sets the target binary architecture "
     "for targets on OS X.  "
     "This property is initialized by the value of the variable "
     "CMAKE_OSX_ARCHITECTURES if it is set when a target is created.  "
     "Use OSX_ARCHITECTURES_<CONFIG> to set the binary architectures on a "
     "per-configuration basis.  "
     "<CONFIG> is an upper-case name (ex: \"OSX_ARCHITECTURES_DEBUG\").");

  cm->DefineProperty
    ("OSX_ARCHITECTURES_<CONFIG>", cmProperty::TARGET,
     "Per-configuration OS X binary architectures for a target.",
     "This property is the configuration-specific version of "
     "OSX_ARCHITECTURES.");

  cm->DefineProperty
    ("OUTPUT_NAME", cmProperty::TARGET,
     "Output name for target files.",
     "This sets the base name for output files created for an executable or "
     "library target.  "
     "If not set, the logical target name is used by default.");

  cm->DefineProperty
    ("OUTPUT_NAME_<CONFIG>", cmProperty::TARGET,
     "Per-configuration target file base name.",
     "This is the configuration-specific version of OUTPUT_NAME.");

  cm->DefineProperty
    ("<CONFIG>_OUTPUT_NAME", cmProperty::TARGET,
     "Old per-configuration target file base name.",
     "This is a configuration-specific version of OUTPUT_NAME.  "
     "Use OUTPUT_NAME_<CONFIG> instead.");

  cm->DefineProperty
    ("PDB_NAME", cmProperty::TARGET,
     "Output name for MS debug symbols .pdb file from linker.",
     "Set the base name for debug symbols file created for an "
     "executable or shared library target.  "
     "If not set, the logical target name is used by default.  "
     "\n"
     "This property is not implemented by the Visual Studio 6 generator.");

  cm->DefineProperty
    ("PDB_NAME_<CONFIG>", cmProperty::TARGET,
     "Per-configuration name for MS debug symbols .pdb file.  ",
     "This is the configuration-specific version of PDB_NAME.  "
     "\n"
     "This property is not implemented by the Visual Studio 6 generator.");

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
    ("POSITION_INDEPENDENT_CODE", cmProperty::TARGET,
     "Whether to create a position-independent target",
     "The POSITION_INDEPENDENT_CODE property determines whether position "
     "independent executables or shared libraries will be created.  "
     "This property is true by default for SHARED and MODULE library "
     "targets and false otherwise.  "
     "This property is initialized by the value of the variable "
     "CMAKE_POSITION_INDEPENDENT_CODE if it is set when a target is "
     "created.");

  cm->DefineProperty
    ("INTERFACE_POSITION_INDEPENDENT_CODE", cmProperty::TARGET,
     "Whether consumers need to create a position-independent target",
     "The INTERFACE_POSITION_INDEPENDENT_CODE property informs consumers of "
     "this target whether they must set their POSITION_INDEPENDENT_CODE "
     "property to ON.  If this property is set to ON, then the "
     "POSITION_INDEPENDENT_CODE property on all consumers will be set to "
     "ON.  Similarly, if this property is set to OFF, then the "
     "POSITION_INDEPENDENT_CODE property on all consumers will be set to "
     "OFF.  If this property is undefined, then consumers will determine "
     "their POSITION_INDEPENDENT_CODE property by other means.  Consumers "
     "must ensure that the targets that they link to have a consistent "
     "requirement for their INTERFACE_POSITION_INDEPENDENT_CODE property.");

  cm->DefineProperty
    ("COMPATIBLE_INTERFACE_BOOL", cmProperty::TARGET,
     "Properties which must be compatible with their link interface",
     "The COMPATIBLE_INTERFACE_BOOL property may contain a list of properties"
     "for this target which must be consistent when evaluated as a boolean "
     "in the INTERFACE of all linked dependees.  For example, if a "
     "property \"FOO\" appears in the list, then for each dependee, the "
     "\"INTERFACE_FOO\" property content in all of its dependencies must be "
     "consistent with each other, and with the \"FOO\" property in the "
     "dependee.  Consistency in this sense has the meaning that if the "
     "property is set, then it must have the same boolean value as all "
     "others, and if the property is not set, then it is ignored.  Note that "
     "for each dependee, the set of properties from this property must not "
     "intersect with the set of properties from the "
     "COMPATIBLE_INTERFACE_STRING property.");

  cm->DefineProperty
    ("COMPATIBLE_INTERFACE_STRING", cmProperty::TARGET,
     "Properties which must be string-compatible with their link interface",
     "The COMPATIBLE_INTERFACE_STRING property may contain a list of "
     "properties for this target which must be the same when evaluated as "
     "a string in the INTERFACE of all linked dependees.  For example, "
     "if a property \"FOO\" appears in the list, then for each dependee, the "
     "\"INTERFACE_FOO\" property content in all of its dependencies must be "
     "equal with each other, and with the \"FOO\" property in the dependee.  "
     "If the property is not set, then it is ignored.  Note that for each "
     "dependee, the set of properties from this property must not intersect "
     "with the set of properties from the COMPATIBLE_INTERFACE_BOOL "
     "property.");

  cm->DefineProperty
    ("POST_INSTALL_SCRIPT", cmProperty::TARGET,
     "Deprecated install support.",
     "The PRE_INSTALL_SCRIPT and POST_INSTALL_SCRIPT properties are the "
     "old way to specify CMake scripts to run before and after "
     "installing a target.  They are used only when the old "
     "INSTALL_TARGETS command is used to install the target.  Use the "
     "INSTALL command instead.");

  cm->DefineProperty
    ("PRIVATE_HEADER", cmProperty::TARGET,
     "Specify private header files in a FRAMEWORK shared library target.",
     "Shared library targets marked with the FRAMEWORK property generate "
     "frameworks on OS X and normal shared libraries on other platforms.  "
     "This property may be set to a list of header files to be placed "
     "in the PrivateHeaders directory inside the framework folder.  "
     "On non-Apple platforms these headers may be installed using the "
     "PRIVATE_HEADER option to the install(TARGETS) command.");

  cm->DefineProperty
    ("PUBLIC_HEADER", cmProperty::TARGET,
     "Specify public header files in a FRAMEWORK shared library target.",
     "Shared library targets marked with the FRAMEWORK property generate "
     "frameworks on OS X and normal shared libraries on other platforms.  "
     "This property may be set to a list of header files to be placed "
     "in the Headers directory inside the framework folder.  "
     "On non-Apple platforms these headers may be installed using the "
     "PUBLIC_HEADER option to the install(TARGETS) command.");

  cm->DefineProperty
    ("RESOURCE", cmProperty::TARGET,
     "Specify resource files in a FRAMEWORK shared library target.",
     "Shared library targets marked with the FRAMEWORK property generate "
     "frameworks on OS X and normal shared libraries on other platforms.  "
     "This property may be set to a list of files to be placed "
     "in the Resources directory inside the framework folder.  "
     "On non-Apple platforms these files may be installed using the "
     "RESOURCE option to the install(TARGETS) command.");

  cm->DefineProperty
    ("RULE_LAUNCH_COMPILE", cmProperty::TARGET,
     "Specify a launcher for compile rules.",
     "See the global property of the same name for details.  "
     "This overrides the global and directory property for a target.",
     true);
  cm->DefineProperty
    ("RULE_LAUNCH_LINK", cmProperty::TARGET,
     "Specify a launcher for link rules.",
     "See the global property of the same name for details.  "
     "This overrides the global and directory property for a target.",
     true);
  cm->DefineProperty
    ("RULE_LAUNCH_CUSTOM", cmProperty::TARGET,
     "Specify a launcher for custom rules.",
     "See the global property of the same name for details.  "
     "This overrides the global and directory property for a target.",
     true);

  cm->DefineProperty
    ("SKIP_BUILD_RPATH", cmProperty::TARGET,
     "Should rpaths be used for the build tree.",
     "SKIP_BUILD_RPATH is a boolean specifying whether to skip automatic "
     "generation of an rpath allowing the target to run from the "
     "build tree.  "
     "This property is initialized by the value of the variable "
     "CMAKE_SKIP_BUILD_RPATH if it is set when a target is created.");

  cm->DefineProperty
    ("NO_SONAME", cmProperty::TARGET,
     "Whether to set \"soname\" when linking a shared library or module.",
     "Enable this boolean property if a generated shared library or module "
     "should not have \"soname\" set. Default is to set \"soname\" on all "
     "shared libraries and modules as long as the platform supports it. "
     "Generally, use this property only for leaf private libraries or "
     "plugins. If you use it on normal shared libraries which other targets "
     "link against, on some platforms a linker will insert a full path to "
     "the library (as specified at link time) into the dynamic section of "
     "the dependent binary. Therefore, once installed, dynamic loader may "
     "eventually fail to locate the library for the binary.");

  cm->DefineProperty
    ("SOVERSION", cmProperty::TARGET,
     "What version number is this target.",
     "For shared libraries VERSION and SOVERSION can be used to specify "
     "the build version and api version respectively. When building or "
     "installing appropriate symlinks are created if the platform "
     "supports symlinks and the linker supports so-names. "
     "If only one of both is specified the missing is assumed to have "
     "the same version number. "
     "SOVERSION is ignored if NO_SONAME property is set. "
     "For shared libraries and executables on Windows the VERSION "
     "attribute is parsed to extract a \"major.minor\" version number. "
     "These numbers are used as the image version of the binary. ");

  cm->DefineProperty
    ("STATIC_LIBRARY_FLAGS", cmProperty::TARGET,
     "Extra flags to use when linking static libraries.",
     "Extra flags to use when linking a static library.");

  cm->DefineProperty
    ("STATIC_LIBRARY_FLAGS_<CONFIG>", cmProperty::TARGET,
     "Per-configuration flags for creating a static library.",
     "This is the configuration-specific version of STATIC_LIBRARY_FLAGS.");

  cm->DefineProperty
    ("SUFFIX", cmProperty::TARGET,
     "What comes after the target name.",
     "A target property that can be set to override the suffix "
     "(such as \".so\" or \".exe\") on the name of a library, module or "
     "executable.");

  cm->DefineProperty
    ("TYPE", cmProperty::TARGET,
     "The type of the target.",
     "This read-only property can be used to test the type of the given "
     "target. It will be one of STATIC_LIBRARY, MODULE_LIBRARY, "
     "SHARED_LIBRARY, EXECUTABLE or one of the internal target types.");

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
     "Build an executable with a WinMain entry point on windows.",
     "When this property is set to true the executable when linked "
     "on Windows will be created with a WinMain() entry point instead "
     "of just main().  "
     "This makes it a GUI executable instead of a console application.  "
     "See the CMAKE_MFC_FLAG variable documentation to configure use "
     "of MFC for WinMain executables.  "
     "This property is initialized by the value of the variable "
     "CMAKE_WIN32_EXECUTABLE if it is set when a target is created.");

  cm->DefineProperty
    ("MACOSX_BUNDLE", cmProperty::TARGET,
     "Build an executable as an application bundle on Mac OS X.",
     "When this property is set to true the executable when built "
     "on Mac OS X will be created as an application bundle.  "
     "This makes it a GUI executable that can be launched from "
     "the Finder.  "
     "See the MACOSX_BUNDLE_INFO_PLIST target property for information "
     "about creation of the Info.plist file for the application bundle.  "
     "This property is initialized by the value of the variable "
     "CMAKE_MACOSX_BUNDLE if it is set when a target is created.");

  cm->DefineProperty
    ("MACOSX_BUNDLE_INFO_PLIST", cmProperty::TARGET,
     "Specify a custom Info.plist template for a Mac OS X App Bundle.",
     "An executable target with MACOSX_BUNDLE enabled will be built as an "
     "application bundle on Mac OS X.  "
     "By default its Info.plist file is created by configuring a template "
     "called MacOSXBundleInfo.plist.in located in the CMAKE_MODULE_PATH.  "
     "This property specifies an alternative template file name which "
     "may be a full path.\n"
     "The following target properties may be set to specify content to "
     "be configured into the file:\n"
     "  MACOSX_BUNDLE_INFO_STRING\n"
     "  MACOSX_BUNDLE_ICON_FILE\n"
     "  MACOSX_BUNDLE_GUI_IDENTIFIER\n"
     "  MACOSX_BUNDLE_LONG_VERSION_STRING\n"
     "  MACOSX_BUNDLE_BUNDLE_NAME\n"
     "  MACOSX_BUNDLE_SHORT_VERSION_STRING\n"
     "  MACOSX_BUNDLE_BUNDLE_VERSION\n"
     "  MACOSX_BUNDLE_COPYRIGHT\n"
     "CMake variables of the same name may be set to affect all targets "
     "in a directory that do not have each specific property set.  "
     "If a custom Info.plist is specified by this property it may of course "
     "hard-code all the settings instead of using the target properties.");

  cm->DefineProperty
    ("MACOSX_FRAMEWORK_INFO_PLIST", cmProperty::TARGET,
     "Specify a custom Info.plist template for a Mac OS X Framework.",
     "An library target with FRAMEWORK enabled will be built as a "
     "framework on Mac OS X.  "
     "By default its Info.plist file is created by configuring a template "
     "called MacOSXFrameworkInfo.plist.in located in the CMAKE_MODULE_PATH.  "
     "This property specifies an alternative template file name which "
     "may be a full path.\n"
     "The following target properties may be set to specify content to "
     "be configured into the file:\n"
     "  MACOSX_FRAMEWORK_ICON_FILE\n"
     "  MACOSX_FRAMEWORK_IDENTIFIER\n"
     "  MACOSX_FRAMEWORK_SHORT_VERSION_STRING\n"
     "  MACOSX_FRAMEWORK_BUNDLE_VERSION\n"
     "CMake variables of the same name may be set to affect all targets "
     "in a directory that do not have each specific property set.  "
     "If a custom Info.plist is specified by this property it may of course "
     "hard-code all the settings instead of using the target properties.");

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
     "For DLL platforms an import library will be created for the "
     "exported symbols and then used for linking.  "
     "All Windows-based systems including Cygwin are DLL platforms.  "
     "For non-DLL platforms that require all symbols to be resolved at "
     "link time, such as Mac OS X, the module will \"link\" to the "
     "executable using a flag like \"-bundle_loader\".  "
     "For other non-DLL platforms the link rule is simply ignored since "
     "the dynamic loader will automatically bind symbols when the "
     "module is loaded.  "
      );

  cm->DefineProperty
    ("Fortran_FORMAT", cmProperty::TARGET,
     "Set to FIXED or FREE to indicate the Fortran source layout.",
     "This property tells CMake whether the Fortran source files "
     "in a target use fixed-format or free-format.  "
     "CMake will pass the corresponding format flag to the compiler.  "
     "Use the source-specific Fortran_FORMAT property to change the "
     "format of a specific source file.  "
     "If the variable CMAKE_Fortran_FORMAT is set when a target "
     "is created its value is used to initialize this property.");

  cm->DefineProperty
    ("Fortran_MODULE_DIRECTORY", cmProperty::TARGET,
     "Specify output directory for Fortran modules provided by the target.",
     "If the target contains Fortran source files that provide modules "
     "and the compiler supports a module output directory this specifies "
     "the directory in which the modules will be placed.  "
     "When this property is not set the modules will be placed in the "
     "build directory corresponding to the target's source directory.  "
     "If the variable CMAKE_Fortran_MODULE_DIRECTORY is set when a target "
     "is created its value is used to initialize this property."
     "\n"
     "Note that some compilers will automatically search the module output "
     "directory for modules USEd during compilation but others will not.  "
     "If your sources USE modules their location must be specified by "
     "INCLUDE_DIRECTORIES regardless of this property.");

  cm->DefineProperty
    ("GNUtoMS", cmProperty::TARGET,
     "Convert GNU import library (.dll.a) to MS format (.lib).",
     "When linking a shared library or executable that exports symbols "
     "using GNU tools on Windows (MinGW/MSYS) with Visual Studio installed "
     "convert the import library (.dll.a) from GNU to MS format (.lib).  "
     "Both import libraries will be installed by install(TARGETS) and "
     "exported by install(EXPORT) and export() to be linked by applications "
     "with either GNU- or MS-compatible tools."
     "\n"
     "If the variable CMAKE_GNUtoMS is set when a target "
     "is created its value is used to initialize this property.  "
     "The variable must be set prior to the first command that enables "
     "a language such as project() or enable_language().  "
     "CMake provides the variable as an option to the user automatically "
     "when configuring on Windows with GNU tools.");

  cm->DefineProperty
    ("XCODE_ATTRIBUTE_<an-attribute>", cmProperty::TARGET,
     "Set Xcode target attributes directly.",
     "Tell the Xcode generator to set '<an-attribute>' to a given value "
     "in the generated Xcode project.  Ignored on other generators.");

  cm->DefineProperty
    ("GENERATOR_FILE_NAME", cmProperty::TARGET,
     "Generator's file for this target.",
     "An internal property used by some generators to record the name of "
     "project or dsp file associated with this target. Note that at configure "
     "time, this property is only set for targets created by "
     "include_external_msproject().");

  cm->DefineProperty
    ("SOURCES", cmProperty::TARGET,
     "Source names specified for a target.",
     "Read-only list of sources specified for a target.  "
     "The names returned are suitable for passing to the "
     "set_source_files_properties command.");

  cm->DefineProperty
    ("FOLDER", cmProperty::TARGET,
     "Set the folder name. Use to organize targets in an IDE.",
     "Targets with no FOLDER property will appear as top level "
     "entities in IDEs like Visual Studio. Targets with the same "
     "FOLDER property value will appear next to each other in a "
     "folder of that name. To nest folders, use FOLDER values such "
     "as 'GUI/Dialogs' with '/' characters separating folder levels.");

  cm->DefineProperty
    ("PROJECT_LABEL", cmProperty::TARGET,
     "Change the name of a target in an IDE.",
     "Can be used to change the name of the target in an IDE "
     "like Visual Studio. ");
  cm->DefineProperty
    ("VS_KEYWORD", cmProperty::TARGET,
     "Visual Studio project keyword.",
     "Can be set to change the visual studio keyword, for example "
     "Qt integration works better if this is set to Qt4VSv1.0. ");
  cm->DefineProperty
    ("VS_SCC_PROVIDER", cmProperty::TARGET,
     "Visual Studio Source Code Control Provider.",
     "Can be set to change the visual studio source code control "
     "provider property.");
  cm->DefineProperty
    ("VS_SCC_LOCALPATH", cmProperty::TARGET,
     "Visual Studio Source Code Control Local Path.",
     "Can be set to change the visual studio source code control "
     "local path property.");
  cm->DefineProperty
    ("VS_SCC_PROJECTNAME", cmProperty::TARGET,
     "Visual Studio Source Code Control Project.",
     "Can be set to change the visual studio source code control "
     "project name property.");
  cm->DefineProperty
    ("VS_SCC_AUXPATH", cmProperty::TARGET,
     "Visual Studio Source Code Control Aux Path.",
     "Can be set to change the visual studio source code control "
     "auxpath property.");
  cm->DefineProperty
    ("VS_GLOBAL_PROJECT_TYPES", cmProperty::TARGET,
     "Visual Studio project type(s).",
     "Can be set to one or more UUIDs recognized by Visual Studio "
     "to indicate the type of project. This value is copied "
     "verbatim into the generated project file. Example for a "
     "managed C++ unit testing project:\n"
     " {3AC096D0-A1C2-E12C-1390-A8335801FDAB};"
     "{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}\n"
     "UUIDs are semicolon-delimited.");
  cm->DefineProperty
    ("VS_GLOBAL_KEYWORD", cmProperty::TARGET,
     "Visual Studio project keyword.",
     "Sets the \"keyword\" attribute for a generated Visual Studio "
     "project. Defaults to \"Win32Proj\". You may wish to override "
     "this value with \"ManagedCProj\", for example, in a Visual "
     "Studio managed C++ unit test project.");
  cm->DefineProperty
    ("VS_DOTNET_REFERENCES", cmProperty::TARGET,
     "Visual Studio managed project .NET references",
     "Adds one or more semicolon-delimited .NET references to a "
     "generated Visual Studio project. For example, \"System;"
     "System.Windows.Forms\".");
  cm->DefineProperty
    ("VS_WINRT_EXTENSIONS", cmProperty::TARGET,
     "Visual Studio project C++/CX language extensions for Windows Runtime",
     "Can be set to enable C++/CX language extensions.");
  cm->DefineProperty
    ("VS_WINRT_REFERENCES", cmProperty::TARGET,
     "Visual Studio project Windows Runtime Metadata references",
     "Adds one or more semicolon-delimited WinRT references to a "
     "generated Visual Studio project. For example, \"Windows;"
     "Windows.UI.Core\".");
  cm->DefineProperty
    ("VS_GLOBAL_<variable>", cmProperty::TARGET,
     "Visual Studio project-specific global variable.",
     "Tell the Visual Studio generator to set the global variable "
     "'<variable>' to a given value in the generated Visual Studio "
     "project. Ignored on other generators. Qt integration works "
     "better if VS_GLOBAL_QtVersion is set to the version "
     "FindQt4.cmake found. For example, \"4.7.3\"");

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

#define CM_TARGET_OUTDIR_DOC(TYPE, type)                                    \
     "This property specifies the directory into which " #type " target "   \
     "files should be built. "                                              \
     "Multi-configuration generators (VS, Xcode) append "                   \
     "a per-configuration subdirectory to the specified directory.  "       \
     CM_TARGET_FILE_TYPES_DOC "  "                                          \
     "This property is initialized by the value of the variable "           \
     "CMAKE_" #TYPE "_OUTPUT_DIRECTORY if it is set when a target is created."

#define CM_TARGET_OUTDIR_CONFIG_DOC(TYPE)                                   \
     "This is a per-configuration version of " #TYPE "_OUTPUT_DIRECTORY, "  \
     "but multi-configuration generators (VS, Xcode) do NOT append "        \
     "a per-configuration subdirectory to the specified directory.  "       \
     "This property is initialized by the value of the variable "           \
     "CMAKE_" #TYPE "_OUTPUT_DIRECTORY_<CONFIG> "                           \
     "if it is set when a target is created."

  cm->DefineProperty
    ("ARCHIVE_OUTPUT_DIRECTORY", cmProperty::TARGET,
     "Output directory in which to build ARCHIVE target files.",
     CM_TARGET_OUTDIR_DOC(ARCHIVE, archive));
  cm->DefineProperty
    ("ARCHIVE_OUTPUT_DIRECTORY_<CONFIG>", cmProperty::TARGET,
     "Per-configuration output directory for ARCHIVE target files.",
     CM_TARGET_OUTDIR_CONFIG_DOC(ARCHIVE));
  cm->DefineProperty
    ("LIBRARY_OUTPUT_DIRECTORY", cmProperty::TARGET,
     "Output directory in which to build LIBRARY target files.",
     CM_TARGET_OUTDIR_DOC(LIBRARY, library));
  cm->DefineProperty
    ("LIBRARY_OUTPUT_DIRECTORY_<CONFIG>", cmProperty::TARGET,
     "Per-configuration output directory for LIBRARY target files.",
     CM_TARGET_OUTDIR_CONFIG_DOC(LIBRARY));
  cm->DefineProperty
    ("RUNTIME_OUTPUT_DIRECTORY", cmProperty::TARGET,
     "Output directory in which to build RUNTIME target files.",
     CM_TARGET_OUTDIR_DOC(RUNTIME, runtime));
  cm->DefineProperty
    ("RUNTIME_OUTPUT_DIRECTORY_<CONFIG>", cmProperty::TARGET,
     "Per-configuration output directory for RUNTIME target files.",
     CM_TARGET_OUTDIR_CONFIG_DOC(RUNTIME));

  cm->DefineProperty
    ("PDB_OUTPUT_DIRECTORY", cmProperty::TARGET,
     "Output directory for MS debug symbols .pdb file from linker.",
     "This property specifies the directory into which the MS debug symbols "
     "will be placed by the linker.  "
     "This property is initialized by the value of the variable "
     "CMAKE_PDB_OUTPUT_DIRECTORY if it is set when a target is created."
     "\n"
     "This property is not implemented by the Visual Studio 6 generator.");
  cm->DefineProperty
    ("PDB_OUTPUT_DIRECTORY_<CONFIG>", cmProperty::TARGET,
     "Per-configuration output directory for MS debug symbols .pdb files.",
     "This is a per-configuration version of PDB_OUTPUT_DIRECTORY, "
     "but multi-configuration generators (VS, Xcode) do NOT append "
     "a per-configuration subdirectory to the specified directory. "
     "This property is initialized by the value of the variable "
     "CMAKE_PDB_OUTPUT_DIRECTORY_<CONFIG> "
     "if it is set when a target is created."
     "\n"
     "This property is not implemented by the Visual Studio 6 generator.");

  cm->DefineProperty
    ("ARCHIVE_OUTPUT_NAME", cmProperty::TARGET,
     "Output name for ARCHIVE target files.",
     "This property specifies the base name for archive target files. "
     "It overrides OUTPUT_NAME and OUTPUT_NAME_<CONFIG> properties.  "
     CM_TARGET_FILE_TYPES_DOC);
  cm->DefineProperty
    ("ARCHIVE_OUTPUT_NAME_<CONFIG>", cmProperty::TARGET,
     "Per-configuration output name for ARCHIVE target files.",
     "This is the configuration-specific version of ARCHIVE_OUTPUT_NAME.");
  cm->DefineProperty
    ("LIBRARY_OUTPUT_NAME", cmProperty::TARGET,
     "Output name for LIBRARY target files.",
     "This property specifies the base name for library target files. "
     "It overrides OUTPUT_NAME and OUTPUT_NAME_<CONFIG> properties.  "
     CM_TARGET_FILE_TYPES_DOC);
  cm->DefineProperty
    ("LIBRARY_OUTPUT_NAME_<CONFIG>", cmProperty::TARGET,
     "Per-configuration output name for LIBRARY target files.",
     "This is the configuration-specific version of LIBRARY_OUTPUT_NAME.");
  cm->DefineProperty
    ("RUNTIME_OUTPUT_NAME", cmProperty::TARGET,
     "Output name for RUNTIME target files.",
     "This property specifies the base name for runtime target files.  "
     "It overrides OUTPUT_NAME and OUTPUT_NAME_<CONFIG> properties.  "
     CM_TARGET_FILE_TYPES_DOC);
  cm->DefineProperty
    ("RUNTIME_OUTPUT_NAME_<CONFIG>", cmProperty::TARGET,
     "Per-configuration output name for RUNTIME target files.",
     "This is the configuration-specific version of RUNTIME_OUTPUT_NAME.");
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

  // Check whether we are targeting an Apple platform.
  this->IsApple = this->Makefile->IsOn("APPLE");

  // Setup default property values.
  this->SetPropertyDefault("INSTALL_NAME_DIR", "");
  this->SetPropertyDefault("INSTALL_RPATH", "");
  this->SetPropertyDefault("INSTALL_RPATH_USE_LINK_PATH", "OFF");
  this->SetPropertyDefault("SKIP_BUILD_RPATH", "OFF");
  this->SetPropertyDefault("BUILD_WITH_INSTALL_RPATH", "OFF");
  this->SetPropertyDefault("ARCHIVE_OUTPUT_DIRECTORY", 0);
  this->SetPropertyDefault("LIBRARY_OUTPUT_DIRECTORY", 0);
  this->SetPropertyDefault("RUNTIME_OUTPUT_DIRECTORY", 0);
  this->SetPropertyDefault("PDB_OUTPUT_DIRECTORY", 0);
  this->SetPropertyDefault("Fortran_FORMAT", 0);
  this->SetPropertyDefault("Fortran_MODULE_DIRECTORY", 0);
  this->SetPropertyDefault("GNUtoMS", 0);
  this->SetPropertyDefault("OSX_ARCHITECTURES", 0);
  this->SetPropertyDefault("AUTOMOC", 0);
  this->SetPropertyDefault("AUTOMOC_MOC_OPTIONS", 0);
  this->SetPropertyDefault("LINK_DEPENDS_NO_SHARED", 0);
  this->SetPropertyDefault("LINK_INTERFACE_LIBRARIES", 0);
  this->SetPropertyDefault("WIN32_EXECUTABLE", 0);
  this->SetPropertyDefault("MACOSX_BUNDLE", 0);

  // Collect the set of configuration types.
  std::vector<std::string> configNames;
  mf->GetConfigurations(configNames);

  // Setup per-configuration property default values.
  const char* configProps[] = {
    "ARCHIVE_OUTPUT_DIRECTORY_",
    "LIBRARY_OUTPUT_DIRECTORY_",
    "RUNTIME_OUTPUT_DIRECTORY_",
    "PDB_OUTPUT_DIRECTORY_",
    0};
  for(std::vector<std::string>::iterator ci = configNames.begin();
      ci != configNames.end(); ++ci)
    {
    std::string configUpper = cmSystemTools::UpperCase(*ci);
    for(const char** p = configProps; *p; ++p)
      {
      std::string property = *p;
      property += configUpper;
      this->SetPropertyDefault(property.c_str(), 0);
      }

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

  // Save the backtrace of target construction.
  this->Makefile->GetBacktrace(this->Internal->Backtrace);

  // Initialize the INCLUDE_DIRECTORIES property based on the current value
  // of the same directory property:
  const std::vector<cmValueWithOrigin> parentIncludes =
                              this->Makefile->GetIncludeDirectoriesEntries();

  for (std::vector<cmValueWithOrigin>::const_iterator it
              = parentIncludes.begin(); it != parentIncludes.end(); ++it)
    {
    this->InsertInclude(*it);
    }

  if(this->TargetTypeValue == cmTarget::SHARED_LIBRARY
      || this->TargetTypeValue == cmTarget::MODULE_LIBRARY)
    {
    this->SetProperty("POSITION_INDEPENDENT_CODE", "True");
    }
  this->SetPropertyDefault("POSITION_INDEPENDENT_CODE", 0);

  // Record current policies for later use.
  this->PolicyStatusCMP0003 =
    this->Makefile->GetPolicyStatus(cmPolicies::CMP0003);
  this->PolicyStatusCMP0004 =
    this->Makefile->GetPolicyStatus(cmPolicies::CMP0004);
  this->PolicyStatusCMP0008 =
    this->Makefile->GetPolicyStatus(cmPolicies::CMP0008);
  this->PolicyStatusCMP0020 =
    this->Makefile->GetPolicyStatus(cmPolicies::CMP0020);
}

//----------------------------------------------------------------------------
void cmTarget::FinishConfigure()
{
  // Erase any cached link information that might have been comptued
  // on-demand during the configuration.  This ensures that build
  // system generation uses up-to-date information even if other cache
  // invalidation code in this source file is buggy.
  this->ClearLinkMaps();

  // Do old-style link dependency analysis.
  this->AnalyzeLibDependencies(*this->Makefile);
}

//----------------------------------------------------------------------------
void cmTarget::ClearLinkMaps()
{
  this->Internal->LinkImplMap.clear();
  this->Internal->LinkInterfaceMap.clear();
  this->Internal->LinkClosureMap.clear();
  for (cmTargetLinkInformationMap::const_iterator it
      = this->LinkInformation.begin();
      it != this->LinkInformation.end(); ++it)
    {
    delete it->second;
    }
  this->LinkInformation.clear();
}

//----------------------------------------------------------------------------
cmListFileBacktrace const& cmTarget::GetBacktrace() const
{
  return this->Internal->Backtrace;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetSupportDirectory() const
{
  std::string dir = this->Makefile->GetCurrentOutputDirectory();
  dir += cmake::GetCMakeFilesDirectory();
  dir += "/";
  dir += this->Name;
#if defined(__VMS)
  dir += "_dir";
#else
  dir += ".dir";
#endif
  return dir;
}

//----------------------------------------------------------------------------
bool cmTarget::IsExecutableWithExports()
{
  return (this->GetType() == cmTarget::EXECUTABLE &&
          this->GetPropertyAsBool("ENABLE_EXPORTS"));
}

//----------------------------------------------------------------------------
bool cmTarget::IsLinkable()
{
  return (this->GetType() == cmTarget::STATIC_LIBRARY ||
          this->GetType() == cmTarget::SHARED_LIBRARY ||
          this->GetType() == cmTarget::MODULE_LIBRARY ||
          this->GetType() == cmTarget::UNKNOWN_LIBRARY ||
          this->IsExecutableWithExports());
}

//----------------------------------------------------------------------------
bool cmTarget::HasImportLibrary()
{
  return (this->DLLPlatform &&
          (this->GetType() == cmTarget::SHARED_LIBRARY ||
           this->IsExecutableWithExports()));
}

//----------------------------------------------------------------------------
bool cmTarget::IsFrameworkOnApple()
{
  return (this->GetType() == cmTarget::SHARED_LIBRARY &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("FRAMEWORK"));
}

//----------------------------------------------------------------------------
bool cmTarget::IsAppBundleOnApple()
{
  return (this->GetType() == cmTarget::EXECUTABLE &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("MACOSX_BUNDLE"));
}

//----------------------------------------------------------------------------
bool cmTarget::IsCFBundleOnApple()
{
  return (this->GetType() == cmTarget::MODULE_LIBRARY &&
          this->Makefile->IsOn("APPLE") &&
          this->GetPropertyAsBool("BUNDLE"));
}

//----------------------------------------------------------------------------
class cmTargetTraceDependencies
{
public:
  cmTargetTraceDependencies(cmTarget* target, cmTargetInternals* internal,
                            const char* vsProjectFile);
  void Trace();
private:
  cmTarget* Target;
  cmTargetInternals* Internal;
  cmMakefile* Makefile;
  cmGlobalGenerator* GlobalGenerator;
  typedef cmTargetInternals::SourceEntry SourceEntry;
  SourceEntry* CurrentEntry;
  std::queue<cmSourceFile*> SourceQueue;
  std::set<cmSourceFile*> SourcesQueued;
  typedef std::map<cmStdString, cmSourceFile*> NameMapType;
  NameMapType NameMap;

  void QueueSource(cmSourceFile* sf);
  void FollowName(std::string const& name);
  void FollowNames(std::vector<std::string> const& names);
  bool IsUtility(std::string const& dep);
  void CheckCustomCommand(cmCustomCommand const& cc);
  void CheckCustomCommands(const std::vector<cmCustomCommand>& commands);
};

//----------------------------------------------------------------------------
cmTargetTraceDependencies
::cmTargetTraceDependencies(cmTarget* target, cmTargetInternals* internal,
                            const char* vsProjectFile):
  Target(target), Internal(internal)
{
  // Convenience.
  this->Makefile = this->Target->GetMakefile();
  this->GlobalGenerator =
    this->Makefile->GetLocalGenerator()->GetGlobalGenerator();
  this->CurrentEntry = 0;

  // Queue all the source files already specified for the target.
  std::vector<cmSourceFile*> const& sources = this->Target->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator si = sources.begin();
      si != sources.end(); ++si)
    {
    this->QueueSource(*si);
    }

  // Queue the VS project file to check dependencies on the rule to
  // generate it.
  if(vsProjectFile)
    {
    this->FollowName(vsProjectFile);
    }

  // Queue pre-build, pre-link, and post-build rule dependencies.
  this->CheckCustomCommands(this->Target->GetPreBuildCommands());
  this->CheckCustomCommands(this->Target->GetPreLinkCommands());
  this->CheckCustomCommands(this->Target->GetPostBuildCommands());
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::Trace()
{
  // Process one dependency at a time until the queue is empty.
  while(!this->SourceQueue.empty())
    {
    // Get the next source from the queue.
    cmSourceFile* sf = this->SourceQueue.front();
    this->SourceQueue.pop();
    this->CurrentEntry = &this->Internal->SourceEntries[sf];

    // Queue dependencies added explicitly by the user.
    if(const char* additionalDeps = sf->GetProperty("OBJECT_DEPENDS"))
      {
      std::vector<std::string> objDeps;
      cmSystemTools::ExpandListArgument(additionalDeps, objDeps);
      this->FollowNames(objDeps);
      }

    // Queue the source needed to generate this file, if any.
    this->FollowName(sf->GetFullPath());

    // Queue dependencies added programatically by commands.
    this->FollowNames(sf->GetDepends());

    // Queue custom command dependencies.
    if(cmCustomCommand const* cc = sf->GetCustomCommand())
      {
      this->CheckCustomCommand(*cc);
      }
    }
  this->CurrentEntry = 0;
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::QueueSource(cmSourceFile* sf)
{
  if(this->SourcesQueued.insert(sf).second)
    {
    this->SourceQueue.push(sf);

    // Make sure this file is in the target.
    this->Target->AddSourceFile(sf);
    }
}

//----------------------------------------------------------------------------
void cmTargetTraceDependencies::FollowName(std::string const& name)
{
  NameMapType::iterator i = this->NameMap.find(name);
  if(i == this->NameMap.end())
    {
    // Check if we know how to generate this file.
    cmSourceFile* sf = this->Makefile->GetSourceFileWithOutput(name.c_str());
    NameMapType::value_type entry(name, sf);
    i = this->NameMap.insert(entry).first;
    }
  if(cmSourceFile* sf = i->second)
    {
    // Record the dependency we just followed.
    if(this->CurrentEntry)
      {
      this->CurrentEntry->Depends.push_back(sf);
      }

    this->QueueSource(sf);
    }
}

//----------------------------------------------------------------------------
void
cmTargetTraceDependencies::FollowNames(std::vector<std::string> const& names)
{
  for(std::vector<std::string>::const_iterator i = names.begin();
      i != names.end(); ++i)
    {
    this->FollowName(*i);
    }
}

//----------------------------------------------------------------------------
bool cmTargetTraceDependencies::IsUtility(std::string const& dep)
{
  // Dependencies on targets (utilities) are supposed to be named by
  // just the target name.  However for compatibility we support
  // naming the output file generated by the target (assuming there is
  // no output-name property which old code would not have set).  In
  // that case the target name will be the file basename of the
  // dependency.
  std::string util = cmSystemTools::GetFilenameName(dep);
  if(cmSystemTools::GetFilenameLastExtension(util) == ".exe")
    {
    util = cmSystemTools::GetFilenameWithoutLastExtension(util);
    }

  // Check for a target with this name.
  if(cmTarget* t = this->Makefile->FindTargetToUse(util.c_str()))
    {
    // If we find the target and the dep was given as a full path,
    // then make sure it was not a full path to something else, and
    // the fact that the name matched a target was just a coincidence.
    if(cmSystemTools::FileIsFullPath(dep.c_str()))
      {
      if(t->GetType() >= cmTarget::EXECUTABLE &&
         t->GetType() <= cmTarget::MODULE_LIBRARY)
        {
        // This is really only for compatibility so we do not need to
        // worry about configuration names and output names.
        std::string tLocation = t->GetLocation(0);
        tLocation = cmSystemTools::GetFilenamePath(tLocation);
        std::string depLocation = cmSystemTools::GetFilenamePath(dep);
        depLocation = cmSystemTools::CollapseFullPath(depLocation.c_str());
        tLocation = cmSystemTools::CollapseFullPath(tLocation.c_str());
        if(depLocation == tLocation)
          {
          this->Target->AddUtility(util.c_str());
          return true;
          }
        }
      }
    else
      {
      // The original name of the dependency was not a full path.  It
      // must name a target, so add the target-level dependency.
      this->Target->AddUtility(util.c_str());
      return true;
      }
    }

  // The dependency does not name a target built in this project.
  return false;
}

//----------------------------------------------------------------------------
void
cmTargetTraceDependencies
::CheckCustomCommand(cmCustomCommand const& cc)
{
  // Transform command names that reference targets built in this
  // project to corresponding target-level dependencies.
  cmGeneratorExpression ge(cc.GetBacktrace());

  // Add target-level dependencies referenced by generator expressions.
  std::set<cmTarget*> targets;

  for(cmCustomCommandLines::const_iterator cit = cc.GetCommandLines().begin();
      cit != cc.GetCommandLines().end(); ++cit)
    {
    std::string const& command = *cit->begin();
    // Check for a target with this name.
    if(cmTarget* t = this->Makefile->FindTargetToUse(command.c_str()))
      {
      if(t->GetType() == cmTarget::EXECUTABLE)
        {
        // The command refers to an executable target built in
        // this project.  Add the target-level dependency to make
        // sure the executable is up to date before this custom
        // command possibly runs.
        this->Target->AddUtility(command.c_str());
        }
      }

    // Check for target references in generator expressions.
    for(cmCustomCommandLine::const_iterator cli = cit->begin();
        cli != cit->end(); ++cli)
      {
      const cmsys::auto_ptr<cmCompiledGeneratorExpression> cge
                                                              = ge.Parse(*cli);
      cge->Evaluate(this->Makefile, 0, true);
      std::set<cmTarget*> geTargets = cge->GetTargets();
      for(std::set<cmTarget*>::const_iterator it = geTargets.begin();
          it != geTargets.end(); ++it)
        {
        targets.insert(*it);
        }
      }
    }

  for(std::set<cmTarget*>::iterator ti = targets.begin();
      ti != targets.end(); ++ti)
    {
    this->Target->AddUtility((*ti)->GetName());
    }

  // Queue the custom command dependencies.
  std::vector<std::string> const& depends = cc.GetDepends();
  for(std::vector<std::string>::const_iterator di = depends.begin();
      di != depends.end(); ++di)
    {
    std::string const& dep = *di;
    if(!this->IsUtility(dep))
      {
      // The dependency does not name a target and may be a file we
      // know how to generate.  Queue it.
      this->FollowName(dep);
      }
    }
}

//----------------------------------------------------------------------------
void
cmTargetTraceDependencies
::CheckCustomCommands(const std::vector<cmCustomCommand>& commands)
{
  for(std::vector<cmCustomCommand>::const_iterator cli = commands.begin();
      cli != commands.end(); ++cli)
    {
    this->CheckCustomCommand(*cli);
    }
}

//----------------------------------------------------------------------------
void cmTarget::TraceDependencies(const char* vsProjectFile)
{
  // CMake-generated targets have no dependencies to trace.  Normally tracing
  // would find nothing anyway, but when building CMake itself the "install"
  // target command ends up referencing the "cmake" target but we do not
  // really want the dependency because "install" depend on "all" anyway.
  if(this->GetType() == cmTarget::GLOBAL_TARGET)
    {
    return;
    }

  // Use a helper object to trace the dependencies.
  cmTargetTraceDependencies tracer(this, this->Internal.Get(), vsProjectFile);
  tracer.Trace();
}

//----------------------------------------------------------------------------
bool cmTarget::FindSourceFiles()
{
  for(std::vector<cmSourceFile*>::const_iterator
        si = this->SourceFiles.begin();
      si != this->SourceFiles.end(); ++si)
    {
    std::string e;
    if((*si)->GetFullPath(&e).empty())
      {
      if(!e.empty())
        {
        cmake* cm = this->Makefile->GetCMakeInstance();
        cm->IssueMessage(cmake::FATAL_ERROR, e,
                         this->GetBacktrace());
        }
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
std::vector<cmSourceFile*> const& cmTarget::GetSourceFiles()
{
  return this->SourceFiles;
}

//----------------------------------------------------------------------------
void cmTarget::AddSourceFile(cmSourceFile* sf)
{
  typedef cmTargetInternals::SourceEntriesType SourceEntriesType;
  SourceEntriesType::iterator i = this->Internal->SourceEntries.find(sf);
  if(i == this->Internal->SourceEntries.end())
    {
    typedef cmTargetInternals::SourceEntry SourceEntry;
    SourceEntriesType::value_type entry(sf, SourceEntry());
    i = this->Internal->SourceEntries.insert(entry).first;
    this->SourceFiles.push_back(sf);
    }
}

//----------------------------------------------------------------------------
std::vector<cmSourceFile*> const*
cmTarget::GetSourceDepends(cmSourceFile* sf)
{
  typedef cmTargetInternals::SourceEntriesType SourceEntriesType;
  SourceEntriesType::iterator i = this->Internal->SourceEntries.find(sf);
  if(i != this->Internal->SourceEntries.end())
    {
    return &i->second.Depends;
    }
  return 0;
}

//----------------------------------------------------------------------------
void cmTarget::AddSources(std::vector<std::string> const& srcs)
{
  for(std::vector<std::string>::const_iterator i = srcs.begin();
      i != srcs.end(); ++i)
    {
    const char* src = i->c_str();
    if(src[0] == '$' && src[1] == '<')
      {
      this->ProcessSourceExpression(*i);
      }
    else
      {
      this->AddSource(src);
      }
    }
}

//----------------------------------------------------------------------------
cmSourceFile* cmTarget::AddSource(const char* s)
{
  std::string src = s;

  // For backwards compatibility replace varibles in source names.
  // This should eventually be removed.
  this->Makefile->ExpandVariablesInString(src);

  cmSourceFile* sf = this->Makefile->GetOrCreateSource(src.c_str());
  this->AddSourceFile(sf);
  return sf;
}

//----------------------------------------------------------------------------
void cmTarget::ProcessSourceExpression(std::string const& expr)
{
  if(strncmp(expr.c_str(), "$<TARGET_OBJECTS:", 17) == 0 &&
     expr[expr.size()-1] == '>')
    {
    std::string objLibName = expr.substr(17, expr.size()-18);
    this->ObjectLibraries.push_back(objLibName);
    }
  else
    {
    cmOStringStream e;
    e << "Unrecognized generator expression:\n"
      << "  " << expr;
    this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
    }
}

//----------------------------------------------------------------------------
struct cmTarget::SourceFileFlags
cmTarget::GetTargetSourceFileFlags(const cmSourceFile* sf)
{
  struct SourceFileFlags flags;
  this->ConstructSourceFileFlags();
  std::map<cmSourceFile const*, SourceFileFlags>::iterator si =
    this->Internal->SourceFlagsMap.find(sf);
  if(si != this->Internal->SourceFlagsMap.end())
    {
    flags = si->second;
    }
  return flags;
}

//----------------------------------------------------------------------------
void cmTarget::ConstructSourceFileFlags()
{
  if(this->Internal->SourceFileFlagsConstructed)
    {
    return;
    }
  this->Internal->SourceFileFlagsConstructed = true;

  // Process public headers to mark the source files.
  if(const char* files = this->GetProperty("PUBLIC_HEADER"))
    {
    std::vector<std::string> relFiles;
    cmSystemTools::ExpandListArgument(files, relFiles);
    for(std::vector<std::string>::iterator it = relFiles.begin();
        it != relFiles.end(); ++it)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(it->c_str()))
        {
        SourceFileFlags& flags = this->Internal->SourceFlagsMap[sf];
        flags.MacFolder = "Headers";
        flags.Type = cmTarget::SourceFileTypePublicHeader;
        }
      }
    }

  // Process private headers after public headers so that they take
  // precedence if a file is listed in both.
  if(const char* files = this->GetProperty("PRIVATE_HEADER"))
    {
    std::vector<std::string> relFiles;
    cmSystemTools::ExpandListArgument(files, relFiles);
    for(std::vector<std::string>::iterator it = relFiles.begin();
        it != relFiles.end(); ++it)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(it->c_str()))
        {
        SourceFileFlags& flags = this->Internal->SourceFlagsMap[sf];
        flags.MacFolder = "PrivateHeaders";
        flags.Type = cmTarget::SourceFileTypePrivateHeader;
        }
      }
    }

  // Mark sources listed as resources.
  if(const char* files = this->GetProperty("RESOURCE"))
    {
    std::vector<std::string> relFiles;
    cmSystemTools::ExpandListArgument(files, relFiles);
    for(std::vector<std::string>::iterator it = relFiles.begin();
        it != relFiles.end(); ++it)
      {
      if(cmSourceFile* sf = this->Makefile->GetSource(it->c_str()))
        {
        SourceFileFlags& flags = this->Internal->SourceFlagsMap[sf];
        flags.MacFolder = "Resources";
        flags.Type = cmTarget::SourceFileTypeResource;
        }
      }
    }

  // Handle the MACOSX_PACKAGE_LOCATION property on source files that
  // were not listed in one of the other lists.
  std::vector<cmSourceFile*> const& sources = this->GetSourceFiles();
  for(std::vector<cmSourceFile*>::const_iterator si = sources.begin();
      si != sources.end(); ++si)
    {
    cmSourceFile* sf = *si;
    if(const char* location = sf->GetProperty("MACOSX_PACKAGE_LOCATION"))
      {
      SourceFileFlags& flags = this->Internal->SourceFlagsMap[sf];
      if(flags.Type == cmTarget::SourceFileTypeNormal)
        {
        flags.MacFolder = location;
        if(strcmp(location, "Resources") == 0)
          {
          flags.Type = cmTarget::SourceFileTypeResource;
          }
        else
          {
          flags.Type = cmTarget::SourceFileTypeMacContent;
          }
        }
      }
    }
}

//----------------------------------------------------------------------------
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
  if(this->LinkDirectoriesEmmitted.insert(d).second)
    {
    this->LinkDirectories.push_back(d);
    }
}

//----------------------------------------------------------------------------
const std::vector<std::string>& cmTarget::GetLinkDirectories()
{
  return this->LinkDirectories;
}

//----------------------------------------------------------------------------
cmTarget::LinkLibraryType cmTarget::ComputeLinkType(const char* config)
{
  // No configuration is always optimized.
  if(!(config && *config))
    {
    return cmTarget::OPTIMIZED;
    }

  // Get the list of configurations considered to be DEBUG.
  std::vector<std::string> const& debugConfigs =
    this->Makefile->GetCMakeInstance()->GetDebugConfigs();

  // Check if any entry in the list matches this configuration.
  std::string configUpper = cmSystemTools::UpperCase(config);
  for(std::vector<std::string>::const_iterator i = debugConfigs.begin();
      i != debugConfigs.end(); ++i)
    {
    if(*i == configUpper)
      {
      return cmTarget::DEBUG;
      }
    }

  // The current configuration is not a debug configuration.
  return cmTarget::OPTIMIZED;
}

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
bool cmTarget::NameResolvesToFramework(const std::string& libname)
{
  return this->GetMakefile()->GetLocalGenerator()->GetGlobalGenerator()->
    NameResolvesToFramework(libname);
}

//----------------------------------------------------------------------------
void cmTarget::GetDirectLinkLibraries(const char *config,
                            std::vector<std::string> &libs, cmTarget *head)
{
  const char *prop = this->GetProperty("LINK_LIBRARIES");
  if (prop)
    {
    cmListFileBacktrace lfbt;
    cmGeneratorExpression ge(lfbt);
    const cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(prop);

    cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                        this->GetName(),
                                        "LINK_LIBRARIES", 0, 0);
    cmSystemTools::ExpandListArgument(cge->Evaluate(this->Makefile,
                                        config,
                                        false,
                                        head,
                                        &dagChecker),
                                      libs);

    std::set<cmStdString> seenProps = cge->GetSeenTargetProperties();
    for (std::set<cmStdString>::const_iterator it = seenProps.begin();
        it != seenProps.end(); ++it)
      {
      if (!this->GetProperty(it->c_str()))
        {
        this->LinkImplicitNullProperties.insert(*it);
        }
      }
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::GetDebugGeneratorExpressions(const std::string &value,
                                  cmTarget::LinkLibraryType llt)
{
  if (llt == GENERAL)
    {
    return value;
    }

  // Get the list of configurations considered to be DEBUG.
  std::vector<std::string> const& debugConfigs =
                      this->Makefile->GetCMakeInstance()->GetDebugConfigs();

  std::string configString = "$<CONFIG:" + debugConfigs[0] + ">";

  if (debugConfigs.size() > 1)
    {
    for(std::vector<std::string>::const_iterator
          li = debugConfigs.begin() + 1; li != debugConfigs.end(); ++li)
      {
      configString += ",$<CONFIG:" + *li + ">";
      }
    configString = "$<OR:" + configString + ">";
    }

  if (llt == OPTIMIZED)
    {
    configString = "$<NOT:" + configString + ">";
    }
  return "$<" + configString + ":" + value + ">";
}

//----------------------------------------------------------------------------
static std::string targetNameGenex(const char *lib)
{
  return std::string("$<TARGET_NAME:") + lib + ">";
}

//----------------------------------------------------------------------------
void cmTarget::AddLinkLibrary(cmMakefile& mf,
                              const char *target, const char* lib,
                              LinkLibraryType llt)
{
  // Never add a self dependency, even if the user asks for it.
  if(strcmp( target, lib ) == 0)
    {
    return;
    }

  {
  cmTarget *tgt = this->Makefile->FindTargetToUse(lib);
  const bool isNonImportedTarget = tgt && !tgt->IsImported();

  const std::string libName = (isNonImportedTarget && llt != GENERAL)
                                                        ? targetNameGenex(lib)
                                                        : std::string(lib);
  this->AppendProperty("LINK_LIBRARIES",
                       this->GetDebugGeneratorExpressions(libName,
                                                          llt).c_str());
  }

  if (cmGeneratorExpression::Find(lib) != std::string::npos)
    {
    return;
    }

  cmTarget::LibraryID tmp;
  tmp.first = lib;
  tmp.second = llt;
  this->LinkLibraries.push_back( tmp );
  this->OriginalLinkLibraries.push_back(tmp);
  this->ClearLinkMaps();

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

//----------------------------------------------------------------------------
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

 {
 // Expand variables in link library names.  This is for backwards
 // compatibility with very early CMake versions and should
 // eventually be removed.  This code was moved here from the end of
 // old source list processing code which was called just before this
 // method.
 for(LinkLibraryVectorType::iterator p = this->LinkLibraries.begin();
     p != this->LinkLibraries.end(); ++p)
   {
   this->Makefile->ExpandVariablesInString(p->first, true, true);
   }
 }

 typedef std::vector< std::string > LinkLine;

 // The dependency map.
 DependencyMap dep_map;

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

//----------------------------------------------------------------------------
void cmTarget::InsertDependency( DependencyMap& depMap,
                                 const LibraryID& lib,
                                 const LibraryID& dep)
{
  depMap[lib].push_back(dep);
}

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
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
      // emitted directly from the current node, and not from a
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

//----------------------------------------------------------------------------
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

//----------------------------------------------------------------------------
void cmTarget::SetProperty(const char* prop, const char* value)
{
  if (!prop)
    {
    return;
    }

  if(strcmp(prop,"INCLUDE_DIRECTORIES") == 0)
    {
    cmListFileBacktrace lfbt;
    this->Makefile->GetBacktrace(lfbt);
    cmGeneratorExpression ge(lfbt);
    deleteAndClear(this->Internal->IncludeDirectoriesEntries);
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(value);
    this->Internal->IncludeDirectoriesEntries.push_back(
                          new cmTargetInternals::IncludeDirectoriesEntry(cge));
    return;
    }
  this->Properties.SetProperty(prop, value, cmProperty::TARGET);
  this->MaybeInvalidatePropertyCache(prop);
}

//----------------------------------------------------------------------------
void cmTarget::AppendProperty(const char* prop, const char* value,
                              bool asString)
{
  if (!prop)
    {
    return;
    }
  if(strcmp(prop,"INCLUDE_DIRECTORIES") == 0)
    {
    cmListFileBacktrace lfbt;
    this->Makefile->GetBacktrace(lfbt);
    cmGeneratorExpression ge(lfbt);
    this->Internal->IncludeDirectoriesEntries.push_back(
              new cmTargetInternals::IncludeDirectoriesEntry(ge.Parse(value)));
    return;
    }
  this->Properties.AppendProperty(prop, value, cmProperty::TARGET, asString);
  this->MaybeInvalidatePropertyCache(prop);
}

//----------------------------------------------------------------------------
void cmTarget::AppendBuildInterfaceIncludes()
{
  if(this->GetType() != cmTarget::SHARED_LIBRARY &&
     this->GetType() != cmTarget::STATIC_LIBRARY &&
     this->GetType() != cmTarget::MODULE_LIBRARY &&
     !this->IsExecutableWithExports())
    {
    return;
    }
  if (this->BuildInterfaceIncludesAppended)
    {
    return;
    }
  this->BuildInterfaceIncludesAppended = true;

  if (this->Makefile->IsOn("CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE"))
    {
    const char *binDir = this->Makefile->GetStartOutputDirectory();
    const char *srcDir = this->Makefile->GetStartDirectory();
    const std::string dirs = std::string(binDir ? binDir : "")
                            + std::string(binDir ? ";" : "")
                            + std::string(srcDir ? srcDir : "");
    if (!dirs.empty())
      {
      this->AppendProperty("INTERFACE_INCLUDE_DIRECTORIES",
                            ("$<BUILD_INTERFACE:" + dirs + ">").c_str());
      }
    }
}

//----------------------------------------------------------------------------
void cmTarget::AppendTllInclude(const cmValueWithOrigin &entry)
{
  this->Internal->LinkInterfaceIncludeDirectoriesEntries.push_back(entry);
}

//----------------------------------------------------------------------------
void cmTarget::InsertInclude(const cmValueWithOrigin &entry,
                     bool before)
{
  cmGeneratorExpression ge(entry.Backtrace);

  std::vector<cmTargetInternals::IncludeDirectoriesEntry*>::iterator position
                = before ? this->Internal->IncludeDirectoriesEntries.begin()
                         : this->Internal->IncludeDirectoriesEntries.end();

  this->Internal->IncludeDirectoriesEntries.insert(position,
      new cmTargetInternals::IncludeDirectoriesEntry(ge.Parse(entry.Value)));
}

//----------------------------------------------------------------------------
static void processIncludeDirectories(cmTarget *tgt,
      const std::vector<cmTargetInternals::IncludeDirectoriesEntry*> &entries,
      std::vector<std::string> &includes,
      std::set<std::string> &uniqueIncludes,
      cmGeneratorExpressionDAGChecker *dagChecker,
      const char *config, bool debugIncludes)
{
  cmMakefile *mf = tgt->GetMakefile();

  for (std::vector<cmTargetInternals::IncludeDirectoriesEntry*>::const_iterator
      it = entries.begin(), end = entries.end(); it != end; ++it)
    {
    bool testIsOff = true;
    bool cacheIncludes = false;
    std::vector<std::string> entryIncludes = (*it)->CachedIncludes;
    if(!entryIncludes.empty())
      {
      testIsOff = false;
      }
    else
      {
      cmSystemTools::ExpandListArgument((*it)->ge->Evaluate(mf,
                                                config,
                                                false,
                                                tgt,
                                                dagChecker),
                                      entryIncludes);
      if (mf->IsGeneratingBuildSystem()
          && !(*it)->ge->GetHadContextSensitiveCondition())
        {
        cacheIncludes = true;
        }
      }
    std::string usedIncludes;
    for(std::vector<std::string>::iterator
          li = entryIncludes.begin(); li != entryIncludes.end(); ++li)
      {
      cmTarget *dependentTarget =
                              mf->FindTargetToUse((*it)->TargetName.c_str());

      const bool fromImported = dependentTarget
                             && dependentTarget->IsImported();

      if (fromImported && !cmSystemTools::FileExists(li->c_str()))
        {
        cmOStringStream e;
        e << "Imported target \"" << (*it)->TargetName << "\" includes "
             "non-existent path\n  \"" << *li << "\"\nin its "
             "INTERFACE_INCLUDE_DIRECTORIES. Possible reasons include:\n"
             "* The path was deleted, renamed, or moved to another "
             "location.\n"
             "* An install or uninstall procedure did not complete "
             "successfully.\n"
             "* The installation package was faulty and references files it "
             "does not provide.\n";
        tgt->GetMakefile()->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
        return;
        }

      if (!cmSystemTools::FileIsFullPath(li->c_str()))
        {
        if (!(*it)->TargetName.empty())
          {
          cmOStringStream e;
          e << "Target \"" << (*it)->TargetName << "\" contains relative "
            "path in its INTERFACE_INCLUDE_DIRECTORIES:\n"
            "  \"" << *li << "\" ";
          tgt->GetMakefile()->IssueMessage(cmake::FATAL_ERROR,
                                           e.str().c_str());
          return;
          }
        }

      if (testIsOff && !cmSystemTools::IsOff(li->c_str()))
        {
        cmSystemTools::ConvertToUnixSlashes(*li);
        }
      std::string inc = *li;

      if(uniqueIncludes.insert(inc).second)
        {
        includes.push_back(inc);
        if (debugIncludes)
          {
          usedIncludes += " * " + inc + "\n";
          }
        }
      }
    if (cacheIncludes)
      {
      (*it)->CachedIncludes = entryIncludes;
      }
    if (!usedIncludes.empty())
      {
      mf->GetCMakeInstance()->IssueMessage(cmake::LOG,
                            std::string("Used includes for target ")
                            + tgt->GetName() + ":\n"
                            + usedIncludes, (*it)->ge->GetBacktrace());
      }
    }
}

//----------------------------------------------------------------------------
std::vector<std::string> cmTarget::GetIncludeDirectories(const char *config)
{
  std::vector<std::string> includes;
  std::set<std::string> uniqueIncludes;
  cmListFileBacktrace lfbt;

  cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                              this->GetName(),
                                              "INCLUDE_DIRECTORIES", 0, 0);

  this->AppendBuildInterfaceIncludes();

  std::vector<std::string> debugProperties;
  const char *debugProp =
              this->Makefile->GetDefinition("CMAKE_DEBUG_TARGET_PROPERTIES");
  if (debugProp)
    {
    cmSystemTools::ExpandListArgument(debugProp, debugProperties);
    }

  bool debugIncludes = !this->DebugIncludesDone
                    && std::find(debugProperties.begin(),
                                 debugProperties.end(),
                                 "INCLUDE_DIRECTORIES")
                        != debugProperties.end();

  if (this->Makefile->IsGeneratingBuildSystem())
    {
    this->DebugIncludesDone = true;
    }

  processIncludeDirectories(this,
                            this->Internal->IncludeDirectoriesEntries,
                            includes,
                            uniqueIncludes,
                            &dagChecker,
                            config,
                            debugIncludes);

  std::string configString = config ? config : "";
  if (!this->Internal->CacheLinkInterfaceIncludeDirectoriesDone[configString])
    {
    for (std::vector<cmValueWithOrigin>::const_iterator
        it = this->Internal->LinkInterfaceIncludeDirectoriesEntries.begin(),
        end = this->Internal->LinkInterfaceIncludeDirectoriesEntries.end();
        it != end; ++it)
      {
      {
      cmGeneratorExpression ge(lfbt);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge =
                                                        ge.Parse(it->Value);
      std::string result = cge->Evaluate(this->Makefile, config,
                                        false, this, 0, 0);
      if (!cmGeneratorExpression::IsValidTargetName(result.c_str())
          || !this->Makefile->FindTargetToUse(result.c_str()))
        {
        continue;
        }
      }
      cmGeneratorExpression ge(it->Backtrace);
      cmsys::auto_ptr<cmCompiledGeneratorExpression> cge = ge.Parse(
          "$<TARGET_PROPERTY:" +
                              it->Value + ",INTERFACE_INCLUDE_DIRECTORIES>");

      this->Internal->CachedLinkInterfaceIncludeDirectoriesEntries.push_back(
                        new cmTargetInternals::IncludeDirectoriesEntry(cge,
                                                              it->Value));
      }
    }

  processIncludeDirectories(this,
              this->Internal->CachedLinkInterfaceIncludeDirectoriesEntries,
                            includes,
                            uniqueIncludes,
                            &dagChecker,
                            config,
                            debugIncludes);

  if (!this->Makefile->IsGeneratingBuildSystem())
    {
    deleteAndClear(
              this->Internal->CachedLinkInterfaceIncludeDirectoriesEntries);
    }
  else
    {
    this->Internal->CacheLinkInterfaceIncludeDirectoriesDone[configString]
                                                                      = true;
    }

  return includes;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetCompileDefinitions(const char *config)
{
  const char *configProp = 0;
  if (config)
    {
    std::string configPropName;
    configPropName = "COMPILE_DEFINITIONS_" + cmSystemTools::UpperCase(config);
    configProp = this->GetProperty(configPropName.c_str());
    }

  const char *noconfigProp = this->GetProperty("COMPILE_DEFINITIONS");
  cmListFileBacktrace lfbt;
  cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                            this->GetName(),
                                            "COMPILE_DEFINITIONS", 0, 0);

  std::string defsString = (noconfigProp ? noconfigProp : "");
  if (configProp && noconfigProp)
    {
    defsString += ";";
    }
  defsString += (configProp ? configProp : "");

  cmGeneratorExpression ge(lfbt);
  std::string result = ge.Parse(defsString.c_str())->Evaluate(this->Makefile,
                                config,
                                false,
                                this,
                                &dagChecker);

  std::vector<std::string> libs;
  this->GetDirectLinkLibraries(config, libs, this);

  if (libs.empty())
    {
    return result;
    }

  std::string sep;
  std::string depString;
  for (std::vector<std::string>::const_iterator it = libs.begin();
      it != libs.end(); ++it)
    {
    if ((cmGeneratorExpression::IsValidTargetName(it->c_str())
          || cmGeneratorExpression::Find(it->c_str()) != std::string::npos)
        && this->Makefile->FindTargetToUse(it->c_str()))
      {
      depString += sep + "$<TARGET_PROPERTY:"
                + *it + ",INTERFACE_COMPILE_DEFINITIONS>";
      sep = ";";
      }
    }

  std::string configString = config ? config : "";
  if (!this->Internal->CacheLinkInterfaceCompileDefinitionsDone[configString])
    {
    cmGeneratorExpression ge2(lfbt);
    cmsys::auto_ptr<cmCompiledGeneratorExpression> cge2 =
                                                        ge2.Parse(depString);
    this->Internal->CachedLinkInterfaceCompileDefinitions[configString] =
                                                cge2->Evaluate(this->Makefile,
                                                               config,
                                                               false,
                                                               this,
                                                               &dagChecker);
    }
  if (!this->Internal->CachedLinkInterfaceCompileDefinitions[configString]
                                                                    .empty())
    {
    result += (result.empty() ? "" : ";")
        + this->Internal->CachedLinkInterfaceCompileDefinitions[configString];
    }

  if (!this->Makefile->IsGeneratingBuildSystem())
    {
    this->Internal->CachedLinkInterfaceCompileDefinitions[configString] = "";
    }
  else
    {
    this->Internal->CacheLinkInterfaceCompileDefinitionsDone[configString]
                                                                      = true;
    }

  return result;
}

//----------------------------------------------------------------------------
void cmTarget::MaybeInvalidatePropertyCache(const char* prop)
{
  // Wipe out maps caching information affected by this property.
  if(this->IsImported() && strncmp(prop, "IMPORTED", 8) == 0)
    {
    this->Internal->ImportInfoMap.clear();
    }
  if(!this->IsImported() && strncmp(prop, "LINK_INTERFACE_", 15) == 0)
    {
    this->ClearLinkMaps();
    }
}

//----------------------------------------------------------------------------
static void cmTargetCheckLINK_INTERFACE_LIBRARIES(
  const char* prop, const char* value, cmMakefile* context, bool imported
  )
{
  // Look for link-type keywords in the value.
  static cmsys::RegularExpression
    keys("(^|;)(debug|optimized|general)(;|$)");
  if(!keys.find(value))
    {
    return;
    }

  // Support imported and non-imported versions of the property.
  const char* base = (imported?
                      "IMPORTED_LINK_INTERFACE_LIBRARIES" :
                      "LINK_INTERFACE_LIBRARIES");

  // Report an error.
  cmOStringStream e;
  e << "Property " << prop << " may not contain link-type keyword \""
    << keys.match(2) << "\".  "
    << "The " << base << " property has a per-configuration "
    << "version called " << base << "_<CONFIG> which may be "
    << "used to specify per-configuration rules.";
  if(!imported)
    {
    e << "  "
      << "Alternatively, an IMPORTED library may be created, configured "
      << "with a per-configuration location, and then named in the "
      << "property value.  "
      << "See the add_library command's IMPORTED mode for details."
      << "\n"
      << "If you have a list of libraries that already contains the "
      << "keyword, use the target_link_libraries command with its "
      << "LINK_INTERFACE_LIBRARIES mode to set the property.  "
      << "The command automatically recognizes link-type keywords and sets "
      << "the LINK_INTERFACE_LIBRARIES and LINK_INTERFACE_LIBRARIES_DEBUG "
      << "properties accordingly.";
    }
  context->IssueMessage(cmake::FATAL_ERROR, e.str());
}

//----------------------------------------------------------------------------
void cmTarget::CheckProperty(const char* prop, cmMakefile* context)
{
  // Certain properties need checking.
  if(strncmp(prop, "LINK_INTERFACE_LIBRARIES", 24) == 0)
    {
    if(const char* value = this->GetProperty(prop))
      {
      cmTargetCheckLINK_INTERFACE_LIBRARIES(prop, value, context, false);
      }
    }
  if(strncmp(prop, "IMPORTED_LINK_INTERFACE_LIBRARIES", 33) == 0)
    {
    if(const char* value = this->GetProperty(prop))
      {
      cmTargetCheckLINK_INTERFACE_LIBRARIES(prop, value, context, true);
      }
    }
}

//----------------------------------------------------------------------------
void cmTarget::MarkAsImported()
{
  this->IsImportedTarget = true;
}

//----------------------------------------------------------------------------
bool cmTarget::HaveWellDefinedOutputFiles()
{
  return
    this->GetType() == cmTarget::STATIC_LIBRARY ||
    this->GetType() == cmTarget::SHARED_LIBRARY ||
    this->GetType() == cmTarget::MODULE_LIBRARY ||
    this->GetType() == cmTarget::EXECUTABLE;
}

//----------------------------------------------------------------------------
cmTarget::OutputInfo const* cmTarget::GetOutputInfo(const char* config)
{
  // There is no output information for imported targets.
  if(this->IsImported())
    {
    return 0;
    }

  // Only libraries and executables have well-defined output files.
  if(!this->HaveWellDefinedOutputFiles())
    {
    std::string msg = "cmTarget::GetOutputInfo called for ";
    msg += this->GetName();
    msg += " which has type ";
    msg += cmTarget::GetTargetTypeName(this->GetType());
    this->GetMakefile()->IssueMessage(cmake::INTERNAL_ERROR, msg);
    abort();
    return 0;
    }

  // Lookup/compute/cache the output information for this configuration.
  std::string config_upper;
  if(config && *config)
    {
    config_upper = cmSystemTools::UpperCase(config);
    }
  typedef cmTargetInternals::OutputInfoMapType OutputInfoMapType;
  OutputInfoMapType::const_iterator i =
    this->Internal->OutputInfoMap.find(config_upper);
  if(i == this->Internal->OutputInfoMap.end())
    {
    OutputInfo info;
    this->ComputeOutputDir(config, false, info.OutDir);
    this->ComputeOutputDir(config, true, info.ImpDir);
    if(!this->ComputePDBOutputDir(config, info.PdbDir))
      {
      info.PdbDir = info.OutDir;
      }
    OutputInfoMapType::value_type entry(config_upper, info);
    i = this->Internal->OutputInfoMap.insert(entry).first;
    }
  return &i->second;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetDirectory(const char* config, bool implib)
{
  if (this->IsImported())
    {
    // Return the directory from which the target is imported.
    return
      cmSystemTools::GetFilenamePath(
      this->ImportedGetFullPath(config, implib));
    }
  else if(OutputInfo const* info = this->GetOutputInfo(config))
    {
    // Return the directory in which the target will be built.
    return implib? info->ImpDir : info->OutDir;
    }
  return "";
}

//----------------------------------------------------------------------------
std::string cmTarget::GetPDBDirectory(const char* config)
{
  if(OutputInfo const* info = this->GetOutputInfo(config))
    {
    // Return the directory in which the target will be built.
    return info->PdbDir;
    }
  return "";
}

//----------------------------------------------------------------------------
const char* cmTarget::GetLocation(const char* config)
{
  if (this->IsImported())
    {
    return this->ImportedGetLocation(config);
    }
  else
    {
    return this->NormalGetLocation(config);
    }
}

//----------------------------------------------------------------------------
const char* cmTarget::ImportedGetLocation(const char* config)
{
  this->Location = this->ImportedGetFullPath(config, false);
  return this->Location.c_str();
}

//----------------------------------------------------------------------------
const char* cmTarget::NormalGetLocation(const char* config)
{
  // Handle the configuration-specific case first.
  if(config)
    {
    this->Location = this->GetFullPath(config, false);
    return this->Location.c_str();
    }

  // Now handle the deprecated build-time configuration location.
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
  this->Location = this->BuildMacContentDirectory(this->Location, config);
  this->Location += this->GetFullName(config, false);
  return this->Location.c_str();
}

//----------------------------------------------------------------------------
void cmTarget::GetTargetVersion(int& major, int& minor)
{
  int patch;
  this->GetTargetVersion(false, major, minor, patch);
}

//----------------------------------------------------------------------------
void cmTarget::GetTargetVersion(bool soversion,
                                int& major, int& minor, int& patch)
{
  // Set the default values.
  major = 0;
  minor = 0;
  patch = 0;

  // Look for a VERSION or SOVERSION property.
  const char* prop = soversion? "SOVERSION" : "VERSION";
  if(const char* version = this->GetProperty(prop))
    {
    // Try to parse the version number and store the results that were
    // successfully parsed.
    int parsed_major;
    int parsed_minor;
    int parsed_patch;
    switch(sscanf(version, "%d.%d.%d",
                  &parsed_major, &parsed_minor, &parsed_patch))
      {
      case 3: patch = parsed_patch; // no break!
      case 2: minor = parsed_minor; // no break!
      case 1: major = parsed_major; // no break!
      default: break;
      }
    }
}

//----------------------------------------------------------------------------
const char* cmTarget::GetFeature(const char* feature, const char* config)
{
  if(config && *config)
    {
    std::string featureConfig = feature;
    featureConfig += "_";
    featureConfig += cmSystemTools::UpperCase(config);
    if(const char* value = this->GetProperty(featureConfig.c_str()))
      {
      return value;
      }
    }
  if(const char* value = this->GetProperty(feature))
    {
    return value;
    }
  return this->Makefile->GetFeature(feature, config);
}

//----------------------------------------------------------------------------
const char *cmTarget::GetProperty(const char* prop)
{
  return this->GetProperty(prop, cmProperty::TARGET);
}

//----------------------------------------------------------------------------
const char *cmTarget::GetProperty(const char* prop,
                                  cmProperty::ScopeType scope)
{
  if(!prop)
    {
    return 0;
    }

  // Watch for special "computed" properties that are dependent on
  // other properties or variables.  Always recompute them.
  if(this->GetType() == cmTarget::EXECUTABLE ||
     this->GetType() == cmTarget::STATIC_LIBRARY ||
     this->GetType() == cmTarget::SHARED_LIBRARY ||
     this->GetType() == cmTarget::MODULE_LIBRARY ||
     this->GetType() == cmTarget::UNKNOWN_LIBRARY)
    {
    if(strcmp(prop,"LOCATION") == 0)
      {
      // Set the LOCATION property of the target.
      //
      // For an imported target this is the location of an arbitrary
      // available configuration.
      //
      // For a non-imported target this is deprecated because it
      // cannot take into account the per-configuration name of the
      // target because the configuration type may not be known at
      // CMake time.
      this->SetProperty("LOCATION", this->GetLocation(0));
      }

    // Support "LOCATION_<CONFIG>".
    if(strncmp(prop, "LOCATION_", 9) == 0)
      {
      std::string configName = prop+9;
      this->SetProperty(prop, this->GetLocation(configName.c_str()));
      }
    else
      {
      // Support "<CONFIG>_LOCATION" for compatiblity.
      int len = static_cast<int>(strlen(prop));
      if(len > 9 && strcmp(prop+len-9, "_LOCATION") == 0)
        {
        std::string configName(prop, len-9);
        if(configName != "IMPORTED")
          {
          this->SetProperty(prop, this->GetLocation(configName.c_str()));
          }
        }
      }
    }
  if(strcmp(prop,"INCLUDE_DIRECTORIES") == 0)
    {
    static std::string output;
    output = "";
    std::string sep;
    typedef cmTargetInternals::IncludeDirectoriesEntry
                                IncludeDirectoriesEntry;
    for (std::vector<IncludeDirectoriesEntry*>::const_iterator
        it = this->Internal->IncludeDirectoriesEntries.begin(),
        end = this->Internal->IncludeDirectoriesEntries.end();
        it != end; ++it)
      {
      output += sep;
      output += (*it)->ge->GetInput();
      sep = ";";
      }
    return output.c_str();
    }

  if (strcmp(prop,"IMPORTED") == 0)
    {
    return this->IsImported()?"TRUE":"FALSE";
    }

  if(!strcmp(prop,"SOURCES"))
    {
    cmOStringStream ss;
    const char* sep = "";
    for(std::vector<cmSourceFile*>::const_iterator
          i = this->SourceFiles.begin();
        i != this->SourceFiles.end(); ++i)
      {
      // Separate from the previous list entries.
      ss << sep;
      sep = ";";

      // Construct what is known about this source file location.
      cmSourceFileLocation const& location = (*i)->GetLocation();
      std::string sname = location.GetDirectory();
      if(!sname.empty())
        {
        sname += "/";
        }
      sname += location.GetName();

      // Append this list entry.
      ss << sname;
      }
    this->SetProperty("SOURCES", ss.str().c_str());
    }

  // the type property returns what type the target is
  if (!strcmp(prop,"TYPE"))
    {
    return cmTarget::GetTargetTypeName(this->GetType());
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

//----------------------------------------------------------------------------
bool cmTarget::GetPropertyAsBool(const char* prop)
{
  return cmSystemTools::IsOn(this->GetProperty(prop));
}

//----------------------------------------------------------------------------
class cmTargetCollectLinkLanguages
{
public:
  cmTargetCollectLinkLanguages(cmTarget* target, const char* config,
                               std::set<cmStdString>& languages,
                               cmTarget* head):
    Config(config), Languages(languages), HeadTarget(head)
  { this->Visited.insert(target); }

  void Visit(cmTarget* target)
    {
    if(!target || !this->Visited.insert(target).second)
      {
      return;
      }

    cmTarget::LinkInterface const* iface =
      target->GetLinkInterface(this->Config, this->HeadTarget);
    if(!iface) { return; }

    for(std::vector<std::string>::const_iterator
          li = iface->Languages.begin(); li != iface->Languages.end(); ++li)
      {
      this->Languages.insert(*li);
      }

    cmMakefile* mf = target->GetMakefile();
    for(std::vector<std::string>::const_iterator
          li = iface->Libraries.begin(); li != iface->Libraries.end(); ++li)
      {
      this->Visit(mf->FindTargetToUse(li->c_str()));
      }
    }
private:
  const char* Config;
  std::set<cmStdString>& Languages;
  cmTarget* HeadTarget;
  std::set<cmTarget*> Visited;
};

//----------------------------------------------------------------------------
const char* cmTarget::GetLinkerLanguage(const char* config, cmTarget *head)
{
  cmTarget *headTarget = head ? head : this;
  const char* lang = this->GetLinkClosure(config, headTarget)
                                                    ->LinkerLanguage.c_str();
  return *lang? lang : 0;
}

//----------------------------------------------------------------------------
cmTarget::LinkClosure const* cmTarget::GetLinkClosure(const char* config,
                                                      cmTarget *head)
{
  TargetConfigPair key(head, cmSystemTools::UpperCase(config ? config : ""));
  cmTargetInternals::LinkClosureMapType::iterator
    i = this->Internal->LinkClosureMap.find(key);
  if(i == this->Internal->LinkClosureMap.end())
    {
    LinkClosure lc;
    this->ComputeLinkClosure(config, lc, head);
    cmTargetInternals::LinkClosureMapType::value_type entry(key, lc);
    i = this->Internal->LinkClosureMap.insert(entry).first;
    }
  return &i->second;
}

//----------------------------------------------------------------------------
class cmTargetSelectLinker
{
  int Preference;
  cmTarget* Target;
  cmMakefile* Makefile;
  cmGlobalGenerator* GG;
  std::set<cmStdString> Preferred;
public:
  cmTargetSelectLinker(cmTarget* target): Preference(0), Target(target)
    {
    this->Makefile = this->Target->GetMakefile();
    this->GG = this->Makefile->GetLocalGenerator()->GetGlobalGenerator();
    }
  void Consider(const char* lang)
    {
    int preference = this->GG->GetLinkerPreference(lang);
    if(preference > this->Preference)
      {
      this->Preference = preference;
      this->Preferred.clear();
      }
    if(preference == this->Preference)
      {
      this->Preferred.insert(lang);
      }
    }
  std::string Choose()
    {
    if(this->Preferred.empty())
      {
      return "";
      }
    else if(this->Preferred.size() > 1)
      {
      cmOStringStream e;
      e << "Target " << this->Target->GetName()
        << " contains multiple languages with the highest linker preference"
        << " (" << this->Preference << "):\n";
      for(std::set<cmStdString>::const_iterator
            li = this->Preferred.begin(); li != this->Preferred.end(); ++li)
        {
        e << "  " << *li << "\n";
        }
      e << "Set the LINKER_LANGUAGE property for this target.";
      cmake* cm = this->Makefile->GetCMakeInstance();
      cm->IssueMessage(cmake::FATAL_ERROR, e.str(),
                       this->Target->GetBacktrace());
      }
    return *this->Preferred.begin();
    }
};

//----------------------------------------------------------------------------
void cmTarget::ComputeLinkClosure(const char* config, LinkClosure& lc,
                                  cmTarget *head)
{
  // Get languages built in this target.
  std::set<cmStdString> languages;
  LinkImplementation const* impl = this->GetLinkImplementation(config, head);
  for(std::vector<std::string>::const_iterator li = impl->Languages.begin();
      li != impl->Languages.end(); ++li)
    {
    languages.insert(*li);
    }

  // Add interface languages from linked targets.
  cmTargetCollectLinkLanguages cll(this, config, languages, head);
  for(std::vector<std::string>::const_iterator li = impl->Libraries.begin();
      li != impl->Libraries.end(); ++li)
    {
    cll.Visit(this->Makefile->FindTargetToUse(li->c_str()));
    }

  // Store the transitive closure of languages.
  for(std::set<cmStdString>::const_iterator li = languages.begin();
      li != languages.end(); ++li)
    {
    lc.Languages.push_back(*li);
    }

  // Choose the language whose linker should be used.
  if(this->GetProperty("HAS_CXX"))
    {
    lc.LinkerLanguage = "CXX";
    }
  else if(const char* linkerLang = this->GetProperty("LINKER_LANGUAGE"))
    {
    lc.LinkerLanguage = linkerLang;
    }
  else
    {
    // Find the language with the highest preference value.
    cmTargetSelectLinker tsl(this);

    // First select from the languages compiled directly in this target.
    for(std::vector<std::string>::const_iterator li = impl->Languages.begin();
        li != impl->Languages.end(); ++li)
      {
      tsl.Consider(li->c_str());
      }

    // Now consider languages that propagate from linked targets.
    for(std::set<cmStdString>::const_iterator sit = languages.begin();
        sit != languages.end(); ++sit)
      {
      std::string propagates = "CMAKE_"+*sit+"_LINKER_PREFERENCE_PROPAGATES";
      if(this->Makefile->IsOn(propagates.c_str()))
        {
        tsl.Consider(sit->c_str());
        }
      }

    lc.LinkerLanguage = tsl.Choose();
    }
}

//----------------------------------------------------------------------------
const char* cmTarget::GetSuffixVariableInternal(bool implib)
{
  switch(this->GetType())
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
    default:
      break;
    }
  return "";
}


//----------------------------------------------------------------------------
const char* cmTarget::GetPrefixVariableInternal(bool implib)
{
  switch(this->GetType())
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
    default:
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
  this->GetFullNameInternal(config, false, prefix, base, suffix);

  std::vector<std::string> props;
  std::string configUpper =
    cmSystemTools::UpperCase(config? config : "");
  if(!configUpper.empty())
    {
    // PDB_NAME_<CONFIG>
    props.push_back("PDB_NAME_" + configUpper);
    }

  // PDB_NAME
  props.push_back("PDB_NAME");

  for(std::vector<std::string>::const_iterator i = props.begin();
      i != props.end(); ++i)
    {
    if(const char* outName = this->GetProperty(i->c_str()))
      {
      base = outName;
      break;
      }
    }
  return prefix+base+".pdb";
}

//----------------------------------------------------------------------------
bool cmTarget::HasSOName(const char* config)
{
  // soname is supported only for shared libraries and modules,
  // and then only when the platform supports an soname flag.
  return ((this->GetType() == cmTarget::SHARED_LIBRARY ||
           this->GetType() == cmTarget::MODULE_LIBRARY) &&
          !this->GetPropertyAsBool("NO_SONAME") &&
          this->Makefile->GetSONameFlag(this->GetLinkerLanguage(config,
                                                                this)));
}

//----------------------------------------------------------------------------
std::string cmTarget::GetSOName(const char* config)
{
  if(this->IsImported())
    {
    // Lookup the imported soname.
    if(cmTarget::ImportInfo const* info = this->GetImportInfo(config, this))
      {
      if(info->NoSOName)
        {
        // The imported library has no builtin soname so the name
        // searched at runtime will be just the filename.
        return cmSystemTools::GetFilenameName(info->Location);
        }
      else
        {
        // Use the soname given if any.
        return info->SOName;
        }
      }
    else
      {
      return "";
      }
    }
  else
    {
    // Compute the soname that will be built.
    std::string name;
    std::string soName;
    std::string realName;
    std::string impName;
    std::string pdbName;
    this->GetLibraryNames(name, soName, realName, impName, pdbName, config);
    return soName;
    }
}

//----------------------------------------------------------------------------
bool cmTarget::IsImportedSharedLibWithoutSOName(const char* config)
{
  if(this->IsImported() && this->GetType() == cmTarget::SHARED_LIBRARY)
    {
    if(cmTarget::ImportInfo const* info = this->GetImportInfo(config, this))
      {
      return info->NoSOName;
      }
    }
  return false;
}

//----------------------------------------------------------------------------
std::string cmTarget::NormalGetRealName(const char* config)
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if(this->IsImported())
    {
    std::string msg =  "NormalGetRealName called on imported target: ";
    msg += this->GetName();
    this->GetMakefile()->
      IssueMessage(cmake::INTERNAL_ERROR,
                   msg.c_str());
    }

  if(this->GetType() == cmTarget::EXECUTABLE)
    {
    // Compute the real name that will be built.
    std::string name;
    std::string realName;
    std::string impName;
    std::string pdbName;
    this->GetExecutableNames(name, realName, impName, pdbName, config);
    return realName;
    }
  else
    {
    // Compute the real name that will be built.
    std::string name;
    std::string soName;
    std::string realName;
    std::string impName;
    std::string pdbName;
    this->GetLibraryNames(name, soName, realName, impName, pdbName, config);
    return realName;
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullName(const char* config, bool implib)
{
  if(this->IsImported())
    {
    return this->GetFullNameImported(config, implib);
    }
  else
    {
    return this->GetFullNameInternal(config, implib);
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullNameImported(const char* config, bool implib)
{
  return cmSystemTools::GetFilenameName(
    this->ImportedGetFullPath(config, implib));
}

//----------------------------------------------------------------------------
void cmTarget::GetFullNameComponents(std::string& prefix, std::string& base,
                                     std::string& suffix, const char* config,
                                     bool implib)
{
  this->GetFullNameInternal(config, implib, prefix, base, suffix);
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullPath(const char* config, bool implib,
                                  bool realname)
{
  if(this->IsImported())
    {
    return this->ImportedGetFullPath(config, implib);
    }
  else
    {
    return this->NormalGetFullPath(config, implib, realname);
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::NormalGetFullPath(const char* config, bool implib,
                                        bool realname)
{
  std::string fpath = this->GetMacContentDirectory(config, implib);

  // Add the full name of the target.
  if(implib)
    {
    fpath += this->GetFullName(config, true);
    }
  else if(realname)
    {
    fpath += this->NormalGetRealName(config);
    }
  else
    {
    fpath += this->GetFullName(config, false);
    }
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmTarget::ImportedGetFullPath(const char* config, bool implib)
{
  std::string result;
  if(cmTarget::ImportInfo const* info = this->GetImportInfo(config, this))
    {
    result = implib? info->ImportLibrary : info->Location;
    }
  if(result.empty())
    {
    result = this->GetName();
    result += "-NOTFOUND";
    }
  return result;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFullNameInternal(const char* config, bool implib)
{
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, implib, prefix, base, suffix);
  return prefix+base+suffix;
}

//----------------------------------------------------------------------------
void cmTarget::GetFullNameInternal(const char* config,
                                   bool implib,
                                   std::string& outPrefix,
                                   std::string& outBase,
                                   std::string& outSuffix)
{
  // Use just the target name for non-main target types.
  if(this->GetType() != cmTarget::STATIC_LIBRARY &&
     this->GetType() != cmTarget::SHARED_LIBRARY &&
     this->GetType() != cmTarget::MODULE_LIBRARY &&
     this->GetType() != cmTarget::EXECUTABLE)
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
  if(this->GetType() != cmTarget::SHARED_LIBRARY &&
     this->GetType() != cmTarget::MODULE_LIBRARY &&
     this->GetType() != cmTarget::EXECUTABLE)
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
    // Mac application bundles and frameworks have no postfix.
    if(configPostfix &&
       (this->IsAppBundleOnApple() || this->IsFrameworkOnApple()))
      {
      configPostfix = 0;
      }
    }
  const char* prefixVar = this->GetPrefixVariableInternal(implib);
  const char* suffixVar = this->GetSuffixVariableInternal(implib);

  // Check for language-specific default prefix and suffix.
  if(const char* ll = this->GetLinkerLanguage(config, this))
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

  // frameworks do not have a prefix or a suffix
  if(this->IsFrameworkOnApple())
    {
    targetPrefix = 0;
    targetSuffix = 0;
    }

  // Begin the final name with the prefix.
  outPrefix = targetPrefix?targetPrefix:"";

  // Append the target name or property-specified name.
  outBase += this->GetOutputName(config, implib);

  // Append the per-configuration postfix.
  outBase += configPostfix?configPostfix:"";

  // Name shared libraries with their version number on some platforms.
  if(const char* soversion = this->GetProperty("SOVERSION"))
    {
    if(this->GetType() == cmTarget::SHARED_LIBRARY && !implib &&
       this->Makefile->IsOn("CMAKE_SHARED_LIBRARY_NAME_WITH_VERSION"))
      {
      outBase += "-";
      outBase += soversion;
      }
    }

  // Append the suffix.
  outSuffix = targetSuffix?targetSuffix:"";
}

//----------------------------------------------------------------------------
void cmTarget::GetLibraryNames(std::string& name,
                               std::string& soName,
                               std::string& realName,
                               std::string& impName,
                               std::string& pdbName,
                               const char* config)
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if(this->IsImported())
    {
    std::string msg =  "GetLibraryNames called on imported target: ";
    msg += this->GetName();
    this->Makefile->IssueMessage(cmake::INTERNAL_ERROR,
                                 msg.c_str());
    return;
    }

  // Check for library version properties.
  const char* version = this->GetProperty("VERSION");
  const char* soversion = this->GetProperty("SOVERSION");
  if(!this->HasSOName(config) ||
     this->IsFrameworkOnApple())
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
  if(!version && soversion)
    {
    // Use the soversion as the library version.
    version = soversion;
    }

  // Get the components of the library name.
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, false, prefix, base, suffix);

  // The library name.
  name = prefix+base+suffix;

  // The library's soname.
  this->ComputeVersionedName(soName, prefix, base, suffix,
                             name, soversion);

  // The library's real name on disk.
  this->ComputeVersionedName(realName, prefix, base, suffix,
                             name, version);

  // The import library name.
  if(this->GetType() == cmTarget::SHARED_LIBRARY ||
     this->GetType() == cmTarget::MODULE_LIBRARY)
    {
    impName = this->GetFullNameInternal(config, true);
    }
  else
    {
    impName = "";
    }

  // The program database file name.
  pdbName = this->GetPDBName(config);
}

//----------------------------------------------------------------------------
void cmTarget::ComputeVersionedName(std::string& vName,
                                    std::string const& prefix,
                                    std::string const& base,
                                    std::string const& suffix,
                                    std::string const& name,
                                    const char* version)
{
  vName = this->IsApple? (prefix+base) : name;
  if(version)
    {
    vName += ".";
    vName += version;
    }
  vName += this->IsApple? suffix : std::string();
}

//----------------------------------------------------------------------------
void cmTarget::GetExecutableNames(std::string& name,
                                  std::string& realName,
                                  std::string& impName,
                                  std::string& pdbName,
                                  const char* config)
{
  // This should not be called for imported targets.
  // TODO: Split cmTarget into a class hierarchy to get compile-time
  // enforcement of the limited imported target API.
  if(this->IsImported())
    {
    std::string msg =
      "GetExecutableNames called on imported target: ";
    msg += this->GetName();
    this->GetMakefile()->IssueMessage(cmake::INTERNAL_ERROR, msg.c_str());
    }

  // This versioning is supported only for executables and then only
  // when the platform supports symbolic links.
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* version = 0;
#else
  // Check for executable version properties.
  const char* version = this->GetProperty("VERSION");
  if(this->GetType() != cmTarget::EXECUTABLE || this->Makefile->IsOn("XCODE"))
    {
    version = 0;
    }
#endif

  // Get the components of the executable name.
  std::string prefix;
  std::string base;
  std::string suffix;
  this->GetFullNameInternal(config, false, prefix, base, suffix);

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
  impName = this->GetFullNameInternal(config, true);

  // The program database file name.
  pdbName = this->GetPDBName(config);
}

//----------------------------------------------------------------------------
bool cmTarget::HasImplibGNUtoMS()
{
  return this->HasImportLibrary() && this->GetPropertyAsBool("GNUtoMS");
}

//----------------------------------------------------------------------------
bool cmTarget::GetImplibGNUtoMS(std::string const& gnuName,
                                std::string& out, const char* newExt)
{
  if(this->HasImplibGNUtoMS() &&
     gnuName.size() > 6 && gnuName.substr(gnuName.size()-6) == ".dll.a")
    {
    out = gnuName.substr(0, gnuName.size()-6);
    out += newExt? newExt : ".lib";
    return true;
    }
  return false;
}

//----------------------------------------------------------------------------
void cmTarget::GenerateTargetManifest(const char* config)
{
  cmMakefile* mf = this->Makefile;
  cmLocalGenerator* lg = mf->GetLocalGenerator();
  cmGlobalGenerator* gg = lg->GetGlobalGenerator();

  // Get the names.
  std::string name;
  std::string soName;
  std::string realName;
  std::string impName;
  std::string pdbName;
  if(this->GetType() == cmTarget::EXECUTABLE)
    {
    this->GetExecutableNames(name, realName, impName, pdbName, config);
    }
  else if(this->GetType() == cmTarget::STATIC_LIBRARY ||
          this->GetType() == cmTarget::SHARED_LIBRARY ||
          this->GetType() == cmTarget::MODULE_LIBRARY)
    {
    this->GetLibraryNames(name, soName, realName, impName, pdbName, config);
    }
  else
    {
    return;
    }

  // Get the directory.
  std::string dir = this->GetDirectory(config, false);

  // Add each name.
  std::string f;
  if(!name.empty())
    {
    f = dir;
    f += "/";
    f += name;
    gg->AddToManifest(config? config:"", f);
    }
  if(!soName.empty())
    {
    f = dir;
    f += "/";
    f += soName;
    gg->AddToManifest(config? config:"", f);
    }
  if(!realName.empty())
    {
    f = dir;
    f += "/";
    f += realName;
    gg->AddToManifest(config? config:"", f);
    }
  if(!pdbName.empty())
    {
    f = this->GetPDBDirectory(config);
    f += "/";
    f += pdbName;
    gg->AddToManifest(config? config:"", f);
    }
  if(!impName.empty())
    {
    f = this->GetDirectory(config, true);
    f += "/";
    f += impName;
    gg->AddToManifest(config? config:"", f);
    }
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
bool cmTarget::HaveBuildTreeRPATH(const char *config)
{
  if (this->GetPropertyAsBool("SKIP_BUILD_RPATH"))
    {
    return false;
    }
  std::vector<std::string> libs;
  this->GetDirectLinkLibraries(config, libs, this);
  return !libs.empty();
}

//----------------------------------------------------------------------------
bool cmTarget::HaveInstallTreeRPATH()
{
  const char* install_rpath = this->GetProperty("INSTALL_RPATH");
  return (install_rpath && *install_rpath) &&
          !this->Makefile->IsOn("CMAKE_SKIP_INSTALL_RPATH");
}

//----------------------------------------------------------------------------
bool cmTarget::NeedRelinkBeforeInstall(const char* config)
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

  // If chrpath is going to be used no relinking is needed.
  if(this->IsChrpathUsed(config))
    {
    return false;
    }

  // Check for rpath support on this platform.
  if(const char* ll = this->GetLinkerLanguage(config, this))
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
  return this->HaveBuildTreeRPATH(config) || this->HaveInstallTreeRPATH();
}

//----------------------------------------------------------------------------
std::string cmTarget::GetInstallNameDirForBuildTree(const char* config,
                                                    bool for_xcode)
{
  // If building directly for installation then the build tree install_name
  // is the same as the install tree.
  if(this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"))
    {
    return GetInstallNameDirForInstallTree(config, for_xcode);
    }

  // Use the build tree directory for the target.
  if(this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME") &&
     !this->Makefile->IsOn("CMAKE_SKIP_RPATH") &&
     !this->GetPropertyAsBool("SKIP_BUILD_RPATH"))
    {
    std::string dir = this->GetDirectory(config);
    dir += "/";
    if(this->IsFrameworkOnApple() && !for_xcode)
      {
      dir += this->GetFrameworkDirectory(config);
      }
    return dir;
    }
  else
    {
    return "";
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::GetInstallNameDirForInstallTree(const char* config,
                                                      bool for_xcode)
{
  if(this->Makefile->IsOn("CMAKE_PLATFORM_HAS_INSTALLNAME"))
    {
    std::string dir;

    if(!this->Makefile->IsOn("CMAKE_SKIP_RPATH") &&
       !this->Makefile->IsOn("CMAKE_SKIP_INSTALL_RPATH"))
      {
      const char* install_name_dir = this->GetProperty("INSTALL_NAME_DIR");
      if(install_name_dir && *install_name_dir)
        {
        dir = install_name_dir;
        dir += "/";
        }
      }

    if(this->IsFrameworkOnApple() && !for_xcode)
      {
      dir += this->GetFrameworkDirectory(config);
      }

    return dir;
    }
  else
    {
    return "";
    }
}

//----------------------------------------------------------------------------
const char* cmTarget::GetOutputTargetType(bool implib)
{
  switch(this->GetType())
    {
    case cmTarget::SHARED_LIBRARY:
      if(this->DLLPlatform)
        {
        if(implib)
          {
          // A DLL import library is treated as an archive target.
          return "ARCHIVE";
          }
        else
          {
          // A DLL shared library is treated as a runtime target.
          return "RUNTIME";
          }
        }
      else
        {
        // For non-DLL platforms shared libraries are treated as
        // library targets.
        return "LIBRARY";
        }
    case cmTarget::STATIC_LIBRARY:
      // Static libraries are always treated as archive targets.
      return "ARCHIVE";
    case cmTarget::MODULE_LIBRARY:
      if(implib)
        {
        // Module libraries are always treated as library targets.
        return "ARCHIVE";
        }
      else
        {
        // Module import libraries are treated as archive targets.
        return "LIBRARY";
        }
    case cmTarget::EXECUTABLE:
      if(implib)
        {
        // Executable import libraries are treated as archive targets.
        return "ARCHIVE";
        }
      else
        {
        // Executables are always treated as runtime targets.
        return "RUNTIME";
        }
    default:
      break;
    }
  return "";
}

//----------------------------------------------------------------------------
bool cmTarget::ComputeOutputDir(const char* config,
                                bool implib, std::string& out)
{
  bool usesDefaultOutputDir = false;

  // Look for a target property defining the target output directory
  // based on the target type.
  std::string targetTypeName = this->GetOutputTargetType(implib);
  const char* propertyName = 0;
  std::string propertyNameStr = targetTypeName;
  if(!propertyNameStr.empty())
    {
    propertyNameStr += "_OUTPUT_DIRECTORY";
    propertyName = propertyNameStr.c_str();
    }

  // Check for a per-configuration output directory target property.
  std::string configUpper = cmSystemTools::UpperCase(config? config : "");
  const char* configProp = 0;
  std::string configPropStr = targetTypeName;
  if(!configPropStr.empty())
    {
    configPropStr += "_OUTPUT_DIRECTORY_";
    configPropStr += configUpper;
    configProp = configPropStr.c_str();
    }

  // Select an output directory.
  if(const char* config_outdir = this->GetProperty(configProp))
    {
    // Use the user-specified per-configuration output directory.
    out = config_outdir;

    // Skip per-configuration subdirectory.
    config = 0;
    }
  else if(const char* outdir = this->GetProperty(propertyName))
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
    usesDefaultOutputDir = true;
    out = ".";
    }

  // Convert the output path to a full path in case it is
  // specified as a relative path.  Treat a relative path as
  // relative to the current output directory for this makefile.
  out = (cmSystemTools::CollapseFullPath
         (out.c_str(), this->Makefile->GetStartOutputDirectory()));

  // The generator may add the configuration's subdirectory.
  if(config && *config)
    {
    const char *platforms = this->Makefile->GetDefinition(
      "CMAKE_XCODE_EFFECTIVE_PLATFORMS");
    std::string suffix =
      usesDefaultOutputDir && platforms ? "$(EFFECTIVE_PLATFORM_NAME)" : "";
    this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
      AppendDirectoryForConfig("/", config, suffix.c_str(), out);
    }

  return usesDefaultOutputDir;
}

//----------------------------------------------------------------------------
bool cmTarget::ComputePDBOutputDir(const char* config, std::string& out)
{
  // Look for a target property defining the target output directory
  // based on the target type.
  std::string targetTypeName = "PDB";
  const char* propertyName = 0;
  std::string propertyNameStr = targetTypeName;
  if(!propertyNameStr.empty())
    {
    propertyNameStr += "_OUTPUT_DIRECTORY";
    propertyName = propertyNameStr.c_str();
    }

  // Check for a per-configuration output directory target property.
  std::string configUpper = cmSystemTools::UpperCase(config? config : "");
  const char* configProp = 0;
  std::string configPropStr = targetTypeName;
  if(!configPropStr.empty())
    {
    configPropStr += "_OUTPUT_DIRECTORY_";
    configPropStr += configUpper;
    configProp = configPropStr.c_str();
    }

  // Select an output directory.
  if(const char* config_outdir = this->GetProperty(configProp))
    {
    // Use the user-specified per-configuration output directory.
    out = config_outdir;

    // Skip per-configuration subdirectory.
    config = 0;
    }
  else if(const char* outdir = this->GetProperty(propertyName))
    {
    // Use the user-specified output directory.
    out = outdir;
    }
  if(out.empty())
    {
    return false;
    }

  // Convert the output path to a full path in case it is
  // specified as a relative path.  Treat a relative path as
  // relative to the current output directory for this makefile.
  out = (cmSystemTools::CollapseFullPath
         (out.c_str(), this->Makefile->GetStartOutputDirectory()));

  // The generator may add the configuration's subdirectory.
  if(config && *config)
    {
    this->Makefile->GetLocalGenerator()->GetGlobalGenerator()->
      AppendDirectoryForConfig("/", config, "", out);
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmTarget::UsesDefaultOutputDir(const char* config, bool implib)
{
  std::string dir;
  return this->ComputeOutputDir(config, implib, dir);
}

//----------------------------------------------------------------------------
std::string cmTarget::GetOutputName(const char* config, bool implib)
{
  std::vector<std::string> props;
  std::string type = this->GetOutputTargetType(implib);
  std::string configUpper = cmSystemTools::UpperCase(config? config : "");
  if(!type.empty() && !configUpper.empty())
    {
    // <ARCHIVE|LIBRARY|RUNTIME>_OUTPUT_NAME_<CONFIG>
    props.push_back(type + "_OUTPUT_NAME_" + configUpper);
    }
  if(!type.empty())
    {
    // <ARCHIVE|LIBRARY|RUNTIME>_OUTPUT_NAME
    props.push_back(type + "_OUTPUT_NAME");
    }
  if(!configUpper.empty())
    {
    // OUTPUT_NAME_<CONFIG>
    props.push_back("OUTPUT_NAME_" + configUpper);
    // <CONFIG>_OUTPUT_NAME
    props.push_back(configUpper + "_OUTPUT_NAME");
    }
  // OUTPUT_NAME
  props.push_back("OUTPUT_NAME");

  for(std::vector<std::string>::const_iterator i = props.begin();
      i != props.end(); ++i)
    {
    if(const char* outName = this->GetProperty(i->c_str()))
      {
      return outName;
      }
    }
  return this->GetName();
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFrameworkVersion()
{
  if(const char* fversion = this->GetProperty("FRAMEWORK_VERSION"))
    {
    return fversion;
    }
  else if(const char* tversion = this->GetProperty("VERSION"))
    {
    return tversion;
    }
  else
    {
    return "A";
    }
}

//----------------------------------------------------------------------------
const char* cmTarget::GetExportMacro()
{
  // Define the symbol for targets that export symbols.
  if(this->GetType() == cmTarget::SHARED_LIBRARY ||
     this->GetType() == cmTarget::MODULE_LIBRARY ||
     this->IsExecutableWithExports())
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

//----------------------------------------------------------------------------
bool cmTarget::IsNullImpliedByLinkLibraries(const std::string &p)
{
  return this->LinkImplicitNullProperties.find(p)
      != this->LinkImplicitNullProperties.end();
}

//----------------------------------------------------------------------------
template<typename PropertyType>
PropertyType getTypedProperty(cmTarget *tgt, const char *prop,
                              PropertyType *);

//----------------------------------------------------------------------------
template<>
bool getTypedProperty<bool>(cmTarget *tgt, const char *prop, bool *)
{
  return tgt->GetPropertyAsBool(prop);
}

//----------------------------------------------------------------------------
template<>
const char *getTypedProperty<const char *>(cmTarget *tgt, const char *prop,
                                          const char **)
{
  return tgt->GetProperty(prop);
}

//----------------------------------------------------------------------------
template<typename PropertyType>
bool consistentProperty(PropertyType lhs, PropertyType rhs);

//----------------------------------------------------------------------------
template<>
bool consistentProperty(bool lhs, bool rhs)
{
  return lhs == rhs;
}

//----------------------------------------------------------------------------
template<>
bool consistentProperty(const char *lhs, const char *rhs)
{
  if (!lhs && !rhs)
    return true;
  if (!lhs || !rhs)
    return false;
  return strcmp(lhs, rhs) == 0;
}

//----------------------------------------------------------------------------
template<typename PropertyType>
PropertyType checkInterfacePropertyCompatibility(cmTarget *tgt,
                                          const std::string &p,
                                          const char *config,
                                          const char *defaultValue,
                                          PropertyType *)
{
  PropertyType propContent = getTypedProperty<PropertyType>(tgt, p.c_str(),
                                                            0);
  const bool explicitlySet = tgt->GetProperties()
                                  .find(p.c_str())
                                  != tgt->GetProperties().end();
  const bool impliedByUse =
          tgt->IsNullImpliedByLinkLibraries(p);
  assert((impliedByUse ^ explicitlySet)
      || (!impliedByUse && !explicitlySet));

  cmComputeLinkInformation *info = tgt->GetLinkInformation(config);
  if(!info)
    {
    return propContent;
    }
  const cmComputeLinkInformation::ItemVector &deps = info->GetItems();
  bool propInitialized = explicitlySet;

  for(cmComputeLinkInformation::ItemVector::const_iterator li =
      deps.begin();
      li != deps.end(); ++li)
    {
    // An error should be reported if one dependency
    // has INTERFACE_POSITION_INDEPENDENT_CODE ON and the other
    // has INTERFACE_POSITION_INDEPENDENT_CODE OFF, or if the
    // target itself has a POSITION_INDEPENDENT_CODE which disagrees
    // with a dependency.

    if (!li->Target)
      {
      continue;
      }

    const bool ifaceIsSet = li->Target->GetProperties()
                            .find("INTERFACE_" + p)
                            != li->Target->GetProperties().end();
    PropertyType ifacePropContent =
                    getTypedProperty<PropertyType>(li->Target,
                              ("INTERFACE_" + p).c_str(), 0);
    if (explicitlySet)
      {
      if (ifaceIsSet)
        {
        if (!consistentProperty(propContent, ifacePropContent))
          {
          cmOStringStream e;
          e << "Property " << p << " on target \""
            << tgt->GetName() << "\" does\nnot match the "
            "INTERFACE_" << p << " property requirement\nof "
            "dependency \"" << li->Target->GetName() << "\".\n";
          cmSystemTools::Error(e.str().c_str());
          break;
          }
        else
          {
          // Agree
          continue;
          }
        }
      else
        {
        // Explicitly set on target and not set in iface. Can't disagree.
        continue;
        }
      }
    else if (impliedByUse)
      {
      if (ifaceIsSet)
        {
        if (!consistentProperty(propContent, ifacePropContent))
          {
          cmOStringStream e;
          e << "Property " << p << " on target \""
            << tgt->GetName() << "\" is\nimplied to be " << defaultValue
            << " because it was used to determine the link libraries\n"
               "already. The INTERFACE_" << p << " property on\ndependency \""
            << li->Target->GetName() << "\" is in conflict.\n";
          cmSystemTools::Error(e.str().c_str());
          break;
          }
        else
          {
          // Agree
          continue;
          }
        }
      else
        {
        // Implicitly set on target and not set in iface. Can't disagree.
        continue;
        }
      }
    else
      {
      if (ifaceIsSet)
        {
        if (propInitialized)
          {
          if (!consistentProperty(propContent, ifacePropContent))
            {
            cmOStringStream e;
            e << "The INTERFACE_" << p << " property of \""
              << li->Target->GetName() << "\" does\nnot agree with the value "
                "of " << p << " already determined\nfor \""
              << tgt->GetName() << "\".\n";
            cmSystemTools::Error(e.str().c_str());
            break;
            }
          else
            {
            // Agree.
            continue;
            }
          }
        else
          {
          propContent = ifacePropContent;
          propInitialized = true;
          }
        }
      else
        {
        // Not set. Nothing to agree on.
        continue;
        }
      }
    }
  return propContent;
}

//----------------------------------------------------------------------------
bool cmTarget::GetLinkInterfaceDependentBoolProperty(const std::string &p,
                                                     const char *config)
{
  return checkInterfacePropertyCompatibility<bool>(this, p, config, "FALSE",
                                                   0);
}

//----------------------------------------------------------------------------
const char * cmTarget::GetLinkInterfaceDependentStringProperty(
                                                      const std::string &p,
                                                      const char *config)
{
  return checkInterfacePropertyCompatibility<const char *>(this,
                                                           p,
                                                           config,
                                                           "empty", 0);
}

//----------------------------------------------------------------------------
bool isLinkDependentProperty(cmTarget *tgt, const std::string &p,
                             const char *interfaceProperty,
                             const char *config)
{
  cmComputeLinkInformation *info = tgt->GetLinkInformation(config);
  if(!info)
    {
    return false;
    }

  const cmComputeLinkInformation::ItemVector &deps = info->GetItems();

  for(cmComputeLinkInformation::ItemVector::const_iterator li =
      deps.begin();
      li != deps.end(); ++li)
    {
    if (!li->Target)
      {
      continue;
      }
    const char *prop = li->Target->GetProperty(interfaceProperty);
    if (!prop)
      {
      continue;
      }

    std::vector<std::string> props;
    cmSystemTools::ExpandListArgument(prop, props);

    for(std::vector<std::string>::iterator pi = props.begin();
        pi != props.end(); ++pi)
      {
      if (*pi == p)
        {
        return true;
        }
      }
    }

  return false;
}

//----------------------------------------------------------------------------
bool cmTarget::IsLinkInterfaceDependentBoolProperty(const std::string &p,
                                           const char *config)
{
  if (this->TargetTypeValue == OBJECT_LIBRARY)
    {
    return false;
    }
  return (p == "POSITION_INDEPENDENT_CODE") ||
    isLinkDependentProperty(this, p, "COMPATIBLE_INTERFACE_BOOL",
                                 config);
}

//----------------------------------------------------------------------------
bool cmTarget::IsLinkInterfaceDependentStringProperty(const std::string &p,
                                    const char *config)
{
  if (this->TargetTypeValue == OBJECT_LIBRARY)
    {
    return false;
    }
  return isLinkDependentProperty(this, p, "COMPATIBLE_INTERFACE_STRING",
                                 config);
}

//----------------------------------------------------------------------------
void cmTarget::GetLanguages(std::set<cmStdString>& languages) const
{
  for(std::vector<cmSourceFile*>::const_iterator
        i = this->SourceFiles.begin(); i != this->SourceFiles.end(); ++i)
    {
    if(const char* lang = (*i)->GetLanguage())
      {
      languages.insert(lang);
      }
    }
}

//----------------------------------------------------------------------------
bool cmTarget::IsChrpathUsed(const char* config)
{
#if defined(CMAKE_USE_ELF_PARSER)
  // Only certain target types have an rpath.
  if(!(this->GetType() == cmTarget::SHARED_LIBRARY ||
       this->GetType() == cmTarget::MODULE_LIBRARY ||
       this->GetType() == cmTarget::EXECUTABLE))
    {
    return false;
    }

  // If the target will not be installed we do not need to change its
  // rpath.
  if(!this->GetHaveInstallRule())
    {
    return false;
    }

  // Skip chrpath if skipping rpath altogether.
  if(this->Makefile->IsOn("CMAKE_SKIP_RPATH"))
    {
    return false;
    }

  // Skip chrpath if it does not need to be changed at install time.
  if(this->GetPropertyAsBool("BUILD_WITH_INSTALL_RPATH"))
    {
    return false;
    }

  // Allow the user to disable builtin chrpath explicitly.
  if(this->Makefile->IsOn("CMAKE_NO_BUILTIN_CHRPATH"))
    {
    return false;
    }

  // Enable if the rpath flag uses a separator and the target uses ELF
  // binaries.
  if(const char* ll = this->GetLinkerLanguage(config, this))
    {
    std::string sepVar = "CMAKE_SHARED_LIBRARY_RUNTIME_";
    sepVar += ll;
    sepVar += "_FLAG_SEP";
    const char* sep = this->Makefile->GetDefinition(sepVar.c_str());
    if(sep && *sep)
      {
      // TODO: Add ELF check to ABI detection and get rid of
      // CMAKE_EXECUTABLE_FORMAT.
      if(const char* fmt =
         this->Makefile->GetDefinition("CMAKE_EXECUTABLE_FORMAT"))
        {
        return strcmp(fmt, "ELF") == 0;
        }
      }
    }
#endif
  static_cast<void>(config);
  return false;
}

//----------------------------------------------------------------------------
cmTarget::ImportInfo const*
cmTarget::GetImportInfo(const char* config, cmTarget *headTarget)
{
  // There is no imported information for non-imported targets.
  if(!this->IsImported())
    {
    return 0;
    }

  // Lookup/compute/cache the import information for this
  // configuration.
  std::string config_upper;
  if(config && *config)
    {
    config_upper = cmSystemTools::UpperCase(config);
    }
  else
    {
    config_upper = "NOCONFIG";
    }
  TargetConfigPair key(headTarget, config_upper);
  typedef cmTargetInternals::ImportInfoMapType ImportInfoMapType;

  ImportInfoMapType::const_iterator i =
    this->Internal->ImportInfoMap.find(key);
  if(i == this->Internal->ImportInfoMap.end())
    {
    ImportInfo info;
    this->ComputeImportInfo(config_upper, info, headTarget);
    ImportInfoMapType::value_type entry(key, info);
    i = this->Internal->ImportInfoMap.insert(entry).first;
    }

  // If the location is empty then the target is not available for
  // this configuration.
  if(i->second.Location.empty() && i->second.ImportLibrary.empty())
    {
    return 0;
    }

  // Return the import information.
  return &i->second;
}

bool cmTarget::GetMappedConfig(std::string const& desired_config,
                               const char** loc,
                               const char** imp,
                               std::string& suffix)
{
  // Track the configuration-specific property suffix.
  suffix = "_";
  suffix += desired_config;

  std::vector<std::string> mappedConfigs;
  {
  std::string mapProp = "MAP_IMPORTED_CONFIG_";
  mapProp += desired_config;
  if(const char* mapValue = this->GetProperty(mapProp.c_str()))
    {
    cmSystemTools::ExpandListArgument(mapValue, mappedConfigs);
    }
  }

  // If we needed to find one of the mapped configurations but did not
  // On a DLL platform there may be only IMPORTED_IMPLIB for a shared
  // library or an executable with exports.
  bool allowImp = this->HasImportLibrary();

  // If a mapping was found, check its configurations.
  for(std::vector<std::string>::const_iterator mci = mappedConfigs.begin();
      !*loc && !*imp && mci != mappedConfigs.end(); ++mci)
    {
    // Look for this configuration.
    std::string mcUpper = cmSystemTools::UpperCase(mci->c_str());
    std::string locProp = "IMPORTED_LOCATION_";
    locProp += mcUpper;
    *loc = this->GetProperty(locProp.c_str());
    if(allowImp)
      {
      std::string impProp = "IMPORTED_IMPLIB_";
      impProp += mcUpper;
      *imp = this->GetProperty(impProp.c_str());
      }

    // If it was found, use it for all properties below.
    if(*loc || *imp)
      {
      suffix = "_";
      suffix += mcUpper;
      }
    }

  // If we needed to find one of the mapped configurations but did not
  // then the target is not found.  The project does not want any
  // other configuration.
  if(!mappedConfigs.empty() && !*loc && !*imp)
    {
    return false;
    }

  // If we have not yet found it then there are no mapped
  // configurations.  Look for an exact-match.
  if(!*loc && !*imp)
    {
    std::string locProp = "IMPORTED_LOCATION";
    locProp += suffix;
    *loc = this->GetProperty(locProp.c_str());
    if(allowImp)
      {
      std::string impProp = "IMPORTED_IMPLIB";
      impProp += suffix;
      *imp = this->GetProperty(impProp.c_str());
      }
    }

  // If we have not yet found it then there are no mapped
  // configurations and no exact match.
  if(!*loc && !*imp)
    {
    // The suffix computed above is not useful.
    suffix = "";

    // Look for a configuration-less location.  This may be set by
    // manually-written code.
    *loc = this->GetProperty("IMPORTED_LOCATION");
    if(allowImp)
      {
      *imp = this->GetProperty("IMPORTED_IMPLIB");
      }
    }

  // If we have not yet found it then the project is willing to try
  // any available configuration.
  if(!*loc && !*imp)
    {
    std::vector<std::string> availableConfigs;
    if(const char* iconfigs = this->GetProperty("IMPORTED_CONFIGURATIONS"))
      {
      cmSystemTools::ExpandListArgument(iconfigs, availableConfigs);
      }
    for(std::vector<std::string>::const_iterator
          aci = availableConfigs.begin();
        !*loc && !*imp && aci != availableConfigs.end(); ++aci)
      {
      suffix = "_";
      suffix += cmSystemTools::UpperCase(*aci);
      std::string locProp = "IMPORTED_LOCATION";
      locProp += suffix;
      *loc = this->GetProperty(locProp.c_str());
      if(allowImp)
        {
        std::string impProp = "IMPORTED_IMPLIB";
        impProp += suffix;
        *imp = this->GetProperty(impProp.c_str());
        }
      }
    }
  // If we have not yet found it then the target is not available.
  if(!*loc && !*imp)
    {
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
void cmTarget::ComputeImportInfo(std::string const& desired_config,
                                 ImportInfo& info,
                                 cmTarget *headTarget)
{
  (void)headTarget;
  // This method finds information about an imported target from its
  // properties.  The "IMPORTED_" namespace is reserved for properties
  // defined by the project exporting the target.

  // Initialize members.
  info.NoSOName = false;

  const char* loc = 0;
  const char* imp = 0;
  std::string suffix;
  if (!this->GetMappedConfig(desired_config, &loc, &imp, suffix))
    {
    return;
    }

  // A provided configuration has been chosen.  Load the
  // configuration's properties.

  // Get the location.
  if(loc)
    {
    info.Location = loc;
    }
  else
    {
    std::string impProp = "IMPORTED_LOCATION";
    impProp += suffix;
    if(const char* config_location = this->GetProperty(impProp.c_str()))
      {
      info.Location = config_location;
      }
    else if(const char* location = this->GetProperty("IMPORTED_LOCATION"))
      {
      info.Location = location;
      }
    }

  // Get the soname.
  if(this->GetType() == cmTarget::SHARED_LIBRARY)
    {
    std::string soProp = "IMPORTED_SONAME";
    soProp += suffix;
    if(const char* config_soname = this->GetProperty(soProp.c_str()))
      {
      info.SOName = config_soname;
      }
    else if(const char* soname = this->GetProperty("IMPORTED_SONAME"))
      {
      info.SOName = soname;
      }
    }

  // Get the "no-soname" mark.
  if(this->GetType() == cmTarget::SHARED_LIBRARY)
    {
    std::string soProp = "IMPORTED_NO_SONAME";
    soProp += suffix;
    if(const char* config_no_soname = this->GetProperty(soProp.c_str()))
      {
      info.NoSOName = cmSystemTools::IsOn(config_no_soname);
      }
    else if(const char* no_soname = this->GetProperty("IMPORTED_NO_SONAME"))
      {
      info.NoSOName = cmSystemTools::IsOn(no_soname);
      }
    }

  // Get the import library.
  if(imp)
    {
    info.ImportLibrary = imp;
    }
  else if(this->GetType() == cmTarget::SHARED_LIBRARY ||
          this->IsExecutableWithExports())
    {
    std::string impProp = "IMPORTED_IMPLIB";
    impProp += suffix;
    if(const char* config_implib = this->GetProperty(impProp.c_str()))
      {
      info.ImportLibrary = config_implib;
      }
    else if(const char* implib = this->GetProperty("IMPORTED_IMPLIB"))
      {
      info.ImportLibrary = implib;
      }
    }

  // Get the link interface.
  {
  std::string linkProp = "IMPORTED_LINK_INTERFACE_LIBRARIES";
  linkProp += suffix;

  const char *propertyLibs = this->GetProperty(linkProp.c_str());

  if(!propertyLibs)
    {
    linkProp = "IMPORTED_LINK_INTERFACE_LIBRARIES";
    propertyLibs = this->GetProperty(linkProp.c_str());
    }
  if(propertyLibs)
    {
    cmListFileBacktrace lfbt;
    cmGeneratorExpression ge(lfbt);

    cmGeneratorExpressionDAGChecker dagChecker(lfbt,
                                        this->GetName(),
                                        linkProp, 0, 0);
    cmSystemTools::ExpandListArgument(ge.Parse(propertyLibs)
                                       ->Evaluate(this->Makefile,
                                                  desired_config.c_str(),
                                                  false,
                                                  headTarget,
                                                  this,
                                                  &dagChecker),
                                    info.LinkInterface.Libraries);
    }
  }

  // Get the link dependencies.
  {
  std::string linkProp = "IMPORTED_LINK_DEPENDENT_LIBRARIES";
  linkProp += suffix;
  if(const char* config_libs = this->GetProperty(linkProp.c_str()))
    {
    cmSystemTools::ExpandListArgument(config_libs,
                                      info.LinkInterface.SharedDeps);
    }
  else if(const char* libs =
          this->GetProperty("IMPORTED_LINK_DEPENDENT_LIBRARIES"))
    {
    cmSystemTools::ExpandListArgument(libs, info.LinkInterface.SharedDeps);
    }
  }

  // Get the link languages.
  if(this->GetType() == cmTarget::STATIC_LIBRARY)
    {
    std::string linkProp = "IMPORTED_LINK_INTERFACE_LANGUAGES";
    linkProp += suffix;
    if(const char* config_libs = this->GetProperty(linkProp.c_str()))
      {
      cmSystemTools::ExpandListArgument(config_libs,
                                        info.LinkInterface.Languages);
      }
    else if(const char* libs =
            this->GetProperty("IMPORTED_LINK_INTERFACE_LANGUAGES"))
      {
      cmSystemTools::ExpandListArgument(libs,
                                        info.LinkInterface.Languages);
      }
    }

  // Get the cyclic repetition count.
  if(this->GetType() == cmTarget::STATIC_LIBRARY)
    {
    std::string linkProp = "IMPORTED_LINK_INTERFACE_MULTIPLICITY";
    linkProp += suffix;
    if(const char* config_reps = this->GetProperty(linkProp.c_str()))
      {
      sscanf(config_reps, "%u", &info.LinkInterface.Multiplicity);
      }
    else if(const char* reps =
            this->GetProperty("IMPORTED_LINK_INTERFACE_MULTIPLICITY"))
      {
      sscanf(reps, "%u", &info.LinkInterface.Multiplicity);
      }
    }
}

//----------------------------------------------------------------------------
cmTarget::LinkInterface const* cmTarget::GetLinkInterface(const char* config,
                                                      cmTarget *head)
{
  // Imported targets have their own link interface.
  if(this->IsImported())
    {
    if(cmTarget::ImportInfo const* info = this->GetImportInfo(config, head))
      {
      return &info->LinkInterface;
      }
    return 0;
    }

  // Link interfaces are not supported for executables that do not
  // export symbols.
  if(this->GetType() == cmTarget::EXECUTABLE &&
     !this->IsExecutableWithExports())
    {
    return 0;
    }

  // Lookup any existing link interface for this configuration.
  TargetConfigPair key(head, cmSystemTools::UpperCase(config? config : ""));

  cmTargetInternals::LinkInterfaceMapType::iterator
    i = this->Internal->LinkInterfaceMap.find(key);
  if(i == this->Internal->LinkInterfaceMap.end())
    {
    // Compute the link interface for this configuration.
    cmTargetInternals::OptionalLinkInterface iface;
    iface.Exists = this->ComputeLinkInterface(config, iface, head);

    // Store the information for this configuration.
    cmTargetInternals::LinkInterfaceMapType::value_type entry(key, iface);
    i = this->Internal->LinkInterfaceMap.insert(entry).first;
    }

  return i->second.Exists? &i->second : 0;
}

//----------------------------------------------------------------------------
bool cmTarget::ComputeLinkInterface(const char* config, LinkInterface& iface,
                                    cmTarget *headTarget)
{
  // Construct the property name suffix for this configuration.
  std::string suffix = "_";
  if(config && *config)
    {
    suffix += cmSystemTools::UpperCase(config);
    }
  else
    {
    suffix += "NOCONFIG";
    }

  // An explicit list of interface libraries may be set for shared
  // libraries and executables that export symbols.
  const char* explicitLibraries = 0;
  std::string linkIfaceProp;
  if(this->GetType() == cmTarget::SHARED_LIBRARY ||
     this->IsExecutableWithExports())
    {
    // Lookup the per-configuration property.
    linkIfaceProp = "LINK_INTERFACE_LIBRARIES";
    linkIfaceProp += suffix;
    explicitLibraries = this->GetProperty(linkIfaceProp.c_str());

    // If not set, try the generic property.
    if(!explicitLibraries)
      {
      linkIfaceProp = "LINK_INTERFACE_LIBRARIES";
      explicitLibraries = this->GetProperty(linkIfaceProp.c_str());
      }
    }

  // There is no implicit link interface for executables or modules
  // so if none was explicitly set then there is no link interface.
  // Note that CMake versions 2.2 and below allowed linking to modules.
  bool canLinkModules = this->Makefile->NeedBackwardsCompatibility(2,2);
  if(!explicitLibraries &&
     (this->GetType() == cmTarget::EXECUTABLE ||
      (this->GetType() == cmTarget::MODULE_LIBRARY && !canLinkModules)))
    {
    return false;
    }

  if(explicitLibraries)
    {
    // The interface libraries have been explicitly set.
    cmListFileBacktrace lfbt;
    cmGeneratorExpression ge(lfbt);
    cmGeneratorExpressionDAGChecker dagChecker(lfbt, this->GetName(),
                                               linkIfaceProp, 0, 0);
    cmSystemTools::ExpandListArgument(ge.Parse(explicitLibraries)->Evaluate(
                                        this->Makefile,
                                        config,
                                        false,
                                        headTarget,
                                        this, &dagChecker), iface.Libraries);

    if(this->GetType() == cmTarget::SHARED_LIBRARY)
      {
      // Shared libraries may have runtime implementation dependencies
      // on other shared libraries that are not in the interface.
      std::set<cmStdString> emitted;
      for(std::vector<std::string>::const_iterator
            li = iface.Libraries.begin(); li != iface.Libraries.end(); ++li)
        {
        emitted.insert(*li);
        }
      LinkImplementation const* impl = this->GetLinkImplementation(config,
                                                                headTarget);
      for(std::vector<std::string>::const_iterator
            li = impl->Libraries.begin(); li != impl->Libraries.end(); ++li)
        {
        if(emitted.insert(*li).second)
          {
          if(cmTarget* tgt = this->Makefile->FindTargetToUse(li->c_str()))
            {
            // This is a runtime dependency on another shared library.
            if(tgt->GetType() == cmTarget::SHARED_LIBRARY)
              {
              iface.SharedDeps.push_back(*li);
              }
            }
          else
            {
            // TODO: Recognize shared library file names.  Perhaps this
            // should be moved to cmComputeLinkInformation, but that creates
            // a chicken-and-egg problem since this list is needed for its
            // construction.
            }
          }
        }
      }
    }
  else
    {
    // The link implementation is the default link interface.
    LinkImplementation const* impl = this->GetLinkImplementation(config,
                                                              headTarget);
    iface.ImplementationIsInterface = true;
    iface.Libraries = impl->Libraries;
    iface.WrongConfigLibraries = impl->WrongConfigLibraries;
    if(this->GetType() == cmTarget::STATIC_LIBRARY)
      {
      // Targets using this archive need its language runtime libraries.
      iface.Languages = impl->Languages;
      }
    }

  if(this->GetType() == cmTarget::STATIC_LIBRARY)
    {
    // How many repetitions are needed if this library has cyclic
    // dependencies?
    std::string propName = "LINK_INTERFACE_MULTIPLICITY";
    propName += suffix;
    if(const char* config_reps = this->GetProperty(propName.c_str()))
      {
      sscanf(config_reps, "%u", &iface.Multiplicity);
      }
    else if(const char* reps =
            this->GetProperty("LINK_INTERFACE_MULTIPLICITY"))
      {
      sscanf(reps, "%u", &iface.Multiplicity);
      }
    }

  return true;
}

//----------------------------------------------------------------------------
cmTarget::LinkImplementation const*
cmTarget::GetLinkImplementation(const char* config, cmTarget *head)
{
  // There is no link implementation for imported targets.
  if(this->IsImported())
    {
    return 0;
    }

  // Lookup any existing link implementation for this configuration.
  TargetConfigPair key(head, cmSystemTools::UpperCase(config? config : ""));

  cmTargetInternals::LinkImplMapType::iterator
    i = this->Internal->LinkImplMap.find(key);
  if(i == this->Internal->LinkImplMap.end())
    {
    // Compute the link implementation for this configuration.
    LinkImplementation impl;
    this->ComputeLinkImplementation(config, impl, head);

    // Store the information for this configuration.
    cmTargetInternals::LinkImplMapType::value_type entry(key, impl);
    i = this->Internal->LinkImplMap.insert(entry).first;
    }

  return &i->second;
}

//----------------------------------------------------------------------------
void cmTarget::ComputeLinkImplementation(const char* config,
                                         LinkImplementation& impl,
                                         cmTarget *head)
{
  // Compute which library configuration to link.
  cmTarget::LinkLibraryType linkType = this->ComputeLinkType(config);

  // Collect libraries directly linked in this configuration.
  std::vector<std::string> llibs;
  this->GetDirectLinkLibraries(config, llibs, head);
  for(std::vector<std::string>::const_iterator li = llibs.begin();
      li != llibs.end(); ++li)
    {
    // Skip entries that resolve to the target itself or are empty.
    std::string item = this->CheckCMP0004(*li);
    if(item == this->GetName() || item.empty())
      {
      continue;
      }
    // The entry is meant for this configuration.
    impl.Libraries.push_back(item);
    }

  LinkLibraryVectorType const& oldllibs = this->GetOriginalLinkLibraries();
  for(cmTarget::LinkLibraryVectorType::const_iterator li = oldllibs.begin();
      li != oldllibs.end(); ++li)
    {
    if(li->second != cmTarget::GENERAL && li->second != linkType)
      {
      std::string item = this->CheckCMP0004(li->first);
      if(item == this->GetName() || item.empty())
        {
        continue;
        }
      // Support OLD behavior for CMP0003.
      impl.WrongConfigLibraries.push_back(item);
      }
    }

  // This target needs runtime libraries for its source languages.
  std::set<cmStdString> languages;
  // Get languages used in our source files.
  this->GetLanguages(languages);
  // Get languages used in object library sources.
  for(std::vector<std::string>::iterator i = this->ObjectLibraries.begin();
      i != this->ObjectLibraries.end(); ++i)
    {
    if(cmTarget* objLib = this->Makefile->FindTargetToUse(i->c_str()))
      {
      if(objLib->GetType() == cmTarget::OBJECT_LIBRARY)
        {
        objLib->GetLanguages(languages);
        }
      }
    }
  // Copy the set of langauges to the link implementation.
  for(std::set<cmStdString>::iterator li = languages.begin();
      li != languages.end(); ++li)
    {
    impl.Languages.push_back(*li);
    }
}

//----------------------------------------------------------------------------
std::string cmTarget::CheckCMP0004(std::string const& item)
{
  // Strip whitespace off the library names because we used to do this
  // in case variables were expanded at generate time.  We no longer
  // do the expansion but users link to libraries like " ${VAR} ".
  std::string lib = item;
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
  if(lib != item)
    {
    cmake* cm = this->Makefile->GetCMakeInstance();
    switch(this->PolicyStatusCMP0004)
      {
      case cmPolicies::WARN:
        {
        cmOStringStream w;
        w << (this->Makefile->GetPolicies()
              ->GetPolicyWarning(cmPolicies::CMP0004)) << "\n"
          << "Target \"" << this->GetName() << "\" links to item \""
          << item << "\" which has leading or trailing whitespace.";
        cm->IssueMessage(cmake::AUTHOR_WARNING, w.str(),
                         this->GetBacktrace());
        }
      case cmPolicies::OLD:
        break;
      case cmPolicies::NEW:
        {
        cmOStringStream e;
        e << "Target \"" << this->GetName() << "\" links to item \""
          << item << "\" which has leading or trailing whitespace.  "
          << "This is now an error according to policy CMP0004.";
        cm->IssueMessage(cmake::FATAL_ERROR, e.str(), this->GetBacktrace());
        }
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        {
        cmOStringStream e;
        e << (this->Makefile->GetPolicies()
              ->GetRequiredPolicyError(cmPolicies::CMP0004)) << "\n"
          << "Target \"" << this->GetName() << "\" links to item \""
          << item << "\" which has leading or trailing whitespace.";
        cm->IssueMessage(cmake::FATAL_ERROR, e.str(), this->GetBacktrace());
        }
        break;
      }
    }
  return lib;
}

template<typename PropertyType>
PropertyType getLinkInterfaceDependentProperty(cmTarget *tgt,
                                               const std::string prop,
                                               const char *config,
                                               PropertyType *);

template<>
bool getLinkInterfaceDependentProperty(cmTarget *tgt,
                                         const std::string prop,
                                         const char *config, bool *)
{
  return tgt->GetLinkInterfaceDependentBoolProperty(prop, config);
}

template<>
const char * getLinkInterfaceDependentProperty(cmTarget *tgt,
                                                 const std::string prop,
                                                 const char *config,
                                                 const char **)
{
  return tgt->GetLinkInterfaceDependentStringProperty(prop, config);
}

//----------------------------------------------------------------------------
template<typename PropertyType>
void checkPropertyConsistency(cmTarget *depender, cmTarget *dependee,
                              const char *propName,
                              std::set<cmStdString> &emitted,
                              const char *config,
                              PropertyType *)
{
  const char *prop = dependee->GetProperty(propName);
  if (!prop)
    {
    return;
    }

  std::vector<std::string> props;
  cmSystemTools::ExpandListArgument(prop, props);

  for(std::vector<std::string>::iterator pi = props.begin();
      pi != props.end(); ++pi)
    {
    if (depender->GetMakefile()->GetCMakeInstance()
                      ->GetIsPropertyDefined(pi->c_str(),
                                              cmProperty::TARGET))
      {
      cmOStringStream e;
      e << "Target \"" << dependee->GetName() << "\" has property \""
        << *pi << "\" listed in its " << propName << " property.  "
          "This is not allowed.  Only user-defined properties may appear "
          "listed in the " << propName << " property.";
      depender->GetMakefile()->IssueMessage(cmake::FATAL_ERROR, e.str());
      return;
      }
    if(emitted.insert(*pi).second)
      {
      getLinkInterfaceDependentProperty<PropertyType>(depender, *pi, config,
                                                      0);
      if (cmSystemTools::GetErrorOccuredFlag())
        {
        return;
        }
      }
    }
}

//----------------------------------------------------------------------------
void cmTarget::CheckPropertyCompatibility(cmComputeLinkInformation *info,
                                          const char* config)
{
  const cmComputeLinkInformation::ItemVector &deps = info->GetItems();

  std::set<cmStdString> emittedBools;
  std::set<cmStdString> emittedStrings;

  for(cmComputeLinkInformation::ItemVector::const_iterator li =
      deps.begin();
      li != deps.end(); ++li)
    {
    if (!li->Target)
      {
      continue;
      }

    checkPropertyConsistency<bool>(this, li->Target,
                                   "COMPATIBLE_INTERFACE_BOOL",
                                   emittedBools, config, 0);
    if (cmSystemTools::GetErrorOccuredFlag())
      {
      return;
      }
    checkPropertyConsistency<const char *>(this, li->Target,
                                           "COMPATIBLE_INTERFACE_STRING",
                                           emittedStrings, config, 0);
    if (cmSystemTools::GetErrorOccuredFlag())
      {
      return;
      }
    }

  for(std::set<cmStdString>::const_iterator li = emittedBools.begin();
      li != emittedBools.end(); ++li)
    {
    const std::set<cmStdString>::const_iterator si = emittedStrings.find(*li);
    if (si != emittedStrings.end())
      {
      cmOStringStream e;
      e << "Property \"" << *li << "\" appears in both the "
      "COMPATIBLE_INTERFACE_BOOL and the COMPATIBLE_INTERFACE_STRING "
      "property in the dependencies of target \"" << this->GetName() <<
      "\".  This is not allowed. A property may only require compatibility "
      "in a boolean interpretation or a string interpretation, but not both.";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      break;
      }
    }
}

//----------------------------------------------------------------------------
cmComputeLinkInformation*
cmTarget::GetLinkInformation(const char* config, cmTarget *head)
{
  cmTarget *headTarget = head ? head : this;
  // Lookup any existing information for this configuration.
  TargetConfigPair key(headTarget,
                                  cmSystemTools::UpperCase(config?config:""));
  cmTargetLinkInformationMap::iterator
    i = this->LinkInformation.find(key);
  if(i == this->LinkInformation.end())
    {
    // Compute information for this configuration.
    cmComputeLinkInformation* info =
      new cmComputeLinkInformation(this, config, headTarget);
    if(!info || !info->Compute())
      {
      delete info;
      info = 0;
      }

    // Store the information for this configuration.
    cmTargetLinkInformationMap::value_type entry(key, info);
    i = this->LinkInformation.insert(entry).first;

    if (info)
      {
      this->CheckPropertyCompatibility(info, config);
      }
    }
  return i->second;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetFrameworkDirectory(const char* config)
{
  std::string fpath;
  fpath += this->GetFullName(config, false);
  fpath += ".framework/Versions/";
  fpath += this->GetFrameworkVersion();
  fpath += "/";
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmTarget::BuildMacContentDirectory(const std::string& base,
                                               const char* config,
                                               bool includeMacOS)
{
  std::string fpath = base;
  if(this->IsAppBundleOnApple())
    {
    fpath += this->GetFullName(config, false);
    fpath += ".app/Contents/";
    if(includeMacOS)
      fpath += "MacOS/";
    }
  if(this->IsFrameworkOnApple())
    {
    fpath += this->GetFrameworkDirectory(config);
    }
  if(this->IsCFBundleOnApple())
    {
    fpath += this->GetFullName(config, false);
    fpath += ".";
    const char *ext = this->GetProperty("BUNDLE_EXTENSION");
    if (!ext)
      {
      ext = "bundle";
      }
    fpath += ext;
    fpath += "/Contents/";
    if(includeMacOS)
      fpath += "MacOS/";
    }
  return fpath;
}

//----------------------------------------------------------------------------
std::string cmTarget::GetMacContentDirectory(const char* config,
                                             bool implib,
                                             bool includeMacOS)
{
  // Start with the output directory for the target.
  std::string fpath = this->GetDirectory(config, implib);
  fpath += "/";
  fpath = this->BuildMacContentDirectory(fpath, config, includeMacOS);
  return fpath;
}

//----------------------------------------------------------------------------
cmTargetLinkInformationMap
::cmTargetLinkInformationMap(cmTargetLinkInformationMap const& r): derived()
{
  // Ideally cmTarget instances should never be copied.  However until
  // we can make a sweep to remove that, this copy constructor avoids
  // allowing the resources (LinkInformation) from getting copied.  In
  // the worst case this will lead to extra cmComputeLinkInformation
  // instances.  We also enforce in debug mode that the map be emptied
  // when copied.
  static_cast<void>(r);
  assert(r.empty());
}

//----------------------------------------------------------------------------
cmTargetLinkInformationMap::~cmTargetLinkInformationMap()
{
  for(derived::iterator i = this->begin(); i != this->end(); ++i)
    {
    delete i->second;
    }
}

//----------------------------------------------------------------------------
cmTargetInternalPointer::cmTargetInternalPointer()
{
  this->Pointer = new cmTargetInternals;
}

//----------------------------------------------------------------------------
cmTargetInternalPointer
::cmTargetInternalPointer(cmTargetInternalPointer const& r)
{
  // Ideally cmTarget instances should never be copied.  However until
  // we can make a sweep to remove that, this copy constructor avoids
  // allowing the resources (Internals) to be copied.
  this->Pointer = new cmTargetInternals(*r.Pointer);
}

//----------------------------------------------------------------------------
cmTargetInternalPointer::~cmTargetInternalPointer()
{
  deleteAndClear(this->Pointer->IncludeDirectoriesEntries);
  delete this->Pointer;
}

//----------------------------------------------------------------------------
cmTargetInternalPointer&
cmTargetInternalPointer::operator=(cmTargetInternalPointer const& r)
{
  if(this == &r) { return *this; } // avoid warning on HP about self check
  // Ideally cmTarget instances should never be copied.  However until
  // we can make a sweep to remove that, this copy constructor avoids
  // allowing the resources (Internals) to be copied.
  cmTargetInternals* oldPointer = this->Pointer;
  this->Pointer = new cmTargetInternals(*r.Pointer);
  delete oldPointer;
  return *this;
}
