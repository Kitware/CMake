
#include "libshared.h"

#include "libstatic.h"

// #define BUILD_FAIL

#ifndef BUILD_FAIL
#define DOES_NOT_BUILD(function)
#else
#define DOES_NOT_BUILD(function) function
#endif

int main()
{
  {
    Libshared l;
    l.libshared();
    l.libshared_exported();
    l.libshared_deprecated();
    l.libshared_not_exported();

    DOES_NOT_BUILD(l.libshared_excluded();)
  }

  {
    LibsharedNotExported l;
    DOES_NOT_BUILD(l.libshared();)
    l.libshared_exported();
    l.libshared_deprecated();
    DOES_NOT_BUILD(l.libshared_not_exported();)
    DOES_NOT_BUILD(l.libshared_excluded();)
  }

  {
    LibsharedExcluded l;
    DOES_NOT_BUILD(l.libshared();)
    l.libshared_exported();
    l.libshared_deprecated();
    DOES_NOT_BUILD(l.libshared_not_exported();)
    DOES_NOT_BUILD(l.libshared_excluded();)
  }

  libshared_exported();
  libshared_deprecated();
  DOES_NOT_BUILD(libshared_not_exported();)
  DOES_NOT_BUILD(libshared_excluded();)

  {
    Libstatic l;
    l.libstatic();
    l.libstatic_exported();
    l.libstatic_deprecated();
    l.libstatic_not_exported();
    l.libstatic_excluded();
  }

  {
    LibstaticNotExported l;
    l.libstatic();
    l.libstatic_exported();
    l.libstatic_deprecated();
    l.libstatic_not_exported();
    l.libstatic_excluded();
  }

  {
    LibstaticExcluded l;
    l.libstatic();
    l.libstatic_exported();
    l.libstatic_deprecated();
    l.libstatic_not_exported();
    l.libstatic_excluded();
  }

  libstatic_exported();
  libstatic_deprecated();
  libstatic_not_exported();
  libstatic_excluded();

  return 0;
}
