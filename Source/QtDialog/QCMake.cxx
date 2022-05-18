/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "QCMake.h"

#include <algorithm>

#include <cm/memory>

#include <QCoreApplication>
#include <QDir>
#include <QString>
#include <QVector>

#include "cmExternalMakefileProjectGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmMessageMetadata.h"
#include "cmState.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"

#ifdef Q_OS_WIN
#  include "qt_windows.h" // For SetErrorMode
#endif

QCMake::QCMake(QObject* p)
  : QObject(p)
  , StartEnvironment(QProcessEnvironment::systemEnvironment())
  , Environment(QProcessEnvironment::systemEnvironment())
{
  this->WarnUninitializedMode = false;
  qRegisterMetaType<QCMakeProperty>();
  qRegisterMetaType<QCMakePropertyList>();
  qRegisterMetaType<QProcessEnvironment>();
  qRegisterMetaType<QVector<QCMakePreset>>();
  qRegisterMetaType<cmCMakePresetsGraph::ReadFileResult>();

  cmSystemTools::DisableRunCommandOutput();
  cmSystemTools::SetRunCommandHideConsole(true);

  cmSystemTools::SetMessageCallback(
    [this](std::string const& msg, const cmMessageMetadata& md) {
      this->messageCallback(msg, md.title);
    });
  cmSystemTools::SetStdoutCallback(
    [this](std::string const& msg) { this->stdoutCallback(msg); });
  cmSystemTools::SetStderrCallback(
    [this](std::string const& msg) { this->stderrCallback(msg); });

  this->CMakeInstance =
    cm::make_unique<cmake>(cmake::RoleProject, cmState::Project);
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

  connect(&this->LoadPresetsTimer, &QTimer::timeout, this, [this]() {
    this->loadPresets();
    if (!this->PresetName.isEmpty() &&
        this->CMakePresetsGraph.ConfigurePresets.find(
          std::string(this->PresetName.toLocal8Bit())) ==
          this->CMakePresetsGraph.ConfigurePresets.end()) {
      this->setPreset(QString{});
    }
  });
  this->LoadPresetsTimer.start(1000);
}

QCMake::~QCMake() = default;

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
    this->loadPresets();
    this->setPreset(QString{});
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
    cmValue homeDir = state->GetCacheEntryValue("CMAKE_HOME_DIRECTORY");
    if (homeDir) {
      setSourceDirectory(QString::fromLocal8Bit(homeDir->c_str()));
    }
    cmValue gen = state->GetCacheEntryValue("CMAKE_GENERATOR");
    if (gen) {
      cmValue extraGen =
        state->GetInitializedCacheValue("CMAKE_EXTRA_GENERATOR");
      std::string curGen =
        cmExternalMakefileProjectGenerator::CreateFullGeneratorName(*gen,
                                                                    *extraGen);
      this->setGenerator(QString::fromLocal8Bit(curGen.c_str()));
    }

    cmValue platform = state->GetCacheEntryValue("CMAKE_GENERATOR_PLATFORM");
    if (platform) {
      this->setPlatform(QString::fromLocal8Bit(platform->c_str()));
    }

    cmValue toolset = state->GetCacheEntryValue("CMAKE_GENERATOR_TOOLSET");
    if (toolset) {
      this->setToolset(QString::fromLocal8Bit(toolset->c_str()));
    }

    checkOpenPossible();
  }
}

void QCMake::setPreset(const QString& name, bool setBinary)
{
  if (this->PresetName != name) {
    this->PresetName = name;
    emit this->presetChanged(this->PresetName);

    if (!name.isNull()) {
      std::string presetName(name.toLocal8Bit());
      auto const& expandedPreset =
        this->CMakePresetsGraph.ConfigurePresets[presetName].Expanded;
      if (expandedPreset) {
        if (setBinary && !expandedPreset->BinaryDir.empty()) {
          QString binaryDir =
            QString::fromLocal8Bit(expandedPreset->BinaryDir.data());
          this->setBinaryDirectory(binaryDir);
        }
        if (expandedPreset->WarnDev) {
          this->CMakeInstance->SetSuppressDevWarnings(
            !*expandedPreset->WarnDev);
        }
        if (expandedPreset->ErrorDev) {
          this->CMakeInstance->SetDevWarningsAsErrors(
            *expandedPreset->ErrorDev);
        }
        if (expandedPreset->WarnDeprecated) {
          this->CMakeInstance->SetSuppressDeprecatedWarnings(
            !*expandedPreset->WarnDeprecated);
        }
        if (expandedPreset->ErrorDeprecated) {
          this->CMakeInstance->SetDeprecatedWarningsAsErrors(
            *expandedPreset->ErrorDeprecated);
        }
        if (expandedPreset->WarnUninitialized) {
          this->WarnUninitializedMode = *expandedPreset->WarnUninitialized;
          emit this->warnUninitializedModeChanged(
            *expandedPreset->WarnUninitialized);
        }
        this->Environment = this->StartEnvironment;
        for (auto const& v : expandedPreset->Environment) {
          if (v.second) {
            this->Environment.insert(QString::fromLocal8Bit(v.first.data()),
                                     QString::fromLocal8Bit(v.second->data()));
          }
        }
      }
    }
    emit this->propertiesChanged(this->properties());
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

void QCMake::setEnvironment(const QProcessEnvironment& environment)
{
  this->Environment = environment;
}

void QCMake::configure()
{
  int err;
  {
    cmSystemTools::SaveRestoreEnvironment restoreEnv;
    this->setUpEnvironment();

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
    this->CMakeInstance->SetGeneratorToolset(
      this->Toolset.toLocal8Bit().data());
    this->CMakeInstance->LoadCache();
    this->CMakeInstance->SetWarnUninitialized(this->WarnUninitializedMode);
    this->CMakeInstance->PreLoadCMakeFiles();

    InterruptFlag = 0;
    cmSystemTools::ResetErrorOccuredFlag();

    err = this->CMakeInstance->Configure();

#ifdef Q_OS_WIN
    SetErrorMode(lastErrorMode);
#endif
  }

  emit this->propertiesChanged(this->properties());
  emit this->configureDone(err);
}

void QCMake::generate()
{
  int err;
  {
    cmSystemTools::SaveRestoreEnvironment restoreEnv;
    this->setUpEnvironment();

#ifdef Q_OS_WIN
    UINT lastErrorMode = SetErrorMode(0);
#endif

    InterruptFlag = 0;
    cmSystemTools::ResetErrorOccuredFlag();

    err = this->CMakeInstance->Generate();

#ifdef Q_OS_WIN
    SetErrorMode(lastErrorMode);
#endif
  }

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
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
      const bool isBool = prop.Value.type() == QVariant::Bool;
#else
      const bool isBool = prop.Value.metaType() == QMetaType::fromType<bool>();
#endif
      if (isBool) {
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

    cmValue cachedValue = state->GetCacheEntryValue(key);

    QCMakeProperty prop;
    prop.Key = QString::fromLocal8Bit(key.c_str());
    if (cmValue hs = state->GetCacheEntryProperty(key, "HELPSTRING")) {
      prop.Help = QString::fromLocal8Bit(hs->c_str());
    }
    prop.Value = QString::fromLocal8Bit(cachedValue->c_str());
    prop.Advanced = state->GetCacheEntryPropertyAsBool(key, "ADVANCED");
    if (t == cmStateEnums::BOOL) {
      prop.Type = QCMakeProperty::BOOL;
      prop.Value = cmIsOn(*cachedValue);
    } else if (t == cmStateEnums::PATH) {
      prop.Type = QCMakeProperty::PATH;
    } else if (t == cmStateEnums::FILEPATH) {
      prop.Type = QCMakeProperty::FILEPATH;
    } else if (t == cmStateEnums::STRING) {
      prop.Type = QCMakeProperty::STRING;
      cmValue stringsProperty = state->GetCacheEntryProperty(key, "STRINGS");
      if (stringsProperty) {
        prop.Strings =
          QString::fromLocal8Bit(stringsProperty->c_str()).split(";");
      }
    }

    ret.append(prop);
  }

  if (!this->PresetName.isNull()) {
    std::string presetName(this->PresetName.toLocal8Bit());
    auto const& p =
      this->CMakePresetsGraph.ConfigurePresets.at(presetName).Expanded;
    if (p) {
      for (auto const& v : p->CacheVariables) {
        if (!v.second) {
          continue;
        }
        QCMakeProperty prop;
        prop.Key = QString::fromLocal8Bit(v.first.data());
        prop.Value = QString::fromLocal8Bit(v.second->Value.data());
        prop.Type = QCMakeProperty::STRING;
        if (!v.second->Type.empty()) {
          auto type = cmState::StringToCacheEntryType(v.second->Type);
          switch (type) {
            case cmStateEnums::BOOL:
              prop.Type = QCMakeProperty::BOOL;
              prop.Value = cmIsOn(v.second->Value);
              break;
            case cmStateEnums::PATH:
              prop.Type = QCMakeProperty::PATH;
              break;
            case cmStateEnums::FILEPATH:
              prop.Type = QCMakeProperty::FILEPATH;
              break;
            default:
              prop.Type = QCMakeProperty::STRING;
              break;
          }
        }

        // QCMakeCacheModel prefers variables earlier in the list rather than
        // later, so overwrite them if they already exist rather than simply
        // appending
        bool found = false;
        for (auto& orig : ret) {
          if (orig.Key == prop.Key) {
            orig = prop;
            found = true;
            break;
          }
        }
        if (!found) {
          ret.append(prop);
        }
      }
    }
  }

  return ret;
}

void QCMake::interrupt()
{
  this->InterruptFlag.ref();
}

bool QCMake::interruptCallback()
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
  return this->InterruptFlag.load();
#else
  return this->InterruptFlag.loadRelaxed();
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

void QCMake::setUpEnvironment() const
{
  auto env = QProcessEnvironment::systemEnvironment();
  for (auto const& key : env.keys()) {
    cmSystemTools::UnsetEnv(key.toLocal8Bit().data());
  }

  for (auto const& var : this->Environment.toStringList()) {
    cmSystemTools::PutEnv(var.toLocal8Bit().data());
  }
}

void QCMake::loadPresets()
{
  auto result = this->CMakePresetsGraph.ReadProjectPresets(
    this->SourceDirectory.toLocal8Bit().data(), true);
  if (result != this->LastLoadPresetsResult &&
      result != cmCMakePresetsGraph::ReadFileResult::READ_OK) {
    emit this->presetLoadError(this->SourceDirectory, result);
  }
  this->LastLoadPresetsResult = result;

  QVector<QCMakePreset> presets;
  for (auto const& name : this->CMakePresetsGraph.ConfigurePresetOrder) {
    auto const& it = this->CMakePresetsGraph.ConfigurePresets[name];
    auto const& p = it.Unexpanded;
    if (p.Hidden) {
      continue;
    }

    QCMakePreset preset;
    preset.name = QString::fromLocal8Bit(p.Name.data());
    preset.displayName = QString::fromLocal8Bit(p.DisplayName.data());
    preset.description = QString::fromLocal8Bit(p.Description.data());
    preset.generator = QString::fromLocal8Bit(p.Generator.data());
    preset.architecture = QString::fromLocal8Bit(p.Architecture.data());
    preset.setArchitecture = !p.ArchitectureStrategy ||
      p.ArchitectureStrategy == cmCMakePresetsGraph::ArchToolsetStrategy::Set;
    preset.toolset = QString::fromLocal8Bit(p.Toolset.data());
    preset.setToolset = !p.ToolsetStrategy ||
      p.ToolsetStrategy == cmCMakePresetsGraph::ArchToolsetStrategy::Set;
    preset.enabled = it.Expanded && it.Expanded->ConditionResult &&
      std::find_if(this->AvailableGenerators.begin(),
                   this->AvailableGenerators.end(),
                   [&p](const cmake::GeneratorInfo& g) {
                     return g.name == p.Generator;
                   }) != this->AvailableGenerators.end();
    presets.push_back(preset);
  }
  emit this->presetsChanged(presets);
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

QProcessEnvironment QCMake::environment() const
{
  return this->Environment;
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

void QCMake::checkOpenPossible()
{
  std::string data = this->BinaryDirectory.toLocal8Bit().data();
  auto possible = this->CMakeInstance->Open(data, true);
  emit openPossible(possible);
}
