#ifdef CMAKE_HAS_X

#include <X11/Xlib.h>
#include <X11/Xutil.h>

char hello[]="hello.world";
char hi[]="Hi";

main(argc, argv)
int argc;
char **argv;
{
  Display *mydisplay;
  Window  mywindow;
  GC mygc;

  XEvent myevent;
  KeySym mykey;

  XSizeHints myhint;
  int myscreen;
  unsigned long myforeground, mybackground;
  int i;
  char text[10];
  int done;
  char* display = ":0";


  if ( getenv("DISPLAY") )
    {
    display = getenv("DISPLAY");
    }

  mydisplay=XOpenDisplay(display);
  if ( mydisplay )
    {
    printf("Opened...\n");
    XCloseDisplay(mydisplay);
    }
  exit(0);
}


#else

#include "stdio.h"

int main()
{
  printf("No X on this computer\n");
  return 0;
}

#endif
