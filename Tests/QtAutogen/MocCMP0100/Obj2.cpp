#include "Obj2.hh"

#include <QObject>

class Obj2Private : public QObject
{
  Q_OBJECT
public:
  Obj2Private();
  ~Obj2Private();
};

Obj2Private::Obj2Private()
{
}

Obj2Private::~Obj2Private()
{
}

Obj2::Obj2()
  : d(new Obj2Private)
{
}

Obj2::~Obj2()
{
  delete d;
}

#include "Obj2.moc"
