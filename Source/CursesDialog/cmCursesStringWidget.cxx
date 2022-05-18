/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCursesStringWidget.h"

#include <cstdio>

#include "cmCursesColor.h"
#include "cmCursesForm.h"
#include "cmCursesMainForm.h"
#include "cmCursesStandardIncludes.h"
#include "cmCursesWidget.h"
#include "cmStateTypes.h"

inline int ctrl(int z)
{
  return (z & 037);
}

cmCursesStringWidget::cmCursesStringWidget(int width, int height, int left,
                                           int top)
  : cmCursesWidget(width, height, left, top)
{
  this->InEdit = false;
  this->Type = cmStateEnums::STRING;
  if (cmCursesColor::HasColors()) {
    set_field_fore(this->Field, COLOR_PAIR(cmCursesColor::String));
    set_field_back(this->Field, COLOR_PAIR(cmCursesColor::String));
  } else {
    set_field_fore(this->Field, A_NORMAL);
    set_field_back(this->Field, A_STANDOUT);
  }
  field_opts_off(this->Field, O_STATIC);
}

void cmCursesStringWidget::OnTab(cmCursesMainForm* /*unused*/,
                                 WINDOW* /*unused*/)
{
  // FORM* form = fm->GetForm();
}

void cmCursesStringWidget::OnReturn(cmCursesMainForm* fm, WINDOW* /*unused*/)
{
  if (this->InEdit) {
    cmCursesForm::LogMessage("String widget leaving edit.");
    this->InEdit = false;
    fm->PrintKeys();
    this->OriginalString.clear();
    // trick to force forms to update the field buffer
    FORM* form = fm->GetForm();
    form_driver(form, REQ_NEXT_FIELD);
    form_driver(form, REQ_PREV_FIELD);
    this->Done = true;
  } else {
    cmCursesForm::LogMessage("String widget entering edit.");
    this->InEdit = true;
    fm->PrintKeys();
    this->OriginalString = field_buffer(this->Field, 0);
  }
}

void cmCursesStringWidget::OnType(int& key, cmCursesMainForm* fm,
                                  WINDOW* /*unused*/)
{
  form_driver(fm->GetForm(), key);
}

bool cmCursesStringWidget::HandleInput(int& key, cmCursesMainForm* fm,
                                       WINDOW* w)
{
  int x;
  int y;

  FORM* form = fm->GetForm();
  // when not in edit mode, edit mode is entered by pressing enter or i (vim
  // binding)
  // 10 == enter
  if (!this->InEdit && (key != 10 && key != KEY_ENTER && key != 'i')) {
    return false;
  }

  this->OriginalString.clear();
  this->Done = false;

  char debugMessage[128];

  // <Enter> is used to change edit mode (like <Esc> in vi).
  while (!this->Done) {
    snprintf(debugMessage, sizeof(debugMessage),
             "String widget handling input, key: %d", key);
    cmCursesForm::LogMessage(debugMessage);

    fm->PrintKeys();

    getmaxyx(stdscr, y, x);
    // If window too small, handle 'q' only
    if (x < cmCursesMainForm::MIN_WIDTH || y < cmCursesMainForm::MIN_HEIGHT) {
      // quit
      if (key == 'q') {
        return false;
      }
      key = getch();
      continue;
    }

    // If resize occurred during edit, move out of edit mode
    if (!this->InEdit && (key != 10 && key != KEY_ENTER && key != 'i')) {
      return false;
    }
    // toggle edit with return
    if ((key == 10 || key == KEY_ENTER)
        // enter edit with i (and not-edit mode)
        || (!this->InEdit && key == 'i')) {
      this->OnReturn(fm, w);
    } else if (key == KEY_DOWN || key == ctrl('n') || key == KEY_UP ||
               key == ctrl('p') || key == KEY_NPAGE || key == ctrl('d') ||
               key == KEY_PPAGE || key == ctrl('u')) {
      this->InEdit = false;
      this->OriginalString.clear();
      // trick to force forms to update the field buffer
      form_driver(form, REQ_NEXT_FIELD);
      form_driver(form, REQ_PREV_FIELD);
      return false;
    }
    // esc
    else if (key == 27) {
      if (this->InEdit) {
        this->InEdit = false;
        fm->PrintKeys();
        this->SetString(this->OriginalString);
        this->OriginalString.clear();
        touchwin(w);
        wrefresh(w);
        return true;
      }
    } else if (key == 9) {
      this->OnTab(fm, w);
    } else if (key == KEY_LEFT || key == ctrl('b')) {
      form_driver(form, REQ_PREV_CHAR);
    } else if (key == KEY_RIGHT || key == ctrl('f')) {
      form_driver(form, REQ_NEXT_CHAR);
    } else if (key == ctrl('k')) {
      form_driver(form, REQ_CLR_EOL);
    } else if (key == ctrl('a') || key == KEY_HOME) {
      form_driver(form, REQ_BEG_FIELD);
    } else if (key == ctrl('e') || key == KEY_END) {
      form_driver(form, REQ_END_FIELD);
    } else if (key == 127 || key == KEY_BACKSPACE) {
      FIELD* cur = current_field(form);
      form_driver(form, REQ_DEL_PREV);
      if (current_field(form) != cur) {
        set_current_field(form, cur);
      }
    } else if (key == ctrl('d') || key == KEY_DC) {
      form_driver(form, REQ_DEL_CHAR);
    } else {
      this->OnType(key, fm, w);
    }
    if (!this->Done) {
      touchwin(w);
      wrefresh(w);

      key = getch();
    }
  }
  return true;
}

void cmCursesStringWidget::SetString(const std::string& value)
{
  this->SetValue(value);
}

const char* cmCursesStringWidget::GetString()
{
  return this->GetValue();
}

const char* cmCursesStringWidget::GetValue()
{
  return field_buffer(this->Field, 0);
}

bool cmCursesStringWidget::PrintKeys()
{
  int x;
  int y;
  getmaxyx(stdscr, y, x);
  if (x < cmCursesMainForm::MIN_WIDTH || y < cmCursesMainForm::MIN_HEIGHT) {
    return false;
  }
  if (this->InEdit) {
    char fmt_s[] = "%s";
    // Clean the toolbar
    curses_move(y - 4, 0);
    clrtoeol();
    curses_move(y - 3, 0);
    printw(fmt_s, "Editing option, press [enter] to confirm");
    clrtoeol();
    curses_move(y - 2, 0);
    printw(fmt_s, "                press [esc] to cancel");
    clrtoeol();
    curses_move(y - 1, 0);
    clrtoeol();

    return true;
  }
  return false;
}
