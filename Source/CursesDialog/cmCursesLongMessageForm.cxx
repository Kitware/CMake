#include "../cmCacheManager.h"
#include "../cmSystemTools.h"
#include "../cmake.h"
#include "cmCursesLongMessageForm.h"
#include "cmCursesMainForm.h"

inline int ctrl(int z)
{
    return (z&037);
} 

cmCursesLongMessageForm::cmCursesLongMessageForm(std::vector<std::string> 
						 const& messages, const char* 
						 title)
{
  // Append all messages into on big string
  std::vector<std::string>::const_iterator it;
  for(it=messages.begin(); it != messages.end(); it++)
    {
    m_Messages += (*it);
    // Add one blank line after each message
    m_Messages += "\n\n";
    }
  m_Title = title;
  m_Fields[0] = 0;
  m_Fields[1] = 0;
}

cmCursesLongMessageForm::~cmCursesLongMessageForm()
{
  if (m_Fields[0])
    {
    free_field(m_Fields[0]);
    }
}


void cmCursesLongMessageForm::UpdateStatusBar()
{
  int x,y;
  getmaxyx(stdscr, y, x);

  char bar[cmCursesMainForm::MAX_WIDTH];
  int size = strlen(m_Title.c_str());
  if ( size >= cmCursesMainForm::MAX_WIDTH )
    {
    size = cmCursesMainForm::MAX_WIDTH-1;
    }
  strncpy(bar, m_Title.c_str(), size);
  for(int i=size-1; i<cmCursesMainForm::MAX_WIDTH; i++) bar[i] = ' ';

  int width;
  if (x < cmCursesMainForm::MAX_WIDTH )
    {
    width = x;
    }
  else
    {
    width = cmCursesMainForm::MAX_WIDTH;
    }

  bar[width] = '\0';

  char version[cmCursesMainForm::MAX_WIDTH];
  char vertmp[128];
  sprintf(vertmp,"CMake Version %d.%d", cmMakefile::GetMajorVersion(),
	  cmMakefile::GetMinorVersion());
  int sideSpace = (width-strlen(vertmp));
  for(int i=0; i<sideSpace; i++) { version[i] = ' '; }
  sprintf(version+sideSpace, "%s", vertmp);
  version[width] = '\0';

  curses_move(y-4,0);
  attron(A_STANDOUT);
  printw(bar);
  attroff(A_STANDOUT);  
  curses_move(y-3,0);
  printw(version);
  pos_form_cursor(m_Form);
}

void cmCursesLongMessageForm::PrintKeys()
{
  int x,y;
  getmaxyx(stdscr, y, x);
  if ( x < cmCursesMainForm::MIN_WIDTH  || 
       y < cmCursesMainForm::MIN_HEIGHT )
    {
    return;
    }
  char firstLine[512];
  sprintf(firstLine,  "O)k");

  curses_move(y-2,0);
  printw(firstLine);
  pos_form_cursor(m_Form);
  
}

void cmCursesLongMessageForm::Render(int left, int top, int width, int height)
{
  int x,y;
  getmaxyx(stdscr, y, x);

  if (m_Form)
    {
    unpost_form(m_Form);
    free_form(m_Form);
    m_Form = 0;
    }

  const char* msg = m_Messages.c_str();

  curses_clear();

  if (m_Fields[0])
    {
    free_field(m_Fields[0]);
    m_Fields[0] = 0;
    }

  m_Fields[0] = new_field(y-6, x-2, 1, 1, 0, 0);

  field_opts_off(m_Fields[0],  O_STATIC);

  m_Form = new_form(m_Fields);
  post_form(m_Form);

  int i=0;
  form_driver(m_Form, REQ_BEG_FIELD);
  while(msg[i] != '\0')
    {
    if (msg[i] == '\n' && msg[i+1] != '\0')
      {
      form_driver(m_Form, REQ_NEW_LINE);
      }
    else
      {
      form_driver(m_Form, msg[i]);
      }
    i++;
    }
  form_driver(m_Form, REQ_BEG_FIELD);

  this->UpdateStatusBar();
  this->PrintKeys();
  touchwin(stdscr); 
  refresh();

}

void cmCursesLongMessageForm::HandleInput()
{
  if (!m_Form)
    {
    return;
    }

  while(1)
    {
    int key = getch();

    // quit
    if ( key == 'o' )
      {
      break;
      }
    else if ( key == KEY_DOWN || key == ctrl('n') )
      {
      form_driver(m_Form, REQ_SCR_FLINE);
      }
    else if ( key == KEY_UP  || key == ctrl('p') )
      {
      form_driver(m_Form, REQ_SCR_BLINE);
      }
    else if ( key == KEY_NPAGE || key == ctrl('d') )
      {
      form_driver(m_Form, REQ_SCR_FPAGE);
      }
    else if ( key == KEY_PPAGE || key == ctrl('u') )
      {
      form_driver(m_Form, REQ_SCR_BPAGE);
      }

    this->UpdateStatusBar();
    this->PrintKeys();
    touchwin(stdscr); 
    wrefresh(stdscr); 
    }

}
