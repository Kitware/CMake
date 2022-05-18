/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "cmConfigure.h" // IWYU pragma: keep

#include "cmCMakePresetsGraph.h"
#include "cmake.h"

#ifdef _MSC_VER
#  pragma warning(disable : 4127)
#  pragma warning(disable : 4512)
#endif

#include <memory>
#include <vector>

#include "QCMakePreset.h"
#include <QAtomicInt>
#include <QList>
#include <QMetaType>
#include <QObject>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QVariant>

/// struct to represent cmake properties in Qt
/// Value is of type String or Bool
struct QCMakeProperty
{
  enum PropertyType
  {
    BOOL,
    PATH,
    FILEPATH,
    STRING
  };
  QString Key;
  QVariant Value;
  QStringList Strings;
  QString Help;
  PropertyType Type;
  bool Advanced;
  bool operator==(const QCMakeProperty& other) const
  {
    return this->Key == other.Key;
  }
  bool operator<(const QCMakeProperty& other) const
  {
    return this->Key < other.Key;
  }
};

// list of properties
using QCMakePropertyList = QList<QCMakeProperty>;

// allow QVariant to be a property or list of properties
Q_DECLARE_METATYPE(QCMakeProperty)
Q_DECLARE_METATYPE(QCMakePropertyList)
Q_DECLARE_METATYPE(QProcessEnvironment)
Q_DECLARE_METATYPE(cmCMakePresetsGraph::ReadFileResult)

/// Qt API for CMake library.
/// Wrapper like class allows for easier integration with
/// Qt features such as, signal/slot connections, multi-threading, etc..
class QCMake : public QObject
{
  Q_OBJECT
public:
  QCMake(QObject* p = nullptr);
  ~QCMake();
public slots:
  /// load the cache file in a directory
  void loadCache(const QString& dir);
  /// set the source directory containing the source
  void setSourceDirectory(const QString& dir);
  /// set the binary directory to build in
  void setBinaryDirectory(const QString& dir);
  /// set the preset name to use
  void setPreset(const QString& name, bool setBinary = true);
  /// set the desired generator to use
  void setGenerator(const QString& generator);
  /// set the desired generator to use
  void setPlatform(const QString& platform);
  /// set the desired generator to use
  void setToolset(const QString& toolset);
  /// set the configure and generate environment
  void setEnvironment(const QProcessEnvironment& environment);
  /// do the configure step
  void configure();
  /// generate the files
  void generate();
  /// open the project
  void open();
  /// set the property values
  void setProperties(const QCMakePropertyList&);
  /// interrupt the configure or generate process (if connecting, make a direct
  /// connection)
  void interrupt();
  /// delete the cache in binary directory
  void deleteCache();
  /// reload the cache in binary directory
  void reloadCache();
  /// set whether to do debug output
  void setDebugOutput(bool);
  /// get whether to do suppress dev warnings
  bool getSuppressDevWarnings();
  /// set whether to do suppress dev warnings
  void setSuppressDevWarnings(bool value);
  /// get whether to do suppress deprecated warnings
  bool getSuppressDeprecatedWarnings();
  /// set whether to do suppress deprecated warnings
  void setSuppressDeprecatedWarnings(bool value);
  /// get whether to treat developer (author) warnings as errors
  bool getDevWarningsAsErrors();
  /// set whether to treat developer (author) warnings as errors
  void setDevWarningsAsErrors(bool value);
  /// get whether to treat deprecated warnings as errors
  bool getDeprecatedWarningsAsErrors();
  /// set whether to treat deprecated warnings as errors
  void setDeprecatedWarningsAsErrors(bool value);
  /// set whether to run cmake with warnings about uninitialized variables
  void setWarnUninitializedMode(bool value);
  /// check if project IDE open is possible and emit openPossible signal
  void checkOpenPossible();

public:
  /// get the list of cache properties
  QCMakePropertyList properties() const;
  /// get the current binary directory
  QString binaryDirectory() const;
  /// get the current source directory
  QString sourceDirectory() const;
  /// get the current generator
  QString generator() const;
  /// get the configure and generate environment
  QProcessEnvironment environment() const;
  /// get the available generators
  std::vector<cmake::GeneratorInfo> const& availableGenerators() const;
  /// get whether to do debug output
  bool getDebugOutput() const;

signals:
  /// signal when properties change (during read from disk or configure
  /// process)
  void propertiesChanged(const QCMakePropertyList& vars);
  /// signal when the generator changes
  void generatorChanged(const QString& gen);
  /// signal when the source directory changes (binary directory already
  /// containing a CMakeCache.txt file)
  void sourceDirChanged(const QString& dir);
  /// signal when the binary directory changes
  void binaryDirChanged(const QString& dir);
  /// signal when the preset list changes
  void presetsChanged(const QVector<QCMakePreset>& presets);
  /// signal when the selected preset changes
  void presetChanged(const QString& name);
  /// signal when there's an error reading the presets files
  void presetLoadError(const QString& dir,
                       cmCMakePresetsGraph::ReadFileResult error);
  /// signal when uninitialized warning changes
  void warnUninitializedModeChanged(bool value);
  /// signal for progress events
  void progressChanged(const QString& msg, float percent);
  /// signal when configure is done
  void configureDone(int error);
  /// signal when generate is done
  void generateDone(int error);
  /// signal when there is an output message
  void outputMessage(const QString& msg);
  /// signal when there is an error message
  void errorMessage(const QString& msg);
  /// signal when debug output changes
  void debugOutputChanged(bool);
  /// signal when the toolset changes
  void toolsetChanged(const QString& toolset);
  /// signal when the platform changes
  void platformChanged(const QString& platform);
  /// signal when open is done
  void openDone(bool successful);
  /// signal when open is done
  void openPossible(bool possible);

protected:
  std::unique_ptr<cmake> CMakeInstance;

  bool interruptCallback();
  void progressCallback(std::string const& msg, float percent);
  void messageCallback(std::string const& msg, const char* title);
  void stdoutCallback(std::string const& msg);
  void stderrCallback(std::string const& msg);
  void setUpEnvironment() const;

  void loadPresets();

  bool WarnUninitializedMode;
  QString SourceDirectory;
  QString BinaryDirectory;
  QString Generator;
  QString Platform;
  QString Toolset;
  std::vector<cmake::GeneratorInfo> AvailableGenerators;
  cmCMakePresetsGraph CMakePresetsGraph;
  cmCMakePresetsGraph::ReadFileResult LastLoadPresetsResult =
    cmCMakePresetsGraph::ReadFileResult::READ_OK;
  QString PresetName;
  QString CMakeExecutable;
  QAtomicInt InterruptFlag;
  QProcessEnvironment StartEnvironment;
  QProcessEnvironment Environment;
  QTimer LoadPresetsTimer;
};
