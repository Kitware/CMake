#ifndef SHLIBDEPS_PRIVATE

#  include "mylib.h"

int main()
{
  mylib_function();
}

#else

#  include "shlibdeps-with-private-lib/myprivatelib.h"

int main()
{
  myprivatelib_function();
}

#endif
