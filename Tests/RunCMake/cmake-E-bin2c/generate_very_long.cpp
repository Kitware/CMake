#include <iostream>

#ifdef _WIN32
#  include <fcntl.h> // for _O_BINARY
#  include <io.h>    // for _setmode
#  include <stdio.h> // for _fileno
#endif

int main()
{
#ifdef _WIN32
  _setmode(_fileno(stdout), _O_BINARY);
#endif

  for (unsigned long i = 0; i < JACK_COUNT; i++) {
    std::cout
      << " All   work   and   no   play   makes   Jack   a   dull   boy. \n";
  }

  return 0;
}
