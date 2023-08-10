#ifndef Object_HPP
#define Object_HPP

#include <QObject>

#ifdef _EXTRA_DEFINE
class Object : public QObject
{
  Q_OBJECT
public:
  Object();
};
#endif

#endif
