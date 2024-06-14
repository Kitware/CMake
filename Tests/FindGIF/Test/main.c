#include <assert.h>
#include <gif_lib.h>
#include <stdio.h>
#include <string.h>

// GIFLIB before version 5 didn't know this macro
#ifndef GIFLIB_MAJOR
#  define GIFLIB_MAJOR 4
#endif

int main(void)
{
  // because of the API changes we have to test different functions depending
  // on the version of GIFLIB
#if GIFLIB_MAJOR >= 5
  // test the linker
  GifErrorString(D_GIF_SUCCEEDED);

  // check the version
  char gif_version_string[16];
  snprintf(gif_version_string, 16, "%i.%i.%i", GIFLIB_MAJOR, GIFLIB_MINOR,
           GIFLIB_RELEASE);

  assert(strcmp(gif_version_string, CMAKE_EXPECTED_GIF_VERSION) == 0);
#else
  // test the linker
  GifLastError();

  // unfortunately there is no way to check the version in older version of
  // GIFLIB
#endif

  return 0;
}
