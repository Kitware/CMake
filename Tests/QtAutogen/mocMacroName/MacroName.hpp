#ifndef MACRONAME_HPP
#define MACRONAME_HPP

#include "MacroAlias.hpp"

// Test Qt object macro hidden in a macro (AUTOMOC_MACRO_NAMES)
class MacroName : public QObject
{
  QO_ALIAS
public:
  MacroName();

signals:
  void aSignal();

public slots:
  void aSlot();
};

#endif
