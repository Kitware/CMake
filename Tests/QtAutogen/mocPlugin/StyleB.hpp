#ifndef STYLEB_HPP
#define STYLEB_HPP

#include "StyleCommon.hpp"
#include <QStylePlugin>

class StyleB : public QStylePlugin
{
  Q_OBJECT
  // Json file in local subdirectory
  Q_PLUGIN_METADATA(IID "org.styles.B" FILE "jsonIn/StyleB.json")
  A_CUSTOM_MACRO(SomeArg, "jsonIn/StyleB_Custom.json", AnotherArg)
public:
  QStyle* create(const QString& key);
};

#endif
