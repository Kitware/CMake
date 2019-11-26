#ifndef ExternDot_HPP
#define ExternDot_HPP

#include <QObject>

// Object source includes externally generated .moc file
class ExternDot : public QObject
{
  Q_OBJECT
public:
  ExternDot();
  ~ExternDot();
};

#endif
