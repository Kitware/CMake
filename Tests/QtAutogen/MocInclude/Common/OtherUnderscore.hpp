#ifndef OtherUnderscore_HPP
#define OtherUnderscore_HPP

#include <QObject>

// Sources includes a moc_ includes of an extra object
class OtherUnderscorePrivate;
class OtherUnderscore : public QObject
{
  Q_OBJECT
public:
  OtherUnderscore();
  ~OtherUnderscore();

private:
  OtherUnderscorePrivate* const d;
};

#endif
