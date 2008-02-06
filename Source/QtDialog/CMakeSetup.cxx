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
#include <QTranslator>

#include "CMakeSetupDialog.h"
#include "cmDocumentation.h"
#include "cmSystemTools.h"
#include "cmake.h"
#include "cmVersion.h"

//----------------------------------------------------------------------------
static const char * cmDocumentationName[][3] =
{
  {0,
   "  CMakeSetup - CMake GUI.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationUsage[][3] =
{
  {0,
   "  CMakeSetup [options]\n"
   "  CMakeSetup [options] <path-to-source>\n"
   "  CMakeSetup [options] <path-to-existing-build>", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationDescription[][3] =
{
  {0,
   "The \"CMakeSetup\" executable is the CMake GUI.  Project "
   "configuration settings may be specified interactively.  "
   "Brief instructions are provided at the bottom of the "
   "window when the program is running.", 0},
  CMAKE_STANDARD_INTRODUCTION,
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationOptions[][3] =
{
  {0,0,0}
};

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
 
  QDir cmExecDir(QApplication::applicationDirPath());
#if defined(Q_OS_MAC)
  cmExecDir.cd("../../../");
#endif
  cmSystemTools::FindExecutableDirectory(cmExecDir.filePath("cmake").toAscii().data());

  QTranslator translator;
  QString transfile = QString("cmake_%1").arg(QLocale::system().name());
  translator.load(transfile, app.applicationDirPath());
  app.installTranslator(&translator);
  
  app.setApplicationName("CMakeSetup");
  app.setOrganizationName("Kitware");
  app.setWindowIcon(QIcon(":/Icons/CMakeSetup.png"));
  
  cmDocumentation doc;
  if(app.arguments().size() > 1 &&
     doc.CheckOptions(app.argc(), app.argv()))
    {
    // Construct and print requested documentation.
    cmake hcm;
    hcm.AddCMakePaths();
    doc.SetCMakeRoot(hcm.GetCacheDefinition("CMAKE_ROOT"));
    std::vector<cmDocumentationEntry> commands;
    std::vector<cmDocumentationEntry> compatCommands;
    std::map<std::string,cmDocumentationSection *> propDocs;

    std::vector<cmDocumentationEntry> generators;
    hcm.GetCommandDocumentation(commands, true, false);
    hcm.GetCommandDocumentation(compatCommands, false, true);
    hcm.GetGeneratorDocumentation(generators);
    hcm.GetPropertiesDocumentation(propDocs);
    doc.SetName("cmake");
    doc.SetSection("Name",cmDocumentationName);
    doc.SetSection("Usage",cmDocumentationUsage);
    doc.SetSection("Description",cmDocumentationDescription);
    doc.AppendSection("Generators",generators);
    doc.PrependSection("Options",cmDocumentationOptions);
    doc.SetSection("Commands",commands);
    doc.SetSection("Compatilbility Commands", compatCommands);
    doc.SetSections(propDocs);

    return (doc.PrintRequestedDocumentation(std::cout)? 0:1);
    }

  CMakeSetupDialog dialog;
  QString title = QString("CMake %1");
  title = title.arg(cmVersion::GetCMakeVersion().c_str());
  dialog.setWindowTitle(title);
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

