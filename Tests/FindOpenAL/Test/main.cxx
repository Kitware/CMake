#ifdef __APPLE__
#  include "OpenAL/al.h"
#  include "OpenAL/alc.h"
#else
#  include <AL/al.h>
#  include <AL/alc.h>
#endif
#include <stdio.h>

int main()
{
  /* Reference an AL symbol without requiring a context at runtime.  */
  printf("AL_VERSION: %s\n", alGetString(AL_VERSION));

  /* Reference an ALC symbol without requiring a context at runtime.  */
  printf("ALC_DEVICE_SPECIFIER: %s\n",
         alcGetString(NULL, ALC_DEVICE_SPECIFIER));
  return 0;
}
