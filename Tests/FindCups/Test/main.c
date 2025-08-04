#include <cups/cups.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
  int num_options = 0;
  cups_option_t* options = NULL;

  num_options = cupsAddOption(CUPS_COPIES, "1", num_options, &options);
  cupsFreeOptions(num_options, options);

  char version_str[16];
  snprintf(version_str, 16, "%d.%d.%d", CUPS_VERSION_MAJOR, CUPS_VERSION_MINOR,
           CUPS_VERSION_PATCH);

  printf("Found CUPS version %s, expected version %s\n", version_str,
         CMAKE_EXPECTED_CUPS_VERSION);

  return strcmp(version_str, CMAKE_EXPECTED_CUPS_VERSION);
}
