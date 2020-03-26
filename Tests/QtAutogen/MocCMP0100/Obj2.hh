#ifndef OBJ2_HH
#define OBJ2_HH

#include <QObject>

// Qt enabled private class
class Obj2Private;
// Qt enabled class
class Obj2 : public QObject
{
  Q_OBJECT
public:
  Obj2();
  ~Obj2();

private:
  Obj2Private* const d;
};

#endif
