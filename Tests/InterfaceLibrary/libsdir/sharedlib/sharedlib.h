
#ifndef SHAREDLIB_H
#define SHAREDLIB_H

#include "shareddependlib.h"
#include "sharedlib_export.h"

struct SHAREDLIB_EXPORT SharedLibObject
{
  SharedDependLibObject object() const;
  int foo() const;
};

#endif
