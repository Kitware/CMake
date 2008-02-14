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
#include <QLocale>

#include "CMakeSetupDialog.h"
#include "cmDocumentation.h"
#include "cmake.h"
#include "cmVersion.h"
#include <cmsys/CommandLineArguments.hxx>

//----------------------------------------------------------------------------
static const char * cmDocumentationName[][3] =
{
  {0,
   "  cmake-gui - CMake GUI.", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationUsage[][3] =
{
  {0,
   "  cmake-gui [options]\n"
   "  cmake-gui [options] <path-to-source>\n"
   "  cmake-gui [options] <path-to-existing-build>", 0},
  {0,0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationDescription[][3] =
{
  {0,
   "The \"cmake-gui\" executable is the CMake GUI.  Project "
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

  // tell the cmake library where cmake is 
  QDir cmExecDir(QApplication::applicationDirPath());
#if defined(Q_OS_MAC)
  cmExecDir.cd("../../../");
#endif

  // pick up translation files if they exists in the data directory
  QDir translationsDir = cmExecDir;
  translationsDir.cd(".." CMAKE_DATA_DIR);
  translationsDir.cd("i18n");
  QTranslator translator;
  QString transfile = QString("cmake_%1").arg(QLocale::system().name());
  translator.load(transfile, translationsDir.path());
  app.installTranslator(&translator);
  
  // app setup
  app.setApplicationName("CMakeSetup");
  app.setOrganizationName("Kitware");
  app.setWindowIcon(QIcon(":/Icons/CMakeSetup.png"));
  
  // do docs, if args were given
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
 
  cmsys::CommandLineArguments arg;
  arg.Initialize(argc, argv);
  std::string binaryDirectory;
  std::string sourceDirectory;
  typedef cmsys::CommandLineArguments argT;
  arg.AddArgument("-B", argT::CONCAT_ARGUMENT, 
                  &binaryDirectory, "Binary Directory");
  arg.AddArgument("-H", argT::CONCAT_ARGUMENT,
                  &sourceDirectory, "Source Directory");
  // do not complain about unknown options
  arg.StoreUnusedArguments(true);
  arg.Parse();
  if(!sourceDirectory.empty() && !binaryDirectory.empty())
    {
    dialog.setSourceDirectory(sourceDirectory.c_str());
    dialog.setBinaryDirectory(binaryDirectory.c_str());
    }
  else
    {
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
    }
  
  return app.exec();
}

