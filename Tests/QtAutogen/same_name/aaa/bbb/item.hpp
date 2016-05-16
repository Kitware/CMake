#ifndef SDA_SDB_ITEM_HPP
#define SDA_SDB_ITEM_HPP

#include <QObject>

namespace aaa {
namespace bbb {

class Item : public QObject
{
  Q_OBJECT
  Q_SLOT
  void go();
};
}
}

#endif
