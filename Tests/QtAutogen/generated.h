
#ifndef GENERATED_H
#define GENERATED_H

#include <QObject>

#include "myinterface.h"

class Generated : public QObject, MyInterface
{
  Q_OBJECT
  Q_INTERFACES(MyInterface)
public:
  explicit Generated(QObject *parent = 0);
};

#endif
