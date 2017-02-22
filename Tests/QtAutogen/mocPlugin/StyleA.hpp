#ifndef STYLEA_HPP
#define STYLEA_HPP

#include <QStylePlugin>

class StyleA : public QStylePlugin
{
  Q_OBJECT
  // Json file in local directory
  Q_PLUGIN_METADATA(IID "org.styles.A" FILE "StyleA.json")
public:
  QStyle* create(const QString& key);
};

#endif
