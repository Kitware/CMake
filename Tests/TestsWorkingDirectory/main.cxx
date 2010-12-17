#include <cstdlib>
#include <cstring>
#include <ctype.h>

#include <iostream>

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__WATCOMC__) || defined(__BORLANDC__) || defined(__MINGW32__))

#include <io.h>
#include <direct.h>

#if defined(__WATCOMC__)
#include <direct.h>
#define _getcwd getcwd
#endif

inline const char* Getcwd(char* buf, unsigned int len)
{
  const char* ret = _getcwd(buf, len);
  if(!ret)
    {
    std::cerr << "No current working directory." << std::endl;
    abort();
    }
  // make sure the drive letter is capital
  if(strlen(buf) > 1 && buf[1] == ':')
    {
    buf[0] = toupper(buf[0]);
    }
  for(char* p = buf; *p; ++p)
    {
    if(*p == '\\')
      {
      *p = '/';
      }
    }
  return ret;
}

#else
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

inline const char* Getcwd(char* buf, unsigned int len)
{
  const char* ret = getcwd(buf, len);
  if(!ret)
    {
    std::cerr << "No current working directory" << std::endl;
    abort();
    }
  return ret;
}

#endif

int main(int argc, char *argv[])
{
  char buf[2048];
  const char *cwd = Getcwd(buf, sizeof(buf));

  std::cout << "Working directory: -->" << cwd << "<--";

  return 0;
}
