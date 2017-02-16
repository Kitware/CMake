#include "SubObjB.hpp"

namespace subB {

class SubObjB : public QObject
{
  Q_OBJECT

public:
  SubObjB() = default;
  ~SubObjB() = default;

  Q_SLOT
  void aSlot();
};

void SubObjB::aSlot()
{
}

void ObjB::go()
{
  SubObjB subObj;
}
}

#include "SubObjB.moc"
