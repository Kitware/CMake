
#include <cuda.h>

#ifdef _WIN32
#  define IMPORT __declspec(dllimport)
#else
#  define IMPORT
#endif

IMPORT int simplelib();

int main(void)
{
  return simplelib();
}
