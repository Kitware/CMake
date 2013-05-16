
#include "libshared_and_static.h"

#ifdef __cplusplus

int LibsharedAndStatic::libshared_and_static() const
{
  return 0;
}

int LibsharedAndStatic::libshared_and_static_exported() const
{
  return 0;
}

int LibsharedAndStatic::libshared_and_static_deprecated() const
{
  return 0;
}

int LibsharedAndStatic::libshared_and_static_not_exported() const {
  return 0;
}

int LibsharedAndStatic::libshared_and_static_excluded() const {
  return 0;
}

int LibsharedAndStaticNotExported::libshared_and_static() const
{
  return 0;
}

int LibsharedAndStaticNotExported::libshared_and_static_exported() const
{
  return 0;
}

int LibsharedAndStaticNotExported::libshared_and_static_deprecated() const
{
  return 0;
}

int LibsharedAndStaticNotExported::libshared_and_static_not_exported() const {
  return 0;
}

int LibsharedAndStaticNotExported::libshared_and_static_excluded() const {
  return 0;
}

int LibsharedAndStaticExcluded::libshared_and_static() const
{
  return 0;
}

int LibsharedAndStaticExcluded::libshared_and_static_exported() const
{
  return 0;
}

int LibsharedAndStaticExcluded::libshared_and_static_deprecated() const
{
  return 0;
}

int LibsharedAndStaticExcluded::libshared_and_static_not_exported() const {
  return 0;
}

int LibsharedAndStaticExcluded::libshared_and_static_excluded() const {
  return 0;
}

#endif

int libshared_and_static(void) {
  return 0;
}

int libshared_and_static_exported(void) {
  return 0;
}

int libshared_and_static_deprecated(void) {
  return 0;
}

int libshared_and_static_not_exported(void) {
  return 0;
}

int libshared_and_static_excluded(void) {
  return 0;
}
