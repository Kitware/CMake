#ifndef TEST2_HPP
#define TEST2_HPP

#include "simpleLib.hpp"
#include <QObject>

// This object triggers the AUTOMOC on this file
class LObject : public QObject
{
  Q_OBJECT
public:
  Q_SLOT
  void aSlot(){};
};

#endif
