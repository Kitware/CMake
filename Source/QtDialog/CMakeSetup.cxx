/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include <iostream>

#include "QCMake.h" // include to disable MS warnings
#include <QApplication>
#include <QDir>
#include <QLocale>
#include <QString>
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
#include "cmake.h"

static const char* cmDocumentationName[][2] = { { nullptr,
                                                  "  cmake-gui - CMake GUI." },
                                                { nullptr, nullptr } };

static const char* cmDocumentationUsage[][2] = {
  { nullptr,
    "  cmake-gui [options]\n"
    "  cmake-gui [options] <path-to-source>\n"
    "  cmake-gui [options] <path-to-existing-build>\n"
    "  cmake-gui [options] -S <path-to-source> -B <path-to-build>\n"
    "  cmake-gui [options] --browse-manual\n" },
  { nullptr, nullptr }
};

static const char* cmDocumentationOptions[][2] = {
  { "-S <path-to-source>", "Explicitly specify a source directory." },
  { "-B <path-to-build>", "Explicitly specify a build directory." },
  { "--preset=<preset>", "Specify a configure preset." },
  { nullptr, nullptr }
};

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

int CMakeGUIExec(CMakeSetupDialog* window);
void SetupDefaultQSettings();
void OpenReferenceManual();

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

// HighDpiScaling is always enabled starting with Qt6, but will still issue a
// deprecation warning if you try to set the attribute for it
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0) &&                               \
     QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
  QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif

  SetupDefaultQSettings();
  QApplication app(argc, argv);

  setlocale(LC_NUMERIC, "C");

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
  if (translator.load(QLocale(), "cmake", "_", translationsDir.path())) {
    QApplication::installTranslator(&translator);
  }

  // app setup
  QApplication::setApplicationName("CMakeSetup");
  QApplication::setOrganizationName("Kitware");
  QIcon appIcon;
  appIcon.addFile(":/Icons/CMakeSetup32.png");
  appIcon.addFile(":/Icons/CMakeSetup128.png");
  QApplication::setWindowIcon(QIcon::fromTheme("cmake-gui", appIcon));

  CMakeSetupDialog dialog;
  dialog.show();

  QStringList args = QApplication::arguments();
  std::string binaryDirectory;
  std::string sourceDirectory;
  std::string presetName;
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

      sourceDirectory = cmSystemTools::CollapseFullPath(path.toStdString());
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

      binaryDirectory = cmSystemTools::CollapseFullPath(path.toStdString());
      cmSystemTools::ConvertToUnixSlashes(binaryDirectory);
    } else if (arg.startsWith("--preset=")) {
      QString preset = arg.mid(cmStrLen("--preset="));
      if (preset.isEmpty()) {
        std::cerr << "No preset specified for --preset" << std::endl;
        return 1;
      }
      presetName = preset.toStdString();
    } else if (arg == "--browse-manual") {
      OpenReferenceManual();
      return 0;
    }
  }
  if (!sourceDirectory.empty() &&
      (!binaryDirectory.empty() || !presetName.empty())) {
    dialog.setSourceDirectory(QString::fromStdString(sourceDirectory));
    if (!binaryDirectory.empty()) {
      dialog.setBinaryDirectory(QString::fromStdString(binaryDirectory));
      if (!presetName.empty()) {
        dialog.setStartupBinaryDirectory(true);
      }
    }
    if (!presetName.empty()) {
      dialog.setDeferredPreset(QString::fromStdString(presetName));
    }
  } else {
    if (args.count() == 2) {
      std::string filePath =
        cmSystemTools::CollapseFullPath(args[1].toStdString());

      // check if argument is a directory containing CMakeCache.txt
      std::string buildFilePath = cmStrCat(filePath, "/CMakeCache.txt");

      // check if argument is a CMakeCache.txt file
      if (cmSystemTools::GetFilenameName(filePath) == "CMakeCache.txt" &&
          cmSystemTools::FileExists(filePath.c_str())) {
        buildFilePath = filePath;
      }

      // check if argument is a directory containing CMakeLists.txt
      std::string srcFilePath = cmStrCat(filePath, "/CMakeLists.txt");

      if (cmSystemTools::FileExists(buildFilePath.c_str())) {
        dialog.setBinaryDirectory(QString::fromStdString(
          cmSystemTools::GetFilenamePath(buildFilePath)));
      } else if (cmSystemTools::FileExists(srcFilePath.c_str())) {
        dialog.setSourceDirectory(QString::fromStdString(filePath));
        dialog.setBinaryDirectory(
          QString::fromStdString(cmSystemTools::CollapseFullPath(".")));
      }
    }
  }

  return CMakeGUIExec(&dialog);
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
