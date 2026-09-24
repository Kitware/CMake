#pragma once

#include <QObject>

import Mod;

class HeaderObject : public QObject
{
  Q_OBJECT
public:
  int answer() const { return modAnswer(); }
};
