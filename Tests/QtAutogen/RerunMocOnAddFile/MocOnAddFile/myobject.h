#ifndef MYOBJECT_H
#define MYOBJECT_H

#include <qobject.h>

class MyObject : public QObject
{
  Q_OBJECT
public:
  MyObject(QObject* parent = 0);
};

#endif
