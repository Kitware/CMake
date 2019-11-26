
#ifndef LIBA_H
#define LIBA_H

#include <QObject>

#include "liba_export.h"

class LIBA_EXPORT LibA : public QObject
{
  Q_OBJECT
public:
  explicit LibA(QObject* parent = 0);

  int foo();
};

#endif
