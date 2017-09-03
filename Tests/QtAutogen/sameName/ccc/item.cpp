#include "item.hpp"

namespace ccc {

class MocLocal : public QObject
{
  Q_OBJECT;

public:
  MocLocal() = default;
  ~MocLocal() = default;
};

void Item::go()
{
  MocLocal obj;
}
}

// Include own moc files
#include "ccc/item.moc"
#include "moc_item.cpp"
