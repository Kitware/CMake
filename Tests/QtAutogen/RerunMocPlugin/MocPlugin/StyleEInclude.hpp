#ifndef STYLEE_INCLUDE_HPP
#define STYLEE_INCLUDE_HPP

#include <QStylePlugin>

#include "UtilityMacros.hpp"

class StyleE : public QStylePlugin
{
  Q_OBJECT
  // Json files in global root directory
  Q_PLUGIN_METADATA(IID "org.styles.E" FILE "StyleE.json")
  A_CUSTOM_MACRO(org.styles.E, "StyleE_Custom.json", AnotherArg)
public:
  QStyle* create(const QString& key);
};

#endif
