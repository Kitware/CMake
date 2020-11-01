#include "OtherUnderscoreExtra.hpp"

#include "OtherUnderscoreExtra_p.hpp"

OtherUnderscoreExtraPrivate::OtherUnderscoreExtraPrivate()
{
}

OtherUnderscoreExtraPrivate::~OtherUnderscoreExtraPrivate()
{
}

OtherUnderscoreExtra::OtherUnderscoreExtra()
  : d(new OtherUnderscoreExtraPrivate)
{
}

OtherUnderscoreExtra::~OtherUnderscoreExtra()
{
  delete d;
}
