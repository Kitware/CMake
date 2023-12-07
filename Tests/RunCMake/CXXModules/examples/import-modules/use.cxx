#if defined(__has_include)
#  if __has_include(<include/include.h>)
#    error "include directories leaked from private module requirements"
#  endif
#endif

import importable;

int main(int argc, char* argv[])
{
  return from_import();
}
