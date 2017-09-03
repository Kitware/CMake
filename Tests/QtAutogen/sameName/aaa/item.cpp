#include "item.hpp"

namespace aaa {

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

#include "aaa/item.moc"
