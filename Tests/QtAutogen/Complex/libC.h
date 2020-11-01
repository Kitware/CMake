
#ifndef LIBC_H
#define LIBC_H

#include <QObject>

#include "libB.h"
#include "libc_export.h"

class LIBC_EXPORT LibC : public QObject
{
  Q_OBJECT
public:
  explicit LibC(QObject* parent = 0);

  int foo();

private:
  LibB b;
};

#endif
