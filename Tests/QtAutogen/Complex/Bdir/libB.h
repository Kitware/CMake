
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

// moc processes a class marked with a deprecation message only if the
// generated export header shows it the compiler-specific spelling.
class LIBB_DEPRECATED_EXPORT_MSG("use LibB") DeprecatedLibB : public QObject
{
  Q_OBJECT
public:
  explicit DeprecatedLibB(QObject* parent = 0);
};

#endif
