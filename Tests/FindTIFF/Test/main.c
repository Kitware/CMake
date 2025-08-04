#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <tiffio.h>

int main(void)
{
  /* Without any TIFF file to open, test that the call fails as
     expected.  This tests that linking worked. */
  TIFF* tiff = TIFFOpen("invalid.tiff", "r");
  assert(!tiff);

  char const* info = TIFFGetVersion();
  char const* version_prefix = "Version ";
  char const* start = strstr(info, version_prefix);
  char version_str[16];

  if (start) {
    start += strlen(version_prefix);
    int major, minor, patch;

    if (sscanf(start, "%d.%d.%d", &major, &minor, &patch) == 3) {
      snprintf(version_str, sizeof(version_str), "%d.%d.%d", major, minor,
               patch);
      printf("Found TIFF version %s, expected version %s\n", version_str,
             CMAKE_EXPECTED_TIFF_VERSION);

      return strcmp(version_str, CMAKE_EXPECTED_TIFF_VERSION);
    }
  }

  fprintf(stderr,
          "TIFF version not found or TIFF version could not be parsed\n");
  return 1;
}
