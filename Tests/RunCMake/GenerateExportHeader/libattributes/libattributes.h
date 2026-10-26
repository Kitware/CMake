#ifndef LIBATTRIBUTES_H
#define LIBATTRIBUTES_H

#include "libattributes_c.h"
#include "libattributes_export.h"

namespace libattributes {

class Class
{
public:
  int method() const;

  LIBATTRIBUTES_EXPORT int method_exported() const;

  LIBATTRIBUTES_DEPRECATED_EXPORT int method_deprecated_exported() const;

  LIBATTRIBUTES_NO_EXPORT int method_excluded() const;

  static int const data;

  LIBATTRIBUTES_EXPORT static int const data_exported;

  LIBATTRIBUTES_NO_EXPORT static int const data_excluded;
};

class LIBATTRIBUTES_EXPORT ExportedClass
{
public:
  int method() const;

  LIBATTRIBUTES_NO_EXPORT int method_excluded() const;
};

int function();

LIBATTRIBUTES_EXPORT int function_exported();

LIBATTRIBUTES_DEPRECATED_EXPORT int function_deprecated_exported();

LIBATTRIBUTES_NO_EXPORT int function_excluded();

extern int const data;

LIBATTRIBUTES_EXPORT extern int const data_exported;

LIBATTRIBUTES_NO_EXPORT extern int const data_excluded;

} // namespace libattributes

#endif
