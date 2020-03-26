#include "Obj.hh"

#include <QObject>

class ObjPrivate : public QObject
{
  Q_OBJECT
public:
  ObjPrivate();
  ~ObjPrivate();
};

ObjPrivate::ObjPrivate()
{
}

ObjPrivate::~ObjPrivate()
{
}

Obj::Obj()
  : d(new ObjPrivate)
{
}

Obj::~Obj()
{
  delete d;
}

#include "Obj.moc"
