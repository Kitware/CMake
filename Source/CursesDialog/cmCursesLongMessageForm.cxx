/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCursesLongMessageForm.h"

#include <cstdio>
#include <cstring>

#include "cmCursesForm.h"
#include "cmCursesMainForm.h"
#include "cmCursesStandardIncludes.h"
#include "cmStringAlgorithms.h"
#include "cmVersion.h"

inline int ctrl(int z)
{
  return (z & 037);
}

cmCursesLongMessageForm::cmCursesLongMessageForm(
  std::vector<std::string> const& messages, const char* title,
  ScrollBehavior scrollBehavior)
  : Scrolling(scrollBehavior)
{
  // Append all messages into on big string
  this->Messages = cmJoin(messages, "\n");
  this->Title = title;
  this->Fields[0] = nullptr;
  this->Fields[1] = nullptr;
}

cmCursesLongMessageForm::~cmCursesLongMessageForm()
{
  if (this->Fields[0]) {
    free_field(this->Fields[0]);
  }
}

void cmCursesLongMessageForm::UpdateContent(std::string const& output,
                                            std::string const& title)
{
  this->Title = title;

  if (!output.empty() && this->Messages.size() < MAX_CONTENT_SIZE) {
    this->Messages.push_back('\n');
    this->Messages.append(output);
    form_driver(this->Form, REQ_NEXT_LINE);
    form_driver(this->Form, REQ_BEG_LINE);
    this->DrawMessage(output.c_str());
  }

  this->UpdateStatusBar();
  touchwin(stdscr);
  refresh();
}

void cmCursesLongMessageForm::UpdateStatusBar()
{
  int x;
  int y;
  getmaxyx(stdscr, y, x);

  char bar[cmCursesMainForm::MAX_WIDTH];
  size_t size = this->Title.size();
  if (size >= cmCursesMainForm::MAX_WIDTH) {
    size = cmCursesMainForm::MAX_WIDTH - 1;
  }
  strncpy(bar, this->Title.c_str(), size);
  for (size_t i = size; i < cmCursesMainForm::MAX_WIDTH; i++) {
    bar[i] = ' ';
  }
  int width;
  if (x >= 0 && x < cmCursesMainForm::MAX_WIDTH) {
    width = x;
  } else {
    width = cmCursesMainForm::MAX_WIDTH - 1;
  }

  bar[width] = '\0';

  char version[cmCursesMainForm::MAX_WIDTH];
  char vertmp[128];
  snprintf(vertmp, sizeof(vertmp), "CMake Version %s",
           cmVersion::GetCMakeVersion());
  size_t sideSpace = (width - strlen(vertmp));
  for (size_t i = 0; i < sideSpace; i++) {
    version[i] = ' ';
  }
  snprintf(version + sideSpace, sizeof(version) - sideSpace, "%s", vertmp);
  version[width] = '\0';

  char fmt_s[] = "%s";
  curses_move(y - 4, 0);
  attron(A_STANDOUT);
  printw(fmt_s, bar);
  attroff(A_STANDOUT);
  curses_move(y - 3, 0);
  printw(fmt_s, version);
  pos_form_cursor(this->Form);
}

void cmCursesLongMessageForm::PrintKeys()
{
  int x;
  int y;
  getmaxyx(stdscr, y, x);
  if (x < cmCursesMainForm::MIN_WIDTH || y < cmCursesMainForm::MIN_HEIGHT) {
    return;
  }
  char firstLine[512];
  snprintf(firstLine, sizeof(firstLine), "Press [e] to exit screen");

  char fmt_s[] = "%s";
  curses_move(y - 2, 0);
  printw(fmt_s, firstLine);
  pos_form_cursor(this->Form);
}

void cmCursesLongMessageForm::Render(int /*left*/, int /*top*/, int /*width*/,
                                     int /*height*/)
{
  int x;
  int y;
  getmaxyx(stdscr, y, x);

  if (this->Form) {
    unpost_form(this->Form);
    free_form(this->Form);
    this->Form = nullptr;
  }

  if (this->Fields[0]) {
    free_field(this->Fields[0]);
    this->Fields[0] = nullptr;
  }

  this->Fields[0] = new_field(y - 6, x - 2, 1, 1, 0, 0);

  field_opts_off(this->Fields[0], O_STATIC);

  this->Form = new_form(this->Fields);
  post_form(this->Form);

  form_driver(this->Form, REQ_BEG_FIELD);
  this->DrawMessage(this->Messages.c_str());

  this->UpdateStatusBar();
  touchwin(stdscr);
  refresh();
}

void cmCursesLongMessageForm::DrawMessage(const char* msg) const
{
  int i = 0;
  while (msg[i] != '\0' && i < MAX_CONTENT_SIZE) {
    if (msg[i] == '\n' && msg[i + 1] != '\0') {
      form_driver(this->Form, REQ_NEXT_LINE);
      form_driver(this->Form, REQ_BEG_LINE);
    } else {
      form_driver(this->Form, msg[i]);
    }
    i++;
  }
  if (this->Scrolling == ScrollBehavior::ScrollDown) {
    form_driver(this->Form, REQ_END_FIELD);
  } else {
    form_driver(this->Form, REQ_BEG_FIELD);
  }
}

void cmCursesLongMessageForm::HandleInput()
{
  if (!this->Form) {
    return;
  }

  char debugMessage[128];

  for (;;) {
    this->PrintKeys();
    int key = getch();

#ifdef _WIN32
    if (key == KEY_RESIZE) {
      HandleResize();
    }
#endif // _WIN32

    snprintf(debugMessage, sizeof(debugMessage),
             "Message widget handling input, key: %d", key);
    cmCursesForm::LogMessage(debugMessage);

    // quit
    if (key == 'o' || key == 'e') {
      break;
    }
    if (key == KEY_DOWN || key == ctrl('n') || key == 'j') {
      form_driver(this->Form, REQ_SCR_FLINE);
    } else if (key == KEY_UP || key == ctrl('p') || key == 'k') {
      form_driver(this->Form, REQ_SCR_BLINE);
    } else if (key == KEY_NPAGE || key == ctrl('d')) {
      form_driver(this->Form, REQ_SCR_FPAGE);
    } else if (key == KEY_PPAGE || key == ctrl('u')) {
      form_driver(this->Form, REQ_SCR_BPAGE);
    }

    this->UpdateStatusBar();
    touchwin(stdscr);
    wrefresh(stdscr);
  }
}
