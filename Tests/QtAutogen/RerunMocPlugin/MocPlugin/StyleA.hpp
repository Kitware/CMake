#ifndef STYLEA_HPP
#define STYLEA_HPP

#include <QStylePlugin>

#include "UtilityMacros.hpp"

class StyleA : public QStylePlugin
{
  Q_OBJECT
  // Json file in source local directory
  Q_PLUGIN_METADATA(IID "org.styles.A" FILE "StyleA.json")
  A_CUSTOM_MACRO(org.styles.A, "StyleA_Custom.json", AnotherArg)
public:
  QStyle* create(const QString& key);
};

#endif
