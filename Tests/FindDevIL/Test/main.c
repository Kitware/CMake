#include <IL/il.h>

int main(void)
{
  // Test 1 requires to link to the library.
  ilInit();

  ilShutDown();
  return 0;
}
