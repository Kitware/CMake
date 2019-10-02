/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <iostream>

#include "QCMake.h" // include to disable MS warnings
#include <QApplication>
#include <QDir>
#include <QLocale>
#include <QString>
#include <QTextCodec>
#include <QTranslator>
#include <QtPlugin>

#include "cmsys/CommandLineArguments.hxx"
#include "cmsys/Encoding.hxx"
#include "cmsys/SystemTools.hxx"

#include "CMakeSetupDialog.h"
#include "cmAlgorithms.h"
#include "cmDocumentation.h"
#include "cmDocumentationEntry.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h" // IWYU pragma: keep
#include "cmVersion.h"
#include "cmake.h"

static const char* cmDocumentationName[][2] = { { nullptr,
                                                  "  cmake-gui - CMake GUI." },
                                                { nullptr, nullptr } };

static const char* cmDocumentationUsage[][2] = {
  { nullptr,
    "  cmake-gui [options]\n"
    "  cmake-gui [options] <path-to-source>\n"
    "  cmake-gui [options] <path-to-existing-build>\n"
    "  cmake-gui [options] -S <path-to-source> -B <path-to-build>\n" },
  { nullptr, nullptr }
};

static const char* cmDocumentationOptions[][2] = { { nullptr, nullptr } };

#if defined(Q_OS_MAC)
static int cmOSXInstall(std::string dir);
static void cmAddPluginPath();
#endif

#if defined(USE_QXcbIntegrationPlugin)
Q_IMPORT_PLUGIN(QXcbIntegrationPlugin);
#endif

#if defined(USE_QWindowsIntegrationPlugin)
Q_IMPORT_PLUGIN(QWindowsIntegrationPlugin);
#  if QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)
Q_IMPORT_PLUGIN(QWindowsVistaStylePlugin);
#  endif
#endif

int main(int argc, char** argv)
{
  cmSystemTools::EnsureStdPipes();
  cmsys::Encoding::CommandLineArguments encoding_args =
    cmsys::Encoding::CommandLineArguments::Main(argc, argv);
  int argc2 = encoding_args.argc();
  char const* const* argv2 = encoding_args.argv();

  cmSystemTools::InitializeLibUV();
  cmSystemTools::FindCMakeResources(argv2[0]);
  // check docs first so that X is not need to get docs
  // do docs, if args were given
  cmDocumentation doc;
  doc.addCMakeStandardDocSections();
  if (argc2 > 1 && doc.CheckOptions(argc2, argv2)) {
    // Construct and print requested documentation.
    cmake hcm(cmake::RoleInternal, cmState::Unknown);
    hcm.SetHomeDirectory("");
    hcm.SetHomeOutputDirectory("");
    hcm.AddCMakePaths();

    auto generators = hcm.GetGeneratorsDocumentation();
    doc.SetName("cmake");
    doc.SetSection("Name", cmDocumentationName);
    doc.SetSection("Usage", cmDocumentationUsage);
    doc.AppendSection("Generators", generators);
    doc.PrependSection("Options", cmDocumentationOptions);

    return (doc.PrintRequestedDocumentation(std::cout) ? 0 : 1);
  }

#if defined(Q_OS_MAC)
  if (argc2 == 2 && strcmp(argv2[1], "--install") == 0) {
    return cmOSXInstall("/usr/local/bin");
  }
  if (argc2 == 2 && cmHasLiteralPrefix(argv2[1], "--install=")) {
    return cmOSXInstall(argv2[1] + 10);
  }
#endif

// When we are on OSX and we are launching cmake-gui from a symlink, the
// application will fail to launch as it can't find the qt.conf file which
// tells it what the name of the plugin folder is. We need to add this path
// BEFORE the application is constructed as that is what triggers the
// searching for the platform plugins
#if defined(Q_OS_MAC)
  cmAddPluginPath();
#endif

#if QT_VERSION >= 0x050600
  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  QApplication app(argc, argv);

  setlocale(LC_NUMERIC, "C");

  QTextCodec* utf8_codec = QTextCodec::codecForName("UTF-8");
  QTextCodec::setCodecForLocale(utf8_codec);

#if QT_VERSION < 0x050000
  // clean out standard Qt paths for plugins, which we don't use anyway
  // when creating Mac bundles, it potentially causes problems
  foreach (QString p, QApplication::libraryPaths()) {
    QApplication::removeLibraryPath(p);
  }
#endif

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
  QApplication::installTranslator(&translator);

  // app setup
  QApplication::setApplicationName("CMakeSetup");
  QApplication::setOrganizationName("Kitware");
  QIcon appIcon;
  appIcon.addFile(":/Icons/CMakeSetup32.png");
  appIcon.addFile(":/Icons/CMakeSetup128.png");
  QApplication::setWindowIcon(appIcon);

  CMakeSetupDialog dialog;
  dialog.show();

  QStringList args = QApplication::arguments();
  std::string binaryDirectory;
  std::string sourceDirectory;
  for (int i = 1; i < args.size(); ++i) {
    const QString& arg = args[i];
    if (arg.startsWith("-S")) {
      QString path = arg.mid(2);
      if (path.isEmpty()) {
        ++i;
        if (i >= args.size()) {
          std::cerr << "No source directory specified for -S" << std::endl;
          return 1;
        }
        path = args[i];
        if (path[0] == '-') {
          std::cerr << "No source directory specified for -S" << std::endl;
          return 1;
        }
      }

      sourceDirectory =
        cmSystemTools::CollapseFullPath(path.toLocal8Bit().data());
      cmSystemTools::ConvertToUnixSlashes(sourceDirectory);
    } else if (arg.startsWith("-B")) {
      QString path = arg.mid(2);
      if (path.isEmpty()) {
        ++i;
        if (i >= args.size()) {
          std::cerr << "No build directory specified for -B" << std::endl;
          return 1;
        }
        path = args[i];
        if (path[0] == '-') {
          std::cerr << "No build directory specified for -B" << std::endl;
          return 1;
        }
      }

      binaryDirectory =
        cmSystemTools::CollapseFullPath(path.toLocal8Bit().data());
      cmSystemTools::ConvertToUnixSlashes(binaryDirectory);
    }
  }
  if (!sourceDirectory.empty() && !binaryDirectory.empty()) {
    dialog.setSourceDirectory(QString::fromLocal8Bit(sourceDirectory.c_str()));
    dialog.setBinaryDirectory(QString::fromLocal8Bit(binaryDirectory.c_str()));
  } else {
    if (args.count() == 2) {
      std::string filePath =
        cmSystemTools::CollapseFullPath(args[1].toLocal8Bit().data());

      // check if argument is a directory containing CMakeCache.txt
      std::string buildFilePath =
        cmSystemTools::CollapseFullPath("CMakeCache.txt", filePath.c_str());

      // check if argument is a CMakeCache.txt file
      if (cmSystemTools::GetFilenameName(filePath) == "CMakeCache.txt" &&
          cmSystemTools::FileExists(filePath.c_str())) {
        buildFilePath = filePath;
      }

      // check if argument is a directory containing CMakeLists.txt
      std::string srcFilePath =
        cmSystemTools::CollapseFullPath("CMakeLists.txt", filePath.c_str());

      if (cmSystemTools::FileExists(buildFilePath.c_str())) {
        dialog.setBinaryDirectory(QString::fromLocal8Bit(
          cmSystemTools::GetFilenamePath(buildFilePath).c_str()));
      } else if (cmSystemTools::FileExists(srcFilePath.c_str())) {
        dialog.setSourceDirectory(QString::fromLocal8Bit(filePath.c_str()));
        dialog.setBinaryDirectory(QString::fromLocal8Bit(
          cmSystemTools::CollapseFullPath(".").c_str()));
      }
    }
  }

  return QApplication::exec();
}

#if defined(Q_OS_MAC)
#  include <cerrno>
#  include <cstring>

#  include <unistd.h>

#  include "cm_sys_stat.h"
static bool cmOSXInstall(std::string const& dir, std::string const& tool)
{
  if (tool.empty()) {
    return true;
  }
  std::string link = dir + cmSystemTools::GetFilenameName(tool);
  struct stat st;
  if (lstat(link.c_str(), &st) == 0 && S_ISLNK(st.st_mode)) {
    char buf[4096];
    ssize_t s = readlink(link.c_str(), buf, sizeof(buf) - 1);
    if (s >= 0 && std::string(buf, s) == tool) {
      std::cerr << "Exists: '" << link << "' -> '" << tool << "'\n";
      return true;
    }
  }
  cmSystemTools::MakeDirectory(dir);
  if (symlink(tool.c_str(), link.c_str()) == 0) {
    std::cerr << "Linked: '" << link << "' -> '" << tool << "'\n";
    return true;
  }
  int err = errno;
  std::cerr << "Failed: '" << link << "' -> '" << tool
            << "': " << strerror(err) << "\n";
  return false;
}
static int cmOSXInstall(std::string dir)
{
  if (!cmHasLiteralSuffix(dir, "/")) {
    dir += "/";
  }
  return (cmOSXInstall(dir, cmSystemTools::GetCMakeCommand()) &&
          cmOSXInstall(dir, cmSystemTools::GetCTestCommand()) &&
          cmOSXInstall(dir, cmSystemTools::GetCPackCommand()) &&
          cmOSXInstall(dir, cmSystemTools::GetCMakeGUICommand()) &&
          cmOSXInstall(dir, cmSystemTools::GetCMakeCursesCommand()))
    ? 0
    : 1;
}

// Locate the PlugIns directory and add it to the QApplication library paths.
// We need to resolve all symlinks so we have a known relative path between
// MacOS/CMake and the PlugIns directory.
//
// Note we are using cmSystemTools since Qt can't provide the path to the
// executable before the QApplication is created, and that is when plugin
// searching occurs.
static void cmAddPluginPath()
{
  std::string const& path = cmSystemTools::GetCMakeGUICommand();
  if (path.empty()) {
    return;
  }
  std::string const& realPath = cmSystemTools::GetRealPath(path);
  QFileInfo appPath(QString::fromLocal8Bit(realPath.c_str()));
  QDir pluginDir = appPath.dir();
  bool const foundPluginDir = pluginDir.cd("../PlugIns");
  if (foundPluginDir) {
    QApplication::addLibraryPath(pluginDir.path());
  }
}

#endif
