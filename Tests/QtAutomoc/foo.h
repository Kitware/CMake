#ifndef FOO_H
#define FOO_H

#include <QObject>

class Foo : public QObject
{
  Q_OBJECT
  public:
    Foo();
  public slots:
    void doFoo();
};

#endif
