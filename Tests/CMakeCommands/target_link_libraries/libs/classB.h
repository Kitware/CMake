
#ifndef CLASS_B_H
#define CLASS_B_H

#include "libb_export.h"

class classA;

class LIBB_EXPORT classB
{
public:
  classB();

  classA* a() const;
};

#endif
