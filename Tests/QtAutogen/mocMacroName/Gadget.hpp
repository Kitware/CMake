#ifndef GADGET_HPP
#define GADGET_HPP

#include <QMetaType>

class Gadget
{
  Q_GADGET
  Q_PROPERTY(int test MEMBER test)
public:
  Gadget();
  int test;
};

#endif
