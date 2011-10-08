
#ifndef CLASS_C_H
#define CLASS_C_H

#include "libc_export.h"

class classA;

class LIBC_EXPORT classC
{
public:
  classC();

  classA* a() const;
};

#endif