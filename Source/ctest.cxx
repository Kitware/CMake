/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */

#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "cmsys/Encoding.hxx"

#include "cmCTest.h"
#include "cmConsoleBuf.h"
#include "cmDocumentation.h"
#include "cmDocumentationEntry.h"
#include "cmInstrumentation.h"
#include "cmInstrumentationQuery.h"
#include "cmSystemTools.h"

#include "CTest/cmCTestLaunch.h"

namespace {
cmDocumentationEntry const cmDocumentationName = {
  {},
  "  ctest - Testing driver provided by CMake."
};

cmDocumentationEntry const cmDocumentationUsage = { {}, "  ctest [options]" };

cmDocumentationEntry const cmDocumentationOptions[] = {
  { "--preset <preset>, --preset=<preset>",
    "Read arguments from a test preset." },
  { "--list-presets", "List available test presets." },
  { "-C <cfg>, --build-config <cfg>", "Choose configuration to test." },
  { "--progress", "Enable short progress output from tests." },
  { "-V,--verbose", "Enable verbose output from tests." },
  { "-VV,--extra-verbose", "Enable more verbose output from tests." },
  { "--debug", "Displaying more verbose internals of CTest." },
  { "--output-on-failure",
    "Output anything outputted by the test program "
    "if the test should fail." },
  { "--stop-on-failure", "Stop running the tests after one has failed." },
  { "--test-output-size-passed <size>",
    "Limit the output for passed tests "
    "to <size> bytes" },
  { "--test-output-size-failed <size>",
    "Limit the output for failed tests "
    "to <size> bytes" },
  { "--test-output-truncation <mode>",
    "Truncate 'tail' (default), 'middle' or 'head' of test output once "
    "maximum output size is reached" },
  { "-F", "Enable failover." },
  { "-j [<level>], --parallel [<level>]",
    "Run tests in parallel, "
    "optionally limited to a given level of parallelism." },
  { "-Q,--quiet", "Make ctest quiet." },
  { "-O <file>, --output-log <file>", "Output to log file" },
  { "--output-junit <file>", "Output test results to JUnit XML file." },
  { "-N,--show-only[=format]",
    "Disable actual execution of tests. The optional 'format' defines the "
    "format of the test information and can be 'human' for the current text "
    "format or 'json-v1' for json format. Defaults to 'human'." },
  { "-L <regex>, --label-regex <regex>",
    "Run tests with labels matching regular expression. "
    "With multiple -L, run tests where each "
    "regular expression matches at least one label." },
  { "-R <regex>, --tests-regex <regex>",
    "Run tests matching regular "
    "expression." },
  { "-E <regex>, --exclude-regex <regex>",
    "Exclude tests matching regular "
    "expression." },
  { "-LE <regex>, --label-exclude <regex>",
    "Exclude tests with labels matching regular expression. "
    "With multiple -LE, exclude tests where each "
    "regular expression matches at least one label." },
  { "-FA <regex>, --fixture-exclude-any <regex>",
    "Do not automatically "
    "add any tests for "
    "fixtures matching "
    "regular expression." },
  { "-FS <regex>, --fixture-exclude-setup <regex>",
    "Do not automatically "
    "add setup tests for "
    "fixtures matching "
    "regular expression." },
  { "-FC <regex>, --fixture-exclude-cleanup <regex>",
    "Do not automatically "
    "add cleanup tests for "
    "fixtures matching "
    "regular expression." },
  { "-D <dashboard>, --dashboard <dashboard>", "Execute dashboard test" },
  { "-D <var>:<type>=<value>", "Define a variable for script mode" },
  { "-M <model>, --test-model <model>", "Sets the model for a dashboard" },
  { "-T <action>, --test-action <action>",
    "Sets the dashboard action to "
    "perform" },
  { "--group <group>",
    "Specify what build group on the dashboard you'd like to "
    "submit results to." },
  { "-S <script>, --script <script>",
    "Execute a dashboard for a "
    "configuration" },
  { "-SP <script>, --script-new-process <script>",
    "Execute a dashboard for a "
    "configuration" },
  { "-A <file>, --add-notes <file>", "Add a notes file with submission" },
  { "-I [Start,End,Stride,test#,test#|Test file], --tests-information",
    "Run a specific number of tests by number." },
  { "-U, --union", "Take the Union of -I and -R" },
  { "--rerun-failed", "Run only the tests that failed previously" },
  { "--tests-from-file <file>", "Run the tests listed in the given file" },
  { "--exclude-from-file <file>",
    "Run tests except those listed in the given file" },
  { "--repeat until-fail:<n>, --repeat-until-fail <n>",
    "Require each test to run <n> times without failing in order to pass" },
  { "--repeat until-pass:<n>",
    "Allow each test to run up to <n> times in order to pass" },
  { "--repeat after-timeout:<n>",
    "Allow each test to run up to <n> times if it times out" },
  { "--max-width <width>", "Set the max width for a test name to output" },
  { "--interactive-debug-mode [0|1]", "Set the interactive mode to 0 or 1." },
  { "--resource-spec-file <file>", "Set the resource spec file to use." },
  { "--no-label-summary", "Disable timing summary information for labels." },
  { "--no-subproject-summary",
    "Disable timing summary information for "
    "subprojects." },
  { "--test-dir <dir>", "Specify the directory in which to look for tests." },
  { "--build-and-test", "Configure, build and run a test." },
  { "--build-target", "Specify a specific target to build." },
  { "--build-nocmake", "Run the build without running cmake first." },
  { "--build-run-dir", "Specify directory to run programs from." },
  { "--build-two-config", "Run CMake twice" },
  { "--build-exe-dir", "Specify the directory for the executable." },
  { "--build-generator", "Specify the generator to use." },
  { "--build-generator-platform", "Specify the generator-specific platform." },
  { "--build-generator-toolset", "Specify the generator-specific toolset." },
  { "--build-project", "Specify the name of the project to build." },
  { "--build-makeprogram", "Specify the make program to use." },
  { "--build-noclean", "Skip the make clean step." },
  { "--build-config-sample",
    "A sample executable to use to determine the configuration" },
  { "--build-options", "Add extra options to the build step." },

  { "--test-command", "The test to run with the --build-and-test option." },
  { "--test-timeout", "The time limit in seconds, internal use only." },
  { "--test-load", "CPU load threshold for starting new parallel tests." },
  { "--tomorrow-tag", "Nightly or experimental starts with next day tag." },
  { "--overwrite", "Overwrite CTest configuration option." },
  { "--extra-submit <file>[;<file>]", "Submit extra files to the dashboard." },
  { "--http-header <header>", "Append HTTP header when submitting" },
  { "--schedule-random", "Use a random order for scheduling tests" },
  { "--submit-index",
    "Submit individual dashboard tests with specific index" },
  { "--timeout <seconds>", "Set the default test timeout." },
  { "--stop-time <time>",
    "Set a time at which all tests should stop running." },
  { "--http1.0", "Submit using HTTP 1.0." },
  { "--no-compress-output", "Do not compress test output when submitting." },
  { "--print-labels", "Print all available test labels." },
  { "--no-tests=<[error|ignore]>",
    "Regard no tests found either as 'error' or 'ignore' it." }
};
} // anonymous namespace

// this is a test driver program for cmCTest.
int main(int argc, char const* const* argv)
{
  cmSystemTools::EnsureStdPipes();

  // Replace streambuf so we can output Unicode to console
  cmConsoleBuf consoleBuf;
  consoleBuf.SetUTF8Pipes();

  cmsys::Encoding::CommandLineArguments encoding_args =
    cmsys::Encoding::CommandLineArguments::Main(argc, argv);
  argc = encoding_args.argc();
  argv = encoding_args.argv();

  cmSystemTools::InitializeLibUV();
  cmSystemTools::FindCMakeResources(argv[0]);

  // Dispatch 'ctest --launch' mode directly.
  if (argc >= 2 && strcmp(argv[1], "--launch") == 0) {
    return cmCTestLaunch::Main(argc, argv, cmCTestLaunch::Op::Normal);
  }

  // Dispatch 'ctest --instrument' mode directly.
  if (argc >= 2 && strcmp(argv[1], "--instrument") == 0) {
    return cmCTestLaunch::Main(argc, argv, cmCTestLaunch::Op::Instrument);
  }

  // Dispatch post-build instrumentation daemon for ninja
  if (argc == 3 && strcmp(argv[1], "--start-instrumentation") == 0) {
    return cmInstrumentation(argv[2]).SpawnBuildDaemon();
  }

  // Dispatch 'ctest --collect-instrumentation' once given PID finishes
  if (argc == 4 &&
      strcmp(argv[1], "--wait-and-collect-instrumentation") == 0) {
    return cmInstrumentation(argv[2]).CollectTimingAfterBuild(
      std::stoi(argv[3]));
  }

  // Dispatch 'ctest --collect-instrumentation' mode directly.
  if (argc == 3 && strcmp(argv[1], "--collect-instrumentation") == 0) {
    return cmInstrumentation(argv[2]).CollectTimingData(
      cmInstrumentationQuery::Hook::Manual);
  }

  if (cmSystemTools::GetLogicalWorkingDirectory().empty()) {
    std::cerr << "Current working directory cannot be established.\n";
    return 1;
  }

  // If there is a testing input file, check for documentation options
  // only if there are actually arguments.  We want running without
  // arguments to run tests.
  if (argc > 1 ||
      !(cmSystemTools::FileExists("CTestTestfile.cmake") ||
        cmSystemTools::FileExists("DartTestfile.txt"))) {
    if (argc == 1) {
      std::cerr << "*********************************\n"
                   "No test configuration file found!\n"
                   "*********************************\n";
    }
    cmDocumentation doc;
    doc.addCTestStandardDocSections();
    if (doc.CheckOptions(argc, argv)) {
      doc.SetShowGenerators(false);
      doc.SetName("ctest");
      doc.SetSection("Name", cmDocumentationName);
      doc.SetSection("Usage", cmDocumentationUsage);
      doc.PrependSection("Options", cmDocumentationOptions);
      return !doc.PrintRequestedDocumentation(std::cout);
    }
  }

  // copy the args to a vector
  auto args = std::vector<std::string>(argv, argv + argc);

  // run ctest
  return cmCTest{}.Run(args);
}
