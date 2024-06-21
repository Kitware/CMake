#pragma once

#include <QWidget>

#include "ui_MyWindow.h"

class MyWindow : public QWidget
{
  Q_OBJECT

public:
  explicit MyWindow(QWidget* parent = nullptr);

private:
  Ui::MyWindow m_ui;
};
