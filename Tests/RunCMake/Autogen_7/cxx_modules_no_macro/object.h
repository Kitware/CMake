#pragma once
#include <QObject>

class Object : public QObject
{
  Q_OBJECT
public:
  using QObject::QObject;
signals:
  void objectSignal(int value);
};
