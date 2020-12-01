#pragma once

#include <foo.h>

class MyObject : public QObject
{
  Q_OBJECT
public:
  MyObject(QObject* parent = 0);
};
