
#include <stdio.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "display_time.h"

void display_time()
{
#if defined(PYTHON3)
  wchar_t* program = Py_DecodeLocale("display_time", NULL);
  if (program == NULL) {
    fprintf(stderr, "Fatal error: cannot decode argv[0]\n");
    exit(1);
  }
  char* cmd = "from time import time,ctime\n"
              "print('Today is', ctime(time()))\n";
#else
  char* program = "display_time";
  char* cmd = "from time import time,ctime\n"
              "print 'Today is', ctime(time())\n";
#endif

  Py_SetProgramName(program); /* optional but recommended */
  Py_Initialize();
  PyRun_SimpleString(cmd);
#if defined(PYTHON3)
  if (Py_FinalizeEx() < 0) {
    exit(120);
  }
  PyMem_RawFree(program);
#else
  Py_Finalize();
#endif
}
