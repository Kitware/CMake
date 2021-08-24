#include "mainwindow.h"
#include "ui_mainwindow.h"

int main(int argc, char* argv[])
{
  MocWidget mw;
  Ui::Widget mwUi;
  mwUi.setupUi(&mw);
  return mw.objectName() == "Widget2" ? 0 : 1;
}
