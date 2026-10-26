#include "libattributes.h"

int libattributes::Class::method() const
{
  return 0;
}

int libattributes::Class::method_exported() const
{
  return 0;
}

int libattributes::Class::method_deprecated_exported() const
{
  return 0;
}

int libattributes::Class::method_excluded() const
{
  return 0;
}

int const libattributes::Class::data = 1;

int const libattributes::Class::data_exported = 1;

int const libattributes::Class::data_excluded = 1;

int libattributes::ExportedClass::method() const
{
  return 0;
}

int libattributes::ExportedClass::method_excluded() const
{
  return 0;
}

int libattributes::function()
{
  return 0;
}

int libattributes::function_exported()
{
  return 0;
}

int libattributes::function_deprecated_exported()
{
  return 0;
}

int libattributes::function_excluded()
{
  return 0;
}

int const libattributes::data = 1;

int const libattributes::data_exported = 1;

int const libattributes::data_excluded = 1;

int libattributes_function_c(void)
{
  return 0;
}
