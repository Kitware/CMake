#include <kwsys/Directory.hxx>
#include <kwsys/Process.h>
#include <kwsys/std/iostream>

int main()
{
  kwsys::Directory();
  kwsysProcess* kp = kwsysProcess_New();
  const char* cmd[] = {"echo", "Hello, World!", 0};
  kwsysProcess_SetCommand(kp, cmd);
  kwsysProcess_Execute(kp);
  char* data = 0;
  int length = 0;
  while(kwsysProcess_WaitForData(kp, kwsysProcess_STDOUT | kwsysProcess_STDERR,
                                 &data, &length, 0))
    {
    kwsys_std::cout.write(data, length);
    }
  kwsysProcess_Delete(kp);
  kwsys_std::cout << kwsys_std::endl;
  return 0;
}
