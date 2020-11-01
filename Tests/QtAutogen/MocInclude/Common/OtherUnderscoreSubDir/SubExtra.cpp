#include "SubExtra.hpp"

#include "SubExtra_p.hpp"

SubExtraPrivate::SubExtraPrivate()
{
}

SubExtraPrivate::~SubExtraPrivate()
{
}

SubExtra::SubExtra()
  : d(new SubExtraPrivate)
{
}

SubExtra::~SubExtra()
{
  delete d;
}
