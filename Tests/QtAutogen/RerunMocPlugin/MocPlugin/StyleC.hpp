#ifndef STYLEC_HPP
#define STYLEC_HPP

#include <QStylePlugin>

#include "UtilityMacros.hpp"

class StyleC : public QStylePlugin
{
  Q_OBJECT
  // Json file in global root directory
  Q_PLUGIN_METADATA(IID "org.styles.C" FILE "StyleC.json")
  A_CUSTOM_MACRO(org.styles.C, "StyleC_Custom.json", AnotherArg)
public:
  QStyle* create(const QString& key);
};

#endif
