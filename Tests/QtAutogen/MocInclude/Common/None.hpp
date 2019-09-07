#ifndef None_HPP
#define None_HPP

#include <QObject>

// Object source comes without any _moc/.moc includes
class NonePrivate;
class None : public QObject
{
  Q_OBJECT
public:
  None();
  ~None();

private:
  NonePrivate* const d;
};

#endif
