#ifndef OwnDot_HPP
#define OwnDot_HPP

#include <QObject>

// Object source comes with a .moc include
class OwnDotPrivate;
class OwnDot : public QObject
{
  Q_OBJECT
public:
  OwnDot();
  ~OwnDot();

private:
  OwnDotPrivate* const d;
};

#endif
