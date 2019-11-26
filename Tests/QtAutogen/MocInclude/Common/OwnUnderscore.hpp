#ifndef OwnUnderscore_HPP
#define OwnUnderscore_HPP

#include <QObject>

// Object source comes with a _moc include
class OwnUnderscorePrivate;
class OwnUnderscore : public QObject
{
  Q_OBJECT
public:
  OwnUnderscore();
  ~OwnUnderscore();

private:
  OwnUnderscorePrivate* const d;
};

#endif
