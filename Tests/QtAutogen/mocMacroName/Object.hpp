#ifndef OBJECT_HPP
#define OBJECT_HPP

#include <QObject>

class Object : public QObject
{
  Q_OBJECT
  Q_PROPERTY(int test MEMBER test)
public:
  Object();

  Q_SLOT
  void aSlot();

  int test;
};

#endif
