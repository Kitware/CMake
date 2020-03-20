/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "QCMake.h"

#include <QCoreApplication>
#include <QDir>

#include "cmExternalMakefileProjectGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#ifdef Q_OS_WIN
#  include "qt_windows.h" // For SetErrorMode
#endif

QCMake::QCMake(QObject* p)
  : QObject(p)
{
  this->WarnUninitializedMode = false;
  this->WarnUnusedMode = false;
  qRegisterMetaType<QCMakeProperty>();
  qRegisterMetaType<QCMakePropertyList>();

  cmSystemTools::DisableRunCommandOutput();
  cmSystemTools::SetRunCommandHideConsole(true);

  cmSystemTools::SetMessageCallback(
    [this](std::string const& msg, const char* title) {
      this->messageCallback(msg, title);
    });
  cmSystemTools::SetStdoutCallback(
    [this](std::string const& msg) { this->stdoutCallback(msg); });
  cmSystemTools::SetStderrCallback(
    [this](std::string const& msg) { this->stderrCallback(msg); });

  this->CMakeInstance = new cmake(cmake::RoleProject, cmState::Project);
  this->CMakeInstance->SetCMakeEditCommand(
    cmSystemTools::GetCMakeGUICommand());
  this->CMakeInstance->SetProgressCallback(
    [this](const std::string& msg, float percent) {
      this->progressCallback(msg, percent);
    });

  cmSystemTools::SetInterruptCallback(
    [this] { return this->interruptCallback(); });

  std::vector<cmake::GeneratorInfo> generators;
  this->CMakeInstance->GetRegisteredGenerators(
    generators, /*includeNamesWithPlatform=*/false);

  for (cmake::GeneratorInfo const& gen : generators) {
    this->AvailableGenerators.push_back(gen);
  }
}

QCMake::~QCMake()
{
  delete this->CMakeInstance;
  // cmDynamicLoader::FlushCache();
}

void QCMake::loadCache(const QString& dir)
{
  this->setBinaryDirectory(dir);
}

void QCMake::setSourceDirectory(const QString& _dir)
{
  QString dir = QString::fromLocal8Bit(
    cmSystemTools::GetActualCaseForPath(_dir.toLocal8Bit().data()).c_str());
  if (this->SourceDirectory != dir) {
    this->SourceDirectory = QDir::fromNativeSeparators(dir);
    emit this->sourceDirChanged(this->SourceDirectory);
  }
}

void QCMake::setBinaryDirectory(const QString& _dir)
{
  QString dir = QString::fromLocal8Bit(
    cmSystemTools::GetActualCaseForPath(_dir.toLocal8Bit().data()).c_str());
  if (this->BinaryDirectory != dir) {
    this->BinaryDirectory = QDir::fromNativeSeparators(dir);
    emit this->binaryDirChanged(this->BinaryDirectory);
    cmState* state = this->CMakeInstance->GetState();
    this->setGenerator(QString());
    this->setToolset(QString());
    this->setPlatform(QString());
    if (!this->CMakeInstance->LoadCache(
          this->BinaryDirectory.toLocal8Bit().data())) {
      QDir testDir(this->BinaryDirectory);
      if (testDir.exists("CMakeCache.txt")) {
        cmSystemTools::Error(
          "There is a CMakeCache.txt file for the current binary "
          "tree but cmake does not have permission to read it.  "
          "Please check the permissions of the directory you are trying to "
          "run CMake on.");
      }
    }

    QCMakePropertyList props = this->properties();
    emit this->propertiesChanged(props);
    const char* homeDir = state->GetCacheEntryValue("CMAKE_HOME_DIRECTORY");
    if (homeDir) {
      setSourceDirectory(QString::fromLocal8Bit(homeDir));
    }
    const char* gen = state->GetCacheEntryValue("CMAKE_GENERATOR");
    if (gen) {
      const std::string* extraGen =
        state->GetInitializedCacheValue("CMAKE_EXTRA_GENERATOR");
      std::string curGen =
        cmExternalMakefileProjectGenerator::CreateFullGeneratorName(
          gen, extraGen ? *extraGen : "");
      this->setGenerator(QString::fromLocal8Bit(curGen.c_str()));
    }

    const char* platform =
      state->GetCacheEntryValue("CMAKE_GENERATOR_PLATFORM");
    if (platform) {
      this->setPlatform(QString::fromLocal8Bit(platform));
    }

    const char* toolset = state->GetCacheEntryValue("CMAKE_GENERATOR_TOOLSET");
    if (toolset) {
      this->setToolset(QString::fromLocal8Bit(toolset));
    }

    checkOpenPossible();
  }
}

void QCMake::setGenerator(const QString& gen)
{
  if (this->Generator != gen) {
    this->Generator = gen;
    emit this->generatorChanged(this->Generator);
  }
}

void QCMake::setPlatform(const QString& platform)
{
  if (this->Platform != platform) {
    this->Platform = platform;
    emit this->platformChanged(this->Platform);
  }
}

void QCMake::setToolset(const QString& toolset)
{
  if (this->Toolset != toolset) {
    this->Toolset = toolset;
    emit this->toolsetChanged(this->Toolset);
  }
}

void QCMake::configure()
{
#ifdef Q_OS_WIN
  UINT lastErrorMode = SetErrorMode(0);
#endif

  this->CMakeInstance->SetHomeDirectory(
    this->SourceDirectory.toLocal8Bit().data());
  this->CMakeInstance->SetHomeOutputDirectory(
    this->BinaryDirectory.toLocal8Bit().data());
  this->CMakeInstance->SetGlobalGenerator(
    this->CMakeInstance->CreateGlobalGenerator(
      this->Generator.toLocal8Bit().data()));
  this->CMakeInstance->SetGeneratorPlatform(
    this->Platform.toLocal8Bit().data());
  this->CMakeInstance->SetGeneratorToolset(this->Toolset.toLocal8Bit().data());
  this->CMakeInstance->LoadCache();
  this->CMakeInstance->SetWarnUninitialized(this->WarnUninitializedMode);
  this->CMakeInstance->SetWarnUnused(this->WarnUnusedMode);
  this->CMakeInstance->PreLoadCMakeFiles();

  InterruptFlag = 0;
  cmSystemTools::ResetErrorOccuredFlag();

  int err = this->CMakeInstance->Configure();

#ifdef Q_OS_WIN
  SetErrorMode(lastErrorMode);
#endif

  emit this->propertiesChanged(this->properties());
  emit this->configureDone(err);
}

void QCMake::generate()
{
#ifdef Q_OS_WIN
  UINT lastErrorMode = SetErrorMode(0);
#endif

  InterruptFlag = 0;
  cmSystemTools::ResetErrorOccuredFlag();

  int err = this->CMakeInstance->Generate();

#ifdef Q_OS_WIN
  SetErrorMode(lastErrorMode);
#endif

  emit this->generateDone(err);
  checkOpenPossible();
}

void QCMake::open()
{
#ifdef Q_OS_WIN
  UINT lastErrorMode = SetErrorMode(0);
#endif

  InterruptFlag = 0;
  cmSystemTools::ResetErrorOccuredFlag();

  auto successful = this->CMakeInstance->Open(
    this->BinaryDirectory.toLocal8Bit().data(), false);

#ifdef Q_OS_WIN
  SetErrorMode(lastErrorMode);
#endif

  emit this->openDone(successful);
}

void QCMake::setProperties(const QCMakePropertyList& newProps)
{
  QCMakePropertyList props = newProps;

  QStringList toremove;

  // set the value of properties
  cmState* state = this->CMakeInstance->GetState();
  std::vector<std::string> cacheKeys = state->GetCacheEntryKeys();
  for (std::string const& key : cacheKeys) {
    cmStateEnums::CacheEntryType t = state->GetCacheEntryType(key);
    if (t == cmStateEnums::INTERNAL || t == cmStateEnums::STATIC) {
      continue;
    }

    QCMakeProperty prop;
    prop.Key = QString::fromLocal8Bit(key.c_str());
    int idx = props.indexOf(prop);
    if (idx == -1) {
      toremove.append(QString::fromLocal8Bit(key.c_str()));
    } else {
      prop = props[idx];
      if (prop.Value.type() == QVariant::Bool) {
        state->SetCacheEntryValue(key, prop.Value.toBool() ? "ON" : "OFF");
      } else {
        state->SetCacheEntryValue(key,
                                  prop.Value.toString().toLocal8Bit().data());
      }
      props.removeAt(idx);
    }
  }

  // remove some properties
  foreach (QString const& s, toremove) {
    this->CMakeInstance->UnwatchUnusedCli(s.toLocal8Bit().data());

    state->RemoveCacheEntry(s.toLocal8Bit().data());
  }

  // add some new properties
  foreach (QCMakeProperty const& s, props) {
    this->CMakeInstance->WatchUnusedCli(s.Key.toLocal8Bit().data());

    if (s.Type == QCMakeProperty::BOOL) {
      this->CMakeInstance->AddCacheEntry(
        s.Key.toLocal8Bit().data(), s.Value.toBool() ? "ON" : "OFF",
        s.Help.toLocal8Bit().data(), cmStateEnums::BOOL);
    } else if (s.Type == QCMakeProperty::STRING) {
      this->CMakeInstance->AddCacheEntry(
        s.Key.toLocal8Bit().data(), s.Value.toString().toLocal8Bit().data(),
        s.Help.toLocal8Bit().data(), cmStateEnums::STRING);
    } else if (s.Type == QCMakeProperty::PATH) {
      this->CMakeInstance->AddCacheEntry(
        s.Key.toLocal8Bit().data(), s.Value.toString().toLocal8Bit().data(),
        s.Help.toLocal8Bit().data(), cmStateEnums::PATH);
    } else if (s.Type == QCMakeProperty::FILEPATH) {
      this->CMakeInstance->AddCacheEntry(
        s.Key.toLocal8Bit().data(), s.Value.toString().toLocal8Bit().data(),
        s.Help.toLocal8Bit().data(), cmStateEnums::FILEPATH);
    }
  }

  this->CMakeInstance->SaveCache(this->BinaryDirectory.toLocal8Bit().data());
}

QCMakePropertyList QCMake::properties() const
{
  QCMakePropertyList ret;

  cmState* state = this->CMakeInstance->GetState();
  std::vector<std::string> cacheKeys = state->GetCacheEntryKeys();
  for (std::string const& key : cacheKeys) {
    cmStateEnums::CacheEntryType t = state->GetCacheEntryType(key);
    if (t == cmStateEnums::INTERNAL || t == cmStateEnums::STATIC ||
        t == cmStateEnums::UNINITIALIZED) {
      continue;
    }

    const char* cachedValue = state->GetCacheEntryValue(key);

    QCMakeProperty prop;
    prop.Key = QString::fromLocal8Bit(key.c_str());
    prop.Help =
      QString::fromLocal8Bit(state->GetCacheEntryProperty(key, "HELPSTRING"));
    prop.Value = QString::fromLocal8Bit(cachedValue);
    prop.Advanced = state->GetCacheEntryPropertyAsBool(key, "ADVANCED");
    if (t == cmStateEnums::BOOL) {
      prop.Type = QCMakeProperty::BOOL;
      prop.Value = cmIsOn(cachedValue);
    } else if (t == cmStateEnums::PATH) {
      prop.Type = QCMakeProperty::PATH;
    } else if (t == cmStateEnums::FILEPATH) {
      prop.Type = QCMakeProperty::FILEPATH;
    } else if (t == cmStateEnums::STRING) {
      prop.Type = QCMakeProperty::STRING;
      const char* stringsProperty =
        state->GetCacheEntryProperty(key, "STRINGS");
      if (stringsProperty) {
        prop.Strings = QString::fromLocal8Bit(stringsProperty).split(";");
      }
    }

    ret.append(prop);
  }

  return ret;
}

void QCMake::interrupt()
{
  this->InterruptFlag.ref();
}

bool QCMake::interruptCallback()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
  return this->InterruptFlag;
#else
  return this->InterruptFlag.load();
#endif
}

void QCMake::progressCallback(const std::string& msg, float percent)
{
  if (percent >= 0) {
    emit this->progressChanged(QString::fromStdString(msg), percent);
  } else {
    emit this->outputMessage(QString::fromStdString(msg));
  }
  QCoreApplication::processEvents();
}

void QCMake::messageCallback(std::string const& msg, const char* /*title*/)
{
  emit this->errorMessage(QString::fromStdString(msg));
  QCoreApplication::processEvents();
}

void QCMake::stdoutCallback(std::string const& msg)
{
  emit this->outputMessage(QString::fromStdString(msg));
  QCoreApplication::processEvents();
}

void QCMake::stderrCallback(std::string const& msg)
{
  emit this->outputMessage(QString::fromStdString(msg));
  QCoreApplication::processEvents();
}

QString QCMake::binaryDirectory() const
{
  return this->BinaryDirectory;
}

QString QCMake::sourceDirectory() const
{
  return this->SourceDirectory;
}

QString QCMake::generator() const
{
  return this->Generator;
}

std::vector<cmake::GeneratorInfo> const& QCMake::availableGenerators() const
{
  return AvailableGenerators;
}

void QCMake::deleteCache()
{
  // delete cache
  this->CMakeInstance->DeleteCache(this->BinaryDirectory.toLocal8Bit().data());
  // reload to make our cache empty
  this->CMakeInstance->LoadCache(this->BinaryDirectory.toLocal8Bit().data());
  // emit no generator and no properties
  this->setGenerator(QString());
  this->setToolset(QString());
  QCMakePropertyList props = this->properties();
  emit this->propertiesChanged(props);
}

void QCMake::reloadCache()
{
  // emit that the cache was cleaned out
  QCMakePropertyList props;
  emit this->propertiesChanged(props);
  // reload
  this->CMakeInstance->LoadCache(this->BinaryDirectory.toLocal8Bit().data());
  // emit new cache properties
  props = this->properties();
  emit this->propertiesChanged(props);
}

void QCMake::setDebugOutput(bool flag)
{
  if (flag != this->CMakeInstance->GetDebugOutput()) {
    this->CMakeInstance->SetDebugOutputOn(flag);
    emit this->debugOutputChanged(flag);
  }
}

bool QCMake::getDebugOutput() const
{
  return this->CMakeInstance->GetDebugOutput();
}

bool QCMake::getSuppressDevWarnings()
{
  return this->CMakeInstance->GetSuppressDevWarnings();
}

void QCMake::setSuppressDevWarnings(bool value)
{
  this->CMakeInstance->SetSuppressDevWarnings(value);
}

bool QCMake::getSuppressDeprecatedWarnings()
{
  return this->CMakeInstance->GetSuppressDeprecatedWarnings();
}

void QCMake::setSuppressDeprecatedWarnings(bool value)
{
  this->CMakeInstance->SetSuppressDeprecatedWarnings(value);
}

bool QCMake::getDevWarningsAsErrors()
{
  return this->CMakeInstance->GetDevWarningsAsErrors();
}

void QCMake::setDevWarningsAsErrors(bool value)
{
  this->CMakeInstance->SetDevWarningsAsErrors(value);
}

bool QCMake::getDeprecatedWarningsAsErrors()
{
  return this->CMakeInstance->GetDeprecatedWarningsAsErrors();
}

void QCMake::setDeprecatedWarningsAsErrors(bool value)
{
  this->CMakeInstance->SetDeprecatedWarningsAsErrors(value);
}

void QCMake::setWarnUninitializedMode(bool value)
{
  this->WarnUninitializedMode = value;
}

void QCMake::setWarnUnusedMode(bool value)
{
  this->WarnUnusedMode = value;
}

void QCMake::checkOpenPossible()
{
  std::string data = this->BinaryDirectory.toLocal8Bit().data();
  auto possible = this->CMakeInstance->Open(data, true);
  emit openPossible(possible);
}
