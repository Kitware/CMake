#include "cmPolicies.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmSourceFile.h"
#include "cmVersion.h"
#include "cmVersionMacros.h"
#include <map>
#include <set>
#include <queue>
#include <assert.h>

const char* cmPolicies::PolicyStatusNames[] = {
  "OLD", "WARN", "NEW", "REQUIRED_IF_USED", "REQUIRED_ALWAYS"
};

class cmPolicy
{
public:
  cmPolicy(cmPolicies::PolicyID iD,
            const char *idString,
            const char *shortDescription,
            const char *longDescription,
            unsigned int majorVersionIntroduced,
            unsigned int minorVersionIntroduced,
            unsigned int patchVersionIntroduced,
            unsigned int tweakVersionIntroduced,
            cmPolicies::PolicyStatus status)
  {
    if (!idString || !shortDescription || ! longDescription)
    {
      cmSystemTools::Error("Attempt to define a policy without "
        "all parameters being specified!");
      return;
    }
    this->ID = iD;
    this->IDString = idString;
    this->ShortDescription = shortDescription;
    this->LongDescription = longDescription;
    this->MajorVersionIntroduced = majorVersionIntroduced;
    this->MinorVersionIntroduced = minorVersionIntroduced;
    this->PatchVersionIntroduced = patchVersionIntroduced;
    this->TweakVersionIntroduced = tweakVersionIntroduced;
    this->Status = status;
  }

  std::string GetVersionString()
  {
    cmOStringStream v;
    v << this->MajorVersionIntroduced << "." << this->MinorVersionIntroduced;
    v << "." << this->PatchVersionIntroduced;
    if(this->TweakVersionIntroduced > 0)
      {
      v << "." << this->TweakVersionIntroduced;
      }
    return v.str();
  }

  bool IsPolicyNewerThan(unsigned int majorV,
                         unsigned int minorV,
                         unsigned int patchV,
                         unsigned int tweakV)
  {
    if (majorV < this->MajorVersionIntroduced)
    {
      return true;
    }
    if (majorV > this->MajorVersionIntroduced)
    {
      return false;
    }
    if (minorV < this->MinorVersionIntroduced)
    {
      return true;
    }
    if (minorV > this->MinorVersionIntroduced)
    {
      return false;
    }
    if (patchV < this->PatchVersionIntroduced)
    {
      return true;
    }
    if (patchV > this->PatchVersionIntroduced)
    {
      return false;
    }
    return (tweakV < this->TweakVersionIntroduced);
  }

  cmPolicies::PolicyID ID;
  std::string IDString;
  std::string ShortDescription;
  std::string LongDescription;
  unsigned int MajorVersionIntroduced;
  unsigned int MinorVersionIntroduced;
  unsigned int PatchVersionIntroduced;
  unsigned int TweakVersionIntroduced;
  cmPolicies::PolicyStatus Status;
};

cmPolicies::cmPolicies()
{
  // define all the policies
  this->DefinePolicy(
    CMP0000, "CMP0000",
    "A minimum required CMake version must be specified.",
    "CMake requires that projects specify the version of CMake to which "
    "they have been written.  "
    "This policy has been put in place so users trying to build the project "
    "may be told when they need to update their CMake.  "
    "Specifying a version also helps the project build with CMake versions "
    "newer than that specified.  "
    "Use the cmake_minimum_required command at the top of your main "
    " CMakeLists.txt file:\n"
    "  cmake_minimum_required(VERSION <major>.<minor>)\n"
    "where \"<major>.<minor>\" is the version of CMake you want to support "
    "(such as \"2.6\").  "
    "The command will ensure that at least the given version of CMake is "
    "running and help newer versions be compatible with the project.  "
    "See documentation of cmake_minimum_required for details.\n"
    "Note that the command invocation must appear in the CMakeLists.txt "
    "file itself; a call in an included file is not sufficient.  "
    "However, the cmake_policy command may be called to set policy "
    "CMP0000 to OLD or NEW behavior explicitly.  "
    "The OLD behavior is to silently ignore the missing invocation.  "
    "The NEW behavior is to issue an error instead of a warning.  "
    "An included file may set CMP0000 explicitly to affect how this "
    "policy is enforced for the main CMakeLists.txt file.",
    2,6,0,0, cmPolicies::WARN
    );

  this->DefinePolicy(
    CMP0001, "CMP0001",
    "CMAKE_BACKWARDS_COMPATIBILITY should no longer be used.",
    "The OLD behavior is to check CMAKE_BACKWARDS_COMPATIBILITY and present "
    "it to the user.  "
    "The NEW behavior is to ignore CMAKE_BACKWARDS_COMPATIBILITY "
    "completely.\n"
    "In CMake 2.4 and below the variable CMAKE_BACKWARDS_COMPATIBILITY was "
    "used to request compatibility with earlier versions of CMake.  "
    "In CMake 2.6 and above all compatibility issues are handled by policies "
    "and the cmake_policy command.  "
    "However, CMake must still check CMAKE_BACKWARDS_COMPATIBILITY for "
    "projects written for CMake 2.4 and below.",
    2,6,0,0, cmPolicies::WARN
    );

  this->DefinePolicy(
    CMP0002, "CMP0002",
    "Logical target names must be globally unique.",
    "Targets names created with "
    "add_executable, add_library, or add_custom_target "
    "are logical build target names.  "
    "Logical target names must be globally unique because:\n"
    "  - Unique names may be referenced unambiguously both in CMake\n"
    "    code and on make tool command lines.\n"
    "  - Logical names are used by Xcode and VS IDE generators\n"
    "    to produce meaningful project names for the targets.\n"
    "The logical name of executable and library targets does not "
    "have to correspond to the physical file names built.  "
    "Consider using the OUTPUT_NAME target property to create two "
    "targets with the same physical name while keeping logical "
    "names distinct.  "
    "Custom targets must simply have globally unique names (unless one "
    "uses the global property ALLOW_DUPLICATE_CUSTOM_TARGETS with a "
    "Makefiles generator).",
    2,6,0,0, cmPolicies::WARN
    );

  this->DefinePolicy(
    CMP0003, "CMP0003",
    "Libraries linked via full path no longer produce linker search paths.",
    "This policy affects how libraries whose full paths are NOT known "
    "are found at link time, but was created due to a change in how CMake "
    "deals with libraries whose full paths are known.  "
    "Consider the code\n"
    "  target_link_libraries(myexe /path/to/libA.so)\n"
    "CMake 2.4 and below implemented linking to libraries whose full paths "
    "are known by splitting them on the link line into separate components "
    "consisting of the linker search path and the library name.  "
    "The example code might have produced something like\n"
    "  ... -L/path/to -lA ...\n"
    "in order to link to library A.  "
    "An analysis was performed to order multiple link directories such that "
    "the linker would find library A in the desired location, but there "
    "are cases in which this does not work.  "
    "CMake versions 2.6 and above use the more reliable approach of passing "
    "the full path to libraries directly to the linker in most cases.  "
    "The example code now produces something like\n"
    "  ... /path/to/libA.so ....\n"
    "Unfortunately this change can break code like\n"
    "  target_link_libraries(myexe /path/to/libA.so B)\n"
    "where \"B\" is meant to find \"/path/to/libB.so\".  "
    "This code is wrong because the user is asking the linker to find "
    "library B but has not provided a linker search path (which may be "
    "added with the link_directories command).  "
    "However, with the old linking implementation the code would work "
    "accidentally because the linker search path added for library A "
    "allowed library B to be found."
    "\n"
    "In order to support projects depending on linker search paths "
    "added by linking to libraries with known full paths, the OLD "
    "behavior for this policy will add the linker search paths even "
    "though they are not needed for their own libraries.  "
    "When this policy is set to OLD, CMake will produce a link line such as\n"
    "  ... -L/path/to /path/to/libA.so -lB ...\n"
    "which will allow library B to be found as it was previously.  "
    "When this policy is set to NEW, CMake will produce a link line such as\n"
    "  ... /path/to/libA.so -lB ...\n"
    "which more accurately matches what the project specified."
    "\n"
    "The setting for this policy used when generating the link line is that "
    "in effect when the target is created by an add_executable or "
    "add_library command.  For the example described above, the code\n"
    "  cmake_policy(SET CMP0003 OLD) # or cmake_policy(VERSION 2.4)\n"
    "  add_executable(myexe myexe.c)\n"
    "  target_link_libraries(myexe /path/to/libA.so B)\n"
    "will work and suppress the warning for this policy.  "
    "It may also be updated to work with the corrected linking approach:\n"
    "  cmake_policy(SET CMP0003 NEW) # or cmake_policy(VERSION 2.6)\n"
    "  link_directories(/path/to) # needed to find library B\n"
    "  add_executable(myexe myexe.c)\n"
    "  target_link_libraries(myexe /path/to/libA.so B)\n"
    "Even better, library B may be specified with a full path:\n"
    "  add_executable(myexe myexe.c)\n"
    "  target_link_libraries(myexe /path/to/libA.so /path/to/libB.so)\n"
    "When all items on the link line have known paths CMake does not check "
    "this policy so it has no effect.\n"
    "Note that the warning for this policy will be issued for at most "
    "one target.  This avoids flooding users with messages for every "
    "target when setting the policy once will probably fix all targets.",
    2,6,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0004, "CMP0004",
    "Libraries linked may not have leading or trailing whitespace.",
    "CMake versions 2.4 and below silently removed leading and trailing "
    "whitespace from libraries linked with code like\n"
    "  target_link_libraries(myexe \" A \")\n"
    "This could lead to subtle errors in user projects.\n"
    "The OLD behavior for this policy is to silently remove leading and "
    "trailing whitespace.  "
    "The NEW behavior for this policy is to diagnose the existence of "
    "such whitespace as an error.  "
    "The setting for this policy used when checking the library names is "
    "that in effect when the target is created by an add_executable or "
    "add_library command.",
    2,6,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0005, "CMP0005",
    "Preprocessor definition values are now escaped automatically.",
    "This policy determines whether or not CMake should generate escaped "
    "preprocessor definition values added via add_definitions.  "
    "CMake versions 2.4 and below assumed that only trivial values would "
    "be given for macros in add_definitions calls.  "
    "It did not attempt to escape non-trivial values such as string "
    "literals in generated build rules.  "
    "CMake versions 2.6 and above support escaping of most values, but "
    "cannot assume the user has not added escapes already in an attempt to "
    "work around limitations in earlier versions.\n"
    "The OLD behavior for this policy is to place definition values given "
    "to add_definitions directly in the generated build rules without "
    "attempting to escape anything.  "
    "The NEW behavior for this policy is to generate correct escapes "
    "for all native build tools automatically.  "
    "See documentation of the COMPILE_DEFINITIONS target property for "
    "limitations of the escaping implementation.",
    2,6,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0006, "CMP0006",
    "Installing MACOSX_BUNDLE targets requires a BUNDLE DESTINATION.",
    "This policy determines whether the install(TARGETS) command must be "
    "given a BUNDLE DESTINATION when asked to install a target with the "
    "MACOSX_BUNDLE property set.  "
    "CMake 2.4 and below did not distinguish application bundles from "
    "normal executables when installing targets.  "
    "CMake 2.6 provides a BUNDLE option to the install(TARGETS) command "
    "that specifies rules specific to application bundles on the Mac.  "
    "Projects should use this option when installing a target with the "
    "MACOSX_BUNDLE property set.\n"
    "The OLD behavior for this policy is to fall back to the RUNTIME "
    "DESTINATION if a BUNDLE DESTINATION is not given.  "
    "The NEW behavior for this policy is to produce an error if a bundle "
    "target is installed without a BUNDLE DESTINATION.",
    2,6,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0007, "CMP0007",
    "list command no longer ignores empty elements.",
    "This policy determines whether the list command will "
    "ignore empty elements in the list. "
    "CMake 2.4 and below list commands ignored all empty elements"
    " in the list.  For example, a;b;;c would have length 3 and not 4. "
    "The OLD behavior for this policy is to ignore empty list elements. "
    "The NEW behavior for this policy is to correctly count empty "
    "elements in a list. ",
    2,6,0,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0008, "CMP0008",
    "Libraries linked by full-path must have a valid library file name.",
    "In CMake 2.4 and below it is possible to write code like\n"
    "  target_link_libraries(myexe /full/path/to/somelib)\n"
    "where \"somelib\" is supposed to be a valid library file name "
    "such as \"libsomelib.a\" or \"somelib.lib\".  "
    "For Makefile generators this produces an error at build time "
    "because the dependency on the full path cannot be found.  "
    "For VS IDE and Xcode generators this used to work by accident because "
    "CMake would always split off the library directory and ask the "
    "linker to search for the library by name (-lsomelib or somelib.lib).  "
    "Despite the failure with Makefiles, some projects have code like this "
    "and build only with VS and/or Xcode.  "
    "This version of CMake prefers to pass the full path directly to the "
    "native build tool, which will fail in this case because it does "
    "not name a valid library file."
    "\n"
    "This policy determines what to do with full paths that do not appear "
    "to name a valid library file.  "
    "The OLD behavior for this policy is to split the library name from the "
    "path and ask the linker to search for it.  "
    "The NEW behavior for this policy is to trust the given path and "
    "pass it directly to the native build tool unchanged.",
    2,6,1,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0009, "CMP0009",
    "FILE GLOB_RECURSE calls should not follow symlinks by default.",
    "In CMake 2.6.1 and below, FILE GLOB_RECURSE calls would follow "
    "through symlinks, sometimes coming up with unexpectedly large "
    "result sets because of symlinks to top level directories that "
    "contain hundreds of thousands of files."
    "\n"
    "This policy determines whether or not to follow symlinks "
    "encountered during a FILE GLOB_RECURSE call. "
    "The OLD behavior for this policy is to follow the symlinks. "
    "The NEW behavior for this policy is not to follow the symlinks "
    "by default, but only if FOLLOW_SYMLINKS is given as an additional "
    "argument to the FILE command.",
    2,6,2,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0010, "CMP0010",
    "Bad variable reference syntax is an error.",
    "In CMake 2.6.2 and below, incorrect variable reference syntax such as "
    "a missing close-brace (\"${FOO\") was reported but did not stop "
    "processing of CMake code.  "
    "This policy determines whether a bad variable reference is an error.  "
    "The OLD behavior for this policy is to warn about the error, leave "
    "the string untouched, and continue. "
    "The NEW behavior for this policy is to report an error.",
    2,6,3,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0011, "CMP0011",
    "Included scripts do automatic cmake_policy PUSH and POP.",
    "In CMake 2.6.2 and below, CMake Policy settings in scripts loaded by "
    "the include() and find_package() commands would affect the includer.  "
    "Explicit invocations of cmake_policy(PUSH) and cmake_policy(POP) were "
    "required to isolate policy changes and protect the includer.  "
    "While some scripts intend to affect the policies of their includer, "
    "most do not.  "
    "In CMake 2.6.3 and above, include() and find_package() by default PUSH "
    "and POP an entry on the policy stack around an included script, "
    "but provide a NO_POLICY_SCOPE option to disable it.  "
    "This policy determines whether or not to imply NO_POLICY_SCOPE for "
    "compatibility.  "
    "The OLD behavior for this policy is to imply NO_POLICY_SCOPE for "
    "include() and find_package() commands.  "
    "The NEW behavior for this policy is to allow the commands to do their "
    "default cmake_policy PUSH and POP.",
    2,6,3,0, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0012, "CMP0012",
    "if() recognizes numbers and boolean constants.",
    "In CMake versions 2.6.4 and lower the if() command implicitly "
    "dereferenced arguments corresponding to variables, even those named "
    "like numbers or boolean constants, except for 0 and 1.  "
    "Numbers and boolean constants such as true, false, yes, no, "
    "on, off, y, n, notfound, ignore (all case insensitive) were recognized "
    "in some cases but not all.  "
    "For example, the code \"if(TRUE)\" might have evaluated as false.  "
    "Numbers such as 2 were recognized only in "
    "boolean expressions like \"if(NOT 2)\" (leading to false) "
    "but not as a single-argument like \"if(2)\" (also leading to false). "
    "Later versions of CMake prefer to treat numbers and boolean constants "
    "literally, so they should not be used as variable names."
    "\n"
    "The OLD behavior for this policy is to implicitly dereference variables "
    "named like numbers and boolean constants. "
    "The NEW behavior for this policy is to recognize numbers and "
    "boolean constants without dereferencing variables with such names.",
    2,8,0,0, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0013, "CMP0013",
    "Duplicate binary directories are not allowed.",
    "CMake 2.6.3 and below silently permitted add_subdirectory() calls "
    "to create the same binary directory multiple times.  "
    "During build system generation files would be written and then "
    "overwritten in the build tree and could lead to strange behavior.  "
    "CMake 2.6.4 and above explicitly detect duplicate binary directories.  "
    "CMake 2.6.4 always considers this case an error.  "
    "In CMake 2.8.0 and above this policy determines whether or not "
    "the case is an error.  "
    "The OLD behavior for this policy is to allow duplicate binary "
    "directories.  "
    "The NEW behavior for this policy is to disallow duplicate binary "
    "directories with an error.",
    2,8,0,0, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0014, "CMP0014",
    "Input directories must have CMakeLists.txt.",
    "CMake versions before 2.8 silently ignored missing CMakeLists.txt "
    "files in directories referenced by add_subdirectory() or subdirs(), "
    "treating them as if present but empty.  "
    "In CMake 2.8.0 and above this policy determines whether or not "
    "the case is an error.  "
    "The OLD behavior for this policy is to silently ignore the problem.  "
    "The NEW behavior for this policy is to report an error.",
    2,8,0,0, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0015, "CMP0015",
    "link_directories() treats paths relative to the source dir.",
    "In CMake 2.8.0 and lower the link_directories() command passed relative "
    "paths unchanged to the linker.  "
    "In CMake 2.8.1 and above the link_directories() command prefers to "
    "interpret relative paths with respect to CMAKE_CURRENT_SOURCE_DIR, "
    "which is consistent with include_directories() and other commands.  "
    "The OLD behavior for this policy is to use relative paths verbatim in "
    "the linker command.  "
    "The NEW behavior for this policy is to convert relative paths to "
    "absolute paths by appending the relative path to "
    "CMAKE_CURRENT_SOURCE_DIR.",
    2,8,1,0, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0016, "CMP0016",
    "target_link_libraries() reports error if its only argument "
    "is not a target.",
    "In CMake 2.8.2 and lower the target_link_libraries() command silently "
    "ignored if it was called with only one argument, and this argument "
    "wasn't a valid target. "
    "In CMake 2.8.3 and above it reports an error in this case.",
    2,8,3,0, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0017, "CMP0017",
    "Prefer files from the CMake module directory when including from there.",
    "Starting with CMake 2.8.4, if a cmake-module shipped with CMake (i.e. "
    "located in the CMake module directory) calls include() or "
    "find_package(), the files located in the CMake module directory are "
    "preferred over the files in CMAKE_MODULE_PATH.  "
    "This makes sure that the modules belonging to "
    "CMake always get those files included which they expect, and against "
    "which they were developed and tested.  "
    "In all other cases, the files found in "
    "CMAKE_MODULE_PATH still take precedence over the ones in "
    "the CMake module directory.  "
    "The OLD behaviour is to always prefer files from CMAKE_MODULE_PATH over "
    "files from the CMake modules directory.",
    2,8,4,0, cmPolicies::WARN);

    this->DefinePolicy(
    CMP0018, "CMP0018",
    "Ignore CMAKE_SHARED_LIBRARY_<Lang>_FLAGS variable.",
    "CMake 2.8.8 and lower compiled sources in SHARED and MODULE libraries "
    "using the value of the undocumented CMAKE_SHARED_LIBRARY_<Lang>_FLAGS "
    "platform variable.  The variable contained platform-specific flags "
    "needed to compile objects for shared libraries.  Typically it included "
    "a flag such as -fPIC for position independent code but also included "
    "other flags needed on certain platforms.  CMake 2.8.9 and higher "
    "prefer instead to use the POSITION_INDEPENDENT_CODE target property to "
    "determine what targets should be position independent, and new "
    "undocumented platform variables to select flags while ignoring "
    "CMAKE_SHARED_LIBRARY_<Lang>_FLAGS completely."
    "\n"
    "The default for either approach produces identical compilation flags, "
    "but if a project modifies CMAKE_SHARED_LIBRARY_<Lang>_FLAGS from its "
    "original value this policy determines which approach to use."
    "\n"
    "The OLD behavior for this policy is to ignore the "
    "POSITION_INDEPENDENT_CODE property for all targets and use the modified "
    "value of CMAKE_SHARED_LIBRARY_<Lang>_FLAGS for SHARED and MODULE "
    "libraries."
    "\n"
    "The NEW behavior for this policy is to ignore "
    "CMAKE_SHARED_LIBRARY_<Lang>_FLAGS whether it is modified or not and "
    "honor the POSITION_INDEPENDENT_CODE target property.",
    2,8,9,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0019, "CMP0019",
    "Do not re-expand variables in include and link information.",
    "CMake 2.8.10 and lower re-evaluated values given to the "
    "include_directories, link_directories, and link_libraries "
    "commands to expand any leftover variable references at the "
    "end of the configuration step.  "
    "This was for strict compatibility with VERY early CMake versions "
    "because all variable references are now normally evaluated during "
    "CMake language processing.  "
    "CMake 2.8.11 and higher prefer to skip the extra evaluation."
    "\n"
    "The OLD behavior for this policy is to re-evaluate the values "
    "for strict compatibility.  "
    "The NEW behavior for this policy is to leave the values untouched.",
    2,8,11,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0020, "CMP0020",
    "Automatically link Qt executables to qtmain target on Windows.",
    "CMake 2.8.10 and lower required users of Qt to always specify a link "
    "dependency to the qtmain.lib static library manually on Windows.  CMake "
    "2.8.11 gained the ability to evaluate generator expressions while "
    "determining the link dependencies from IMPORTED targets.  This allows "
    "CMake itself to automatically link executables which link to Qt to the "
    "qtmain.lib library when using IMPORTED Qt targets.  For applications "
    "already linking to qtmain.lib, this should have little impact.  For "
    "applications which supply their own alternative WinMain implementation "
    "and for applications which use the QAxServer library, this automatic "
    "linking will need to be disabled as per the documentation."
    "\n"
    "The OLD behavior for this policy is not to link executables to "
    "qtmain.lib automatically when they link to the QtCore IMPORTED"
    "target.  "
    "The NEW behavior for this policy is to link executables to "
    "qtmain.lib automatically when they link to QtCore IMPORTED target.",
    2,8,11,0, cmPolicies::WARN);

  this->DefinePolicy(
    CMP0021, "CMP0021",
    "Fatal error on relative paths in INCLUDE_DIRECTORIES target property.",
    "CMake 2.8.10.2 and lower allowed the INCLUDE_DIRECTORIES target "
    "property to contain relative paths.  The base path for such relative "
    "entries is not well defined.  CMake 2.8.12 issues a FATAL_ERROR if the "
    "INCLUDE_DIRECTORIES property contains a relative path."
    "\n"
    "The OLD behavior for this policy is not to warn about relative paths in "
    "the INCLUDE_DIRECTORIES target property.  "
    "The NEW behavior for this policy is to issue a FATAL_ERROR if "
    "INCLUDE_DIRECTORIES contains a relative path.",
    2,8,11,20130516, cmPolicies::WARN);
}

cmPolicies::~cmPolicies()
{
  // free the policies
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator i
    = this->Policies.begin();
  for (;i != this->Policies.end(); ++i)
  {
    delete i->second;
  }
}

void cmPolicies::DefinePolicy(cmPolicies::PolicyID iD,
                              const char *idString,
                              const char *shortDescription,
                              const char *longDescription,
                              unsigned int majorVersionIntroduced,
                              unsigned int minorVersionIntroduced,
                              unsigned int patchVersionIntroduced,
                              unsigned int tweakVersionIntroduced,
                              cmPolicies::PolicyStatus status)
{
  // a policy must be unique and can only be defined once
  if (this->Policies.find(iD) != this->Policies.end())
  {
    cmSystemTools::Error("Attempt to redefine a CMake policy for policy "
      "ID ", this->GetPolicyIDString(iD).c_str());
    return;
  }

  this->Policies[iD] = new cmPolicy(iD, idString,
                                    shortDescription,
                                    longDescription,
                                    majorVersionIntroduced,
                                    minorVersionIntroduced,
                                    patchVersionIntroduced,
                                    tweakVersionIntroduced,
                                    status);
  this->PolicyStringMap[idString] = iD;
}

//----------------------------------------------------------------------------
bool cmPolicies::ApplyPolicyVersion(cmMakefile *mf,
                                    const char *version)
{
  std::string ver = "2.4.0";

  if (version && strlen(version) > 0)
    {
    ver = version;
    }

  unsigned int majorVer = 2;
  unsigned int minorVer = 0;
  unsigned int patchVer = 0;
  unsigned int tweakVer = 0;

  // parse the string
  if(sscanf(ver.c_str(), "%u.%u.%u.%u",
            &majorVer, &minorVer, &patchVer, &tweakVer) < 2)
    {
    cmOStringStream e;
    e << "Invalid policy version value \"" << ver << "\".  "
      << "A numeric major.minor[.patch[.tweak]] must be given.";
    mf->IssueMessage(cmake::FATAL_ERROR, e.str());
    return false;
    }

  // it is an error if the policy version is less than 2.4
  if (majorVer < 2 || (majorVer == 2 && minorVer < 4))
    {
    mf->IssueMessage(cmake::FATAL_ERROR,
      "An attempt was made to set the policy version of CMake to something "
      "earlier than \"2.4\".  "
      "In CMake 2.4 and below backwards compatibility was handled with the "
      "CMAKE_BACKWARDS_COMPATIBILITY variable.  "
      "In order to get compatibility features supporting versions earlier "
      "than 2.4 set policy CMP0001 to OLD to tell CMake to check the "
      "CMAKE_BACKWARDS_COMPATIBILITY variable.  "
      "One way to do this is to set the policy version to 2.4 exactly."
      );
    return false;
    }

  // It is an error if the policy version is greater than the running
  // CMake.
  if (majorVer > cmVersion::GetMajorVersion() ||
      (majorVer == cmVersion::GetMajorVersion() &&
       minorVer > cmVersion::GetMinorVersion()) ||
      (majorVer == cmVersion::GetMajorVersion() &&
       minorVer == cmVersion::GetMinorVersion() &&
       patchVer > cmVersion::GetPatchVersion()) ||
      (majorVer == cmVersion::GetMajorVersion() &&
       minorVer == cmVersion::GetMinorVersion() &&
       patchVer == cmVersion::GetPatchVersion() &&
       tweakVer > cmVersion::GetTweakVersion()))
    {
    cmOStringStream e;
    e << "An attempt was made to set the policy version of CMake to \""
      << version << "\" which is greater than this version of CMake.  "
      << "This is not allowed because the greater version may have new "
      << "policies not known to this CMake.  "
      << "You may need a newer CMake version to build this project.";
    mf->IssueMessage(cmake::FATAL_ERROR, e.str());
    return false;
    }

  // now loop over all the policies and set them as appropriate
  std::vector<cmPolicies::PolicyID> ancientPolicies;
  for(std::map<cmPolicies::PolicyID,cmPolicy *>::iterator i
                     = this->Policies.begin(); i != this->Policies.end(); ++i)
    {
    if (i->second->IsPolicyNewerThan(majorVer,minorVer,patchVer,tweakVer))
      {
      if(i->second->Status == cmPolicies::REQUIRED_ALWAYS)
        {
        ancientPolicies.push_back(i->first);
        }
      else
        {
        cmPolicies::PolicyStatus status = cmPolicies::WARN;
        if(!this->GetPolicyDefault(mf, i->second->IDString, &status) ||
           !mf->SetPolicy(i->second->ID, status))
          {
          return false;
          }
        }
      }
    else
      {
      if (!mf->SetPolicy(i->second->ID, cmPolicies::NEW))
        {
        return false;
        }
      }
    }

  // Make sure the project does not use any ancient policies.
  if(!ancientPolicies.empty())
    {
    this->DiagnoseAncientPolicies(ancientPolicies,
                                  majorVer, minorVer, patchVer, mf);
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmPolicies::GetPolicyDefault(cmMakefile* mf, std::string const& policy,
                                  cmPolicies::PolicyStatus* defaultSetting)
{
  std::string defaultVar = "CMAKE_POLICY_DEFAULT_" + policy;
  std::string defaultValue = mf->GetSafeDefinition(defaultVar.c_str());
  if(defaultValue == "NEW")
    {
    *defaultSetting = cmPolicies::NEW;
    }
  else if(defaultValue == "OLD")
    {
    *defaultSetting = cmPolicies::OLD;
    }
  else if(defaultValue == "")
    {
    *defaultSetting = cmPolicies::WARN;
    }
  else
    {
    cmOStringStream e;
    e << defaultVar << " has value \"" << defaultValue
      << "\" but must be \"OLD\", \"NEW\", or \"\" (empty).";
    mf->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
    return false;
    }

  return true;
}

bool cmPolicies::GetPolicyID(const char *id, cmPolicies::PolicyID &pid)
{
  if (!id || strlen(id) < 1)
  {
    return false;
  }
  std::map<std::string,cmPolicies::PolicyID>::iterator pos =
    this->PolicyStringMap.find(id);
  if (pos == this->PolicyStringMap.end())
  {
    return false;
  }
  pid = pos->second;
  return true;
}

std::string cmPolicies::GetPolicyIDString(cmPolicies::PolicyID pid)
{
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos =
    this->Policies.find(pid);
  if (pos == this->Policies.end())
  {
    return "";
  }
  return pos->second->IDString;
}


///! return a warning string for a given policy
std::string cmPolicies::GetPolicyWarning(cmPolicies::PolicyID id)
{
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos =
    this->Policies.find(id);
  if (pos == this->Policies.end())
  {
    cmSystemTools::Error(
      "Request for warning text for undefined policy!");
    return "Request for warning text for undefined policy!";
  }

  cmOStringStream msg;
  msg <<
    "Policy " << pos->second->IDString << " is not set: "
    "" << pos->second->ShortDescription << "  "
    "Run \"cmake --help-policy " << pos->second->IDString << "\" for "
    "policy details.  "
    "Use the cmake_policy command to set the policy "
    "and suppress this warning.";
  return msg.str();
}


///! return an error string for when a required policy is unspecified
std::string cmPolicies::GetRequiredPolicyError(cmPolicies::PolicyID id)
{
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos =
    this->Policies.find(id);
  if (pos == this->Policies.end())
  {
    cmSystemTools::Error(
      "Request for error text for undefined policy!");
    return "Request for error text for undefined policy!";
  }

  cmOStringStream error;
  error <<
    "Policy " << pos->second->IDString << " is not set to NEW: "
    "" << pos->second->ShortDescription << "  "
    "Run \"cmake --help-policy " << pos->second->IDString << "\" for "
    "policy details.  "
    "CMake now requires this policy to be set to NEW by the project.  "
    "The policy may be set explicitly using the code\n"
    "  cmake_policy(SET " << pos->second->IDString << " NEW)\n"
    "or by upgrading all policies with the code\n"
    "  cmake_policy(VERSION " << pos->second->GetVersionString() <<
    ") # or later\n"
    "Run \"cmake --help-command cmake_policy\" for more information.";
  return error.str();
}

///! Get the default status for a policy
cmPolicies::PolicyStatus
cmPolicies::GetPolicyStatus(cmPolicies::PolicyID id)
{
  // if the policy is not know then what?
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator pos =
    this->Policies.find(id);
  if (pos == this->Policies.end())
  {
    // TODO is this right?
    return cmPolicies::WARN;
  }

  return pos->second->Status;
}

void cmPolicies::GetDocumentation(std::vector<cmDocumentationEntry>& v)
{
  // now loop over all the policies and set them as appropriate
  std::map<cmPolicies::PolicyID,cmPolicy *>::iterator i
    = this->Policies.begin();
  for (;i != this->Policies.end(); ++i)
  {
    cmOStringStream full;
    full << i->second->LongDescription;
    full << "\nThis policy was introduced in CMake version ";
    full << i->second->GetVersionString() << ".";
    if(i->first != cmPolicies::CMP0000)
      {
      full << "  "
           << "CMake version " << cmVersion::GetCMakeVersion() << " ";
      // add in some more text here based on status
      switch (i->second->Status)
        {
        case cmPolicies::WARN:
          full << "warns when the policy is not set and uses OLD behavior.  "
               << "Use the cmake_policy command to set it to OLD or NEW "
               << "explicitly.";
          break;
        case cmPolicies::OLD:
          full << "defaults to the OLD behavior for this policy.";
          break;
        case cmPolicies::NEW:
          full << "defaults to the NEW behavior for this policy.";
          break;
        case cmPolicies::REQUIRED_IF_USED:
          full << "requires the policy to be set to NEW if you use it.  "
               << "Use the cmake_policy command to set it to NEW.";
          break;
        case cmPolicies::REQUIRED_ALWAYS:
          full << "requires the policy to be set to NEW.  "
               << "Use the cmake_policy command to set it to NEW.";
          break;
        }
      }
    cmDocumentationEntry e(i->second->IDString.c_str(),
                           i->second->ShortDescription.c_str(),
                           full.str().c_str());
    v.push_back(e);
  }
}

//----------------------------------------------------------------------------
std::string
cmPolicies::GetRequiredAlwaysPolicyError(cmPolicies::PolicyID id)
{
  std::string pid = this->GetPolicyIDString(id);
  cmOStringStream e;
  e << "Policy " << pid << " may not be set to OLD behavior because this "
    << "version of CMake no longer supports it.  "
    << "The policy was introduced in "
    << "CMake version " << this->Policies[id]->GetVersionString()
    << ", and use of NEW behavior is now required."
    << "\n"
    << "Please either update your CMakeLists.txt files to conform to "
    << "the new behavior or use an older version of CMake that still "
    << "supports the old behavior.  "
    << "Run cmake --help-policy " << pid << " for more information.";
  return e.str();
}

//----------------------------------------------------------------------------
void
cmPolicies::DiagnoseAncientPolicies(std::vector<PolicyID> const& ancient,
                                    unsigned int majorVer,
                                    unsigned int minorVer,
                                    unsigned int patchVer,
                                    cmMakefile* mf)
{
  cmOStringStream e;
  e << "The project requests behavior compatible with CMake version \""
    << majorVer << "." << minorVer << "." << patchVer
    << "\", which requires the OLD behavior for some policies:\n";
  for(std::vector<PolicyID>::const_iterator
        i = ancient.begin(); i != ancient.end(); ++i)
    {
    cmPolicy const* policy = this->Policies[*i];
    e << "  " << policy->IDString << ": " << policy->ShortDescription << "\n";
    }
  e << "However, this version of CMake no longer supports the OLD "
    << "behavior for these policies.  "
    << "Please either update your CMakeLists.txt files to conform to "
    << "the new behavior or use an older version of CMake that still "
    << "supports the old behavior.";
  mf->IssueMessage(cmake::FATAL_ERROR, e.str().c_str());
}
