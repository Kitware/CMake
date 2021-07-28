#include "ui_mainwindow.h"

extern bool subdircheck();

int main(int argc, char* argv[])
{
  MocWidget mw;
  Ui::Widget mwUi;
  mwUi.setupUi(&mw);
  return mw.objectName() == "Widget2" && subdircheck() ? 0 : 1;
}
