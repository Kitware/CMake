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

#include <QApplication>

#include "cmSystemTools.h"

#include "CMakeSetupDialog.h"

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  app.setApplicationName("CMakeSetup");
  app.setOrganizationName("Kitware");
  app.setWindowIcon(QIcon(":/Icons/CMakeSetup.png"));

  // TODO handle CMake args
    
  CMakeSetupDialog dialog;
  dialog.show();
  
  return app.exec();
}

