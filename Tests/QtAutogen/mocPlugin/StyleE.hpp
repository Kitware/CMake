#ifndef STYLEE_HPP
#define STYLEE_HPP

#include <QStylePlugin>

class StyleE : public QStylePlugin
{
  Q_OBJECT
  // No Json file
  Q_PLUGIN_METADATA(IID "org.styles.E")
public:
  QStyle* create(const QString& key);
};

#endif
