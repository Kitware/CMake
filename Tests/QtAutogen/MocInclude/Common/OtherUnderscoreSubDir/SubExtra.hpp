#ifndef SubExtra_HPP
#define SubExtra_HPP

#include <QObject>

class SubExtraPrivate;
class SubExtra : public QObject
{
  Q_OBJECT
public:
  SubExtra();
  ~SubExtra();

private:
  SubExtraPrivate* const d;
};

#endif
