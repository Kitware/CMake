#include <QObject>

#include "ui_mainwindow.h"

class MainWindow : public QObject
{
  Q_OBJECT
public:
  MainWindow() { mwUi.setupUi(&mw); }

  MocWidget mw;
  Ui::Widget mwUi;
};
