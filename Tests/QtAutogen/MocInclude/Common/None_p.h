#ifndef None_P_HPP
#define None_P_HPP

#include <QObject>

class NonePrivate : public QObject
{
  Q_OBJECT
public:
  NonePrivate();
  ~NonePrivate();
};

#endif
