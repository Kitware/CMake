/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <vector>

#include "cmsys/Encoding.hxx"

#include "cmCursesColor.h"
#include "cmCursesForm.h"
#include "cmCursesMainForm.h"
#include "cmCursesStandardIncludes.h"
#include "cmDocumentation.h"
#include "cmDocumentationEntry.h"
#include "cmMessageMetadata.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmake.h"

namespace {
const cmDocumentationEntry cmDocumentationName = {
  {},
  "  ccmake - Curses Interface for CMake."
};

const cmDocumentationEntry cmDocumentationUsage[2] = {
  { {},
    "  ccmake <path-to-source>\n"
    "  ccmake <path-to-existing-build>" },
  { {},
    "Specify a source directory to (re-)generate a build system for "
    "it in the current working directory.  Specify an existing build "
    "directory to re-generate its build system." },
};

const cmDocumentationEntry cmDocumentationUsageNote = {
  {},
  "Run 'ccmake --help' for more information."
};

#ifndef _WIN32
extern "C" {

void onsig(int /*unused*/)
{
  if (cmCursesForm::CurrentForm) {
    cmCursesForm::CurrentForm->HandleResize();
  }
  signal(SIGWINCH, onsig);
}
}
#endif // _WIN32
} // anonymous namespace

cmCursesForm* cmCursesForm::CurrentForm = nullptr;

int main(int argc, char const* const* argv)
{
  cmSystemTools::EnsureStdPipes();
  cmsys::Encoding::CommandLineArguments encoding_args =
    cmsys::Encoding::CommandLineArguments::Main(argc, argv);
  argc = encoding_args.argc();
  argv = encoding_args.argv();

  cmSystemTools::InitializeLibUV();
  cmSystemTools::FindCMakeResources(argv[0]);
  cmDocumentation doc;
  doc.addCMakeStandardDocSections();
  if (doc.CheckOptions(argc, argv)) {
    cmake hcm(cmake::RoleInternal, cmState::Help);
    hcm.SetHomeDirectory("");
    hcm.SetHomeOutputDirectory("");
    hcm.AddCMakePaths();
    auto generators = hcm.GetGeneratorsDocumentation();
    doc.SetName("ccmake");
    doc.SetSection("Name", cmDocumentationName);
    doc.SetSection("Usage", cmDocumentationUsage);
    if (argc == 1) {
      doc.AppendSection("Usage", cmDocumentationUsageNote);
    }
    doc.AppendSection("Generators", generators);
    doc.PrependSection("Options", cmake::CMAKE_STANDARD_OPTIONS_TABLE);
    return !doc.PrintRequestedDocumentation(std::cout);
  }

  bool debug = false;
  unsigned int i;
  int j;
  std::vector<std::string> args;
  for (j = 0; j < argc; ++j) {
    if (strcmp(argv[j], "-debug") == 0) {
      debug = true;
    } else {
      args.emplace_back(argv[j]);
    }
  }

  std::string cacheDir = cmSystemTools::GetCurrentWorkingDirectory();
  for (i = 1; i < args.size(); ++i) {
    std::string const& arg = args[i];
    if (cmHasPrefix(arg, "-B")) {
      cacheDir = arg.substr(2);
    }
  }

  cmSystemTools::DisableRunCommandOutput();

  if (debug) {
    cmCursesForm::DebugStart();
  }

  if (initscr() == nullptr) {
    fprintf(stderr, "Error: ncurses initialization failed\n");
    exit(1);
  }
  noecho();             /* Echo off */
  cbreak();             /* nl- or cr not needed */
  keypad(stdscr, true); /* Use key symbols as KEY_DOWN */
  cmCursesColor::InitColors();

#ifndef _WIN32
  signal(SIGWINCH, onsig);
#endif // _WIN32

  int x;
  int y;
  getmaxyx(stdscr, y, x);
  if (x < cmCursesMainForm::MIN_WIDTH || y < cmCursesMainForm::MIN_HEIGHT) {
    endwin();
    std::cerr << "Window is too small. A size of at least "
              << cmCursesMainForm::MIN_WIDTH << " x "
              << cmCursesMainForm::MIN_HEIGHT << " is required to run ccmake."
              << std::endl;
    return 1;
  }

  cmCursesMainForm* myform;

  myform = new cmCursesMainForm(args, x);
  if (myform->LoadCache(cacheDir.c_str())) {
    curses_clear();
    touchwin(stdscr);
    endwin();
    delete myform;
    std::cerr << "Error running cmake::LoadCache().  Aborting.\n";
    return 1;
  }

  /*
   * The message is stored in a list by the form which will be
   * joined by '\n' before display.
   * Removing any trailing '\n' avoid extra empty lines in the final results
   */
  auto cleanMessage = [](const std::string& message) -> std::string {
    auto msg = message;
    if (!msg.empty() && msg.back() == '\n') {
      msg.pop_back();
    }
    return msg;
  };
  cmSystemTools::SetMessageCallback(
    [&](const std::string& message, const cmMessageMetadata& md) {
      myform->AddError(cleanMessage(message), md.title);
    });
  cmSystemTools::SetStderrCallback([&](const std::string& message) {
    myform->AddError(cleanMessage(message), "");
  });
  cmSystemTools::SetStdoutCallback([&](const std::string& message) {
    myform->UpdateProgress(cleanMessage(message), -1);
  });

  cmCursesForm::CurrentForm = myform;

  myform->InitializeUI();
  if (myform->Configure(1) == 0) {
    myform->Render(1, 1, x, y);
    myform->HandleInput();
  }

  // Need to clean-up better
  curses_clear();
  touchwin(stdscr);
  endwin();
  delete cmCursesForm::CurrentForm;
  cmCursesForm::CurrentForm = nullptr;

  std::cout << std::endl << std::endl;

  return 0;
}
