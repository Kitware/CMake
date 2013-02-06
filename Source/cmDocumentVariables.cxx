#include "cmDocumentVariables.h"
#include "cmake.h"

#include <cmsys/ios/sstream>

void cmDocumentVariables::DefineVariables(cmake* cm)
{
  // Subsection: variables defined by cmake, that give
  // information about the project, and cmake
  cm->DefineProperty
    ("CMAKE_AR", cmProperty::VARIABLE,
     "Name of archiving tool for static libraries.",
     "This specifies name of the program that creates archive "
     "or static libraries.",false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_COMMAND", cmProperty::VARIABLE,
     "The full path to the cmake executable.",
     "This is the full path to the CMake executable cmake which is "
     "useful from custom commands that want to use the cmake -E "
     "option for portable system commands. "
     "(e.g. /usr/local/bin/cmake", false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_BINARY_DIR", cmProperty::VARIABLE,
     "The path to the top level of the build tree.",
     "This is the full path to the top level of the current CMake "
     "build tree. For an in-source build, this would be the same "
     "as CMAKE_SOURCE_DIR. ", false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_SOURCE_DIR", cmProperty::VARIABLE,
     "The path to the top level of the source tree.",
     "This is the full path to the top level of the current CMake "
     "source tree. For an in-source build, this would be the same "
     "as CMAKE_BINARY_DIR. ", false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_CURRENT_BINARY_DIR", cmProperty::VARIABLE,
     "The path to the binary directory currently being processed.",
     "This the full path to the build directory that is currently "
     "being processed by cmake.  Each directory added by "
     "add_subdirectory will create a binary directory in the build "
     "tree, and as it is being processed this variable will be set. "
     "For in-source builds this is the current source directory "
     "being processed.", false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_CURRENT_SOURCE_DIR", cmProperty::VARIABLE,
     "The path to the source directory currently being processed.",
     "This the full path to the source directory that is currently "
     "being processed by cmake.  ", false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_CURRENT_LIST_FILE", cmProperty::VARIABLE,
     "Full path to the listfile currently being processed.",
     "As CMake processes the listfiles in your project this "
     "variable will always be set to the one currently being "
     "processed.  "
     "The value has dynamic scope.  "
     "When CMake starts processing commands in a source file "
     "it sets this variable to the location of the file.  "
     "When CMake finishes processing commands from the file it "
     "restores the previous value.  "
     "Therefore the value of the variable inside a macro or "
     "function is the file invoking the bottom-most entry on "
     "the call stack, not the file containing the macro or "
     "function definition."
     "\n"
     "See also CMAKE_PARENT_LIST_FILE.",false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_CURRENT_LIST_LINE", cmProperty::VARIABLE,
     "The line number of the current file being processed.",
     "This is the line number of the file currently being"
     " processed by cmake.", false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_CURRENT_LIST_DIR", cmProperty::VARIABLE,
     "Full directory of the listfile currently being processed.",
     "As CMake processes the listfiles in your project this "
     "variable will always be set to the directory where the listfile which "
     "is currently being processed (CMAKE_CURRENT_LIST_FILE) is located.  "
     "The value has dynamic scope.  "
     "When CMake starts processing commands in a source file "
     "it sets this variable to the directory where this file is located.  "
     "When CMake finishes processing commands from the file it "
     "restores the previous value.  "
     "Therefore the value of the variable inside a macro or "
     "function is the directory of the file invoking the bottom-most entry on "
     "the call stack, not the directory of the file containing the macro or "
     "function definition."
     "\n"
     "See also CMAKE_CURRENT_LIST_FILE.",false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_SCRIPT_MODE_FILE", cmProperty::VARIABLE,
     "Full path to the -P script file currently being processed. ",
     "When run in -P script mode, CMake sets this variable to the full "
     "path of the script file. When run to configure a CMakeLists.txt "
     "file, this variable is not set.", false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_ARGC", cmProperty::VARIABLE,
     "Number of command line arguments passed to CMake in script mode. ",
     "When run in -P script mode, CMake sets this variable to the number "
     "of command line arguments. See also CMAKE_ARGV0, 1, 2 ... ", false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_ARGV0", cmProperty::VARIABLE,
     "Command line argument passed to CMake in script mode. ",
     "When run in -P script mode, CMake sets this variable to "
     "the first command line argument. It then also sets CMAKE_ARGV1, "
     "CMAKE_ARGV2, ... and so on, up to the number of command line arguments "
     "given. See also CMAKE_ARGC.", false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_BUILD_TOOL", cmProperty::VARIABLE,
     "Tool used for the actual build process.",
     "This variable is set to the program that will be"
     " needed to build the output of CMake.   If the "
     "generator selected was Visual Studio 6, the "
     "CMAKE_BUILD_TOOL will be set to msdev, for "
     "Unix makefiles it will be set to make or gmake, "
     "and for Visual Studio 7 it set to devenv.  For "
     "Nmake Makefiles the value is nmake. This can be "
     "useful for adding special flags and commands based"
     " on the final build environment. ", false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_CROSSCOMPILING", cmProperty::VARIABLE,
     "Is CMake currently cross compiling.",
     "This variable will be set to true by CMake if CMake is cross "
     "compiling. Specifically if the build platform is different "
     "from the target platform.", false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_CACHEFILE_DIR", cmProperty::VARIABLE,
     "The directory with the CMakeCache.txt file.",
     "This is the full path to the directory that has the "
     "CMakeCache.txt file in it.  This is the same as "
     "CMAKE_BINARY_DIR.", false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_CACHE_MAJOR_VERSION", cmProperty::VARIABLE,
     "Major version of CMake used to create the CMakeCache.txt file",
     "This is stores the major version of CMake used to "
     "write a CMake cache file. It is only different when "
     "a different version of CMake is run on a previously "
     "created cache file.", false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_CACHE_MINOR_VERSION", cmProperty::VARIABLE,
     "Minor version of CMake used to create the CMakeCache.txt file",
     "This is stores the minor version of CMake used to "
     "write a CMake cache file. It is only different when "
     "a different version of CMake is run on a previously "
     "created cache file.", false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_CACHE_PATCH_VERSION", cmProperty::VARIABLE,
     "Patch version of CMake used to create the CMakeCache.txt file",
     "This is stores the patch version of CMake used to "
     "write a CMake cache file. It is only different when "
     "a different version of CMake is run on a previously "
     "created cache file.", false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_CFG_INTDIR", cmProperty::VARIABLE,
     "Build-time reference to per-configuration output subdirectory.",
     "For native build systems supporting multiple configurations "
     "in the build tree (such as Visual Studio and Xcode), "
     "the value is a reference to a build-time variable specifying "
     "the name of the per-configuration output subdirectory.  "
     "On Makefile generators this evaluates to \".\" because there "
     "is only one configuration in a build tree.  "
     "Example values:\n"
     "  $(IntDir)        = Visual Studio 6\n"
     "  $(OutDir)        = Visual Studio 7, 8, 9\n"
     "  $(Configuration) = Visual Studio 10\n"
     "  $(CONFIGURATION) = Xcode\n"
     "  .                = Make-based tools\n"
     "Since these values are evaluated by the native build system, this "
     "variable is suitable only for use in command lines that will be "
     "evaluated at build time.  "
     "Example of intended usage:\n"
     "  add_executable(mytool mytool.c)\n"
     "  add_custom_command(\n"
     "    OUTPUT out.txt\n"
     "    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/mytool\n"
     "            ${CMAKE_CURRENT_SOURCE_DIR}/in.txt out.txt\n"
     "    DEPENDS mytool in.txt\n"
     "    )\n"
     "  add_custom_target(drive ALL DEPENDS out.txt)\n"
     "Note that CMAKE_CFG_INTDIR is no longer necessary for this purpose "
     "but has been left for compatibility with existing projects.  "
     "Instead add_custom_command() recognizes executable target names in "
     "its COMMAND option, so "
     "\"${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/mytool\" can be "
     "replaced by just \"mytool\"."
     "\n"
     "This variable is read-only.  Setting it is undefined behavior.  "
     "In multi-configuration build systems the value of this variable "
     "is passed as the value of preprocessor symbol \"CMAKE_INTDIR\" to "
     "the compilation of all source files.",false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_CTEST_COMMAND", cmProperty::VARIABLE,
     "Full path to ctest command installed with cmake.",
     "This is the full path to the CTest executable ctest "
     "which is useful from custom commands that want "
     "to use the cmake -E option for portable system "
     "commands.",false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_DL_LIBS", cmProperty::VARIABLE,
     "Name of library containing dlopen and dlcose.",
     "The name of the library that has dlopen and "
     "dlclose in it, usually -ldl on most UNIX machines.",false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_EDIT_COMMAND", cmProperty::VARIABLE,
     "Full path to cmake-gui or ccmake.",
     "This is the full path to the CMake executable "
     "that can graphically edit the cache.  For example,"
     " cmake-gui, ccmake, or cmake -i.",false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_GENERATOR", cmProperty::VARIABLE,
     "The generator used to build the project.",
     "The name of the generator that is being used to generate the "
     "build files.  (e.g. \"Unix Makefiles\", "
     "\"Visual Studio 6\", etc.)",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_EXTRA_GENERATOR", cmProperty::VARIABLE,
     "The extra generator used to build the project.",
     "When using the Eclipse, CodeBlocks or KDevelop generators, CMake "
     "generates Makefiles (CMAKE_GENERATOR) and additionally project files "
     "for the respective IDE. This IDE project file generator is stored in "
     "CMAKE_EXTRA_GENERATOR (e.g. \"Eclipse CDT4\").",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_HOME_DIRECTORY", cmProperty::VARIABLE,
     "Path to top of source tree.",
     "This is the path to the top level of the source tree.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_LINK_LIBRARY_SUFFIX", cmProperty::VARIABLE,
     "The suffix for libraries that you link to.",
     "The suffix to use for the end of a library, .lib on Windows.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_EXECUTABLE_SUFFIX", cmProperty::VARIABLE,
     "The suffix for executables on this platform.",
     "The suffix to use for the end of an executable if any, "
     ".exe on Windows."
     "\n"
     "CMAKE_EXECUTABLE_SUFFIX_<LANG> overrides this for language <LANG>."
     ,false, "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_MAJOR_VERSION", cmProperty::VARIABLE,
     "The Major version of cmake (i.e. the 2 in 2.X.X)",
     "This specifies the major version of the CMake executable"
     " being run.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_MAKE_PROGRAM", cmProperty::VARIABLE,
     "See CMAKE_BUILD_TOOL.",
     "This variable is around for backwards compatibility, "
     "see CMAKE_BUILD_TOOL.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_VS_PLATFORM_TOOLSET", cmProperty::VARIABLE,
     "Visual Studio Platform Toolset name.",
     "VS 10 and above use MSBuild under the hood and support multiple "
     "compiler toolchains.  "
     "CMake may specify a toolset explicitly, such as \"v110\" for "
     "VS 11 or \"Windows7.1SDK\" for 64-bit support in VS 10 Express.  "
     "CMake provides the name of the chosen toolset in this variable."
     ,false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_MINOR_VERSION", cmProperty::VARIABLE,
     "The Minor version of cmake (i.e. the 4 in X.4.X).",
     "This specifies the minor version of the CMake"
     " executable being run.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_PATCH_VERSION", cmProperty::VARIABLE,
     "The patch version of cmake (i.e. the 3 in X.X.3).",
     "This specifies the patch version of the CMake"
     " executable being run.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_TWEAK_VERSION", cmProperty::VARIABLE,
     "The tweak version of cmake (i.e. the 1 in X.X.X.1).",
     "This specifies the tweak version of the CMake executable being run.  "
     "Releases use tweak < 20000000 and development versions use the date "
     "format CCYYMMDD for the tweak level."
     ,false, "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_VERSION", cmProperty::VARIABLE,
     "The full version of cmake in major.minor.patch[.tweak[-id]] format.",
     "This specifies the full version of the CMake executable being run.  "
     "This variable is defined by versions 2.6.3 and higher.  "
     "See variables CMAKE_MAJOR_VERSION, CMAKE_MINOR_VERSION, "
     "CMAKE_PATCH_VERSION, and CMAKE_TWEAK_VERSION "
     "for individual version components.  "
     "The [-id] component appears in non-release versions "
     "and may be arbitrary text.", false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_PARENT_LIST_FILE", cmProperty::VARIABLE,
     "Full path to the parent listfile of the one currently being processed.",
     "As CMake processes the listfiles in your project this "
     "variable will always be set to the listfile that included "
     "or somehow invoked the one currently being "
     "processed. See also CMAKE_CURRENT_LIST_FILE.",false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_PROJECT_NAME", cmProperty::VARIABLE,
     "The name of the current project.",
     "This specifies name of the current project from"
     " the closest inherited PROJECT command.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_RANLIB", cmProperty::VARIABLE,
     "Name of randomizing tool for static libraries.",
     "This specifies name of the program that randomizes "
     "libraries on UNIX, not used on Windows, but may be present.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_ROOT", cmProperty::VARIABLE,
     "Install directory for running cmake.",
     "This is the install root for the running CMake and"
     " the Modules directory can be found here. This is"
     " commonly used in this format: ${CMAKE_ROOT}/Modules",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_SIZEOF_VOID_P", cmProperty::VARIABLE,
     "Size of a void pointer.",
     "This is set to the size of a pointer on the machine, "
     "and is determined by a try compile. If a 64 bit size "
     "is found, then the library search path is modified to "
     "look for 64 bit libraries first.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_SKIP_RPATH", cmProperty::VARIABLE,
     "If true, do not add run time path information.",
     "If this is set to TRUE, then the rpath information "
     "is not added to compiled executables.  The default "
     "is to add rpath information if the platform supports it.  "
     "This allows for easy running from the build tree.  To omit RPATH "
     "in the install step, but not the build step, use "
     "CMAKE_SKIP_INSTALL_RPATH instead.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_SOURCE_DIR", cmProperty::VARIABLE,
     "Source directory for project.",
     "This is the top level source directory for the project. "
     "It corresponds to the source directory given to "
     "cmake-gui or ccmake.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_STANDARD_LIBRARIES", cmProperty::VARIABLE,
     "Libraries linked into every executable and shared library.",
     "This is the list of libraries that are linked "
     "into all executables and libraries.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_VERBOSE_MAKEFILE", cmProperty::VARIABLE,
     "Create verbose makefiles if on.",
     "This variable defaults to false. You can set "
     "this variable to true to make CMake produce verbose "
     "makefiles that show each command line as it is used.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("PROJECT_BINARY_DIR", cmProperty::VARIABLE,
     "Full path to build directory for project.",
     "This is the binary directory of the most recent "
     "PROJECT command.",false,"Variables that Provide Information");
  cm->DefineProperty
    ("PROJECT_NAME", cmProperty::VARIABLE,
     "Name of the project given to the project command.",
     "This is the name given to the most "
     "recent PROJECT command. ",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("PROJECT_SOURCE_DIR", cmProperty::VARIABLE,
     "Top level source directory for the current project.",
     "This is the source directory of the most recent "
     "PROJECT command.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("[Project name]_BINARY_DIR", cmProperty::VARIABLE,
     "Top level binary directory for the named project.",
     "A variable is created with the name used in the PROJECT "
     "command, and is the binary directory for the project.  "
     " This can be useful when SUBDIR is used to connect "
     "several projects.",false,
     "Variables that Provide Information");
  cm->DefineProperty
    ("[Project name]_SOURCE_DIR", cmProperty::VARIABLE,
     "Top level source directory for the named project.",
     "A variable is created with the name used in the PROJECT "
     "command, and is the source directory for the project."
     "   This can be useful when add_subdirectory "
     "is used to connect several projects.",false,
     "Variables that Provide Information");

  cm->DefineProperty
    ("CMAKE_IMPORT_LIBRARY_PREFIX", cmProperty::VARIABLE,
     "The prefix for import libraries that you link to.",
     "The prefix to use for the name of an import library if used "
     "on this platform."
     "\n"
     "CMAKE_IMPORT_LIBRARY_PREFIX_<LANG> overrides this for language <LANG>."
     ,false, "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_IMPORT_LIBRARY_SUFFIX", cmProperty::VARIABLE,
     "The suffix for import  libraries that you link to.",
     "The suffix to use for the end of an import library if used "
     "on this platform."
     "\n"
     "CMAKE_IMPORT_LIBRARY_SUFFIX_<LANG> overrides this for language <LANG>."
     ,false, "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_SHARED_LIBRARY_PREFIX", cmProperty::VARIABLE,
     "The prefix for shared libraries that you link to.",
     "The prefix to use for the name of a shared library, lib on UNIX."
     "\n"
     "CMAKE_SHARED_LIBRARY_PREFIX_<LANG> overrides this for language <LANG>."
     ,false, "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_SHARED_LIBRARY_SUFFIX", cmProperty::VARIABLE,
     "The suffix for shared libraries that you link to.",
     "The suffix to use for the end of a shared library, .dll on Windows."
     "\n"
     "CMAKE_SHARED_LIBRARY_SUFFIX_<LANG> overrides this for language <LANG>."
     ,false, "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_SHARED_MODULE_PREFIX", cmProperty::VARIABLE,
     "The prefix for loadable modules that you link to.",
     "The prefix to use for the name of a loadable module on this platform."
     "\n"
     "CMAKE_SHARED_MODULE_PREFIX_<LANG> overrides this for language <LANG>."
     ,false, "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_SHARED_MODULE_SUFFIX", cmProperty::VARIABLE,
     "The suffix for shared libraries that you link to.",
     "The suffix to use for the end of a loadable module on this platform"
     "\n"
     "CMAKE_SHARED_MODULE_SUFFIX_<LANG> overrides this for language <LANG>."
     ,false, "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_STATIC_LIBRARY_PREFIX", cmProperty::VARIABLE,
     "The prefix for static libraries that you link to.",
     "The prefix to use for the name of a static library, lib on UNIX."
     "\n"
     "CMAKE_STATIC_LIBRARY_PREFIX_<LANG> overrides this for language <LANG>."
     ,false, "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_STATIC_LIBRARY_SUFFIX", cmProperty::VARIABLE,
     "The suffix for static libraries that you link to.",
     "The suffix to use for the end of a static library, .lib on Windows."
     "\n"
     "CMAKE_STATIC_LIBRARY_SUFFIX_<LANG> overrides this for language <LANG>."
     ,false, "Variables that Provide Information");
  cm->DefineProperty
    ("CMAKE_EXTRA_SHARED_LIBRARY_SUFFIXES", cmProperty::VARIABLE,
     "Additional suffixes for shared libraries.",
     "Extensions for shared libraries other than that specified by "
     "CMAKE_SHARED_LIBRARY_SUFFIX, if any.  "
     "CMake uses this to recognize external shared library files during "
     "analysis of libraries linked by a target.",
     false,
     "Variables that Provide Information");


  // Variables defined by cmake, that change the behavior
  // of cmake

  cm->DefineProperty
    ("CMAKE_POLICY_DEFAULT_CMP<NNNN>",  cmProperty::VARIABLE,
     "Default for CMake Policy CMP<NNNN> when it is otherwise left unset.",
     "Commands cmake_minimum_required(VERSION) and cmake_policy(VERSION) "
     "by default leave policies introduced after the given version unset.  "
     "Set CMAKE_POLICY_DEFAULT_CMP<NNNN> to OLD or NEW to specify the "
     "default for policy CMP<NNNN>, where <NNNN> is the policy number."
     "\n"
     "This variable should not be set by a project in CMake code; "
     "use cmake_policy(SET) instead.  "
     "Users running CMake may set this variable in the cache "
     "(e.g. -DCMAKE_POLICY_DEFAULT_CMP<NNNN>=<OLD|NEW>) "
     "to set a policy not otherwise set by the project.  "
     "Set to OLD to quiet a policy warning while using old behavior "
     "or to NEW to try building the project with new behavior.",
     false,
     "Variables That Change Behavior");

    cm->DefineProperty
    ("CMAKE_AUTOMOC_RELAXED_MODE",  cmProperty::VARIABLE,
     "Switch between strict and relaxed automoc mode.",
     "By default, automoc behaves exactly as described in the documentation "
     "of the AUTOMOC target property.  "
     "When set to TRUE, it accepts more input and tries to find the correct "
     "input file for moc even if it differs from the documented behaviour. "
     "In this mode it e.g. also checks whether a header file is intended to "
     "be processed by moc when a \"foo.moc\" file has been included.\n"
     "Relaxed mode has to be enabled for KDE4 compatibility.",
     false,
     "Variables That Change Behavior");

    cm->DefineProperty
    ("CMAKE_INSTALL_DEFAULT_COMPONENT_NAME",  cmProperty::VARIABLE,
     "Default component used in install() commands.",
     "If an install() command is used without the COMPONENT argument, "
     "these files will be grouped into a default component. The name of this "
     "default install component will be taken from this variable.  "
     "It defaults to \"Unspecified\". ",
     false,
     "Variables That Change Behavior");

    cm->DefineProperty
    ("CMAKE_FIND_LIBRARY_PREFIXES",  cmProperty::VARIABLE,
     "Prefixes to prepend when looking for libraries.",
     "This specifies what prefixes to add to library names when "
     "the find_library command looks for libraries. On UNIX "
     "systems this is typically lib, meaning that when trying "
     "to find the foo library it will look for libfoo.",
     false,
     "Variables That Change Behavior");

    cm->DefineProperty
    ("CMAKE_FIND_LIBRARY_SUFFIXES",  cmProperty::VARIABLE,
     "Suffixes to append when looking for libraries.",
     "This specifies what suffixes to add to library names when "
     "the find_library command looks for libraries. On Windows "
     "systems this is typically .lib and .dll, meaning that when trying "
     "to find the foo library it will look for foo.dll etc.",
     false,
     "Variables That Change Behavior");

    cm->DefineProperty
    ("CMAKE_CONFIGURATION_TYPES",  cmProperty::VARIABLE,
     "Specifies the available build types.",
     "This specifies what build types will be available such as "
     "Debug, Release, RelWithDebInfo etc. This has reasonable defaults "
     "on most platforms. But can be extended to provide other "
     "build types. See also CMAKE_BUILD_TYPE.",
     false,
     "Variables That Change Behavior");

    cm->DefineProperty
    ("CMAKE_BUILD_TYPE",  cmProperty::VARIABLE,
     "Specifies the build type for make based generators.",
     "This specifies what build type will be built in this tree. "
     " Possible values are empty, Debug, Release, RelWithDebInfo"
     " and MinSizeRel. This variable is only supported for "
     "make based generators. If this variable is supported, "
     "then CMake will also provide initial values for the "
     "variables with the name "
     " CMAKE_C_FLAGS_[DEBUG|RELEASE|RELWITHDEBINFO|MINSIZEREL]."
     " For example, if CMAKE_BUILD_TYPE is Debug, then "
     "CMAKE_C_FLAGS_DEBUG will be added to the CMAKE_C_FLAGS.",false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_BACKWARDS_COMPATIBILITY", cmProperty::VARIABLE,
     "Version of cmake required to build project",
     "From the point of view of backwards compatibility, this "
     "specifies what version of CMake should be supported. By "
     "default this value is the version number of CMake that "
     "you are running. You can set this to an older version of"
     " CMake to support deprecated commands of CMake in projects"
     " that were written to use older versions of CMake. This "
     "can be set by the user or set at the beginning of a "
     "CMakeLists file.",false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_INSTALL_PREFIX", cmProperty::VARIABLE,
     "Install directory used by install.",
     "If \"make install\" is invoked or INSTALL is built"
     ", this directory is pre-pended onto all install "
     "directories. This variable defaults to /usr/local"
     " on UNIX and c:/Program Files on Windows.\n"
     "On UNIX one can use the DESTDIR mechanism in order"
     " to relocate the whole installation. "
     "DESTDIR means DESTination DIRectory. It is "
     "commonly used by makefile users "
     "in order to install software at non-default location. "
     "It is usually invoked like this:\n"
     " make DESTDIR=/home/john install\n"
     "which will install the concerned software using the"
     " installation prefix, e.g. \"/usr/local\" pre-pended with "
     "the DESTDIR value which finally gives \"/home/john/usr/local\".\n"
     "WARNING: DESTDIR may not be used on Windows because installation"
     " prefix usually contains a drive letter like in \"C:/Program Files\""
     " which cannot be pre-pended with some other prefix."
     ,false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_SKIP_INSTALL_ALL_DEPENDENCY", cmProperty::VARIABLE,
     "Don't make the install target depend on the all target.",
     "By default, the \"install\" target depends on the \"all\" target. "
     "This has the effect, that when \"make install\" is invoked or INSTALL "
     "is built, first the \"all\" target is built, then the installation "
     "starts. "
     "If CMAKE_SKIP_INSTALL_ALL_DEPENDENCY is set to TRUE, this dependency "
     "is not created, so the installation process will start immediately, "
     "independent from whether the project has been completely built or not."
     ,false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_MODULE_PATH", cmProperty::VARIABLE,
     "List of directories to search for CMake modules.",
     "Commands like include() and find_package() search for files in "
     "directories listed by this variable before checking the default "
     "modules that come with CMake.",
     false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_PREFIX_PATH", cmProperty::VARIABLE,
     "Path used for searching by FIND_XXX(), with appropriate suffixes added.",
     "Specifies a path which will be used by the FIND_XXX() commands. It "
     "contains the \"base\" directories, the FIND_XXX() commands append "
     "appropriate subdirectories to the base directories. So FIND_PROGRAM() "
     "adds /bin to each of the directories in the path, FIND_LIBRARY() "
     "appends /lib to each of the directories, and FIND_PATH() and "
     "FIND_FILE() append /include . By default it is empty, it is intended "
     "to be set by the project. See also CMAKE_SYSTEM_PREFIX_PATH, "
     "CMAKE_INCLUDE_PATH, CMAKE_LIBRARY_PATH, CMAKE_PROGRAM_PATH.", false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_INCLUDE_PATH", cmProperty::VARIABLE,
     "Path used for searching by FIND_FILE() and FIND_PATH().",
     "Specifies a path which will be used both by FIND_FILE() and "
     "FIND_PATH(). Both commands will check each of the contained directories "
     "for the existence of the file which is currently searched. By default "
     "it is empty, it is intended to be set by the project. See also "
     "CMAKE_SYSTEM_INCLUDE_PATH, CMAKE_PREFIX_PATH.", false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_LIBRARY_PATH", cmProperty::VARIABLE,
     "Path used for searching by FIND_LIBRARY().",
     "Specifies a path which will be used by FIND_LIBRARY(). FIND_LIBRARY() "
     "will check each of the contained directories for the existence of the "
     "library which is currently searched. By default it is empty, it is "
     "intended to be set by the project. See also CMAKE_SYSTEM_LIBRARY_PATH, "
     "CMAKE_PREFIX_PATH.", false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_PROGRAM_PATH", cmProperty::VARIABLE,
     "Path used for searching by FIND_PROGRAM().",
     "Specifies a path which will be used by FIND_PROGRAM(). FIND_PROGRAM() "
     "will check each of the contained directories for the existence of the "
     "program which is currently searched. By default it is empty, it is "
     "intended to be set by the project. See also CMAKE_SYSTEM_PROGRAM_PATH, "
     " CMAKE_PREFIX_PATH.", false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_SYSTEM_PREFIX_PATH", cmProperty::VARIABLE,
     "Path used for searching by FIND_XXX(), with appropriate suffixes added.",
     "Specifies a path which will be used by the FIND_XXX() commands. It "
     "contains the \"base\" directories, the FIND_XXX() commands append "
     "appropriate subdirectories to the base directories. So FIND_PROGRAM() "
     "adds /bin to each of the directories in the path, FIND_LIBRARY() "
     "appends /lib to each of the directories, and FIND_PATH() and "
     "FIND_FILE() append /include . By default this contains the standard "
     "directories for the current system. It is NOT intended "
     "to be modified by the project, use CMAKE_PREFIX_PATH for this. See also "
     "CMAKE_SYSTEM_INCLUDE_PATH, CMAKE_SYSTEM_LIBRARY_PATH, "
     "CMAKE_SYSTEM_PROGRAM_PATH, and CMAKE_SYSTEM_IGNORE_PATH.", false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_SYSTEM_IGNORE_PATH", cmProperty::VARIABLE,
     "Path to be ignored by FIND_XXX() commands.",
     "Specifies directories to be ignored by searches in FIND_XXX() "
     "commands.  "
     "This is useful in cross-compiled environments where some system "
     "directories contain incompatible but possibly linkable libraries. For "
     "example, on cross-compiled cluster environments, this allows a user to "
     "ignore directories containing libraries meant for the front-end "
     "machine that modules like FindX11 (and others) would normally search. "
     "By default this contains a list of directories containing incompatible "
     "binaries for the host system. "
     "See also CMAKE_SYSTEM_PREFIX_PATH, CMAKE_SYSTEM_LIBRARY_PATH, "
     "CMAKE_SYSTEM_INCLUDE_PATH, and CMAKE_SYSTEM_PROGRAM_PATH.", false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_IGNORE_PATH", cmProperty::VARIABLE,
     "Path to be ignored by FIND_XXX() commands.",
     "Specifies directories to be ignored by searches in FIND_XXX() "
     "commands.  "
     "This is useful in cross-compiled environments where some system "
     "directories contain incompatible but possibly linkable libraries. For "
     "example, on cross-compiled cluster environments, this allows a user to "
     "ignore directories containing libraries meant for the front-end "
     "machine that modules like FindX11 (and others) would normally search. "
     "By default this is empty; it is intended to be set by the project. "
     "Note that CMAKE_IGNORE_PATH takes a list of directory names, NOT a "
     "list of prefixes. If you want to ignore paths under prefixes (bin, "
     "include, lib, etc.), you'll need to specify them explicitly. "
     "See also CMAKE_PREFIX_PATH, CMAKE_LIBRARY_PATH, CMAKE_INCLUDE_PATH, "
     "CMAKE_PROGRAM_PATH.", false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_SYSTEM_INCLUDE_PATH", cmProperty::VARIABLE,
     "Path used for searching by FIND_FILE() and FIND_PATH().",
     "Specifies a path which will be used both by FIND_FILE() and "
     "FIND_PATH(). Both commands will check each of the contained directories "
     "for the existence of the file which is currently searched. By default "
     "it contains the standard directories for the current system. It is "
     "NOT intended to be modified by the project, use CMAKE_INCLUDE_PATH "
     "for this. See also CMAKE_SYSTEM_PREFIX_PATH.", false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_SYSTEM_LIBRARY_PATH", cmProperty::VARIABLE,
     "Path used for searching by FIND_LIBRARY().",
     "Specifies a path which will be used by FIND_LIBRARY(). FIND_LIBRARY() "
     "will check each of the contained directories for the existence of the "
     "library which is currently searched. By default it contains the "
     "standard directories for the current system. It is NOT intended to be "
     "modified by the project, use CMAKE_LIBRARY_PATH for this. See "
     "also CMAKE_SYSTEM_PREFIX_PATH.", false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_SYSTEM_PROGRAM_PATH", cmProperty::VARIABLE,
     "Path used for searching by FIND_PROGRAM().",
     "Specifies a path which will be used by FIND_PROGRAM(). FIND_PROGRAM() "
     "will check each of the contained directories for the existence of the "
     "program which is currently searched. By default it contains the "
     "standard directories for the current system. It is NOT intended to be "
     "modified by the project, use CMAKE_PROGRAM_PATH for this. See also "
     "CMAKE_SYSTEM_PREFIX_PATH.", false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_DISABLE_FIND_PACKAGE_<PackageName>", cmProperty::VARIABLE,
     "Variable for disabling find_package() calls.",
     "Every non-REQUIRED find_package() call in a project can be disabled "
     "by setting the variable CMAKE_DISABLE_FIND_PACKAGE_<PackageName> to "
     "TRUE. This can be used to build a project without an optional package, "
     "although that package is installed.\n"
     "This switch should be used during the initial CMake run. Otherwise if "
     "the package has already been found in a previous CMake run, the "
     "variables which have been stored in the cache will still be there. "
     "In the case it is recommended to remove the cache variables for "
     "this package from the cache using the cache editor or cmake -U", false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_FIND_PACKAGE_WARN_NO_MODULE", cmProperty::VARIABLE,
     "Tell find_package to warn if called without an explicit mode.",
     "If find_package is called without an explicit mode option "
     "(MODULE, CONFIG or NO_MODULE) and no Find<pkg>.cmake module is "
     "in CMAKE_MODULE_PATH then CMake implicitly assumes that the "
     "caller intends to search for a package configuration file.  "
     "If no package configuration file is found then the wording "
     "of the failure message must account for both the case that the "
     "package is really missing and the case that the project has a "
     "bug and failed to provide the intended Find module.  "
     "If instead the caller specifies an explicit mode option then "
     "the failure message can be more specific."
     "\n"
     "Set CMAKE_FIND_PACKAGE_WARN_NO_MODULE to TRUE to tell find_package "
     "to warn when it implicitly assumes Config mode.  "
     "This helps developers enforce use of an explicit mode in all calls "
     "to find_package within a project.", false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_USER_MAKE_RULES_OVERRIDE", cmProperty::VARIABLE,
     "Specify a CMake file that overrides platform information.",
     "CMake loads the specified file while enabling support for each "
     "language from either the project() or enable_language() commands.  "
     "It is loaded after CMake's builtin compiler and platform information "
     "modules have been loaded but before the information is used.  "
     "The file may set platform information variables to override CMake's "
     "defaults."
     "\n"
     "This feature is intended for use only in overriding information "
     "variables that must be set before CMake builds its first test "
     "project to check that the compiler for a language works.  "
     "It should not be used to load a file in cases that a normal include() "
     "will work.  "
     "Use it only as a last resort for behavior that cannot be achieved "
     "any other way.  "
     "For example, one may set CMAKE_C_FLAGS_INIT to change the default "
     "value used to initialize CMAKE_C_FLAGS before it is cached.  "
     "The override file should NOT be used to set anything that could "
     "be set after languages are enabled, such as variables like "
     "CMAKE_RUNTIME_OUTPUT_DIRECTORY that affect the placement of binaries.  "
     "Information set in the file will be used for try_compile and try_run "
     "builds too."
     ,false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("BUILD_SHARED_LIBS", cmProperty::VARIABLE,
     "Global flag to cause add_library to create shared libraries if on.",
     "If present and true, this will cause all libraries to be "
     "built shared unless the library was explicitly added as a "
     "static library.  This variable is often added to projects "
     "as an OPTION so that each user of a project can decide if "
     "they want to build the project using shared or static "
     "libraries.",false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_NOT_USING_CONFIG_FLAGS", cmProperty::VARIABLE,
     "Skip _BUILD_TYPE flags if true.",
     "This is an internal flag used by the generators in "
     "CMake to tell CMake to skip the _BUILD_TYPE flags.",false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_MFC_FLAG", cmProperty::VARIABLE,
     "Tell cmake to use MFC for an executable or dll.",
     "This can be set in a CMakeLists.txt file and will "
     "enable MFC in the application.  It should be set "
     "to 1 for the static MFC library, and 2 for "
     "the shared MFC library.  This is used in Visual "
     "Studio 6 and 7 project files.   The CMakeSetup "
     "dialog used MFC and the CMakeLists.txt looks like this:\n"
     "  add_definitions(-D_AFXDLL)\n"
     "  set(CMAKE_MFC_FLAG 2)\n"
     "  add_executable(CMakeSetup WIN32 ${SRCS})\n",false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_COLOR_MAKEFILE", cmProperty::VARIABLE,
     "Enables color output when using the Makefile generator.",
     "When enabled, the generated Makefiles will produce colored output. "
     "Default is ON.",false,
     "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_ABSOLUTE_DESTINATION_FILES", cmProperty::VARIABLE,
      "List of files which have been installed using "
      " an ABSOLUTE DESTINATION path.",
      "This variable is defined by CMake-generated cmake_install.cmake "
      "scripts."
      " It can be used (read-only) by program or script that source those"
      " install scripts. This is used by some CPack generators (e.g. RPM).",
      false,
      "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION", cmProperty::VARIABLE,
      "Ask cmake_install.cmake script to warn each time a file with "
      "absolute INSTALL DESTINATION is encountered.",
      "This variable is used by CMake-generated cmake_install.cmake"
      " scripts. If ones set this variable to ON while running the"
      " script, it may get warning messages from the script.", false,
      "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION", cmProperty::VARIABLE,
      "Ask cmake_install.cmake script to error out as soon as "
      "a file with absolute INSTALL DESTINATION is encountered.",
      "The fatal error is emitted before the installation of "
      "the offending file takes place."
      " This variable is used by CMake-generated cmake_install.cmake"
      " scripts. If ones set this variable to ON while running the"
      " script, it may get fatal error messages from the script.",false,
      "Variables That Change Behavior");

  cm->DefineProperty
    ("CMAKE_DEBUG_TARGET_PROPERTIES", cmProperty::VARIABLE,
     "Enables tracing output for target properties.",
     "This variable can be populated with a list of properties to generate "
     "debug output for when evaluating target properties.  Currently it can "
     "only be used when evaluating the INCLUDE_DIRECTORIES target property.  "
     "In that case, it outputs a backtrace for each include directory in "
     "the build.  Default is unset.",false,"Variables That Change Behavior");

  // Variables defined by CMake that describe the system

  cm->DefineProperty
    ("CMAKE_SYSTEM", cmProperty::VARIABLE,
     "Name of system cmake is compiling for.",
     "This variable is the composite of CMAKE_SYSTEM_NAME "
     "and CMAKE_SYSTEM_VERSION, like this "
     "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_VERSION}. "
     "If CMAKE_SYSTEM_VERSION is not set, then "
     "CMAKE_SYSTEM is the same as CMAKE_SYSTEM_NAME.",false,
     "Variables That Describe the System");
  cm->DefineProperty
    ("CMAKE_SYSTEM_NAME", cmProperty::VARIABLE,
     "Name of the OS CMake is building for.",
     "This is the name of the operating system on "
     "which CMake is targeting.   On systems that "
     "have the uname command, this variable is set "
     "to the output of uname -s.  Linux, Windows, "
     " and Darwin for Mac OSX are the values found "
     " on the big three operating systems."  ,false,
     "Variables That Describe the System");
  cm->DefineProperty
    ("CMAKE_SYSTEM_PROCESSOR", cmProperty::VARIABLE,
     "The name of the CPU CMake is building for.",
     "On systems that support uname, this variable is "
     "set to the output of uname -p, on windows it is "
     "set to the value of the environment variable "
     "PROCESSOR_ARCHITECTURE",false,
     "Variables That Describe the System");
  cm->DefineProperty
    ("CMAKE_SYSTEM_VERSION", cmProperty::VARIABLE,
     "OS version CMake is building for.",
     "A numeric version string for the system, on "
     "systems that support uname, this variable is "
     "set to the output of uname -r. On other "
     "systems this is set to major-minor version numbers.",false,
     "Variables That Describe the System");
  cm->DefineProperty
    ("CMAKE_LIBRARY_ARCHITECTURE", cmProperty::VARIABLE,
     "Target architecture library directory name, if detected.",
     "This is the value of CMAKE_<lang>_LIBRARY_ARCHITECTURE as "
     "detected for one of the enabled languages.",false,
     "Variables That Describe the System");
  cm->DefineProperty
    ("CMAKE_LIBRARY_ARCHITECTURE_REGEX", cmProperty::VARIABLE,
     "Regex matching possible target architecture library directory names.",
     "This is used to detect CMAKE_<lang>_LIBRARY_ARCHITECTURE from the "
     "implicit linker search path by matching the <arch> name.",false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("CMAKE_HOST_SYSTEM", cmProperty::VARIABLE,
     "Name of system cmake is being run on.",
     "The same as CMAKE_SYSTEM but for the host system instead "
     "of the target system when cross compiling.",false,
     "Variables That Describe the System");
  cm->DefineProperty
    ("CMAKE_HOST_SYSTEM_NAME", cmProperty::VARIABLE,
     "Name of the OS CMake is running on.",
     "The same as CMAKE_SYSTEM_NAME but for the host system instead "
     "of the target system when cross compiling.",false,
     "Variables That Describe the System");
  cm->DefineProperty
    ("CMAKE_HOST_SYSTEM_PROCESSOR", cmProperty::VARIABLE,
     "The name of the CPU CMake is running on.",
     "The same as CMAKE_SYSTEM_PROCESSOR but for the host system instead "
     "of the target system when cross compiling.",false,
     "Variables That Describe the System");
  cm->DefineProperty
    ("CMAKE_HOST_SYSTEM_VERSION", cmProperty::VARIABLE,
     "OS version CMake is running on.",
     "The same as CMAKE_SYSTEM_VERSION but for the host system instead "
     "of the target system when cross compiling.",false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("APPLE", cmProperty::VARIABLE,
     "True if running on Mac OSX.",
     "Set to true on Mac OSX.",false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("BORLAND", cmProperty::VARIABLE,
     "True if the borland compiler is being used.",
     "This is set to true if the Borland compiler is being used.",false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("CYGWIN", cmProperty::VARIABLE,
     "True for cygwin.",
     "Set to true when using CYGWIN.",false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("MSVC", cmProperty::VARIABLE,
     "True when using Microsoft Visual C",
     "Set to true when the compiler is some version of Microsoft Visual C.",
     false,
     "Variables That Describe the System");

  int msvc_versions[] = { 60, 70, 71, 80, 90, 100, 110, 0 };
  for (int i = 0; msvc_versions[i] != 0; i ++)
    {
    const char minor = (char)('0' + (msvc_versions[i] % 10));
    cmStdString varName = "MSVC";
    cmsys_ios::ostringstream majorStr;

    majorStr << (msvc_versions[i] / 10);
    varName += majorStr.str();
    if (msvc_versions[i] < 100)
      {
      varName += minor;
      }

    cmStdString verString = majorStr.str() + "." + minor;

    cmStdString shortStr = "True when using Microsoft Visual C " + verString;
    cmStdString fullStr = "Set to true when the compiler is version " +
                          verString +
                          " of Microsoft Visual C.";
    cm->DefineProperty
      (varName.c_str(), cmProperty::VARIABLE,
       shortStr.c_str(),
       fullStr.c_str(),
       false,
       "Variables That Describe the System");
    }

  cm->DefineProperty
    ("MSVC_IDE", cmProperty::VARIABLE,
     "True when using the Microsoft Visual C IDE",
     "Set to true when the target platform is the Microsoft Visual C IDE, "
     "as opposed to the command line compiler.",
     false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("MSVC_VERSION", cmProperty::VARIABLE,
     "The version of Microsoft Visual C/C++ being used if any.",
     "Known version numbers are:\n"
     "  1200 = VS  6.0\n"
     "  1300 = VS  7.0\n"
     "  1310 = VS  7.1\n"
     "  1400 = VS  8.0\n"
     "  1500 = VS  9.0\n"
     "  1600 = VS 10.0\n"
     "  1700 = VS 11.0\n"
     "",
     false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("CMAKE_CL_64", cmProperty::VARIABLE,
     "Using the 64 bit compiler from Microsoft",
     "Set to true when using the 64 bit cl compiler from Microsoft.",
     false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("CMAKE_COMPILER_2005", cmProperty::VARIABLE,
     "Using the Visual Studio 2005 compiler from Microsoft",
     "Set to true when using the Visual Studio 2005 compiler "
     "from Microsoft.",
     false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("UNIX", cmProperty::VARIABLE,
     "True for UNIX and UNIX like operating systems.",
     "Set to true when the target system is UNIX or UNIX like "
     "(i.e. APPLE and CYGWIN).",false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("WIN32", cmProperty::VARIABLE,
     "True on windows systems, including win64.",
     "Set to true when the target system is Windows.",false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("XCODE_VERSION", cmProperty::VARIABLE,
     "Version of Xcode (Xcode generator only).",
     "Under the Xcode generator, this is the version of Xcode as specified in "
     "\"Xcode.app/Contents/version.plist\" (such as \"3.1.2\").",false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("CMAKE_HOST_APPLE", cmProperty::VARIABLE,
     "True for Apple OSXoperating systems.",
     "Set to true when the host system is Apple OSX.",
     false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("CMAKE_HOST_UNIX", cmProperty::VARIABLE,
     "True for UNIX and UNIX like operating systems.",
     "Set to true when the host system is UNIX or UNIX like "
     "(i.e. APPLE and CYGWIN).",false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("CMAKE_HOST_WIN32", cmProperty::VARIABLE,
     "True on windows systems, including win64.",
     "Set to true when the host system is Windows and on cygwin.",false,
     "Variables That Describe the System");

  cm->DefineProperty
    ("CMAKE_OBJECT_PATH_MAX", cmProperty::VARIABLE,
     "Maximum object file full-path length allowed by native build tools.",
     "CMake computes for every source file an object file name that is "
     "unique to the source file and deterministic with respect to the "
     "full path to the source file.  "
     "This allows multiple source files in a target to share the same name "
     "if they lie in different directories without rebuilding when one is "
     "added or removed.  "
     "However, it can produce long full paths in a few cases, so CMake "
     "shortens the path using a hashing scheme when the full path to an "
     "object file exceeds a limit.  "
     "CMake has a built-in limit for each platform that is sufficient for "
     "common tools, but some native tools may have a lower limit.  "
     "This variable may be set to specify the limit explicitly.  "
     "The value must be an integer no less than 128.",false,
     "Variables That Describe the System");

  // Variables that affect the building of object files and
  // targets.
  //
  cm->DefineProperty
    ("CMAKE_INCLUDE_CURRENT_DIR", cmProperty::VARIABLE,
     "Automatically add the current source- and build directories "
     "to the include path.",
     "If this variable is enabled, CMake automatically adds in each "
     "directory ${CMAKE_CURRENT_SOURCE_DIR} and ${CMAKE_CURRENT_BINARY_DIR} "
     "to the include path for this directory. These additional include "
     "directories do not propagate down to subdirectories. This is useful "
     "mainly for out-of-source builds, where files generated into the "
     "build tree are included by files located in the source tree.\n"
     "By default CMAKE_INCLUDE_CURRENT_DIR is OFF.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_BUILD_INTERFACE_INCLUDES", cmProperty::VARIABLE,
     "Automatically add the current source- and build directories "
     "to the INTERFACE_INCLUDE_DIRECTORIES.",
     "If this variable is enabled, CMake automatically adds for each "
     "target ${CMAKE_CURRENT_SOURCE_DIR} and ${CMAKE_CURRENT_BINARY_DIR} "
     "to the INTERFACE_INCLUDE_DIRECTORIES."
     "By default CMAKE_BUILD_INTERFACE_INCLUDES is OFF.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_INSTALL_RPATH", cmProperty::VARIABLE,
     "The rpath to use for installed targets.",
     "A semicolon-separated list specifying the rpath "
     "to use in installed targets (for platforms that support it). "
     "This is used to initialize the target property "
     "INSTALL_RPATH for all targets.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_INSTALL_RPATH_USE_LINK_PATH", cmProperty::VARIABLE,
     "Add paths to linker search and installed rpath.",
     "CMAKE_INSTALL_RPATH_USE_LINK_PATH is a boolean that if set to true "
     "will append directories in the linker search path and outside the "
     "project to the INSTALL_RPATH. "
     "This is used to initialize the target property "
     "INSTALL_RPATH_USE_LINK_PATH for all targets.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_INSTALL_NAME_DIR", cmProperty::VARIABLE,
     "Mac OSX directory name for installed targets.",
     "CMAKE_INSTALL_NAME_DIR is used to initialize the "
     "INSTALL_NAME_DIR property on all targets. See that target "
     "property for more information.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_Fortran_FORMAT", cmProperty::VARIABLE,
     "Set to FIXED or FREE to indicate the Fortran source layout.",
     "This variable is used to initialize the Fortran_FORMAT "
     "property on all the targets. "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_Fortran_MODULE_DIRECTORY", cmProperty::VARIABLE,
     "Fortran module output directory.",
     "This variable is used to initialize the "
     "Fortran_MODULE_DIRECTORY property on all the targets. "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_LIBRARY_OUTPUT_DIRECTORY", cmProperty::VARIABLE,
     "Where to put all the LIBRARY targets when built.",
     "This variable is used to initialize the "
     "LIBRARY_OUTPUT_DIRECTORY property on all the targets. "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_ARCHIVE_OUTPUT_DIRECTORY", cmProperty::VARIABLE,
     "Where to put all the ARCHIVE targets when built.",
     "This variable is used to initialize the "
     "ARCHIVE_OUTPUT_DIRECTORY property on all the targets. "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_RUNTIME_OUTPUT_DIRECTORY", cmProperty::VARIABLE,
     "Where to put all the RUNTIME targets when built.",
     "This variable is used to initialize the "
     "RUNTIME_OUTPUT_DIRECTORY property on all the targets. "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_PDB_OUTPUT_DIRECTORY", cmProperty::VARIABLE,
     "Where to put all the MS debug symbol files.",
     "This variable is used to initialize the "
     "PDB_OUTPUT_DIRECTORY property on all the targets. "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_LINK_DEPENDS_NO_SHARED", cmProperty::VARIABLE,
     "Whether to skip link dependencies on shared library files.",
     "This variable initializes the LINK_DEPENDS_NO_SHARED "
     "property on targets when they are created.  "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_AUTOMOC", cmProperty::VARIABLE,
     "Whether to handle moc automatically for Qt targets.",
     "This variable is used to initialize the "
     "AUTOMOC property on all the targets. "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_AUTOMOC_MOC_OPTIONS", cmProperty::VARIABLE,
     "Additional options for moc when using automoc (see CMAKE_AUTOMOC).",
     "This variable is used to initialize the "
     "AUTOMOC_MOC_OPTIONS property on all the targets. "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_GNUtoMS", cmProperty::VARIABLE,
     "Convert GNU import libraries (.dll.a) to MS format (.lib).",
     "This variable is used to initialize the GNUtoMS property on targets "
     "when they are created.  "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_DEBUG_POSTFIX", cmProperty::VARIABLE,
     "See variable CMAKE_<CONFIG>_POSTFIX.",
     "This variable is a special case of the more-general "
     "CMAKE_<CONFIG>_POSTFIX variable for the DEBUG configuration.",
     false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("CMAKE_<CONFIG>_POSTFIX", cmProperty::VARIABLE,
     "Default filename postfix for libraries under configuration <CONFIG>.",
     "When a non-executable target is created its <CONFIG>_POSTFIX "
     "target property is initialized with the value of this variable "
     "if it is set.",
     false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_BUILD_WITH_INSTALL_RPATH", cmProperty::VARIABLE,
     "Use the install path for the RPATH",
     "Normally CMake uses the build tree for the RPATH when building "
     "executables etc on systems that use RPATH. When the software "
     "is installed the executables etc are relinked by CMake to have "
     "the install RPATH. If this variable is set to true then the software "
     "is always built with the install path for the RPATH and does not "
     "need to be relinked when installed.",false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_NO_BUILTIN_CHRPATH", cmProperty::VARIABLE,
     "Do not use the builtin ELF editor to fix RPATHs on installation.",
     "When an ELF binary needs to have a different RPATH after installation "
     "than it does in the build tree, CMake uses a builtin editor to change "
     "the RPATH in the installed copy.  "
     "If this variable is set to true then CMake will relink the binary "
     "before installation instead of using its builtin editor.",false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_SKIP_BUILD_RPATH", cmProperty::VARIABLE,
     "Do not include RPATHs in the build tree.",
     "Normally CMake uses the build tree for the RPATH when building "
     "executables etc on systems that use RPATH. When the software "
     "is installed the executables etc are relinked by CMake to have "
     "the install RPATH. If this variable is set to true then the software "
     "is always built with no RPATH.",false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_SKIP_INSTALL_RPATH", cmProperty::VARIABLE,
     "Do not include RPATHs in the install tree.",
     "Normally CMake uses the build tree for the RPATH when building "
     "executables etc on systems that use RPATH. When the software "
     "is installed the executables etc are relinked by CMake to have "
     "the install RPATH. If this variable is set to true then the software "
     "is always installed without RPATH, even if RPATH is enabled when "
     "building.  This can be useful for example to allow running tests from "
     "the build directory with RPATH enabled before the installation step.  "
     "To omit RPATH in both the build and install steps, use "
     "CMAKE_SKIP_RPATH instead.",false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_EXE_LINKER_FLAGS", cmProperty::VARIABLE,
     "Linker flags used to create executables.",
     "Flags used by the linker when creating an executable.",false,
     "Variables that Control the Build");

  cm->DefineProperty
    ("CMAKE_EXE_LINKER_FLAGS_[CMAKE_BUILD_TYPE]", cmProperty::VARIABLE,
     "Flag used when linking an executable.",
     "Same as CMAKE_C_FLAGS_* but used by the linker "
     "when creating executables.",false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("CMAKE_LIBRARY_PATH_FLAG", cmProperty::VARIABLE,
     "The flag used to add a library search path to a compiler.",
     "The flag used to specify a library directory to the compiler. "
     "On most compilers this is \"-L\".",false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("CMAKE_LINK_DEF_FILE_FLAG  ", cmProperty::VARIABLE,
     "Linker flag used to specify a .def file for dll creation.",
     "The flag used to add a .def file when creating "
     "a dll on Windows, this is only defined on Windows.",false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("CMAKE_LINK_LIBRARY_FLAG", cmProperty::VARIABLE,
     "Flag used to link a library into an executable.",
     "The flag used to specify a library to link to an executable.  "
     "On most compilers this is \"-l\".",false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("CMAKE_LINK_LIBRARY_FILE_FLAG", cmProperty::VARIABLE,
     "Flag used to link a library specified by a path to its file.",
     "The flag used before a library file path is given to the linker.  "
     "This is needed only on very few platforms.", false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("CMAKE_USE_RELATIVE_PATHS", cmProperty::VARIABLE,
     "Use relative paths (May not work!).",
     "If this is set to TRUE, then the CMake will use "
     "relative paths between the source and binary tree. "
     "This option does not work for more complicated "
     "projects, and relative paths are used when possible.  "
     "In general, it is not possible to move CMake generated"
     " makefiles to a different location regardless "
     "of the value of this variable.",false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("EXECUTABLE_OUTPUT_PATH", cmProperty::VARIABLE,
     "Old executable location variable.",
     "The target property RUNTIME_OUTPUT_DIRECTORY supercedes "
     "this variable for a target if it is set.  "
     "Executable targets are otherwise placed in this directory.",false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("LIBRARY_OUTPUT_PATH", cmProperty::VARIABLE,
     "Old library location variable.",
     "The target properties ARCHIVE_OUTPUT_DIRECTORY, "
     "LIBRARY_OUTPUT_DIRECTORY, and RUNTIME_OUTPUT_DIRECTORY supercede "
     "this variable for a target if they are set.  "
     "Library targets are otherwise placed in this directory.",false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("CMAKE_TRY_COMPILE_CONFIGURATION", cmProperty::VARIABLE,
     "Build configuration used for try_compile and try_run projects.",
     "Projects built by try_compile and try_run are built "
     "synchronously during the CMake configuration step.  "
     "Therefore a specific build configuration must be chosen even "
     "if the generated build system supports multiple configurations.",false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("CMAKE_LINK_INTERFACE_LIBRARIES", cmProperty::VARIABLE,
     "Default value for LINK_INTERFACE_LIBRARIES of targets.",
     "This variable is used to initialize the "
     "LINK_INTERFACE_LIBRARIES property on all the targets. "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("CMAKE_WIN32_EXECUTABLE", cmProperty::VARIABLE,
     "Default value for WIN32_EXECUTABLE of targets.",
     "This variable is used to initialize the "
     "WIN32_EXECUTABLE property on all the targets. "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("CMAKE_MACOSX_BUNDLE", cmProperty::VARIABLE,
     "Default value for MACOSX_BUNDLE of targets.",
     "This variable is used to initialize the "
     "MACOSX_BUNDLE property on all the targets. "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");
  cm->DefineProperty
    ("CMAKE_POSITION_INDEPENDENT_CODE", cmProperty::VARIABLE,
     "Default value for POSITION_INDEPENDENT_CODE of targets.",
     "This variable is used to initialize the "
     "POSITION_INDEPENDENT_CODE property on all the targets. "
     "See that target property for additional information.",
     false,
     "Variables that Control the Build");

//   Variables defined when the a language is enabled These variables will
// also be defined whenever CMake has loaded its support for compiling (LANG)
// programs. This support will be loaded whenever CMake is used to compile
// (LANG) files. C and CXX are examples of the most common values for (LANG).

  cm->DefineProperty
    ("CMAKE_USER_MAKE_RULES_OVERRIDE_<LANG>", cmProperty::VARIABLE,
     "Specify a CMake file that overrides platform information for <LANG>.",
     "This is a language-specific version of "
     "CMAKE_USER_MAKE_RULES_OVERRIDE loaded only when enabling "
     "language <LANG>.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_COMPILER", cmProperty::VARIABLE,
     "The full path to the compiler for LANG.",
     "This is the command that will be used as the <LANG> compiler. "
     "Once set, you can not change this variable.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_COMPILER_ID", cmProperty::VARIABLE,
     "Compiler identification string.",
     "A short string unique to the compiler vendor.  "
     "Possible values include:\n"
     "  Absoft = Absoft Fortran (absoft.com)\n"
     "  ADSP = Analog VisualDSP++ (analog.com)\n"
     "  Clang = LLVM Clang (clang.llvm.org)\n"
     "  Cray = Cray Compiler (cray.com)\n"
     "  Embarcadero, Borland = Embarcadero (embarcadero.com)\n"
     "  G95 = G95 Fortran (g95.org)\n"
     "  GNU = GNU Compiler Collection (gcc.gnu.org)\n"
     "  HP = Hewlett-Packard Compiler (hp.com)\n"
     "  Intel = Intel Compiler (intel.com)\n"
     "  MIPSpro = SGI MIPSpro (sgi.com)\n"
     "  MSVC = Microsoft Visual Studio (microsoft.com)\n"
     "  PGI = The Portland Group (pgroup.com)\n"
     "  PathScale = PathScale (pathscale.com)\n"
     "  SDCC = Small Device C Compiler (sdcc.sourceforge.net)\n"
     "  SunPro = Oracle Solaris Studio (oracle.com)\n"
     "  TI_DSP = Texas Instruments (ti.com)\n"
     "  TinyCC = Tiny C Compiler (tinycc.org)\n"
     "  Watcom = Open Watcom (openwatcom.org)\n"
     "  XL, VisualAge, zOS = IBM XL (ibm.com)\n"
     "This variable is not guaranteed to be defined for all "
     "compilers or languages.",
     false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_PLATFORM_ID", cmProperty::VARIABLE,
     "An internal variable subject to change.",
     "This is used in determining the platform and is subject to change.",
     false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_COMPILER_ABI", cmProperty::VARIABLE,
     "An internal variable subject to change.",
     "This is used in determining the compiler ABI and is subject to change.",
     false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_COMPILER_VERSION", cmProperty::VARIABLE,
     "Compiler version string.",
     "Compiler version in major[.minor[.patch[.tweak]]] format.  "
     "This variable is not guaranteed to be defined for all "
     "compilers or languages.",
     false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_INTERNAL_PLATFORM_ABI", cmProperty::VARIABLE,
     "An internal variable subject to change.",
     "This is used in determining the compiler ABI and is subject to change.",
     false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_SIZEOF_DATA_PTR", cmProperty::VARIABLE,
     "Size of pointer-to-data types for language <LANG>.",
     "This holds the size (in bytes) of pointer-to-data types in the target "
     "platform ABI.  "
     "It is defined for languages C and CXX (C++).",
     false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_COMPILER_IS_GNU<LANG>", cmProperty::VARIABLE,
     "True if the compiler is GNU.",
     "If the selected <LANG> compiler is the GNU "
     "compiler then this is TRUE, if not it is FALSE.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_FLAGS_DEBUG", cmProperty::VARIABLE,
     "Flags for Debug build type or configuration.",
     "<LANG> flags used when CMAKE_BUILD_TYPE is Debug.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_FLAGS_MINSIZEREL", cmProperty::VARIABLE,
     "Flags for MinSizeRel build type or configuration.",
     "<LANG> flags used when CMAKE_BUILD_TYPE is MinSizeRel."
     "Short for minimum size release.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_FLAGS_RELEASE", cmProperty::VARIABLE,
     "Flags for Release build type or configuration.",
     "<LANG> flags used when CMAKE_BUILD_TYPE is Release",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_FLAGS_RELWITHDEBINFO", cmProperty::VARIABLE,
     "Flags for RelWithDebInfo type or configuration.",
     "<LANG> flags used when CMAKE_BUILD_TYPE is RelWithDebInfo. "
     "Short for Release With Debug Information.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_COMPILE_OBJECT", cmProperty::VARIABLE,
     "Rule variable to compile a single object file.",
     "This is a rule variable that tells CMake how to "
     "compile a single object file for for the language <LANG>.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_CREATE_SHARED_LIBRARY", cmProperty::VARIABLE,
     "Rule variable to create a shared library.",
     "This is a rule variable that tells CMake how to "
     "create a shared library for the language <LANG>.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_CREATE_SHARED_MODULE", cmProperty::VARIABLE,
     "Rule variable to create a shared module.",
     "This is a rule variable that tells CMake how to "
     "create a shared library for the language <LANG>.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_CREATE_STATIC_LIBRARY", cmProperty::VARIABLE,
     "Rule variable to create a static library.",
     "This is a rule variable that tells CMake how "
     "to create a static library for the language <LANG>.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_ARCHIVE_CREATE", cmProperty::VARIABLE,
     "Rule variable to create a new static archive.",
     "This is a rule variable that tells CMake how to create a static "
     "archive.  It is used in place of CMAKE_<LANG>_CREATE_STATIC_LIBRARY "
     "on some platforms in order to support large object counts.  "
     "See also CMAKE_<LANG>_ARCHIVE_APPEND and CMAKE_<LANG>_ARCHIVE_FINISH.",
     false, "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_ARCHIVE_APPEND", cmProperty::VARIABLE,
     "Rule variable to append to a static archive.",
     "This is a rule variable that tells CMake how to append to a static "
     "archive.  It is used in place of CMAKE_<LANG>_CREATE_STATIC_LIBRARY "
     "on some platforms in order to support large object counts.  "
     "See also CMAKE_<LANG>_ARCHIVE_CREATE and CMAKE_<LANG>_ARCHIVE_FINISH.",
     false, "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_ARCHIVE_FINISH", cmProperty::VARIABLE,
     "Rule variable to finish an existing static archive.",
     "This is a rule variable that tells CMake how to finish a static "
     "archive.  It is used in place of CMAKE_<LANG>_CREATE_STATIC_LIBRARY "
     "on some platforms in order to support large object counts.  "
     "See also CMAKE_<LANG>_ARCHIVE_CREATE and CMAKE_<LANG>_ARCHIVE_APPEND.",
     false, "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_IGNORE_EXTENSIONS", cmProperty::VARIABLE,
     "File extensions that should be ignored by the build.",
     "This is a list of file extensions that may be "
     "part of a project for a given language but are not compiled. ",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_IMPLICIT_INCLUDE_DIRECTORIES", cmProperty::VARIABLE,
     "Directories implicitly searched by the compiler for header files.",
     "CMake does not explicitly specify these directories on compiler "
     "command lines for language <LANG>.  "
     "This prevents system include directories from being treated as user "
     "include directories on some compilers.", false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_IMPLICIT_LINK_DIRECTORIES", cmProperty::VARIABLE,
     "Implicit linker search path detected for language <LANG>.",
     "Compilers typically pass directories containing language runtime "
     "libraries and default library search paths when they invoke a linker.  "
     "These paths are implicit linker search directories for the compiler's "
     "language.  "
     "CMake automatically detects these directories for each language and "
     "reports the results in this variable."
     "\n"
     "When a library in one of these directories is given by full path to "
     "target_link_libraries() CMake will generate the -l<name> form on "
     "link lines to ensure the linker searches its implicit directories "
     "for the library.  "
     "Note that some toolchains read implicit directories from an "
     "environment variable such as LIBRARY_PATH so keep its value "
     "consistent when operating in a given build tree.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_IMPLICIT_LINK_FRAMEWORK_DIRECTORIES", cmProperty::VARIABLE,
     "Implicit linker framework search path detected for language <LANG>.",
     "These paths are implicit linker framework search directories for "
     "the compiler's language.  "
     "CMake automatically detects these directories for each language and "
     "reports the results in this variable.", false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_IMPLICIT_LINK_LIBRARIES", cmProperty::VARIABLE,
     "Implicit link libraries and flags detected for language <LANG>.",
     "Compilers typically pass language runtime library names and "
     "other flags when they invoke a linker.  "
     "These flags are implicit link options for the compiler's language.  "
     "CMake automatically detects these libraries and flags for each "
     "language and reports the results in this variable.", false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_LIBRARY_ARCHITECTURE", cmProperty::VARIABLE,
     "Target architecture library directory name detected for <lang>.",
     "If the <lang> compiler passes to the linker an architecture-specific "
     "system library search directory such as <prefix>/lib/<arch> this "
     "variable contains the <arch> name if/as detected by CMake.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_LINKER_PREFERENCE_PROPAGATES", cmProperty::VARIABLE,
     "True if CMAKE_<LANG>_LINKER_PREFERENCE propagates across targets.",
     "This is used when CMake selects a linker language for a target.  "
     "Languages compiled directly into the target are always considered.  "
     "A language compiled into static libraries linked by the target is "
     "considered if this variable is true.", false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_LINKER_PREFERENCE", cmProperty::VARIABLE,
     "Preference value for linker language selection.",
     "The \"linker language\" for executable, shared library, and module "
     "targets is the language whose compiler will invoke the linker.  "
     "The LINKER_LANGUAGE target property sets the language explicitly.  "
     "Otherwise, the linker language is that whose linker preference value "
     "is highest among languages compiled and linked into the target.  "
     "See also the CMAKE_<LANG>_LINKER_PREFERENCE_PROPAGATES variable.",
     false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_LINK_EXECUTABLE ", cmProperty::VARIABLE,
     "Rule variable to link and executable.",
     "Rule variable to link and executable for the given language.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_OUTPUT_EXTENSION", cmProperty::VARIABLE,
     "Extension for the output of a compile for a single file.",
     "This is the extension for an object file for "
     "the given <LANG>. For example .obj for C on Windows.",false,
     "Variables for Languages");

  cm->DefineProperty
    ("CMAKE_<LANG>_SOURCE_FILE_EXTENSIONS", cmProperty::VARIABLE,
     "Extensions of source files for the given language.",
     "This is the list of extensions for a "
     "given languages source files.",false,"Variables for Languages");

  cm->DefineProperty(
    "CMAKE_<LANG>_COMPILER_LOADED", cmProperty::VARIABLE,
    "Defined to true if the language is enabled.",
    "When language <LANG> is enabled by project() or enable_language() "
    "this variable is defined to 1.",
    false,"Variables for Languages");

  cm->DefineProperty(
    "CMAKE_Fortran_MODDIR_FLAG", cmProperty::VARIABLE,
    "Fortran flag for module output directory.",
    "This stores the flag needed to pass the value of the "
    "Fortran_MODULE_DIRECTORY target property to the compiler.",
    false,"Variables for Languages");

  cm->DefineProperty(
    "CMAKE_Fortran_MODDIR_DEFAULT", cmProperty::VARIABLE,
    "Fortran default module output directory.",
    "Most Fortran compilers write .mod files to the current working "
    "directory.  "
    "For those that do not, this is set to \".\" and used when the "
    "Fortran_MODULE_DIRECTORY target property is not set.",
    false,"Variables for Languages");

  cm->DefineProperty(
    "CMAKE_Fortran_MODOUT_FLAG", cmProperty::VARIABLE,
    "Fortran flag to enable module output.",
    "Most Fortran compilers write .mod files out by default.  "
    "For others, this stores the flag needed to enable module output.",
    false,"Variables for Languages");

  // variables that are used by cmake but not to be documented
  cm->DefineProperty("CMAKE_MATCH_0", cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_MATCH_1", cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_MATCH_2", cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_MATCH_3", cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_MATCH_4", cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_MATCH_5", cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_MATCH_6", cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_MATCH_7", cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_MATCH_8", cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_MATCH_9", cmProperty::VARIABLE,0,0);

  cm->DefineProperty("CMAKE_<LANG>_COMPILER_ARG1",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_COMPILER_ENV_VAR",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_COMPILER_ID_RUN",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_ABI_FILES",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_CREATE_ASSEMBLY_SOURCE",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_CREATE_PREPROCESSED_SOURCE",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_FLAGS_DEBUG_INIT",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_FLAGS_INIT",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_FLAGS_MINSIZEREL_INIT",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_FLAGS_RELEASE_INIT",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_FLAGS_RELWITHDEBINFO_INIT",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_INFORMATION_LOADED",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_LINK_EXECUTABLE",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_LINK_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_RESPONSE_FILE_LINK_FLAG",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_STANDARD_LIBRARIES",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_STANDARD_LIBRARIES_INIT",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_USE_RESPONSE_FILE_FOR_INCLUDES",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_<LANG>_USE_RESPONSE_FILE_FOR_OBJECTS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_EXECUTABLE_SUFFIX_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_EXE_LINK_DYNAMIC_<LANG>_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_EXE_LINK_STATIC_<LANG>_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_GENERATOR_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_IMPORT_LIBRARY_PREFIX_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_IMPORT_LIBRARY_SUFFIX_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_INCLUDE_FLAG_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_INCLUDE_FLAG_SEP_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_INCLUDE_SYSTEM_FLAG_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_NEEDS_REQUIRES_STEP_<LANG>_FLAG",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_LIBRARY_CREATE_<LANG>_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_LIBRARY_<LANG>_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_LIBRARY_LINK_<LANG>_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_LIBRARY_LINK_DYNAMIC_<LANG>_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_LIBRARY_LINK_STATIC_<LANG>_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_LIBRARY_PREFIX_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_LIBRARY_SUFFIX_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_LIBRARY_RUNTIME_<LANG>_FLAG",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_LIBRARY_RUNTIME_<LANG>_FLAG_SEP",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_LIBRARY_RPATH_LINK_<LANG>_FLAG",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_EXECUTABLE_RUNTIME_<LANG>_FLAG",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_EXECUTABLE_RUNTIME_<LANG>_FLAG_SEP",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_EXECUTABLE_RPATH_LINK_<LANG>_FLAG",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_PLATFORM_REQUIRED_RUNTIME_PATH",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_MODULE_CREATE_<LANG>_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_MODULE_<LANG>_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_MODULE_LINK_DYNAMIC_<LANG>_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_MODULE_LINK_STATIC_<LANG>_FLAGS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_MODULE_PREFIX_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_SHARED_MODULE_SUFFIX_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_STATIC_LIBRARY_PREFIX_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_STATIC_LIBRARY_SUFFIX_<LANG>",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_LINK_DEPENDENT_LIBRARY_FILES",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_LINK_DEPENDENT_LIBRARY_DIRS",
                     cmProperty::VARIABLE,0,0);
  cm->DefineProperty("CMAKE_MAKE_INCLUDE_FROM_ROOT",
                     cmProperty::VARIABLE,0,0);
}
