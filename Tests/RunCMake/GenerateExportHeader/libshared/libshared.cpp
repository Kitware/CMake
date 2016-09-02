
#include "libshared.h"

int Libshared::libshared() const
{
  return 0;
}

int Libshared::libshared_exported() const
{
  return 0;
}

int Libshared::libshared_deprecated() const
{
  return 0;
}

int Libshared::libshared_not_exported() const
{
  return 0;
}

int Libshared::libshared_excluded() const
{
  return 0;
}

int const Libshared::data_exported = 1;

int const Libshared::data_not_exported = 1;

int const Libshared::data_excluded = 1;

int LibsharedNotExported::libshared() const
{
  return 0;
}

int LibsharedNotExported::libshared_exported() const
{
  return 0;
}

int LibsharedNotExported::libshared_deprecated() const
{
  return 0;
}

int LibsharedNotExported::libshared_not_exported() const
{
  return 0;
}

int LibsharedNotExported::libshared_excluded() const
{
  return 0;
}

int const LibsharedNotExported::data_exported = 1;

int const LibsharedNotExported::data_not_exported = 1;

int const LibsharedNotExported::data_excluded = 1;

int LibsharedExcluded::libshared() const
{
  return 0;
}

int LibsharedExcluded::libshared_exported() const
{
  return 0;
}

int LibsharedExcluded::libshared_deprecated() const
{
  return 0;
}

int LibsharedExcluded::libshared_not_exported() const
{
  return 0;
}

int LibsharedExcluded::libshared_excluded() const
{
  return 0;
}

int const LibsharedExcluded::data_exported = 1;

int const LibsharedExcluded::data_not_exported = 1;

int const LibsharedExcluded::data_excluded = 1;

int libshared()
{
  return 0;
}

int libshared_exported()
{
  return 0;
}

int libshared_deprecated()
{
  return 0;
}

int libshared_not_exported()
{
  return 0;
}

int libshared_excluded()
{
  return 0;
}

int const data_exported = 1;

int const data_not_exported = 1;

int const data_excluded = 1;

void use_int(int)
{
}
