/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <csignal>
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
#include "cmDocumentationEntry.h" // IWYU pragma: keep
#include "cmState.h"
#include "cmSystemTools.h"
#include "cmake.h"

static const char* cmDocumentationName[][2] = {
  { nullptr, "  ccmake - Curses Interface for CMake." },
  { nullptr, nullptr }
};

static const char* cmDocumentationUsage[][2] = {
  { nullptr,
    "  ccmake <path-to-source>\n"
    "  ccmake <path-to-existing-build>" },
  { nullptr,
    "Specify a source directory to (re-)generate a build system for "
    "it in the current working directory.  Specify an existing build "
    "directory to re-generate its build system." },
  { nullptr, nullptr }
};

static const char* cmDocumentationUsageNote[][2] = {
  { nullptr, "Run 'ccmake --help' for more information." },
  { nullptr, nullptr }
};

static const char* cmDocumentationOptions[][2] = {
  CMAKE_STANDARD_OPTIONS_TABLE,
  { nullptr, nullptr }
};

cmCursesForm* cmCursesForm::CurrentForm = nullptr;

extern "C" {

void onsig(int /*unused*/)
{
  if (cmCursesForm::CurrentForm) {
    endwin();
    initscr();            /* Initialization */
    noecho();             /* Echo off */
    cbreak();             /* nl- or cr not needed */
    keypad(stdscr, true); /* Use key symbols as KEY_DOWN */
    refresh();
    int x;
    int y;
    getmaxyx(stdscr, y, x);
    cmCursesForm::CurrentForm->Render(1, 1, x, y);
    cmCursesForm::CurrentForm->UpdateStatusBar();
  }
  signal(SIGWINCH, onsig);
}
}

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
    cmake hcm(cmake::RoleInternal, cmState::Unknown);
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
    doc.PrependSection("Options", cmDocumentationOptions);
    return doc.PrintRequestedDocumentation(std::cout) ? 0 : 1;
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
    std::string arg = args[i];
    if (arg.find("-B", 0) == 0) {
      cacheDir = arg.substr(2);
    }
  }

  cmSystemTools::DisableRunCommandOutput();

  if (debug) {
    cmCursesForm::DebugStart();
  }

  initscr();            /* Initialization */
  noecho();             /* Echo off */
  cbreak();             /* nl- or cr not needed */
  keypad(stdscr, true); /* Use key symbols as KEY_DOWN */
  cmCursesColor::InitColors();

  signal(SIGWINCH, onsig);

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
    [&](const std::string& message, const char* title) {
      myform->AddError(cleanMessage(message), title);
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
