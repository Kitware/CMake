

#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include <ui_Compilers.h>

#include <QWidget>

class Compilers
  : public QWidget
  , public Ui::Compilers
{
  Q_OBJECT
public:
  Compilers(QWidget* p = nullptr)
    : QWidget(p)
  {
    this->setupUi(this);
  }
};
