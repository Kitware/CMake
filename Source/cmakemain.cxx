/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include "cmConfigure.h" // IWYU pragma: keep

#include <algorithm>
#include <cassert>
#include <climits>
#include <cstring>
#include <iostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include <cm/memory>
#include <cmext/algorithm>

#include <cm3p/uv.h>

#include "cmCommandLineArgument.h"
#include "cmConsoleBuf.h"
#include "cmDocumentationEntry.h" // IWYU pragma: keep
#include "cmGlobalGenerator.h"
#include "cmMakefile.h"
#include "cmProperty.h"
#include "cmState.h"
#include "cmStateTypes.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"
#include "cmcmd.h"

#ifndef CMAKE_BOOTSTRAP
#  include "cmDocumentation.h"
#  include "cmDynamicLoader.h"
#endif

#include "cmsys/Encoding.hxx"

namespace {
#ifndef CMAKE_BOOTSTRAP
const char* cmDocumentationName[][2] = {
  { nullptr, "  cmake - Cross-Platform Makefile Generator." },
  { nullptr, nullptr }
};

const char* cmDocumentationUsage[][2] = {
  { nullptr,
    "  cmake [options] <path-to-source>\n"
    "  cmake [options] <path-to-existing-build>\n"
    "  cmake [options] -S <path-to-source> -B <path-to-build>" },
  { nullptr,
    "Specify a source directory to (re-)generate a build system for "
    "it in the current working directory.  Specify an existing build "
    "directory to re-generate its build system." },
  { nullptr, nullptr }
};

const char* cmDocumentationUsageNote[][2] = {
  { nullptr, "Run 'cmake --help' for more information." },
  { nullptr, nullptr }
};

const char* cmDocumentationOptions[][2] = {
  CMAKE_STANDARD_OPTIONS_TABLE,
  { "--preset=<preset>", "Specify a configure preset." },
  { "--list-presets", "List available presets." },
  { "-E", "CMake command mode." },
  { "-L[A][H]", "List non-advanced cached variables." },
  { "--build <dir>", "Build a CMake-generated project binary tree." },
  { "--install <dir>", "Install a CMake-generated project binary tree." },
  { "--open <dir>", "Open generated project in the associated application." },
  { "-N", "View mode only." },
  { "-P <file>", "Process script mode." },
  { "--find-package", "Legacy pkg-config like mode.  Do not use." },
  { "--graphviz=[file]",
    "Generate graphviz of dependencies, see "
    "CMakeGraphVizOptions.cmake for more." },
  { "--system-information [file]", "Dump information about this system." },
  { "--log-level=<ERROR|WARNING|NOTICE|STATUS|VERBOSE|DEBUG|TRACE>",
    "Set the verbosity of messages from CMake files. "
    "--loglevel is also accepted for backward compatibility reasons." },
  { "--log-context", "Prepend log messages with context, if given" },
  { "--debug-trycompile",
    "Do not delete the try_compile build tree. Only "
    "useful on one try_compile at a time." },
  { "--debug-output", "Put cmake in a debug mode." },
  { "--debug-find", "Put cmake find in a debug mode." },
  { "--trace", "Put cmake in trace mode." },
  { "--trace-expand", "Put cmake in trace mode with variable expansion." },
  { "--trace-format=<human|json-v1>", "Set the output format of the trace." },
  { "--trace-source=<file>",
    "Trace only this CMake file/module. Multiple options allowed." },
  { "--trace-redirect=<file>",
    "Redirect trace output to a file instead of stderr." },
  { "--warn-uninitialized", "Warn about uninitialized values." },
  { "--no-warn-unused-cli", "Don't warn about command line options." },
  { "--check-system-vars",
    "Find problems with variable usage in system "
    "files." },
#  if !defined(CMAKE_BOOTSTRAP)
  { "--profiling-format=<fmt>",
    "Output data for profiling CMake scripts. Supported formats: "
    "google-trace" },
  { "--profiling-output=<file>",
    "Select an output path for the profiling data enabled through "
    "--profiling-format." },
#  endif
  { nullptr, nullptr }
};

#endif

int do_command(int ac, char const* const* av,
               std::unique_ptr<cmConsoleBuf> consoleBuf)
{
  std::vector<std::string> args;
  args.reserve(ac - 1);
  args.emplace_back(av[0]);
  cm::append(args, av + 2, av + ac);
  return cmcmd::ExecuteCMakeCommand(args, std::move(consoleBuf));
}

cmMakefile* cmakemainGetMakefile(cmake* cm)
{
  if (cm && cm->GetDebugOutput()) {
    cmGlobalGenerator* gg = cm->GetGlobalGenerator();
    if (gg) {
      return gg->GetCurrentMakefile();
    }
  }
  return nullptr;
}

std::string cmakemainGetStack(cmake* cm)
{
  std::string msg;
  cmMakefile* mf = cmakemainGetMakefile(cm);
  if (mf) {
    msg = mf->FormatListFileStack();
    if (!msg.empty()) {
      msg = "\n   Called from: " + msg;
    }
  }

  return msg;
}

void cmakemainMessageCallback(const std::string& m, const char* /*unused*/,
                              cmake* cm)
{
  std::cerr << m << cmakemainGetStack(cm) << std::endl;
}

void cmakemainProgressCallback(const std::string& m, float prog, cmake* cm)
{
  cmMakefile* mf = cmakemainGetMakefile(cm);
  std::string dir;
  if (mf && cmHasLiteralPrefix(m, "Configuring") && (prog < 0)) {
    dir = cmStrCat(' ', mf->GetCurrentSourceDirectory());
  } else if (mf && cmHasLiteralPrefix(m, "Generating")) {
    dir = cmStrCat(' ', mf->GetCurrentBinaryDirectory());
  }

  if ((prog < 0) || (!dir.empty())) {
    std::cout << "-- " << m << dir << cmakemainGetStack(cm) << std::endl;
  }
}

int do_cmake(int ac, char const* const* av)
{
  if (cmSystemTools::GetCurrentWorkingDirectory().empty()) {
    std::cerr << "Current working directory cannot be established."
              << std::endl;
    return 1;
  }

#ifndef CMAKE_BOOTSTRAP
  cmDocumentation doc;
  doc.addCMakeStandardDocSections();
  if (doc.CheckOptions(ac, av)) {
    // Construct and print requested documentation.
    cmake hcm(cmake::RoleInternal, cmState::Unknown);
    hcm.SetHomeDirectory("");
    hcm.SetHomeOutputDirectory("");
    hcm.AddCMakePaths();

    // the command line args are processed here so that you can do
    // -DCMAKE_MODULE_PATH=/some/path and have this value accessible here
    std::vector<std::string> args(av, av + ac);
    hcm.SetCacheArgs(args);

    auto generators = hcm.GetGeneratorsDocumentation();

    doc.SetName("cmake");
    doc.SetSection("Name", cmDocumentationName);
    doc.SetSection("Usage", cmDocumentationUsage);
    if (ac == 1) {
      doc.AppendSection("Usage", cmDocumentationUsageNote);
    }
    doc.AppendSection("Generators", generators);
    doc.PrependSection("Options", cmDocumentationOptions);

    return doc.PrintRequestedDocumentation(std::cout) ? 0 : 1;
  }
#else
  if (ac == 1) {
    std::cout
      << "Bootstrap CMake should not be used outside CMake build process."
      << std::endl;
    return 0;
  }
#endif

  bool wizard_mode = false;
  bool sysinfo = false;
  bool list_cached = false;
  bool list_all_cached = false;
  bool list_help = false;
  bool view_only = false;
  cmake::WorkingMode workingMode = cmake::NORMAL_MODE;
  std::vector<std::string> parsedArgs;

  using CommandArgument =
    cmCommandLineArgument<bool(std::string const& value)>;
  std::vector<CommandArgument> arguments = {
    CommandArgument{
      "-i", CommandArgument::Values::Zero,
      [&wizard_mode](std::string const&) -> bool {
        /* clang-format off */
        std::cerr <<
          "The \"cmake -i\" wizard mode is no longer supported.\n"
          "Use the -D option to set cache values on the command line.\n"
          "Use cmake-gui or ccmake for an interactive dialog.\n";
        /* clang-format on */
        wizard_mode = true;
        return true;
      } },
    CommandArgument{ "--system-information", CommandArgument::Values::Zero,
                     [&](std::string const&) -> bool {
                       sysinfo = true;
                       return true;
                     } },
    CommandArgument{ "-N", CommandArgument::Values::Zero,
                     [&](std::string const&) -> bool {
                       view_only = true;
                       return true;
                     } },
    CommandArgument{ "-LAH", CommandArgument::Values::Zero,
                     [&](std::string const&) -> bool {
                       list_all_cached = true;
                       list_help = true;
                       return true;
                     } },
    CommandArgument{ "-LA", CommandArgument::Values::Zero,
                     [&](std::string const&) -> bool {
                       list_all_cached = true;
                       return true;
                     } },
    CommandArgument{ "-LH", CommandArgument::Values::Zero,
                     [&](std::string const&) -> bool {
                       list_cached = true;
                       list_help = true;
                       return true;
                     } },
    CommandArgument{ "-L", CommandArgument::Values::Zero,
                     [&](std::string const&) -> bool {
                       list_cached = true;
                       return true;
                     } },
    CommandArgument{ "-P", "No script specified for argument -P",
                     CommandArgument::Values::One,
                     [&](std::string const& value) -> bool {
                       workingMode = cmake::SCRIPT_MODE;
                       parsedArgs.emplace_back("-P");
                       parsedArgs.push_back(value);
                       return true;
                     } },
    CommandArgument{ "--find-package", CommandArgument::Values::Zero,
                     [&](std::string const&) -> bool {
                       workingMode = cmake::FIND_PACKAGE_MODE;
                       parsedArgs.emplace_back("--find-package");
                       return true;
                     } },
    CommandArgument{ "--list-presets", CommandArgument::Values::Zero,
                     [&](std::string const&) -> bool {
                       workingMode = cmake::HELP_MODE;
                       parsedArgs.emplace_back("--list-presets");
                       return true;
                     } },
  };

  std::vector<std::string> inputArgs;
  inputArgs.reserve(ac);
  cm::append(inputArgs, av, av + ac);

  for (decltype(inputArgs.size()) i = 0; i < inputArgs.size(); ++i) {
    std::string const& arg = inputArgs[i];
    bool matched = false;
    for (auto const& m : arguments) {
      if (m.matches(arg)) {
        matched = true;
        if (m.parse(arg, i, inputArgs)) {
          break;
        }
        return 1; // failed to parse
      }
    }
    if (!matched) {
      parsedArgs.emplace_back(av[i]);
    }
  }

  if (wizard_mode) {
    return 1;
  }

  if (sysinfo) {
    cmake cm(cmake::RoleProject, cmState::Project);
    cm.SetHomeDirectory("");
    cm.SetHomeOutputDirectory("");
    int ret = cm.GetSystemInformation(parsedArgs);
    return ret;
  }
  cmake::Role const role =
    workingMode == cmake::SCRIPT_MODE ? cmake::RoleScript : cmake::RoleProject;
  cmState::Mode mode = cmState::Unknown;
  switch (workingMode) {
    case cmake::NORMAL_MODE:
    case cmake::HELP_MODE:
      mode = cmState::Project;
      break;
    case cmake::SCRIPT_MODE:
      mode = cmState::Script;
      break;
    case cmake::FIND_PACKAGE_MODE:
      mode = cmState::FindPackage;
      break;
  }
  cmake cm(role, mode);
  cm.SetHomeDirectory("");
  cm.SetHomeOutputDirectory("");
  cmSystemTools::SetMessageCallback(
    [&cm](const std::string& msg, const char* title) {
      cmakemainMessageCallback(msg, title, &cm);
    });
  cm.SetProgressCallback([&cm](const std::string& msg, float prog) {
    cmakemainProgressCallback(msg, prog, &cm);
  });
  cm.SetWorkingMode(workingMode);

  int res = cm.Run(parsedArgs, view_only);
  if (list_cached || list_all_cached) {
    std::cout << "-- Cache values" << std::endl;
    std::vector<std::string> keys = cm.GetState()->GetCacheEntryKeys();
    for (std::string const& k : keys) {
      cmStateEnums::CacheEntryType t = cm.GetState()->GetCacheEntryType(k);
      if (t != cmStateEnums::INTERNAL && t != cmStateEnums::STATIC &&
          t != cmStateEnums::UNINITIALIZED) {
        cmProp advancedProp =
          cm.GetState()->GetCacheEntryProperty(k, "ADVANCED");
        if (list_all_cached || !advancedProp) {
          if (list_help) {
            cmProp help =
              cm.GetState()->GetCacheEntryProperty(k, "HELPSTRING");
            std::cout << "// " << (help ? *help : "") << std::endl;
          }
          std::cout << k << ":" << cmState::CacheEntryTypeToString(t) << "="
                    << cm.GetState()->GetSafeCacheEntryValue(k) << std::endl;
          if (list_help) {
            std::cout << std::endl;
          }
        }
      }
    }
  }

  // Always return a non-negative value.  Windows tools do not always
  // interpret negative return values as errors.
  if (res != 0) {
    return 1;
  }
  return 0;
}

#ifndef CMAKE_BOOTSTRAP
int extract_job_number(std::string const& command,
                       std::string const& jobString)
{
  int jobs = -1;
  unsigned long numJobs = 0;
  if (jobString.empty()) {
    jobs = cmake::DEFAULT_BUILD_PARALLEL_LEVEL;
  } else if (cmStrToULong(jobString, &numJobs)) {
    if (numJobs == 0) {
      std::cerr
        << "The <jobs> value requires a positive integer argument.\n\n";
    } else if (numJobs > INT_MAX) {
      std::cerr << "The <jobs> value is too large.\n\n";
    } else {
      jobs = int(numJobs);
    }
  } else {
    std::cerr << "'" << command << "' invalid number '" << jobString
              << "' given.\n\n";
  }
  return jobs;
}
#endif

int do_build(int ac, char const* const* av)
{
#ifdef CMAKE_BOOTSTRAP
  std::cerr << "This cmake does not support --build\n";
  return -1;
#else
  int jobs = cmake::NO_BUILD_PARALLEL_LEVEL;
  std::vector<std::string> targets;
  std::string config;
  std::string dir;
  std::vector<std::string> nativeOptions;
  bool nativeOptionsPassed = false;
  bool cleanFirst = false;
  bool foundClean = false;
  bool foundNonClean = false;
  bool verbose = cmSystemTools::HasEnv("VERBOSE");

  auto jLambda = [&](std::string const& value) -> bool {
    jobs = extract_job_number("-j", value);
    if (jobs < 0) {
      dir.clear();
    }
    return true;
  };
  auto parallelLambda = [&](std::string const& value) -> bool {
    jobs = extract_job_number("--parallel", value);
    if (jobs < 0) {
      dir.clear();
    }
    return true;
  };
  auto targetLambda = [&](std::string const& value) -> bool {
    if (!value.empty()) {
      std::vector<std::string> values = cmExpandedList(value);
      for (auto const& v : values) {
        targets.emplace_back(v);
        if (v == "clean") {
          foundClean = true;
        } else {
          foundNonClean = true;
        }
      }
      return true;
    }
    return false;
  };
  auto verboseLambda = [&](std::string const&) -> bool {
    verbose = true;
    return true;
  };

  using CommandArgument =
    cmCommandLineArgument<bool(std::string const& value)>;

  std::vector<CommandArgument> arguments = {
    CommandArgument{ "-j", CommandArgument::Values::ZeroOrOne, jLambda },
    CommandArgument{ "--parallel", CommandArgument::Values::ZeroOrOne,
                     parallelLambda },
    CommandArgument{ "-t", CommandArgument::Values::OneOrMore, targetLambda },
    CommandArgument{ "--target", CommandArgument::Values::OneOrMore,
                     targetLambda },
    CommandArgument{ "--config", CommandArgument::Values::One,
                     [&](std::string const& value) -> bool {
                       config = value;
                       return true;
                     } },
    CommandArgument{ "--clean-first", CommandArgument::Values::Zero,
                     [&](std::string const&) -> bool {
                       cleanFirst = true;
                       return true;
                     } },
    CommandArgument{ "-v", CommandArgument::Values::Zero, verboseLambda },
    CommandArgument{ "--verbose", CommandArgument::Values::Zero,
                     verboseLambda },
    /* legacy option no-op*/
    CommandArgument{ "--use-stderr", CommandArgument::Values::Zero,
                     [](std::string const&) -> bool { return true; } },
    CommandArgument{ "--", CommandArgument::Values::Zero,
                     [&](std::string const&) -> bool {
                       nativeOptionsPassed = true;
                       return true;
                     } },
  };

  if (ac >= 3) {
    dir = cmSystemTools::CollapseFullPath(av[2]);

    std::vector<std::string> inputArgs;
    inputArgs.reserve(ac - 3);
    cm::append(inputArgs, av + 3, av + ac);

    decltype(inputArgs.size()) i = 0;
    for (; i < inputArgs.size() && !nativeOptionsPassed; ++i) {

      std::string const& arg = inputArgs[i];
      for (auto const& m : arguments) {
        if (m.matches(arg) && m.parse(arg, i, inputArgs)) {
          break;
        }
      }
    }

    if (nativeOptionsPassed) {
      cm::append(nativeOptions, inputArgs.begin() + i, inputArgs.end());
    }
  }

  if (foundClean && foundNonClean) {
    std::cerr << "Error: Building 'clean' and other targets together "
                 "is not supported."
              << std::endl;
    dir.clear();
  }

  if (jobs == cmake::NO_BUILD_PARALLEL_LEVEL) {
    std::string parallel;
    if (cmSystemTools::GetEnv("CMAKE_BUILD_PARALLEL_LEVEL", parallel)) {
      if (parallel.empty()) {
        jobs = cmake::DEFAULT_BUILD_PARALLEL_LEVEL;
      } else {
        unsigned long numJobs = 0;
        if (cmStrToULong(parallel, &numJobs)) {
          if (numJobs == 0) {
            std::cerr << "The CMAKE_BUILD_PARALLEL_LEVEL environment variable "
                         "requires a positive integer argument.\n\n";
            dir.clear();
          } else if (numJobs > INT_MAX) {
            std::cerr << "The CMAKE_BUILD_PARALLEL_LEVEL environment variable "
                         "is too large.\n\n";
            dir.clear();
          } else {
            jobs = int(numJobs);
          }
        } else {
          std::cerr << "'CMAKE_BUILD_PARALLEL_LEVEL' environment variable\n"
                    << "invalid number '" << parallel << "' given.\n\n";
          dir.clear();
        }
      }
    }
  }

  if (dir.empty()) {
    /* clang-format off */
    std::cerr <<
      "Usage: cmake --build <dir> [options] [-- [native-options]]\n"
      "Options:\n"
      "  <dir>          = Project binary directory to be built.\n"
      "  --parallel [<jobs>], -j [<jobs>]\n"
      "                 = Build in parallel using the given number of jobs. \n"
      "                   If <jobs> is omitted the native build tool's \n"
      "                   default number is used.\n"
      "                   The CMAKE_BUILD_PARALLEL_LEVEL environment "
      "variable\n"
      "                   specifies a default parallel level when this "
      "option\n"
      "                   is not given.\n"
      "  --target <tgt>..., -t <tgt>... \n"
      "                 = Build <tgt> instead of default targets.\n"
      "  --config <cfg> = For multi-configuration tools, choose <cfg>.\n"
      "  --clean-first  = Build target 'clean' first, then build.\n"
      "                   (To clean only, use --target 'clean'.)\n"
      "  --verbose, -v  = Enable verbose output - if supported - including\n"
      "                   the build commands to be executed. \n"
      "  --             = Pass remaining options to the native tool.\n"
      ;
    /* clang-format on */
    return 1;
  }

  cmake cm(cmake::RoleInternal, cmState::Project);
  cmSystemTools::SetMessageCallback(
    [&cm](const std::string& msg, const char* title) {
      cmakemainMessageCallback(msg, title, &cm);
    });
  cm.SetProgressCallback([&cm](const std::string& msg, float prog) {
    cmakemainProgressCallback(msg, prog, &cm);
  });
  return cm.Build(jobs, dir, targets, config, nativeOptions, cleanFirst,
                  verbose);
#endif
}

bool parse_default_directory_permissions(const std::string& permissions,
                                         std::string& parsedPermissionsVar)
{
  std::vector<std::string> parsedPermissions;
  enum Doing
  {
    DoingNone,
    DoingOwner,
    DoingGroup,
    DoingWorld,
    DoingOwnerAssignment,
    DoingGroupAssignment,
    DoingWorldAssignment,
  };
  Doing doing = DoingNone;

  auto uniquePushBack = [&parsedPermissions](const std::string& e) {
    if (std::find(parsedPermissions.begin(), parsedPermissions.end(), e) ==
        parsedPermissions.end()) {
      parsedPermissions.push_back(e);
    }
  };

  for (auto const& e : permissions) {
    switch (doing) {
      case DoingNone:
        if (e == 'u') {
          doing = DoingOwner;
        } else if (e == 'g') {
          doing = DoingGroup;
        } else if (e == 'o') {
          doing = DoingWorld;
        } else {
          return false;
        }
        break;
      case DoingOwner:
        if (e == '=') {
          doing = DoingOwnerAssignment;
        } else {
          return false;
        }
        break;
      case DoingGroup:
        if (e == '=') {
          doing = DoingGroupAssignment;
        } else {
          return false;
        }
        break;
      case DoingWorld:
        if (e == '=') {
          doing = DoingWorldAssignment;
        } else {
          return false;
        }
        break;
      case DoingOwnerAssignment:
        if (e == 'r') {
          uniquePushBack("OWNER_READ");
        } else if (e == 'w') {
          uniquePushBack("OWNER_WRITE");
        } else if (e == 'x') {
          uniquePushBack("OWNER_EXECUTE");
        } else if (e == ',') {
          doing = DoingNone;
        } else {
          return false;
        }
        break;
      case DoingGroupAssignment:
        if (e == 'r') {
          uniquePushBack("GROUP_READ");
        } else if (e == 'w') {
          uniquePushBack("GROUP_WRITE");
        } else if (e == 'x') {
          uniquePushBack("GROUP_EXECUTE");
        } else if (e == ',') {
          doing = DoingNone;
        } else {
          return false;
        }
        break;
      case DoingWorldAssignment:
        if (e == 'r') {
          uniquePushBack("WORLD_READ");
        } else if (e == 'w') {
          uniquePushBack("WORLD_WRITE");
        } else if (e == 'x') {
          uniquePushBack("WORLD_EXECUTE");
        } else if (e == ',') {
          doing = DoingNone;
        } else {
          return false;
        }
        break;
    }
  }
  if (doing != DoingOwnerAssignment && doing != DoingGroupAssignment &&
      doing != DoingWorldAssignment) {
    return false;
  }

  std::ostringstream oss;
  for (auto i = 0u; i < parsedPermissions.size(); i++) {
    if (i != 0) {
      oss << ";";
    }
    oss << parsedPermissions[i];
  }

  parsedPermissionsVar = oss.str();
  return true;
}

int do_install(int ac, char const* const* av)
{
#ifdef CMAKE_BOOTSTRAP
  std::cerr << "This cmake does not support --install\n";
  return -1;
#else
  assert(1 < ac);

  std::string config;
  std::string component;
  std::string defaultDirectoryPermissions;
  std::string prefix;
  std::string dir;
  bool strip = false;
  bool verbose = cmSystemTools::HasEnv("VERBOSE");

  auto verboseLambda = [&](std::string const&) -> bool {
    verbose = true;
    return true;
  };

  using CommandArgument =
    cmCommandLineArgument<bool(std::string const& value)>;

  std::vector<CommandArgument> arguments = {
    CommandArgument{ "--config", CommandArgument::Values::One,
                     [&](std::string const& value) -> bool {
                       config = value;
                       return true;
                     } },
    CommandArgument{ "--component", CommandArgument::Values::One,
                     [&](std::string const& value) -> bool {
                       component = value;
                       return true;
                     } },
    CommandArgument{ "--default-directory-permissions",
                     CommandArgument::Values::One,
                     [&](std::string const& value) -> bool {
                       defaultDirectoryPermissions = value;
                       return true;
                     } },
    CommandArgument{ "--prefix", CommandArgument::Values::One,
                     [&](std::string const& value) -> bool {
                       prefix = value;
                       return true;
                     } },
    CommandArgument{ "--strip", CommandArgument::Values::Zero,
                     [&](std::string const&) -> bool {
                       strip = true;
                       return true;
                     } },
    CommandArgument{ "-v", CommandArgument::Values::Zero, verboseLambda },
    CommandArgument{ "--verbose", CommandArgument::Values::Zero,
                     verboseLambda }
  };

  if (ac >= 3) {
    dir = cmSystemTools::CollapseFullPath(av[2]);

    std::vector<std::string> inputArgs;
    inputArgs.reserve(ac - 3);
    cm::append(inputArgs, av + 3, av + ac);
    for (decltype(inputArgs.size()) i = 0; i < inputArgs.size(); ++i) {

      std::string const& arg = inputArgs[i];
      for (auto const& m : arguments) {
        if (m.matches(arg) && m.parse(arg, i, inputArgs)) {
          break;
        }
      }
    }
  }

  if (dir.empty()) {
    /* clang-format off */
    std::cerr <<
      "Usage: cmake --install <dir> [options]\n"
      "Options:\n"
      "  <dir>              = Project binary directory to install.\n"
      "  --config <cfg>     = For multi-configuration tools, choose <cfg>.\n"
      "  --component <comp> = Component-based install. Only install <comp>.\n"
      "  --default-directory-permissions <permission> \n"
      "     Default install permission. Use default permission <permission>.\n"
      "  --prefix <prefix>  = The installation prefix CMAKE_INSTALL_PREFIX.\n"
      "  --strip            = Performing install/strip.\n"
      "  -v --verbose       = Enable verbose output.\n"
      ;
    /* clang-format on */
    return 1;
  }

  cmake cm(cmake::RoleScript, cmState::Script);

  cmSystemTools::SetMessageCallback(
    [&cm](const std::string& msg, const char* title) {
      cmakemainMessageCallback(msg, title, &cm);
    });
  cm.SetProgressCallback([&cm](const std::string& msg, float prog) {
    cmakemainProgressCallback(msg, prog, &cm);
  });
  cm.SetHomeDirectory("");
  cm.SetHomeOutputDirectory("");
  cm.SetDebugOutputOn(verbose);
  cm.SetWorkingMode(cmake::SCRIPT_MODE);

  std::vector<std::string> args{ av[0] };

  if (!prefix.empty()) {
    args.emplace_back("-DCMAKE_INSTALL_PREFIX=" + prefix);
  }

  if (!component.empty()) {
    args.emplace_back("-DCMAKE_INSTALL_COMPONENT=" + component);
  }

  if (strip) {
    args.emplace_back("-DCMAKE_INSTALL_DO_STRIP=1");
  }

  if (!config.empty()) {
    args.emplace_back("-DCMAKE_INSTALL_CONFIG_NAME=" + config);
  }

  if (!defaultDirectoryPermissions.empty()) {
    std::string parsedPermissionsVar;
    if (!parse_default_directory_permissions(defaultDirectoryPermissions,
                                             parsedPermissionsVar)) {
      std::cerr << "--default-directory-permissions is in incorrect format"
                << std::endl;
      return 1;
    }
    args.emplace_back("-DCMAKE_INSTALL_DEFAULT_DIRECTORY_PERMISSIONS=" +
                      parsedPermissionsVar);
  }

  args.emplace_back("-P");
  args.emplace_back(dir + "/cmake_install.cmake");

  return cm.Run(args) ? 1 : 0;
#endif
}

int do_open(int ac, char const* const* av)
{
#ifdef CMAKE_BOOTSTRAP
  std::cerr << "This cmake does not support --open\n";
  return -1;
#else
  std::string dir;

  enum Doing
  {
    DoingNone,
    DoingDir,
  };
  Doing doing = DoingDir;
  for (int i = 2; i < ac; ++i) {
    switch (doing) {
      case DoingDir:
        dir = cmSystemTools::CollapseFullPath(av[i]);
        doing = DoingNone;
        break;
      default:
        std::cerr << "Unknown argument " << av[i] << std::endl;
        dir.clear();
        break;
    }
  }
  if (dir.empty()) {
    std::cerr << "Usage: cmake --open <dir>\n";
    return 1;
  }

  cmake cm(cmake::RoleInternal, cmState::Unknown);
  cmSystemTools::SetMessageCallback(
    [&cm](const std::string& msg, const char* title) {
      cmakemainMessageCallback(msg, title, &cm);
    });
  cm.SetProgressCallback([&cm](const std::string& msg, float prog) {
    cmakemainProgressCallback(msg, prog, &cm);
  });
  return cm.Open(dir, false) ? 0 : 1;
#endif
}
} // namespace

int main(int ac, char const* const* av)
{
  cmSystemTools::EnsureStdPipes();

  // Replace streambuf so we can output Unicode to console
  auto consoleBuf = cm::make_unique<cmConsoleBuf>();
  consoleBuf->SetUTF8Pipes();

  cmsys::Encoding::CommandLineArguments args =
    cmsys::Encoding::CommandLineArguments::Main(ac, av);
  ac = args.argc();
  av = args.argv();

  cmSystemTools::InitializeLibUV();
  cmSystemTools::FindCMakeResources(av[0]);
  if (ac > 1) {
    if (strcmp(av[1], "--build") == 0) {
      return do_build(ac, av);
    }
    if (strcmp(av[1], "--install") == 0) {
      return do_install(ac, av);
    }
    if (strcmp(av[1], "--open") == 0) {
      return do_open(ac, av);
    }
    if (strcmp(av[1], "-E") == 0) {
      return do_command(ac, av, std::move(consoleBuf));
    }
  }
  int ret = do_cmake(ac, av);
#ifndef CMAKE_BOOTSTRAP
  cmDynamicLoader::FlushCache();
#endif
  if (uv_loop_t* loop = uv_default_loop()) {
    uv_loop_close(loop);
  }
  return ret;
}
