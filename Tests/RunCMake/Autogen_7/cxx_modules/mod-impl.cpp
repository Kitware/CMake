module;
#include <QObject>
module Mod;

// The implementation partition is imported here rather than from mod.cppm,
// where Clang diagnoses it with
// -Wimport-implementation-partition-unit-in-interface-unit.  Referencing the
// meta-object makes the link fail if AUTOMOC did not moc mod-internal.cppm.
import :Internal;

int internalMethodCount()
{
  return InternalObject::staticMetaObject.methodCount();
}
