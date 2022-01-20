/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCursesForm.h"

#include <cstdlib>
#ifndef _WIN32
#  include <unistd.h>
#endif // _WIN32

cmsys::ofstream cmCursesForm::DebugFile;
bool cmCursesForm::Debug = false;

cmCursesForm::cmCursesForm()
{
  this->Form = nullptr;
}

cmCursesForm::~cmCursesForm()
{
  if (this->Form) {
    unpost_form(this->Form);
    free_form(this->Form);
    this->Form = nullptr;
  }
}

void cmCursesForm::DebugStart()
{
  cmCursesForm::Debug = true;
  cmCursesForm::DebugFile.open("ccmakelog.txt");
}

void cmCursesForm::DebugEnd()
{
  if (!cmCursesForm::Debug) {
    return;
  }

  cmCursesForm::Debug = false;
  cmCursesForm::DebugFile.close();
}

void cmCursesForm::LogMessage(const char* msg)
{
  if (!cmCursesForm::Debug) {
    return;
  }

  cmCursesForm::DebugFile << msg << std::endl;
}

void cmCursesForm::HandleResize()
{
  endwin();
  if (initscr() == nullptr) {
    static const char errmsg[] = "Error: ncurses initialization failed\n";
#ifdef _WIN32
    fprintf(stderr, "%s", errmsg);
#else
    auto r = write(STDERR_FILENO, errmsg, sizeof(errmsg) - 1);
    static_cast<void>(r);
#endif // _WIN32
    exit(1);
  }
  noecho();             /* Echo off */
  cbreak();             /* nl- or cr not needed */
  keypad(stdscr, true); /* Use key symbols as KEY_DOWN */
  refresh();
  int x;
  int y;
  getmaxyx(stdscr, y, x);
  this->Render(1, 1, x, y);
  this->UpdateStatusBar();
}
