
#ifndef LIBB_H
#define LIBB_H

#include <QObject>

#include "libA.h"
#include "libb_export.h"

class LIBB_EXPORT LibB : public QObject
{
  Q_OBJECT
public:
  explicit LibB(QObject* parent = 0);

  int foo();

private:
  LibA a;
};

#endif
