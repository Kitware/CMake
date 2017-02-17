#include "StyleD.hpp"

class StyleD : public QStylePlugin
{
  Q_OBJECT
  // Json file in global sub director
  Q_PLUGIN_METADATA(IID "org.styles.D" FILE "sub/StyleD.json")
public:
  QStyle* create(const QString& key);
};

QStyle* StyleD::create(const QString& key)
{
  return 0;
}

#include "StyleD.moc"
