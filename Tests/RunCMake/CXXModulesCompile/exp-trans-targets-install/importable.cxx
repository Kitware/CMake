module;

#include "include/include.h"

#ifndef include_h_included
#  error "include define not found"
#endif

#include "includes/includes.h"

#ifndef includes_h_included
#  error "includes define not found"
#endif

#ifndef FROM_USAGE_REQS
#  error "missing `FROM_USAGE_REQS` define"
#endif

export module importable;

extern "C++" {
int forwarding();
}

export int from_import()
{
  return forwarding();
}
