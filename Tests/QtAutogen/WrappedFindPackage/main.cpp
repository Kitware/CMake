#include <qobject.h>

class MyObject : public QObject
{
  Q_OBJECT
public:
  MyObject(QObject* parent = 0)
    : QObject(parent)
  {
  }
};

int main()
{
  MyObject obj;
  return 0;
}

#include "main.moc"
