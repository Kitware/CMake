#ifdef _WIN32
#  error "GLES2 cannot be tested on WIN32 platforms."
#endif
#ifdef __APPLE__
#  error "GLES2 cannot be tested on macOS platform."
#else
#  include <GLES2/gl2.h>
#endif

#include <stdio.h>

int main(void)
{
  /* Reference a GL symbol without requiring a context at runtime.  */
  printf("&glGetString = %p\n", &glGetString);
  return 0;
}
