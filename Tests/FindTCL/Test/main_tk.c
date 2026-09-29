#include <stdio.h>
#include <tk.h>

int main(int argc, char* argv[])
{
  int major;
  int minor;
  Tcl_Interp* interp;
  Tk_Window mainWindow;

  (void)argc;
  Tcl_FindExecutable(argv[0]);

  Tcl_GetVersion(&major, &minor, NULL, NULL);
  printf("Found Tcl library %d.%d, Tk headers %d.%d\n", major, minor,
         TK_MAJOR_VERSION, TK_MINOR_VERSION);
  if (major != TK_MAJOR_VERSION) {
    fprintf(stderr, "Tcl library does not match Tk headers\n");
    return 1;
  }

  interp = Tcl_CreateInterp();
  mainWindow = Tk_MainWindow(interp);
  Tcl_DeleteInterp(interp);
  return mainWindow ? 1 : 0;
}
