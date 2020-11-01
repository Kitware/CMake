#ifndef OBJ_HH
#define OBJ_HH

#include <QObject>

// Qt enabled private class
class ObjPrivate;
// Qt enabled class
class Obj : public QObject
{
  Q_OBJECT
public:
  Obj();
  ~Obj();

private:
  ObjPrivate* const d;
};

#endif
