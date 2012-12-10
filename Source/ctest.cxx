/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTest.h"
#include "cmSystemTools.h"

// Need these for documentation support.
#include "cmake.h"
#include "cmDocumentation.h"

#include "CTest/cmCTestScriptHandler.h"
#include "CTest/cmCTestLaunch.h"

//----------------------------------------------------------------------------
static const char * cmDocumentationName[][3] =
{
  {0,
   "  ctest - Testing driver provided by CMake.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationUsage[][3] =
{
  {0,
   "  ctest [options]", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationDescription[][3] =
{
  {0,
   "The \"ctest\" executable is the CMake test driver program.  "
   "CMake-generated build trees created for projects that use "
   "the ENABLE_TESTING and ADD_TEST commands have testing support.  "
   "This program will run the tests and report results.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationOptions[][3] =
{
  {"-C <cfg>, --build-config <cfg>", "Choose configuration to test.",
   "Some CMake-generated build trees can have multiple build configurations "
   "in the same tree.  This option can be used to specify which one should "
   "be tested.  Example configurations are \"Debug\" and \"Release\"."},
  {"-V,--verbose", "Enable verbose output from tests.",
   "Test output is normally suppressed and only summary information is "
   "displayed.  This option will show all test output."},
  {"-VV,--extra-verbose", "Enable more verbose output from tests.",
   "Test output is normally suppressed and only summary information is "
   "displayed.  This option will show even more test output."},
  {"--debug", "Displaying more verbose internals of CTest.",
    "This feature will result in large number of output that is mostly "
    "useful for debugging dashboard problems."},
  {"--output-on-failure", "Output anything outputted by the test program "
   "if the test should fail.  This option can also be enabled by setting "
   "the environment variable CTEST_OUTPUT_ON_FAILURE"},
  {"-F", "Enable failover.", "This option allows ctest to resume a test "
   "set execution that was previously interrupted.  If no interruption "
   "occurred, the -F option will have no effect."},
  {"-j <jobs>, --parallel <jobs>", "Run the tests in parallel using the"
   "given number of jobs.",
   "This option tells ctest to run the tests in parallel using given "
   "number of jobs."},
  {"-Q,--quiet", "Make ctest quiet.",
    "This option will suppress all the output. The output log file will "
    "still be generated if the --output-log is specified. Options such "
    "as --verbose, --extra-verbose, and --debug are ignored if --quiet is "
    "specified."},
  {"-O <file>, --output-log <file>", "Output to log file",
   "This option tells ctest to write all its output to a log file."},
  {"-N,--show-only", "Disable actual execution of tests.",
   "This option tells ctest to list the tests that would be run but not "
   "actually run them.  Useful in conjunction with the -R and -E options."},
  {"-L <regex>, --label-regex <regex>", "Run tests with labels matching "
   "regular expression.",
   "This option tells ctest to run only the tests whose labels match the "
   "given regular expression."},
  {"-R <regex>, --tests-regex <regex>", "Run tests matching regular "
   "expression.",
   "This option tells ctest to run only the tests whose names match the "
   "given regular expression."},
  {"-E <regex>, --exclude-regex <regex>", "Exclude tests matching regular "
   "expression.",
   "This option tells ctest to NOT run the tests whose names match the "
   "given regular expression."},
  {"-LE <regex>, --label-exclude <regex>", "Exclude tests with labels "
   "matching regular expression.",
   "This option tells ctest to NOT run the tests whose labels match the "
   "given regular expression."},
  {"-D <dashboard>, --dashboard <dashboard>", "Execute dashboard test",
   "This option tells ctest to perform act as a Dart client and perform "
   "a dashboard test. All tests are <Mode><Test>, where Mode can be "
   "Experimental, Nightly, and Continuous, and Test can be Start, Update, "
   "Configure, Build, Test, Coverage, and Submit."},
  {"-D <var>:<type>=<value>", "Define a variable for script mode",
   "Pass in variable values on the command line. Use in "
   "conjunction with -S to pass variable values to a dashboard script. "
   "Parsing -D arguments as variable values is only attempted if "
   "the value following -D does not match any of the known dashboard "
   "types."},
  {"-M <model>, --test-model <model>", "Sets the model for a dashboard",
   "This option tells ctest to act as a Dart client "
   "where the TestModel can be Experimental, "
   "Nightly, and Continuous. Combining -M and -T is similar to -D"},
  {"-T <action>, --test-action <action>", "Sets the dashboard action to "
   "perform",
   "This option tells ctest to act as a Dart client "
   "and perform some action such as start, build, test etc. "
   "Combining -M and -T is similar to -D"},
  {"--track <track>", "Specify the track to submit dashboard to",
   "Submit dashboard to specified track instead of default one. By "
   "default, the dashboard is submitted to Nightly, Experimental, or "
   "Continuous track, but by specifying this option, the track can be "
   "arbitrary."},
  {"-S <script>, --script <script>", "Execute a dashboard for a "
   "configuration",
   "This option tells ctest to load in a configuration script which sets "
   "a number of parameters such as the binary and source directories. Then "
   "ctest will do what is required to create and run a dashboard. This "
   "option basically sets up a dashboard and then runs ctest -D with the "
   "appropriate options."},
  {"-SP <script>, --script-new-process <script>", "Execute a dashboard for a "
   "configuration",
   "This option does the same operations as -S but it will do them in a "
   "separate process. This is primarily useful in cases where the script "
   "may modify the environment and you do not want the modified environment "
   "to impact other -S scripts."},
  {"-A <file>, --add-notes <file>", "Add a notes file with submission",
   "This option tells ctest to include a notes file when submitting "
   "dashboard. "},
  {"-I [Start,End,Stride,test#,test#|Test file], --tests-information",
   "Run a specific number of tests by number.",
   "This option causes ctest to run tests starting at number Start, ending "
   "at number End, and incrementing by Stride. Any additional numbers after "
   "Stride are considered individual test numbers.  Start, End,or stride "
   "can be empty.  Optionally a file can be given that contains the same "
   "syntax as the command line."},
  {"-U, --union", "Take the Union of -I and -R",
   "When both -R and -I are specified by default the intersection of "
   "tests are run. By specifying -U the union of tests is run instead."},
  {"--max-width <width>", "Set the max width for a test name to output",
   "Set the maximum width for each test name to show in the output.  This "
   "allows the user to widen the output to avoid clipping the test name which "
   "can be very annoying."},
  {"--interactive-debug-mode [0|1]", "Set the interactive mode to 0 or 1.",
   "This option causes ctest to run tests in either an interactive mode or "
   "a non-interactive mode. On Windows this means that in non-interactive "
   "mode, all system debug pop up windows are blocked. In dashboard mode "
   "(Experimental, Nightly, Continuous), the default is non-interactive.  "
   "When just running tests not for a dashboard the default is to allow "
   "popups and interactive "
   "debugging."},
  {"--no-label-summary", "Disable timing summary information for labels.",
   "This option tells ctest not to print summary information for each label "
   "associated with the tests run. If there are no labels on the "
   "tests, nothing extra is printed."},
  {"--build-and-test", "Configure, build and run a test.",
   "This option tells ctest to configure (i.e. run cmake on), build, and or "
   "execute a test. The configure and test steps are optional. The arguments "
   "to this command line are the source and binary directories. By default "
   "this will run CMake on the Source/Bin directories specified unless "
   "--build-nocmake is specified. Both --build-makeprogram and "
   "--build-generator MUST be provided to use --built-and-test. If "
   "--test-command is specified then that will be run after the build is "
   "complete. Other options that affect this mode are --build-target "
   "--build-nocmake, --build-run-dir, "
   "--build-two-config, --build-exe-dir, --build-project,"
   "--build-noclean, --build-options"},
  {"--build-target", "Specify a specific target to build.",
   "This option goes with the --build-and-test option, if left out the all "
   "target is built." },
  {"--build-nocmake", "Run the build without running cmake first.",
   "Skip the cmake step." },
  {"--build-run-dir", "Specify directory to run programs from.",
   "Directory where programs will be after it has been compiled." },
  {"--build-two-config", "Run CMake twice", "" },
  {"--build-exe-dir", "Specify the directory for the executable.", "" },
  {"--build-generator", "Specify the generator to use.", "" },
  {"--build-generator-toolset", "Specify the generator-specific toolset.",""},
  {"--build-project", "Specify the name of the project to build.", "" },
  {"--build-makeprogram", "Specify the make program to use.", "" },
  {"--build-noclean", "Skip the make clean step.", "" },
  {"--build-config-sample",
   "A sample executable to use to determine the configuration",
   "A sample executable to use to determine the configuration that "
   "should be used. e.g. Debug/Release/etc" },
  {"--build-options", "Add extra options to the build step.",
   "This option must be the last option with the exception of --test-command"
  },

  {"--test-command", "The test to run with the --build-and-test option.", ""
  },
  {"--test-timeout", "The time limit in seconds, internal use only.", ""
  },
  {"--tomorrow-tag", "Nightly or experimental starts with next day tag.",
   "This is useful if the build will not finish in one day." },
  {"--ctest-config", "The configuration file used to initialize CTest state "
  "when submitting dashboards.",
   "This option tells CTest to use different initialization file instead of "
   "CTestConfiguration.tcl. This way multiple initialization files can be "
   "used for example to submit to multiple dashboards." },
  {"--overwrite", "Overwrite CTest configuration option.",
   "By default ctest uses configuration options from configuration file. "
   "This option will overwrite the configuration option." },
  {"--extra-submit <file>[;<file>]", "Submit extra files to the dashboard.",
   "This option will submit extra files to the dashboard." },
  {"--force-new-ctest-process", "Run child CTest instances as new processes",
   "By default CTest will run child CTest instances within the same process. "
   "If this behavior is not desired, this argument will enforce new "
   "processes for child CTest processes." },
  {"--schedule-random", "Use a random order for scheduling tests",
   "This option will run the tests in a random order. It is commonly used to "
   "detect implicit dependencies in a test suite." },
  {"--submit-index", "Submit individual dashboard tests with specific index",
   "This option allows performing the same CTest action (such as test) "
   "multiple times and submit all stages to the same dashboard (Dart2 "
   "required). Each execution requires different index." },
  {"--timeout <seconds>", "Set a global timeout on all tests.",
   "This option will set a global timeout on all tests that do not already "
   "have a timeout set on them."},
  {"--stop-time <time>", "Set a time at which all tests should stop running.",
   "Set a real time of day at which all tests should timeout. Example: "
   "7:00:00 -0400. Any time format understood by the curl date parser is "
   "accepted. Local time is assumed if no timezone is specified."},
  {"--http1.0", "Submit using HTTP 1.0.",
  "This option will force CTest to use HTTP 1.0 to submit files to the "
  "dashboard, instead of HTTP 1.1."},
  {"--no-compress-output", "Do not compress test output when submitting.",
   "This flag will turn off automatic compression of test output.  Use this "
   "to maintain compatibility with an older version of CDash which doesn't "
   "support compressed test output."},
  {"--print-labels", "Print all available test labels.",
   "This option will not run any tests, it will simply print the list of "
   "all labels associated with the test set."},
  {"--help-command <cmd> [<file>]", "Show help for a single command and exit.",
   "Prints the help for the command to stdout or to the specified file." },
  {"--help-command-list [<file>]", "List available commands and exit.",
   "Prints the list of all available listfile commands to stdout or the "
   "specified file." },
  {"--help-commands [<file>]", "Print help for all commands and exit.",
   "Prints the help for all commands to stdout or to the specified file." },
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationSeeAlso[][3] =
{
  {0, "cmake", 0},
  {0, "ccmake", 0},
  {0, 0, 0}
};

// this is a test driver program for cmCTest.
int main (int argc, char *argv[])
{
  cmSystemTools::DoNotInheritStdPipes();
  cmSystemTools::EnableMSVCDebugHook();
  cmSystemTools::FindExecutableDirectory(argv[0]);

  // Dispatch 'ctest --launch' mode directly.
  if(argc >= 2 && strcmp(argv[1], "--launch") == 0)
    {
    return cmCTestLaunch::Main(argc, argv);
    }

  int nocwd = 0;
  cmCTest inst;

  if ( cmSystemTools::GetCurrentWorkingDirectory().size() == 0 )
    {
    cmCTestLog(&inst, ERROR_MESSAGE,
      "Current working directory cannot be established." << std::endl);
    nocwd = 1;
    }

  // If there is a testing input file, check for documentation options
  // only if there are actually arguments.  We want running without
  // arguments to run tests.
  if(argc > 1 || !(cmSystemTools::FileExists("CTestTestfile.cmake") ||
                   cmSystemTools::FileExists("DartTestfile.txt")))
    {
    if(argc == 1)
      {
      cmCTestLog(&inst, ERROR_MESSAGE, "*********************************"
        << std::endl
        << "No test configuration file found!" << std::endl
        << "*********************************" << std::endl);
      }
    cmDocumentation doc;
    doc.addCTestStandardDocSections();
    if(doc.CheckOptions(argc, argv) || nocwd)
      {
      // Construct and print requested documentation.
      std::vector<cmDocumentationEntry> commands;
      cmCTestScriptHandler* ch =
                 static_cast<cmCTestScriptHandler*>(inst.GetHandler("script"));
      ch->CreateCMake();
      ch->GetCommandDocumentation(commands);

      doc.SetShowGenerators(false);
      doc.SetName("ctest");
      doc.SetSection("Name",cmDocumentationName);
      doc.SetSection("Usage",cmDocumentationUsage);
      doc.SetSection("Description",cmDocumentationDescription);
      doc.PrependSection("Options",cmDocumentationOptions);
      doc.SetSection("Commands",commands);
      doc.SetSeeAlsoList(cmDocumentationSeeAlso);
#ifdef cout
#  undef cout
#endif
      return doc.PrintRequestedDocumentation(std::cout)? 0:1;
#define cout no_cout_use_cmCTestLog
      }
    }

#ifdef _WIN32
  std::string comspec = "cmw9xcom.exe";
  cmSystemTools::SetWindows9xComspecSubstitute(comspec.c_str());
#endif
  // copy the args to a vector
  std::vector<std::string> args;
  for(int i =0; i < argc; ++i)
    {
    args.push_back(argv[i]);
    }
  // run ctest
  std::string output;
  int res = inst.Run(args,&output);
  cmCTestLog(&inst, OUTPUT, output);

  return res;
}

