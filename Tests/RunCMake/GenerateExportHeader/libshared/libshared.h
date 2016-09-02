
#ifndef LIBSHARED_H
#define LIBSHARED_H

#include "libshared_export.h"

class LIBSHARED_EXPORT Libshared
{
public:
  int libshared() const;

  int libshared_exported() const;

  int LIBSHARED_DEPRECATED libshared_deprecated() const;

  int libshared_not_exported() const;

  int LIBSHARED_NO_EXPORT libshared_excluded() const;

  static int const LIBSHARED_EXPORT data_exported;

  static int const data_not_exported;

  static int const LIBSHARED_NO_EXPORT data_excluded;
};

class LibsharedNotExported
{
public:
  int libshared() const;

  int LIBSHARED_EXPORT libshared_exported() const;

  int LIBSHARED_DEPRECATED_EXPORT libshared_deprecated() const;

  int libshared_not_exported() const;

  int LIBSHARED_NO_EXPORT libshared_excluded() const;

  static int const LIBSHARED_EXPORT data_exported;

  static int const data_not_exported;

  static int const LIBSHARED_NO_EXPORT data_excluded;
};

class LIBSHARED_NO_EXPORT LibsharedExcluded
{
public:
  int libshared() const;

  int LIBSHARED_EXPORT libshared_exported() const;

  int LIBSHARED_DEPRECATED_EXPORT libshared_deprecated() const;

  int libshared_not_exported() const;

  int LIBSHARED_NO_EXPORT libshared_excluded() const;

  static int const LIBSHARED_EXPORT data_exported;

  static int const data_not_exported;

  static int const LIBSHARED_NO_EXPORT data_excluded;
};

LIBSHARED_EXPORT int libshared_exported();

LIBSHARED_DEPRECATED_EXPORT int libshared_deprecated();

int libshared_not_exported();

int LIBSHARED_NO_EXPORT libshared_excluded();

extern int const LIBSHARED_EXPORT data_exported;

extern int const data_not_exported;

extern int const LIBSHARED_NO_EXPORT data_excluded;

LIBSHARED_EXPORT void use_int(int);

#endif
