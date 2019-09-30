#ifndef TEST3_HPP
#define TEST3_HPP

#include <QObject>

#include "simpleLib.hpp"

// This object triggers the AUTOMOC on this file
class LObject : public QObject
{
  Q_OBJECT
public:
  Q_SLOT
  void aSlot(){};
};

#endif
