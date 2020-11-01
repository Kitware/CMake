#include "None.hpp"

#include "None_p.h"

NonePrivate::NonePrivate()
{
}

NonePrivate::~NonePrivate()
{
}

None::None()
  : d(new NonePrivate)
{
}

None::~None()
{
  delete d;
}
