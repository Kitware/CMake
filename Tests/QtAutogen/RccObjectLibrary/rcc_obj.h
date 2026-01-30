#ifndef RCC_OBJ_H
#define RCC_OBJ_H

#include <QObject>
#include <QString>

class RccObj : public QObject
{
  Q_OBJECT
public:
  RccObj();
  static QString resourceContent();
};

#endif
