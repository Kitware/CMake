#include "ObjD.hpp"

class SubObjD : public QObject
{
  Q_OBJECT

public:
  SubObjD() = default;
  ~SubObjD() = default;

  Q_SLOT
  void aSlot();
};

void SubObjD::aSlot()
{
}

void ObjD::go()
{
  SubObjD subObj;
}

#include "ObjD.moc"
// Header in subdirectory
#include "subA/moc_SubObjA.cpp"
