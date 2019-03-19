#include <assert.h>
#include <fontconfig/fontconfig.h>
#include <stdio.h>
#include <string.h>

int main()
{
  FcInit();
  printf("Found Fontconfig.\n");

  char fontconfig_version_string[16];
  snprintf(fontconfig_version_string, 16, "%i.%i.%i", FC_MAJOR, FC_MINOR,
           FC_REVISION);
  assert(
    strcmp(fontconfig_version_string, CMAKE_EXPECTED_FONTCONFIG_VERSION) == 0);
  return 0;
}
