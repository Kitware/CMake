#include "ui_mainwindowsubdir.h"

bool subdircheck()
{
  MocWidget mw;
  Ui::WidgetSubdir mwUi;
  mwUi.setupUi(&mw);
  return mw.objectName() == "WidgetSubdir2";
}
