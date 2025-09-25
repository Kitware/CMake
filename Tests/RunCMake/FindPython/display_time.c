
#include <stdio.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "display_time.h"

void display_time(void)
{
#if defined(PYTHON3)
  PyConfig config;
  PyStatus status;

  PyConfig_InitPythonConfig(&config);
  status =
    PyConfig_SetBytesString(&config, &config.program_name, "display_time");
  if (PyStatus_Exception(status)) {
    Py_ExitStatusException(status);
  }

  char* cmd = "from time import time,ctime\n"
              "print('Today is', ctime(time()))\n";
#else
  char* program = "display_time";
  char* cmd = "from time import time,ctime\n"
              "print 'Today is', ctime(time())\n";

  Py_SetProgramName(program); /* optional but recommended */
#endif
  Py_Initialize();
  PyRun_SimpleString(cmd);
#if defined(PYTHON3)
  if (Py_FinalizeEx() < 0) {
    exit(120);
  }
  PyConfig_Clear(&config);
#else
  Py_Finalize();
#endif
}
