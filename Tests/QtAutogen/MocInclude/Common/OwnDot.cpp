#include "OwnDot.hpp"

#include "OwnDot_p.h"

class OwnDotLocal : public QObject
{
  Q_OBJECT
public:
  OwnDotLocal();
  ~OwnDotLocal();
};

OwnDotLocal::OwnDotLocal()
{
}

OwnDotLocal::~OwnDotLocal()
{
}

OwnDotPrivate::OwnDotPrivate()
{
  OwnDotLocal localObj;
}

OwnDotPrivate::~OwnDotPrivate()
{
}

OwnDot::OwnDot()
  : d(new OwnDotPrivate)
{
}

OwnDot::~OwnDot()
{
  delete d;
}

#include "OwnDot.moc"
