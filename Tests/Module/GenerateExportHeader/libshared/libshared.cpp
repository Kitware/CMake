
#include "libshared.h"

#ifdef __cplusplus

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

int Libshared::libshared_not_exported() const {
  return 0;
}

int Libshared::libshared_excluded() const {
  return 0;
}

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

int LibsharedNotExported::libshared_not_exported() const {
  return 0;
}

int LibsharedNotExported::libshared_excluded() const {
  return 0;
}

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

int LibsharedExcluded::libshared_not_exported() const {
  return 0;
}

int LibsharedExcluded::libshared_excluded() const {
  return 0;
}

#endif

int libshared(void) {
  return 0;
}

int libshared_exported(void) {
  return 0;
}

int libshared_deprecated(void) {
  return 0;
}

int libshared_not_exported(void) {
  return 0;
}

int libshared_excluded(void) {
  return 0;
}
