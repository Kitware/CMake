#include "libshared.h"

int use_deprecated_msg()
{
  return libshared::function_deprecated_msg() +
    libshared::function_deprecated_exported_msg() +
    libshared::function_deprecated_excluded_msg();
}
