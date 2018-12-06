#include <QObject>

class LocalObject : public QObject
{
  Q_OBJECT
public:
  LocalObject(){};
};

void mocOnly()
{
  LocalObject obj;
}

#include "main.moc"
