#ifndef STYLEB_HPP
#define STYLEB_HPP

#include <QStylePlugin>

class StyleB : public QStylePlugin
{
  Q_OBJECT
  // Json file in local subdirectory
  Q_PLUGIN_METADATA(IID "org.styles.B" FILE "jsonIn/StyleB.json")
public:
  QStyle* create(const QString& key);
};

#endif
