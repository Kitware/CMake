
#include "classC.h"

#include "classA.h"
#include "classB.h"

classC::classC()
{
  classB b;
}

classA* classC::a() const
{
  return new classA();
}
