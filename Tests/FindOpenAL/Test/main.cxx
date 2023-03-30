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
  printf("&alGetString = %p\n", &alGetString);

  /* Reference an ALC symbol without requiring a context at runtime.  */
  printf("&alcGetString = %p\n", &alcGetString);
  return 0;
}
