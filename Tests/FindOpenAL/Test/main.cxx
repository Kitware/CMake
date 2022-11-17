#include <AL/al.h>
#include <AL/alc.h>
#include <stdio.h>

int main()
{
  /* Reference an AL symbol without requiring a context at runtime.  */
  printf("&alGetString = %p\n", &alGetString);

  /* Reference an ALC symbol without requiring a context at runtime.  */
  printf("&alcGetString = %p\n", &alcGetString);
  return 0;
}
