/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "QCMake.h"  // include to disable MS warnings
#include <QApplication>
#include <QFileInfo>
#include <QDir>

#include "CMakeSetupDialog.h"

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  app.setApplicationName("CMakeSetup");
  app.setOrganizationName("Kitware");
  app.setWindowIcon(QIcon(":/Icons/CMakeSetup.png"));

  CMakeSetupDialog dialog;
  dialog.setWindowTitle("CMakeSetup");
  dialog.show();
 
  // for now: args support specifying build and/or source directory 
  QStringList args = app.arguments();
  if(args.count() == 2)
    {
    QFileInfo buildFileInfo(args[1], "CMakeCache.txt");
    QFileInfo srcFileInfo(args[1], "CMakeLists.txt");
    if(buildFileInfo.exists())
      {
      dialog.setBinaryDirectory(buildFileInfo.absolutePath());
      }
    else if(srcFileInfo.exists())
      {
      dialog.setSourceDirectory(srcFileInfo.absolutePath());
      dialog.setBinaryDirectory(QDir::currentPath());
      }
    }
  
  return app.exec();
}

