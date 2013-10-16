/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "QCMake.h"  // include to disable MS warnings
#include <QApplication>
#include <QDir>
#include <QTranslator>
#include <QLocale>
#include "QMacInstallDialog.h"
#include "CMakeSetupDialog.h"
#include "cmDocumentation.h"
#include "cmake.h"
#include "cmVersion.h"
#include <cmsys/CommandLineArguments.hxx>
#include <cmsys/SystemTools.hxx>

//----------------------------------------------------------------------------
static const char * cmDocumentationName[][2] =
{
  {0,
   "  cmake-gui - CMake GUI."},
  {0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationUsage[][2] =
{
  {0,
   "  cmake-gui [options]\n"
   "  cmake-gui [options] <path-to-source>\n"
   "  cmake-gui [options] <path-to-existing-build>"},
  {0,0}
};

//----------------------------------------------------------------------------
static const char * cmDocumentationOptions[][2] =
{
  {0,0}
};

int main(int argc, char** argv)
{
  cmSystemTools::FindExecutableDirectory(argv[0]);
  // check docs first so that X is not need to get docs
  // do docs, if args were given
  cmDocumentation doc;
  doc.addCMakeStandardDocSections();
  if(argc >1 && doc.CheckOptions(argc, argv))
    {
    // Construct and print requested documentation.
    cmake hcm;
    hcm.AddCMakePaths();
    // just incase the install is bad avoid a seg fault
    const char* root = hcm.GetCacheDefinition("CMAKE_ROOT");
    if(root)
      {
      doc.SetCMakeRoot(root);
      }

    std::vector<cmDocumentationEntry> generators;
    hcm.GetGeneratorDocumentation(generators);
    doc.SetName("cmake");
    doc.SetSection("Name",cmDocumentationName);
    doc.SetSection("Usage",cmDocumentationUsage);
    doc.AppendSection("Generators",generators);
    doc.PrependSection("Options",cmDocumentationOptions);

    return (doc.PrintRequestedDocumentation(std::cout)? 0:1);
    }

  QApplication app(argc, argv);

  // clean out standard Qt paths for plugins, which we don't use anyway
  // when creating Mac bundles, it potentially causes problems
  foreach(QString p, QApplication::libraryPaths())
    {
    QApplication::removeLibraryPath(p);
    }

  // if arg for install
  for(int i =0; i < argc; i++)
    {
    if(strcmp(argv[i], "--mac-install") == 0)
      {
      QMacInstallDialog setupdialog(0);
      setupdialog.exec();
      return 0;
      }
    }
  // tell the cmake library where cmake is
  QDir cmExecDir(QApplication::applicationDirPath());
#if defined(Q_OS_MAC)
  cmExecDir.cd("../../../");
#endif

  // pick up translation files if they exists in the data directory
  QDir translationsDir = cmExecDir;
  translationsDir.cd(QString::fromLocal8Bit(".." CMAKE_DATA_DIR));
  translationsDir.cd("i18n");
  QTranslator translator;
  QString transfile = QString("cmake_%1").arg(QLocale::system().name());
  translator.load(transfile, translationsDir.path());
  app.installTranslator(&translator);

  // app setup
  app.setApplicationName("CMakeSetup");
  app.setOrganizationName("Kitware");
  QIcon appIcon;
  appIcon.addFile(":/Icons/CMakeSetup32.png");
  appIcon.addFile(":/Icons/CMakeSetup128.png");
  app.setWindowIcon(appIcon);

  CMakeSetupDialog dialog;
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
    dialog.setSourceDirectory(QString::fromLocal8Bit(sourceDirectory.c_str()));
    dialog.setBinaryDirectory(QString::fromLocal8Bit(binaryDirectory.c_str()));
    }
  else
    {
    QStringList args = app.arguments();
    if(args.count() == 2)
      {
      cmsys_stl::string filePath = cmSystemTools::CollapseFullPath(args[1].toLocal8Bit().data());

      // check if argument is a directory containing CMakeCache.txt
      cmsys_stl::string buildFilePath =
        cmSystemTools::CollapseFullPath("CMakeCache.txt", filePath.c_str());

      // check if argument is a CMakeCache.txt file
      if(cmSystemTools::GetFilenameName(filePath) == "CMakeCache.txt" &&
         cmSystemTools::FileExists(filePath.c_str()))
        {
        buildFilePath = filePath;
        }

      // check if argument is a directory containing CMakeLists.txt
      cmsys_stl::string srcFilePath =
        cmSystemTools::CollapseFullPath("CMakeLists.txt", filePath.c_str());

      if(cmSystemTools::FileExists(buildFilePath.c_str()))
        {
        dialog.setBinaryDirectory(
          QString::fromLocal8Bit(
            cmSystemTools::GetFilenamePath(buildFilePath).c_str()
            )
          );
        }
      else if(cmSystemTools::FileExists(srcFilePath.c_str()))
        {
        dialog.setSourceDirectory(QString::fromLocal8Bit(filePath.c_str()));
        dialog.setBinaryDirectory(
          QString::fromLocal8Bit(cmSystemTools::CollapseFullPath(".").c_str())
          );
        }
      }
    }

  return app.exec();
}

