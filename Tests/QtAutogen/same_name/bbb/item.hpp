#ifndef SDB_ITEM_HPP
#define SDB_ITEM_HPP

#include <QObject>

namespace bbb {

class Item : public QObject
{
  Q_OBJECT
  Q_SLOT
  void go();
};
}

#endif
