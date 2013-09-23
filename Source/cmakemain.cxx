/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
// include these first, otherwise there will be problems on Windows
// with GetCurrentDirectory() being redefined
#ifdef CMAKE_BUILD_WITH_CMAKE
#include "cmDynamicLoader.h"
#include "cmDocumentation.h"
#endif

#include "cmake.h"
#include "cmcmd.h"
#include "cmCacheManager.h"
#include "cmListFileCache.h"
#include "cmakewizard.h"
#include "cmSourceFile.h"
#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"

#ifdef CMAKE_BUILD_WITH_CMAKE
//----------------------------------------------------------------------------
static const char * cmDocumentationName[][3] =
{
  {0,
   "  cmake - Cross-Platform Makefile Generator.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationUsage[][3] =
{
  {0,
   "  cmake [options] <path-to-source>\n"
   "  cmake [options] <path-to-existing-build>", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationDescription[][3] =
{
  {0,
   "The \"cmake\" executable is the CMake command-line interface.  It may "
   "be used to configure projects in scripts.  Project configuration "
   "settings "
   "may be specified on the command line with the -D option.  The -i option "
   "will cause cmake to interactively prompt for such settings.", 0},
  CMAKE_STANDARD_INTRODUCTION,
  {0,0,0}
};

#define CMAKE_BUILD_OPTIONS                                             \
  "  <dir>          = Project binary directory to be built.\n"          \
  "  --target <tgt> = Build <tgt> instead of default targets.\n"        \
  "  --config <cfg> = For multi-configuration tools, choose <cfg>.\n"   \
  "  --clean-first  = Build target 'clean' first, then build.\n"        \
  "                   (To clean only, use --target 'clean'.)\n"         \
  "  --use-stderr   = Don't merge stdout/stderr output and pass the\n"  \
  "                   original stdout/stderr handles to the native\n"   \
  "                   tool so it can use the capabilities of the\n"     \
  "                   calling terminal (e.g. colored output).\n"        \
  "  --             = Pass remaining options to the native tool.\n"

//----------------------------------------------------------------------------
static const char * cmDocumentationOptions[][3] =
{
  CMAKE_STANDARD_OPTIONS_TABLE,
  {"-E", "CMake command mode.",
   "For true platform independence, CMake provides a list of commands "
   "that can be used on all systems. Run with -E help for the usage "
   "information. Commands available are: chdir, compare_files, copy, "
   "copy_directory, copy_if_different, echo, echo_append, environment, "
   "make_directory, md5sum, remove, remove_directory, rename, tar, time, "
   "touch, touch_nocreate. In addition, some platform specific commands "
   "are available. "
   "On Windows: comspec, delete_regv, write_regv. "
   "On UNIX: create_symlink."},
  {"-i", "Run in wizard mode.",
   "Wizard mode runs cmake interactively without a GUI.  The user is "
   "prompted to answer questions about the project configuration.  "
   "The answers are used to set cmake cache values."},
  {"-L[A][H]", "List non-advanced cached variables.",
   "List cache variables will run CMake and list all the variables from the "
   "CMake cache that are not marked as INTERNAL or ADVANCED. This will "
   "effectively display current CMake settings, which can then be changed "
   "with -D option. Changing some of the variables may result in more "
   "variables being created. If A is specified, then it will display also "
   "advanced variables. If H is specified, it will also display help for "
   "each variable."},
  {"--build <dir>", "Build a CMake-generated project binary tree.",
   "This abstracts a native build tool's command-line interface with the "
   "following options:\n"
   CMAKE_BUILD_OPTIONS
   "Run cmake --build with no options for quick help."},
  {"-N", "View mode only.",
   "Only load the cache. Do not actually run configure and generate steps."},
  {"-P <file>", "Process script mode.",
   "Process the given cmake file as a script written in the CMake language.  "
   "No configure or generate step is performed and the cache is not"
   " modified. If variables are defined using -D, this must be done "
   "before the -P argument."},
  {"--find-package", "Run in pkg-config like mode.",
   "Search a package using find_package() and print the resulting flags "
   "to stdout. This can be used to use cmake instead of pkg-config to find "
   "installed libraries in plain Makefile-based projects or in "
   "autoconf-based projects (via share/aclocal/cmake.m4)."},
  {"--graphviz=[file]", "Generate graphviz of dependencies, see "
   "CMakeGraphVizOptions.cmake for more.",
   "Generate a graphviz input file that will contain all the library and "
   "executable dependencies in the project. See the documentation for "
   "CMakeGraphVizOptions.cmake for more details. "},
  {"--system-information [file]", "Dump information about this system.",
   "Dump a wide range of information about the current system. If run "
   "from the top of a binary tree for a CMake project it will dump "
   "additional information such as the cache, log files etc."},
  {"--debug-trycompile", "Do not delete the try_compile build tree. Only "
   "useful on one try_compile at a time.",
   "Do not delete the files and directories created for try_compile calls. "
   "This is useful in debugging failed try_compiles. It may however "
   "change the results of the try-compiles as old junk from a previous "
   "try-compile may cause a different test to either pass or fail "
   "incorrectly.  This option is best used for one try-compile at a time, "
   "and only when debugging." },
  {"--debug-output", "Put cmake in a debug mode.",
   "Print extra stuff during the cmake run like stack traces with "
   "message(send_error ) calls."},
  {"--trace", "Put cmake in trace mode.",
   "Print a trace of all calls made and from where with "
   "message(send_error ) calls."},
  {"--warn-uninitialized", "Warn about uninitialized values.",
   "Print a warning when an uninitialized variable is used."},
  {"--warn-unused-vars", "Warn about unused variables.",
   "Find variables that are declared or set, but not used."},
  {"--no-warn-unused-cli", "Don't warn about command line options.",
   "Don't find variables that are declared on the command line, but not "
   "used."},
  {"--check-system-vars", "Find problems with variable usage in system "
   "files.", "Normally, unused and uninitialized variables are searched for "
   "only in CMAKE_SOURCE_DIR and CMAKE_BINARY_DIR. This flag tells CMake to "
   "warn about other files as well."},
  {"--help-command cmd [file]", "Print help for a single command and exit.",
   "Full documentation specific to the given command is displayed. "
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-command-list [file]", "List available listfile commands and exit.",
   "The list contains all commands for which help may be obtained by using "
   "the --help-command argument followed by a command name. "
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-commands [file]", "Print help for all commands and exit.",
   "Full documentation specific for all current commands is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-compatcommands [file]", "Print help for compatibility commands. ",
   "Full documentation specific for all compatibility commands is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-module module [file]", "Print help for a single module and exit.",
   "Full documentation specific to the given module is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-module-list [file]", "List available modules and exit.",
   "The list contains all modules for which help may be obtained by using "
   "the --help-module argument followed by a module name. "
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-modules [file]", "Print help for all modules and exit.",
   "Full documentation for all modules is displayed. "
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-custom-modules [file]" , "Print help for all custom modules and "
   "exit.",
   "Full documentation for all custom modules is displayed. "
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-policy cmp [file]",
   "Print help for a single policy and exit.",
   "Full documentation specific to the given policy is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-policy-list [file]", "List available policies and exit.",
   "The list contains all policies for which help may be obtained by using "
   "the --help-policy argument followed by a policy name. "
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-policies [file]", "Print help for all policies and exit.",
   "Full documentation for all policies is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-property prop [file]",
   "Print help for a single property and exit.",
   "Full documentation specific to the given property is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-property-list [file]", "List available properties and exit.",
   "The list contains all properties for which help may be obtained by using "
   "the --help-property argument followed by a property name.  If a file is "
   "specified, the help is written into it."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-properties [file]", "Print help for all properties and exit.",
   "Full documentation for all properties is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-variable var [file]",
   "Print help for a single variable and exit.",
   "Full documentation specific to the given variable is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-variable-list [file]", "List documented variables and exit.",
   "The list contains all variables for which help may be obtained by using "
   "the --help-variable argument followed by a variable name.  If a file is "
   "specified, the help is written into it."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {"--help-variables [file]", "Print help for all variables and exit.",
   "Full documentation for all variables is displayed."
   "If a file is specified, the documentation is written into and the output "
   "format is determined depending on the filename suffix. Supported are man "
   "page, HTML, DocBook and plain text."},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationSeeAlso[][3] =
{
  {0, "ccmake", 0},
  {0, "cpack", 0},
  {0, "ctest", 0},
  {0, "cmakecommands", 0},
  {0, "cmakecompat", 0},
  {0, "cmakemodules", 0},
  {0, "cmakeprops", 0},
  {0, "cmakevars", 0},
  {0, 0, 0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationNOTE[][3] =
{
  {0,
   "CMake no longer configures a project when run with no arguments.  "
   "In order to configure the project in the current directory, run\n"
   "  cmake .", 0},
  {0,0,0}
};
#endif

int do_cmake(int ac, char** av);
static int do_build(int ac, char** av);

static cmMakefile* cmakemainGetMakefile(void *clientdata)
{
  cmake* cm = (cmake *)clientdata;
  if(cm && cm->GetDebugOutput())
    {
    cmGlobalGenerator* gg=cm->GetGlobalGenerator();
    if (gg)
      {
      cmLocalGenerator* lg=gg->GetCurrentLocalGenerator();
      if (lg)
        {
        cmMakefile* mf = lg->GetMakefile();
        return mf;
        }
      }
    }
  return 0;
}

static std::string cmakemainGetStack(void *clientdata)
{
  std::string msg;
  cmMakefile* mf=cmakemainGetMakefile(clientdata);
  if (mf)
    {
    msg = mf->GetListFileStack();
    if (!msg.empty())
      {
      msg = "\n   Called from: " + msg;
      }
    }

  return msg;
}

static void cmakemainErrorCallback(const char* m, const char*, bool&,
                                   void *clientdata)
{
  std::cerr << m << cmakemainGetStack(clientdata) << std::endl << std::flush;
}

static void cmakemainProgressCallback(const char *m, float prog,
                                      void* clientdata)
{
  cmMakefile* mf = cmakemainGetMakefile(clientdata);
  std::string dir;
  if ((mf) && (strstr(m, "Configuring")==m) && (prog<0))
    {
    dir = " ";
    dir += mf->GetCurrentDirectory();
    }
  else if ((mf) && (strstr(m, "Generating")==m))
    {
    dir = " ";
    dir += mf->GetCurrentOutputDirectory();
    }

  if ((prog < 0) || (!dir.empty()))
    {
    std::cout << "-- " << m << dir << cmakemainGetStack(clientdata)<<std::endl;
    }

  std::cout.flush();
}


int main(int ac, char** av)
{
  cmSystemTools::EnableMSVCDebugHook();
  cmSystemTools::FindExecutableDirectory(av[0]);
  if(ac > 1 && strcmp(av[1], "--build") == 0)
    {
    return do_build(ac, av);
    }
  int ret = do_cmake(ac, av);
#ifdef CMAKE_BUILD_WITH_CMAKE
  cmDynamicLoader::FlushCache();
#endif
  return ret;
}

int do_cmake(int ac, char** av)
{
  if ( cmSystemTools::GetCurrentWorkingDirectory().size() == 0 )
    {
    std::cerr << "Current working directory cannot be established."
              << std::endl;
    return 1;
    }

#ifdef CMAKE_BUILD_WITH_CMAKE
  cmDocumentation doc;
  doc.addCMakeStandardDocSections();
  if(doc.CheckOptions(ac, av, "-E"))
    {
    // Construct and print requested documentation.
    cmake hcm;
    hcm.AddCMakePaths();
    doc.SetCMakeRoot(hcm.GetCacheDefinition("CMAKE_ROOT"));

    // the command line args are processed here so that you can do
    // -DCMAKE_MODULE_PATH=/some/path and have this value accessible here
    std::vector<std::string> args;
    for(int i =0; i < ac; ++i)
      {
      args.push_back(av[i]);
      }
    hcm.SetCacheArgs(args);
    const char* modulePath = hcm.GetCacheDefinition("CMAKE_MODULE_PATH");
    if (modulePath)
      {
      doc.SetCMakeModulePath(modulePath);
      }

    std::vector<cmDocumentationEntry> commands;
    std::vector<cmDocumentationEntry> policies;
    std::vector<cmDocumentationEntry> compatCommands;
    std::vector<cmDocumentationEntry> generators;
    std::map<std::string,cmDocumentationSection *> propDocs;

    hcm.GetPolicyDocumentation(policies);
    hcm.GetCommandDocumentation(commands, true, false);
    hcm.GetCommandDocumentation(compatCommands, false, true);
    hcm.GetPropertiesDocumentation(propDocs);
    hcm.GetGeneratorDocumentation(generators);

    doc.SetName("cmake");
    doc.SetSection("Name",cmDocumentationName);
    doc.SetSection("Usage",cmDocumentationUsage);
    doc.SetSection("Description",cmDocumentationDescription);
    doc.AppendSection("Generators",generators);
    doc.PrependSection("Options",cmDocumentationOptions);
    doc.SetSection("Commands",commands);
    doc.SetSection("Policies",policies);
    doc.AppendSection("Compatibility Commands",compatCommands);
    doc.SetSections(propDocs);

    cmDocumentationEntry e;
    e.Brief =
      "variables defined by cmake, that give information about the project, "
      "and cmake";
    doc.PrependSection("Variables that Provide Information",e);

    doc.SetSeeAlsoList(cmDocumentationSeeAlso);
    int result = doc.PrintRequestedDocumentation(std::cout)? 0:1;

    // If we were run with no arguments, but a CMakeLists.txt file
    // exists, the user may have been trying to use the old behavior
    // of cmake to build a project in-source.  Print a message
    // explaining the change to standard error and return an error
    // condition in case the program is running from a script.
    if((ac == 1) && cmSystemTools::FileExists("CMakeLists.txt"))
      {
      doc.ClearSections();
      doc.SetSection("NOTE", cmDocumentationNOTE);
      doc.Print(cmDocumentation::UsageForm, 0, std::cerr);
      return 1;
      }
    return result;
    }
#else
  if ( ac == 1 )
    {
    std::cout <<
      "Bootstrap CMake should not be used outside CMake build process."
              << std::endl;
    return 0;
    }
#endif

  bool wiz = false;
  bool sysinfo = false;
  bool command = false;
  bool list_cached = false;
  bool list_all_cached = false;
  bool list_help = false;
  bool view_only = false;
  cmake::WorkingMode workingMode = cmake::NORMAL_MODE;
  std::vector<std::string> args;
  for(int i =0; i < ac; ++i)
    {
    if(!command && strcmp(av[i], "-i") == 0)
      {
      wiz = true;
      }
    else if(!command && strcmp(av[i], "--system-information") == 0)
      {
      sysinfo = true;
      }
    // if command has already been set, then
    // do not eat the -E
    else if (!command && strcmp(av[i], "-E") == 0)
      {
      command = true;
      }
    else if (!command && strcmp(av[i], "-N") == 0)
      {
      view_only = true;
      }
    else if (!command && strcmp(av[i], "-L") == 0)
      {
      list_cached = true;
      }
    else if (!command && strcmp(av[i], "-LA") == 0)
      {
      list_all_cached = true;
      }
    else if (!command && strcmp(av[i], "-LH") == 0)
      {
      list_cached = true;
      list_help = true;
      }
    else if (!command && strcmp(av[i], "-LAH") == 0)
      {
      list_all_cached = true;
      list_help = true;
      }
    else if (!command && strncmp(av[i], "-P", strlen("-P")) == 0)
      {
      if ( i == ac -1 )
        {
        cmSystemTools::Error("No script specified for argument -P");
        }
      else
        {
        workingMode = cmake::SCRIPT_MODE;
        args.push_back(av[i]);
        i++;
        args.push_back(av[i]);
        }
      }
    else if (!command && strncmp(av[i], "--find-package",
                                 strlen("--find-package")) == 0)
      {
      workingMode = cmake::FIND_PACKAGE_MODE;
      args.push_back(av[i]);
      }
    else
      {
      args.push_back(av[i]);
      }
    }
  if(command)
    {
    int ret = cmcmd::ExecuteCMakeCommand(args);
    return ret;
    }
  if (wiz)
    {
    cmakewizard wizard;
    return wizard.RunWizard(args);
    }
  if (sysinfo)
    {
    cmake cm;
    int ret = cm.GetSystemInformation(args);
    return ret;
    }
  cmake cm;
  cmSystemTools::SetErrorCallback(cmakemainErrorCallback, (void *)&cm);
  cm.SetProgressCallback(cmakemainProgressCallback, (void *)&cm);
  cm.SetWorkingMode(workingMode);

  int res = cm.Run(args, view_only);
  if ( list_cached || list_all_cached )
    {
    cmCacheManager::CacheIterator it =
      cm.GetCacheManager()->GetCacheIterator();
    std::cout << "-- Cache values" << std::endl;
    for ( it.Begin(); !it.IsAtEnd(); it.Next() )
      {
      cmCacheManager::CacheEntryType t = it.GetType();
      if ( t != cmCacheManager::INTERNAL && t != cmCacheManager::STATIC &&
        t != cmCacheManager::UNINITIALIZED )
        {
        bool advanced = it.PropertyExists("ADVANCED");
        if ( list_all_cached || !advanced)
          {
          if ( list_help )
            {
            std::cout << "// " << it.GetProperty("HELPSTRING") << std::endl;
            }
          std::cout << it.GetName() << ":" <<
            cmCacheManager::TypeToString(it.GetType())
            << "=" << it.GetValue() << std::endl;
          if ( list_help )
            {
            std::cout << std::endl;
            }
          }
        }
      }
    }

  // Always return a non-negative value.  Windows tools do not always
  // interpret negative return values as errors.
  if(res != 0)
    {
    return 1;
    }
  else
    {
    return 0;
    }
}

//----------------------------------------------------------------------------
static int do_build(int ac, char** av)
{
#ifndef CMAKE_BUILD_WITH_CMAKE
  std::cerr << "This cmake does not support --build\n";
  return -1;
#else
  std::string target;
  std::string config = "Debug";
  std::string dir;
  std::vector<std::string> nativeOptions;
  bool clean = false;
  cmSystemTools::OutputOption outputflag = cmSystemTools::OUTPUT_MERGE;

  enum Doing { DoingNone, DoingDir, DoingTarget, DoingConfig, DoingNative};
  Doing doing = DoingDir;
  for(int i=2; i < ac; ++i)
    {
    if(doing == DoingNative)
      {
      nativeOptions.push_back(av[i]);
      }
    else if(strcmp(av[i], "--target") == 0)
      {
      doing = DoingTarget;
      }
    else if(strcmp(av[i], "--config") == 0)
      {
      doing = DoingConfig;
      }
    else if(strcmp(av[i], "--clean-first") == 0)
      {
      clean = true;
      doing = DoingNone;
      }
    else if(strcmp(av[i], "--use-stderr") == 0)
      {
      outputflag = cmSystemTools::OUTPUT_PASSTHROUGH;
      }
    else if(strcmp(av[i], "--") == 0)
      {
      doing = DoingNative;
      }
    else
      {
      switch (doing)
        {
        case DoingDir:
          dir = av[i];
          doing = DoingNone;
          break;
        case DoingTarget:
          target = av[i];
          doing = DoingNone;
          break;
        case DoingConfig:
          config = av[i];
          doing = DoingNone;
          break;
        default:
          std::cerr << "Unknown argument " << av[i] << std::endl;
          dir = "";
          break;
        }
      }
    }
  if(dir.empty())
    {
    std::cerr <<
      "Usage: cmake --build <dir> [options] [-- [native-options]]\n"
      "Options:\n"
      CMAKE_BUILD_OPTIONS
      ;
    return 1;
    }

  // Hack for vs6 that passes ".\Debug" as "$(IntDir)" value:
  //
  if (cmSystemTools::StringStartsWith(config.c_str(), ".\\"))
    {
    config = config.substr(2);
    }

  cmake cm;
  return cm.Build(dir, target, config, nativeOptions, clean, outputflag);
#endif
}
