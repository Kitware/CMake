#if defined SHLIBDEPS_OTHER

#  include "subdir/myotherlib.h"

int main()
{
  myotherlib_function();
}

#elif defined SHLIBDEPS_PRIVATE

#  include "shlibdeps-with-private-lib/myprivatelib.h"

int main()
{
  myprivatelib_function();
}

#else

#  include "mylib.h"

int main()
{
  mylib_function();
}

#endif
