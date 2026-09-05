module;
#include <QObject>
module Mod;

// A module implementation unit is not a member of the CXX_MODULES file set, so
// AUTOMOC treats it like any other source and honors the ".moc" include below.
// moc then rejects the macro.
class ImplObject : public QObject
{
  Q_OBJECT
public:
  using QObject::QObject;
signals:
  void implSignal();
};

bool implObjectWorks()
{
  return ImplObject::staticMetaObject.methodCount() > 0;
}

#include "mod-impl.moc"
