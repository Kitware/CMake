
#ifndef LIBWIDGET_H
#define LIBWIDGET_H

#include <QWidget>

#include "ui_libwidget.h"

class LibWidget : public QWidget
{
  Q_OBJECT
public:
  explicit LibWidget(QWidget *parent = 0);

private:
  QScopedPointer<Ui::LibWidget> ui;
};

#endif
