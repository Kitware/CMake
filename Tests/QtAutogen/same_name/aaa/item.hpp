#ifndef SDA_ITEM_HPP
#define SDA_ITEM_HPP

#include <QObject>

namespace aaa {

class Item : public QObject
{
  Q_OBJECT
  Q_SLOT
  void go();
};
}

#endif
