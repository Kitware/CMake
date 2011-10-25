
#include "classB.h"

#include "classA.h"

classB::classB()
{
}

classA* classB::a() const
{
  return new classA();
}
