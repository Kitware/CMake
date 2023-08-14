#ifdef _WIN32
#  error "GLES3 cannot be tested on WIN32 platforms."
#endif
#ifdef __APPLE__
#  error "GLES3 cannot be tested on macOS platform."
#else
#  include <GLES3/gl3.h>
#endif

#include <stdio.h>

int main()
{
  /* Reference a GL symbol without requiring a context at runtime.  */
  printf("&glGetString = %p\n", &glGetString);
  return 0;
}
