#include <qobject.h>

class AnotherObject : public QObject
{
  Q_OBJECT
public:
  AnotherObject() {}
};

AnotherObject* createAnotherObject()
{
  return new AnotherObject();
}

#include "anotherobject.moc"
