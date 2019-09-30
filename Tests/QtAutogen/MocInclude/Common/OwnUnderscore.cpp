#include "OwnUnderscore.hpp"

#include "OwnUnderscore_p.h"

OwnUnderscorePrivate::OwnUnderscorePrivate()
{
}

OwnUnderscorePrivate::~OwnUnderscorePrivate()
{
}

OwnUnderscore::OwnUnderscore()
  : d(new OwnUnderscorePrivate)
{
}

OwnUnderscore::~OwnUnderscore()
{
  delete d;
}

#include "moc_OwnUnderscore.cpp"
