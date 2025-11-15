module;

#include <subdir/header.h>

#ifndef from_subdir_header_h
#  error "Define from header found"
#endif

export module importable;

export int from_import()
{
  return 0;
}
