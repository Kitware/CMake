#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Usage:
//
//  /path/to/program arg1 [arg2 [...]]
//
// Return EXIT_SUCCESS if 'custom_argument' string was found
// in <arg1> and 'generated_exe_emulator_expected'
// string was found in <arg2>
// Return EXIT_FAILURE if 'custom_argument' string was not
// found in <arg1> or 'generated_exe_emulator_expected'
// string was not found in <arg2>.

int main(int argc, const char* argv[])
{
  // Require a slash to make sure it is a path and not a target name.
  const char* substring_success = "/generated_exe_emulator_expected";
  const char* substring_custom_argument = "custom_argument";

  if (argc < 2) {
    return EXIT_FAILURE;
  }
  if (strstr(argv[1], substring_custom_argument) != 0 &&
      strstr(argv[2], substring_success) != 0) {
    return EXIT_SUCCESS;
  }
  return EXIT_FAILURE;
}
