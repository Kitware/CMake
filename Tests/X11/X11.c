#ifdef CMAKE_HAS_X

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdlib.h>

int main()
{
  Display *mydisplay;
  char* display = ":0";


  if ( getenv("DISPLAY") )
    {
    display = getenv("DISPLAY");
    }

  mydisplay=XOpenDisplay(display);
  if ( mydisplay )
    {
    XCloseDisplay(mydisplay);
    }
  return 0;
}


#else

#include "stdio.h"

int main()
{
  printf("No X on this computer\n");
  return 0;
}

#endif
