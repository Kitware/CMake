#include <stdio.h>
#include <string.h>
#include <tcl.h>

int main(int argc, char* argv[])
{
  int major;
  int minor;
  int ok;
  Tcl_Interp* interp;

  (void)argc;
  Tcl_FindExecutable(argv[0]);

  Tcl_GetVersion(&major, &minor, NULL, NULL);
  printf("Found Tcl library %d.%d, headers %d.%d\n", major, minor,
         TCL_MAJOR_VERSION, TCL_MINOR_VERSION);
  if (major != TCL_MAJOR_VERSION || minor != TCL_MINOR_VERSION) {
    fprintf(stderr, "Tcl library does not match Tcl headers\n");
    return 1;
  }

  interp = Tcl_CreateInterp();
  ok = Tcl_EvalEx(interp, "expr {6 * 7}", -1, 0) == TCL_OK &&
    strcmp(Tcl_GetStringResult(interp), "42") == 0;
  if (!ok) {
    fprintf(stderr, "Tcl evaluation failed: %s\n",
            Tcl_GetStringResult(interp));
  }
  Tcl_DeleteInterp(interp);
  return ok ? 0 : 1;
}
