#include <IL/il.h>

int main()
{
  // Test 1 requires to link to the library.
  ilInit();

  ilShutDown();
  return 0;
}
