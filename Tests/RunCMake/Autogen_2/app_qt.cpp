#include <QObject>

class Mango : public QObject
{
  Q_OBJECT
public:
Q_SIGNALS:
  void eatFruit();
};

#include "app_qt.moc"
