#include "cmCursesMainForm.h"
#include "../cmCacheManager.h"
#include "../cmSystemTools.h"

#include <curses.h>
#include <form.h>
#include <signal.h>
#include <sys/ioctl.h>

static cmCursesMainForm* myform=0;

void onsig(int sig)
{
  if (myform)
    {
    endwin();
    WINDOW* w= initscr(); /* Initialization */ 
    noecho(); /* Echo off */ 
    cbreak(); /* nl- or cr not needed */ 
    keypad(stdscr,TRUE); /* Use key symbols as 
			    KEY_DOWN*/ 
    refresh();
    int x,y;
    getmaxyx(w, y, x);
    myform->SetWindow(w);
    myform->Render(1,1,x,y);
    std::cerr << "Size change: " << x << " " << y << std::endl;
    }
  signal(SIGWINCH, onsig);
}

int main(int argc, char** argv)
{

  if ( argc != 2 )
    {
    std::cerr << "Usage: " << argv[0] << " source_directory" 
	      << std::endl;
    return -1;
    }

  int newCache = false;
  if (!cmCacheManager::GetInstance()->LoadCache(cmSystemTools::GetCurrentWorkingDirectory().c_str()))
   {
   newCache = true;
   }


  WINDOW* w=initscr(); /* Initialization */ 
  noecho(); /* Echo off */ 
  cbreak(); /* nl- or cr not needed */ 
  keypad(stdscr,TRUE); /* Use key symbols as 
                          KEY_DOWN*/ 

  signal(SIGWINCH, onsig);

  int x,y;
  getmaxyx(w, y, x);

  myform = new cmCursesMainForm(argv[1], newCache);
  myform->InitializeUI(w);
  myform->Render(1, 1, x, y);
  myform->HandleInput();
  
  // Need to clean-up better
  endwin();
  delete myform;
  return 0;

}
