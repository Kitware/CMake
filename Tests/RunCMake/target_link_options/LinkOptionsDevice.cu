#include <cuda.h>

#ifdef _WIN32
__declspec(dllexport)
#endif
  int simplelib()
{
  return 0;
}

int main(void)
{
  return simplelib();
}
