#include "../cmCacheManager.h"
#include "../cmSystemTools.h"
#include "../cmake.h"

#include <signal.h>
#include <sys/ioctl.h>

#include "cmCursesMainForm.h"

#include <curses.h>
#include <form.h>

cmCursesForm* cmCursesForm::CurrentForm=0;

void onsig(int sig)
{
  if (cmCursesForm::CurrentForm)
    {
    endwin();
    initscr(); /* Initialization */ 
    noecho(); /* Echo off */ 
    cbreak(); /* nl- or cr not needed */ 
    keypad(stdscr,TRUE); /* Use key symbols as 
			    KEY_DOWN*/ 
    refresh();
    int x,y;
    getmaxyx(stdscr, y, x);
    cmCursesForm::CurrentForm->Render(1,1,x,y);
    cmCursesForm::CurrentForm->UpdateStatusBar();
    }
  signal(SIGWINCH, onsig);
}

void CMakeErrorHandler(const char* message, const char* title, bool& disable)
{
  cmCursesForm::CurrentForm->AddError(message, title);
}

int main(int argc, char** argv)
{
  unsigned int i;
  int j;
  cmake msg;
  std::vector<std::string> args;
  for(j =0; j < argc; ++j)
    {
    args.push_back(argv[j]);
    }

  for(i=1; i < args.size(); ++i)
    {
    std::string arg = args[i];
    if(arg.find("-help",0) != std::string::npos ||
       arg.find("--help",0) != std::string::npos ||
       arg.find("/?",0) != std::string::npos ||
       arg.find("-usage",0) != std::string::npos)
      {
      msg.Usage(args[0].c_str());
      return -1;
      }
    }

  cmSystemTools::DisableRunCommandOutput();

  cmCacheManager::GetInstance()->LoadCache(cmSystemTools::GetCurrentWorkingDirectory().c_str());

  initscr(); /* Initialization */ 
  noecho(); /* Echo off */ 
  cbreak(); /* nl- or cr not needed */ 
  keypad(stdscr,TRUE); /* Use key symbols as 
                          KEY_DOWN*/ 

  signal(SIGWINCH, onsig);

  int x,y;
  getmaxyx(stdscr, y, x);
  if ( x < cmCursesMainForm::MIN_WIDTH  || 
       y < cmCursesMainForm::MIN_HEIGHT )
    {
    endwin();
    std::cerr << "Window is too small. A size of at least "
	      << cmCursesMainForm::MIN_WIDTH << " x " 
	      <<  cmCursesMainForm::MIN_HEIGHT
	      << " is required to run ccmake." <<  std::endl;
    return 1;
    }


  cmCursesMainForm* myform;

  myform = new cmCursesMainForm(args);

  cmSystemTools::SetErrorCallback(CMakeErrorHandler);

  cmCursesForm::CurrentForm = myform;

  myform->InitializeUI();
  myform->RunCMake(false);

  myform->Render(1, 1, x, y);
  myform->HandleInput();
  
  // Need to clean-up better
  curses_clear();
  touchwin(stdscr);
  endwin();
  delete cmCursesForm::CurrentForm;
  cmCursesForm::CurrentForm = 0;

  return 0;

}
