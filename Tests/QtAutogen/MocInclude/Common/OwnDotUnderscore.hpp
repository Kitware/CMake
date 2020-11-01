#ifndef LOwnDotUnderscore_HPP
#define LOwnDotUnderscore_HPP

#include <QObject>

// Object source comes with a .moc and a _moc include
class OwnDotUnderscorePrivate;
class OwnDotUnderscore : public QObject
{
  Q_OBJECT
public:
  OwnDotUnderscore();
  ~OwnDotUnderscore();

private:
  OwnDotUnderscorePrivate* const d;
};

#endif
