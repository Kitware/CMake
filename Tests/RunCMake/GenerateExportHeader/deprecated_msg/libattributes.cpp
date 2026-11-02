#include "libattributes.h"

int use_deprecated_msg()
{
  return libattributes::function_deprecated_msg() +
    libattributes::function_deprecated_exported_msg() +
    libattributes::function_deprecated_excluded_msg();
}
