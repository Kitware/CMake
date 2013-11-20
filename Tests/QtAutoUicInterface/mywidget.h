
#ifndef MYWIDGET_H
#define MYWIDGET_H

#include <QWidget>

#include "ui_mywidget.h"

class MyWidget : public QWidget
{
  Q_OBJECT
public:
  explicit MyWidget(QWidget *parent = 0);

private:
  QScopedPointer<Ui::MyWidget> ui;
};

#endif
