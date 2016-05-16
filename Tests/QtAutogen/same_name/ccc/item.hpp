#ifndef SDC_ITEM_HPP
#define SDC_ITEM_HPP

#include <QObject>

namespace ccc {

class Item : public QObject
{
  Q_OBJECT
  Q_SLOT
  void go();
};
}

#endif
