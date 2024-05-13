#include <SDL.h>

int main(void)
{
  // Test 1 requires headers only.
  SDL_version compiled;
  SDL_VERSION(&compiled);
  if (compiled.major != CMAKE_EXPECTED_SDL_VERSION_MAJOR ||
      compiled.minor != CMAKE_EXPECTED_SDL_VERSION_MINOR ||
      compiled.patch != CMAKE_EXPECTED_SDL_VERSION_PATCH)
    return 1;

  // Test 2 requires to link to the library.
  if (SDL_WasInit(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0)
    return 2;

  return 0;
}
