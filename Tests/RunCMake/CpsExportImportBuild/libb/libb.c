#include <liba.h>

#ifdef _WIN32
__declspec(dllexport)
#endif
int ask(void)
{
  return answer();
}
