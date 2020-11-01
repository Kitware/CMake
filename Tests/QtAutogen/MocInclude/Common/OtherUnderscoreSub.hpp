#ifndef OtherUnderscoreSub_HPP
#define OtherUnderscoreSub_HPP

#include <QObject>

// Sources includes a moc_ includes of an extra object in a subdirectory
class OtherUnderscoreSubPrivate;
class OtherUnderscoreSub : public QObject
{
  Q_OBJECT
public:
  OtherUnderscoreSub();
  ~OtherUnderscoreSub();

private:
  OtherUnderscoreSubPrivate* const d;
};

#endif
