/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <bitset>
#include <string>

class cmMakefile;

#define CM_FOR_EACH_POLICY_TABLE(POLICY, SELECT)                              \
  SELECT(POLICY, CMP0000,                                                     \
         "A minimum required CMake version must be specified.", 2, 6, 0,      \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0001,                                                     \
         "CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.", 2, 6, 0,  \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0002, "Logical target names must be globally unique.", 2, \
         6, 0, cmPolicies::WARN)                                              \
  SELECT(                                                                     \
    POLICY, CMP0003,                                                          \
    "Libraries linked via full path no longer produce linker search paths.",  \
    2, 6, 0, cmPolicies::WARN)                                                \
  SELECT(POLICY, CMP0004,                                                     \
         "Libraries linked may not have leading or trailing whitespace.", 2,  \
         6, 0, cmPolicies::WARN)                                              \
  SELECT(POLICY, CMP0005,                                                     \
         "Preprocessor definition values are now escaped automatically.", 2,  \
         6, 0, cmPolicies::WARN)                                              \
  SELECT(POLICY, CMP0006,                                                     \
         "Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.",   \
         2, 6, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0007, "list command no longer ignores empty elements.",   \
         2, 6, 0, cmPolicies::WARN)                                           \
  SELECT(                                                                     \
    POLICY, CMP0008,                                                          \
    "Libraries linked by full-path must have a valid library file name.", 2,  \
    6, 1, cmPolicies::WARN)                                                   \
  SELECT(POLICY, CMP0009,                                                     \
         "FILE GLOB_RECURSE calls should not follow symlinks by default.", 2, \
         6, 2, cmPolicies::WARN)                                              \
  SELECT(POLICY, CMP0010, "Bad variable reference syntax is an error.", 2, 6, \
         3, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0011,                                                     \
         "Included scripts do automatic cmake_policy PUSH and POP.", 2, 6, 3, \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0012, "if() recognizes numbers and boolean constants.",   \
         2, 8, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0013, "Duplicate binary directories are not allowed.", 2, \
         8, 0, cmPolicies::WARN)                                              \
  SELECT(POLICY, CMP0014, "Input directories must have CMakeLists.txt.", 2,   \
         8, 0, cmPolicies::WARN)                                              \
  SELECT(POLICY, CMP0015,                                                     \
         "link_directories() treats paths relative to the source dir.", 2, 8, \
         1, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0016,                                                     \
         "target_link_libraries() reports error if its only argument "        \
         "is not a target.",                                                  \
         2, 8, 3, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0017,                                                     \
         "Prefer files from the CMake module directory when including from "  \
         "there.",                                                            \
         2, 8, 4, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0018,                                                     \
         "Ignore CMAKE_SHARED_LIBRARY_<Lang>_FLAGS variable.", 2, 8, 9,       \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0019,                                                     \
         "Do not re-expand variables in include and link information.", 2, 8, \
         11, cmPolicies::WARN)                                                \
  SELECT(POLICY, CMP0020,                                                     \
         "Automatically link Qt executables to qtmain target on Windows.", 2, \
         8, 11, cmPolicies::WARN)                                             \
  SELECT(                                                                     \
    POLICY, CMP0021,                                                          \
    "Fatal error on relative paths in INCLUDE_DIRECTORIES target property.",  \
    2, 8, 12, cmPolicies::WARN)                                               \
  SELECT(POLICY, CMP0022,                                                     \
         "INTERFACE_LINK_LIBRARIES defines the link interface.", 2, 8, 12,    \
         cmPolicies::WARN)                                                    \
  SELECT(                                                                     \
    POLICY, CMP0023,                                                          \
    "Plain and keyword target_link_libraries signatures cannot be mixed.", 2, \
    8, 12, cmPolicies::WARN)                                                  \
  SELECT(POLICY, CMP0024, "Disallow include export result.", 3, 0, 0,         \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0025, "Compiler id for Apple Clang is now AppleClang.",   \
         3, 0, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0026, "Disallow use of the LOCATION target property.", 3, \
         0, 0, cmPolicies::WARN)                                              \
  SELECT(POLICY, CMP0027,                                                     \
         "Conditionally linked imported targets with missing include "        \
         "directories.",                                                      \
         3, 0, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0028,                                                     \
         "Double colon in target name means ALIAS or IMPORTED target.", 3, 0, \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0029, "The subdir_depends command should not be called.", \
         3, 0, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0030,                                                     \
         "The use_mangled_mesa command should not be called.", 3, 0, 0,       \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0031, "The load_command command should not be called.",   \
         3, 0, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0032,                                                     \
         "The output_required_files command should not be called.", 3, 0, 0,  \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0033,                                                     \
         "The export_library_dependencies command should not be called.", 3,  \
         0, 0, cmPolicies::WARN)                                              \
  SELECT(POLICY, CMP0034, "The utility_source command should not be called.", \
         3, 0, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0035,                                                     \
         "The variable_requires command should not be called.", 3, 0, 0,      \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0036, "The build_name command should not be called.", 3,  \
         0, 0, cmPolicies::WARN)                                              \
  SELECT(POLICY, CMP0037,                                                     \
         "Target names should not be reserved and should match a validity "   \
         "pattern.",                                                          \
         3, 0, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0038, "Targets may not link directly to themselves.", 3,  \
         0, 0, cmPolicies::WARN)                                              \
  SELECT(POLICY, CMP0039, "Utility targets may not have link dependencies.",  \
         3, 0, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0040,                                                     \
         "The target in the TARGET signature of add_custom_command() must "   \
         "exist and must be defined in the current directory.",               \
         3, 0, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0041,                                                     \
         "Error on relative include with generator expression.", 3, 0, 0,     \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0042, "MACOSX_RPATH is enabled by default.", 3, 0, 0,     \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0043, "Ignore COMPILE_DEFINITIONS_<Config> properties.",  \
         3, 0, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0044,                                                     \
         "Case sensitive <LANG>_COMPILER_ID generator expressions.", 3, 0, 0, \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0045,                                                     \
         "Error on non-existent target in get_target_property.", 3, 0, 0,     \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0046,                                                     \
         "Error on non-existent dependency in add_dependencies.", 3, 0, 0,    \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0047, "Use QCC compiler id for the qcc drivers on QNX.",  \
         3, 0, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0048, "project() command manages VERSION variables.", 3,  \
         0, 0, cmPolicies::WARN)                                              \
  SELECT(POLICY, CMP0049,                                                     \
         "Do not expand variables in target source entries.", 3, 0, 0,        \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0050, "Disallow add_custom_command SOURCE signatures.",   \
         3, 0, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0051, "List TARGET_OBJECTS in SOURCES target property.",  \
         3, 1, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0052,                                                     \
         "Reject source and build dirs in installed "                         \
         "INTERFACE_INCLUDE_DIRECTORIES.",                                    \
         3, 1, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0053,                                                     \
         "Simplify variable reference and escape sequence evaluation.", 3, 1, \
         0, cmPolicies::WARN)                                                 \
  SELECT(                                                                     \
    POLICY, CMP0054,                                                          \
    "Only interpret if() arguments as variables or keywords when unquoted.",  \
    3, 1, 0, cmPolicies::WARN)                                                \
  SELECT(POLICY, CMP0055, "Strict checking for break() command.", 3, 2, 0,    \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0056,                                                     \
         "Honor link flags in try_compile() source-file signature.", 3, 2, 0, \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0057, "Support new IN_LIST if() operator.", 3, 3, 0,      \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0058,                                                     \
         "Ninja requires custom command byproducts to be explicit.", 3, 3, 0, \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0059,                                                     \
         "Do not treat DEFINITIONS as a built-in directory property.", 3, 3,  \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0060,                                                     \
         "Link libraries by full path even in implicit directories.", 3, 3,   \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0061,                                                     \
         "CTest does not by default tell make to ignore errors (-i).", 3, 3,  \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0062, "Disallow install() of export() result.", 3, 3, 0,  \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0063,                                                     \
         "Honor visibility properties for all target types.", 3, 3, 0,        \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0064, "Support new TEST if() operator.", 3, 4, 0,         \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0065,                                                     \
         "Do not add flags to export symbols from executables without "       \
         "the ENABLE_EXPORTS target property.",                               \
         3, 4, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0066,                                                     \
         "Honor per-config flags in try_compile() source-file signature.", 3, \
         7, 0, cmPolicies::WARN)                                              \
  SELECT(POLICY, CMP0067,                                                     \
         "Honor language standard in try_compile() source-file signature.",   \
         3, 8, 0, cmPolicies::WARN)                                           \
  SELECT(POLICY, CMP0068,                                                     \
         "RPATH settings on macOS do not affect install_name.", 3, 9, 0,      \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0069,                                                     \
         "INTERPROCEDURAL_OPTIMIZATION is enforced when enabled.", 3, 9, 0,   \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0070,                                                     \
         "Define file(GENERATE) behavior for relative paths.", 3, 10, 0,      \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0071, "Let AUTOMOC and AUTOUIC process GENERATED files.", \
         3, 10, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0072,                                                     \
         "FindOpenGL prefers GLVND by default when available.", 3, 11, 0,     \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0073,                                                     \
         "Do not produce legacy _LIB_DEPENDS cache entries.", 3, 12, 0,       \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0074, "find_package uses <PackageName>_ROOT variables.",  \
         3, 12, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0075,                                                     \
         "Include file check macros honor CMAKE_REQUIRED_LIBRARIES.", 3, 12,  \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0076,                                                     \
         "target_sources() command converts relative paths to absolute.", 3,  \
         13, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0077, "option() honors normal variables.", 3, 13, 0,      \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0078, "UseSWIG generates standard target names.", 3, 13,  \
         0, cmPolicies::WARN)                                                 \
  SELECT(                                                                     \
    POLICY, CMP0079,                                                          \
    "target_link_libraries allows use with targets in other directories.", 3, \
    13, 0, cmPolicies::WARN)                                                  \
  SELECT(POLICY, CMP0080,                                                     \
         "BundleUtilities cannot be included at configure time.", 3, 13, 0,   \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0081,                                                     \
         "Relative paths not allowed in LINK_DIRECTORIES target property.",   \
         3, 13, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0082,                                                     \
         "Install rules from add_subdirectory() are interleaved with those "  \
         "in caller.",                                                        \
         3, 14, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0083, "Add PIE options when linking executable.", 3, 14,  \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0084,                                                     \
         "The FindQt module does not exist for find_package().", 3, 14, 0,    \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0085, "$<IN_LIST:...> handles empty list items.", 3, 14,  \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0086,                                                     \
         "UseSWIG honors SWIG_MODULE_NAME via -module flag.", 3, 14, 0,       \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0087,                                                     \
         "Install CODE|SCRIPT allow the use of generator "                    \
         "expressions.",                                                      \
         3, 14, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0088,                                                     \
         "FindBISON runs bison in CMAKE_CURRENT_BINARY_DIR when executing.",  \
         3, 14, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0089,                                                     \
         "Compiler id for IBM Clang-based XL compilers is now XLClang.", 3,   \
         15, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0090,                                                     \
         "export(PACKAGE) does not populate package registry by default.", 3, \
         15, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0091,                                                     \
         "MSVC runtime library flags are selected by an abstraction.", 3, 15, \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0092,                                                     \
         "MSVC warning flags are not in CMAKE_<LANG>_FLAGS by default.", 3,   \
         15, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0093, "FindBoost reports Boost_VERSION in x.y.z format.", \
         3, 15, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0094,                                                     \
         "FindPython3,  FindPython2 and FindPyton use "                       \
         "LOCATION for lookup strategy.",                                     \
         3, 15, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0095,                                                     \
         "RPATH entries are properly escaped in the intermediary CMake "      \
         "install script.",                                                   \
         3, 16, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0096,                                                     \
         "project() preserves leading zeros in version components.", 3, 16,   \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0097,                                                     \
         "ExternalProject_Add with GIT_SUBMODULES \"\" initializes no "       \
         "submodules.",                                                       \
         3, 16, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0098,                                                     \
         "FindFLEX runs flex in CMAKE_CURRENT_BINARY_DIR when executing.", 3, \
         17, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0099,                                                     \
         "Link properties are transitive over private dependency on static "  \
         "libraries.",                                                        \
         3, 17, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0100, "Let AUTOMOC and AUTOUIC process .hh files.", 3,    \
         17, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0101,                                                     \
         "target_compile_options honors BEFORE keyword in all scopes.", 3,    \
         17, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0102,                                                     \
         "mark_as_advanced() does nothing if a cache entry does not exist.",  \
         3, 17, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0103,                                                     \
         "Multiple export() with same FILE without APPEND is not allowed.",   \
         3, 18, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0104,                                                     \
         "CMAKE_CUDA_ARCHITECTURES now detected for NVCC, empty "             \
         "CUDA_ARCHITECTURES not allowed.",                                   \
         3, 18, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0105, "Device link step uses the link options.", 3, 18,   \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0106, "The Documentation module is removed.", 3, 18, 0,   \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0107, "An ALIAS target cannot overwrite another target.", \
         3, 18, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0108, "A target cannot link to itself through an alias.", \
         3, 18, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0109,                                                     \
         "find_program() requires permission to execute but not to read.", 3, \
         19, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0110,                                                     \
         "add_test() supports arbitrary characters in test names.", 3, 19, 0, \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0111,                                                     \
         "An imported target missing its location property fails during "     \
         "generation.",                                                       \
         3, 19, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0112,                                                     \
         "Target file component generator expressions do not add target "     \
         "dependencies.",                                                     \
         3, 19, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0113,                                                     \
         "Makefile generators do not repeat custom commands from target "     \
         "dependencies.",                                                     \
         3, 19, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0114,                                                     \
         "ExternalProject step targets fully adopt their steps.", 3, 19, 0,   \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0115, "Source file extensions must be explicit.", 3, 20,  \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0116,                                                     \
         "Ninja generators transform DEPFILEs from add_custom_command().", 3, \
         20, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0117,                                                     \
         "MSVC RTTI flag /GR is not added to CMAKE_CXX_FLAGS by default.", 3, \
         20, 0, cmPolicies::WARN)                                             \
  SELECT(                                                                     \
    POLICY, CMP0118,                                                          \
    "The GENERATED source file property is now visible in all directories.",  \
    3, 20, 0, cmPolicies::WARN)                                               \
  SELECT(POLICY, CMP0119,                                                     \
         "LANGUAGE source file property explicitly compiles as specified "    \
         "language.",                                                         \
         3, 20, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0120,                                                     \
         "The WriteCompilerDetectionHeader module is removed.", 3, 20, 0,     \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0121,                                                     \
         "The list() command now validates parsing of index arguments.", 3,   \
         21, 0, cmPolicies::WARN)                                             \
  SELECT(                                                                     \
    POLICY, CMP0122,                                                          \
    "UseSWIG use standard library name conventions for csharp language.", 3,  \
    21, 0, cmPolicies::WARN)                                                  \
  SELECT(POLICY, CMP0123,                                                     \
         "ARMClang cpu/arch compile and link flags must be set explicitly.",  \
         3, 21, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0124,                                                     \
         "foreach() loop variables are only available in the loop scope.", 3, \
         21, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0125,                                                     \
         "find_(path|file|library|program) have consistent behavior for "     \
         "cache variables.",                                                  \
         3, 21, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0126,                                                     \
         "set(CACHE) does not remove a normal variable of the same name.", 3, \
         21, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0127,                                                     \
         "cmake_dependent_option() supports full Condition Syntax.", 3, 22,   \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0128,                                                     \
         "Selection of language standard and extension flags improved.", 3,   \
         22, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0129,                                                     \
         "Compiler id for MCST LCC compilers is now LCC, not GNU.", 3, 23, 0, \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0130, "while() diagnoses condition evaluation errors.",   \
         3, 24, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0131,                                                     \
         "LINK_LIBRARIES supports the LINK_ONLY generator expression.", 3,    \
         24, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0132,                                                     \
         "Do not set compiler environment variables on first run", 3, 24, 0,  \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0133,                                                     \
         "The CPack module disables SLA by default in the CPack DragNDrop "   \
         "Generator.",                                                        \
         3, 24, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0134,                                                     \
         "Fallback to \"HOST\" Windows registry view when \"TARGET\" view "   \
         "is not usable.",                                                    \
         3, 24, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0135,                                                     \
         "ExternalProject ignores timestamps in archives by default for the " \
         "URL download method",                                               \
         3, 24, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0136,                                                     \
         "Watcom runtime library flags are selected by an abstraction.", 3,   \
         24, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0137,                                                     \
         "try_compile() passes platform variables in project mode", 3, 24, 0, \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0138,                                                     \
         "CheckIPOSupported uses flags from calling project.", 3, 24, 0,      \
         cmPolicies::WARN)                                                    \
  SELECT(                                                                     \
    POLICY, CMP0139,                                                          \
    "The if() command supports path comparisons using PATH_EQUAL operator.",  \
    3, 24, 0, cmPolicies::WARN)                                               \
  SELECT(POLICY, CMP0140, "The return() command checks its arguments.", 3,    \
         25, 0, cmPolicies::WARN)                                             \
  SELECT(                                                                     \
    POLICY, CMP0141,                                                          \
    "MSVC debug information format flags are selected by an abstraction.", 3, \
    25, 0, cmPolicies::WARN)                                                  \
  SELECT(POLICY, CMP0142,                                                     \
         "The Xcode generator does not append per-config suffixes to "        \
         "library search paths.",                                             \
         3, 25, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0143,                                                     \
         "Global property USE_FOLDERS treated as ON by default", 3, 26, 0,    \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0144,                                                     \
         "find_package uses upper-case <PACKAGENAME>_ROOT variables.", 3, 27, \
         0, cmPolicies::WARN)                                                 \
  SELECT(POLICY, CMP0145, "The Dart and FindDart modules are removed.", 3,    \
         27, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0146, "The FindCUDA module is removed.", 3, 27, 0,        \
         cmPolicies::WARN)                                                    \
  SELECT(POLICY, CMP0147,                                                     \
         "Visual Studio generators build custom commands in parallel.", 3,    \
         27, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0148,                                                     \
         "The FindPythonInterp and FindPythonLibs modules are removed.", 3,   \
         27, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0149,                                                     \
         "Visual Studio generators select latest Windows SDK by default.", 3, \
         27, 0, cmPolicies::WARN)                                             \
  SELECT(POLICY, CMP0150,                                                     \
         "ExternalProject_Add and FetchContent_Declare commands "             \
         "treat relative GIT_REPOSITORY paths as being relative "             \
         "to the parent project's remote.",                                   \
         3, 27, 0, cmPolicies::WARN)                                          \
  SELECT(POLICY, CMP0151,                                                     \
         "AUTOMOC include directory is a system include directory by "        \
         "default.",                                                          \
         3, 27, 0, cmPolicies::WARN)                                          \
  SELECT(                                                                     \
    POLICY, CMP0152,                                                          \
    "file(REAL_PATH) resolves symlinks before collapsing ../ components.", 3, \
    28, 0, cmPolicies::WARN)                                                  \
  SELECT(POLICY, CMP0153, "The exec_program command should not be called.",   \
         3, 28, 0, cmPolicies::WARN)                                          \
  SELECT(                                                                     \
    POLICY, CMP0154,                                                          \
    "Generated files are private by default in targets using file sets.", 3,  \
    28, 0, cmPolicies::WARN)                                                  \
  SELECT(POLICY, CMP0155,                                                     \
         "C++ sources in targets with at least C++20 are scanned for "        \
         "imports when supported.",                                           \
         3, 28, 0, cmPolicies::WARN)

#define CM_SELECT_ID(F, A1, A2, A3, A4, A5, A6) F(A1)
#define CM_FOR_EACH_POLICY_ID(POLICY)                                         \
  CM_FOR_EACH_POLICY_TABLE(POLICY, CM_SELECT_ID)

#define CM_FOR_EACH_TARGET_POLICY(F)                                          \
  F(CMP0003)                                                                  \
  F(CMP0004)                                                                  \
  F(CMP0008)                                                                  \
  F(CMP0020)                                                                  \
  F(CMP0021)                                                                  \
  F(CMP0022)                                                                  \
  F(CMP0027)                                                                  \
  F(CMP0037)                                                                  \
  F(CMP0038)                                                                  \
  F(CMP0041)                                                                  \
  F(CMP0042)                                                                  \
  F(CMP0046)                                                                  \
  F(CMP0052)                                                                  \
  F(CMP0060)                                                                  \
  F(CMP0063)                                                                  \
  F(CMP0065)                                                                  \
  F(CMP0068)                                                                  \
  F(CMP0069)                                                                  \
  F(CMP0073)                                                                  \
  F(CMP0076)                                                                  \
  F(CMP0081)                                                                  \
  F(CMP0083)                                                                  \
  F(CMP0095)                                                                  \
  F(CMP0099)                                                                  \
  F(CMP0104)                                                                  \
  F(CMP0105)                                                                  \
  F(CMP0108)                                                                  \
  F(CMP0112)                                                                  \
  F(CMP0113)                                                                  \
  F(CMP0119)                                                                  \
  F(CMP0131)                                                                  \
  F(CMP0142)                                                                  \
  F(CMP0154)                                                                  \
  F(CMP0155)

#define CM_FOR_EACH_CUSTOM_COMMAND_POLICY(F)                                  \
  F(CMP0116)                                                                  \
  F(CMP0147)

/** \class cmPolicies
 * \brief Handles changes in CMake behavior and policies
 *
 * See the cmake-policies(7) manual for an overview of this class's purpose.
 */
class cmPolicies
{
public:
  /// Status of a policy
  enum PolicyStatus
  {
    OLD,  ///< Use old behavior
    WARN, ///< Use old behavior but issue a warning
    NEW,  ///< Use new behavior
    /// Issue an error if user doesn't set policy status to NEW and hits the
    /// check
    REQUIRED_IF_USED,
    REQUIRED_ALWAYS ///< Issue an error unless user sets policy status to NEW.
  };

  /// Policy identifiers
  enum PolicyID
  {
#define POLICY_ENUM(POLICY_ID) POLICY_ID,
    CM_FOR_EACH_POLICY_ID(POLICY_ENUM)
#undef POLICY_ENUM

    /** \brief Always the last entry.
     *
     * Useful mostly to avoid adding a comma the last policy when adding a new
     * one.
     */
    CMPCOUNT
  };

  //! convert a string policy ID into a number
  static bool GetPolicyID(const char* id, /* out */ cmPolicies::PolicyID& pid);

  //! Get the default status for a policy
  static cmPolicies::PolicyStatus GetPolicyStatus(cmPolicies::PolicyID id);

  enum class WarnCompat
  {
    Off,
    On
  };

  //! Set a policy level for this listfile
  static bool ApplyPolicyVersion(cmMakefile* mf,
                                 std::string const& version_min,
                                 std::string const& version_max,
                                 WarnCompat warnCompat);
  static bool ApplyPolicyVersion(cmMakefile* mf, unsigned int majorVer,
                                 unsigned int minorVer, unsigned int patchVer,
                                 WarnCompat warnCompat);

  //! return a warning string for a given policy
  static std::string GetPolicyWarning(cmPolicies::PolicyID id);
  static std::string GetPolicyDeprecatedWarning(cmPolicies::PolicyID id);

  //! return an error string for when a required policy is unspecified
  static std::string GetRequiredPolicyError(cmPolicies::PolicyID id);

  //! return an error string for when a required policy is unspecified
  static std::string GetRequiredAlwaysPolicyError(cmPolicies::PolicyID id);

  /** Represent a set of policy values.  */
  struct PolicyMap
  {
    PolicyStatus Get(PolicyID id) const;
    void Set(PolicyID id, PolicyStatus status);
    bool IsDefined(PolicyID id) const;
    bool IsEmpty() const;

  private:
#define POLICY_STATUS_COUNT 3
    std::bitset<cmPolicies::CMPCOUNT * POLICY_STATUS_COUNT> Status;
  };
};
