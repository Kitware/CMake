#include "ObjA.hpp"

class SubObjA : public QObject
{
  Q_OBJECT

public:
  SubObjA() = default;
  ~SubObjA() = default;

  Q_SLOT
  void aSlot();
};

void SubObjA::aSlot()
{
}

void ObjA::go()
{
  SubObjA subObj;
}

#include "ObjA.moc"
