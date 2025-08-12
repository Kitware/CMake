#include <IL/il.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
  // Test 1 requires to link to the library.
  ilInit();

  ilShutDown();

  int version = IL_VERSION;
  int major = version / 100;
  int minor = version / 10 % 10;
  int patch = version % 10;
  char version_string[100];
  snprintf(version_string, sizeof(version_string), "%d.%d.%d", major, minor,
           patch);

  printf("Found DevIL version %s, expected version %s\n", version_string,
         CMAKE_EXPECTED_DEVIL_VERSION);
  return strcmp(version_string, CMAKE_EXPECTED_DEVIL_VERSION);
}
