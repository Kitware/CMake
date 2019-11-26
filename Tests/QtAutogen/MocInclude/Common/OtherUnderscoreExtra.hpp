#ifndef OtherUnderscoreEXTRA_HPP
#define OtherUnderscoreEXTRA_HPP

#include <QObject>

class OtherUnderscoreExtraPrivate;
class OtherUnderscoreExtra : public QObject
{
  Q_OBJECT
public:
  OtherUnderscoreExtra();
  ~OtherUnderscoreExtra();

private:
  OtherUnderscoreExtraPrivate* const d;
};

#endif
