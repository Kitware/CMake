#include "OwnDotUnderscore.hpp"

#include "OwnDotUnderscore_p.h"

class OwnDotUnderscoreLocal : public QObject
{
  Q_OBJECT
public:
  OwnDotUnderscoreLocal();
  ~OwnDotUnderscoreLocal();
};

OwnDotUnderscoreLocal::OwnDotUnderscoreLocal()
{
}

OwnDotUnderscoreLocal::~OwnDotUnderscoreLocal()
{
}

OwnDotUnderscorePrivate::OwnDotUnderscorePrivate()
{
  OwnDotUnderscoreLocal localObj;
}

OwnDotUnderscorePrivate::~OwnDotUnderscorePrivate()
{
}

OwnDotUnderscore::OwnDotUnderscore()
  : d(new OwnDotUnderscorePrivate)
{
}

OwnDotUnderscore::~OwnDotUnderscore()
{
  delete d;
}

#include "OwnDotUnderscore.moc"
#include "moc_OwnDotUnderscore.cpp"
