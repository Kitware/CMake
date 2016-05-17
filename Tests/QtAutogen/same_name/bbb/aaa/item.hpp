#ifndef SDB_SDA_ITEM_HPP
#define SDB_SDA_ITEM_HPP

#include <QObject>

namespace bbb {
namespace aaa {

class Item : public QObject
{
  Q_OBJECT
  Q_SLOT
  void go();
};
}
}

#endif
