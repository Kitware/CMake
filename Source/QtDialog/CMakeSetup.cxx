
#include <QApplication>

#include "cmSystemTools.h"

#include "CMakeSetupDialog.h"

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  app.setApplicationName("CMakeSetup");
  app.setOrganizationName("Kitware");

  // TODO handle CMake args
    
  CMakeSetupDialog dialog;
  dialog.show();
  
  return app.exec();
}

