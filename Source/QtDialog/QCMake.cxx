/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "QCMake.h"

#include <algorithm>

#include <cm/memory>

#include "QCMakeSizeType.h"
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
          std::string(this->PresetName.toStdString())) ==
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
  QString dir = QString::fromStdString(
    cmSystemTools::GetActualCaseForPath(_dir.toStdString()));
  if (this->SourceDirectory != dir) {
    this->SourceDirectory = QDir::fromNativeSeparators(dir);
    emit this->sourceDirChanged(this->SourceDirectory);
    this->loadPresets();
    this->setPreset(QString{});
  }
}

void QCMake::setBinaryDirectory(const QString& _dir)
{
  QString dir = QString::fromStdString(
    cmSystemTools::GetActualCaseForPath(_dir.toStdString()));
  if (this->BinaryDirectory != dir) {
    this->BinaryDirectory = QDir::fromNativeSeparators(dir);
    emit this->binaryDirChanged(this->BinaryDirectory);
    cmState* state = this->CMakeInstance->GetState();
    this->setGenerator(QString());
    this->setToolset(QString());
    this->setPlatform(QString());
    if (!this->CMakeInstance->LoadCache(this->BinaryDirectory.toStdString())) {
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
      setSourceDirectory(QString(homeDir->c_str()));
    }
    cmValue gen = state->GetCacheEntryValue("CMAKE_GENERATOR");
    if (gen) {
      cmValue extraGen =
        state->GetInitializedCacheValue("CMAKE_EXTRA_GENERATOR");
      std::string curGen =
        cmExternalMakefileProjectGenerator::CreateFullGeneratorName(*gen,
                                                                    *extraGen);
      this->setGenerator(QString::fromStdString(curGen));
    }

    cmValue platform = state->GetCacheEntryValue("CMAKE_GENERATOR_PLATFORM");
    if (platform) {
      this->setPlatform(QString(platform->c_str()));
    }

    cmValue toolset = state->GetCacheEntryValue("CMAKE_GENERATOR_TOOLSET");
    if (toolset) {
      this->setToolset(QString(toolset->c_str()));
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
      std::string presetName(name.toStdString());
      auto const& expandedPreset =
        this->CMakePresetsGraph.ConfigurePresets[presetName].Expanded;
      if (expandedPreset) {
        if (setBinary && !expandedPreset->BinaryDir.empty()) {
          QString binaryDir =
            QString::fromStdString(expandedPreset->BinaryDir);
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
            this->Environment.insert(QString::fromStdString(v.first),
                                     QString::fromStdString(v.second.value()));
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

    this->CMakeInstance->SetHomeDirectory(this->SourceDirectory.toStdString());
    this->CMakeInstance->SetHomeOutputDirectory(
      this->BinaryDirectory.toStdString());
    this->CMakeInstance->SetGlobalGenerator(
      this->CMakeInstance->CreateGlobalGenerator(
        this->Generator.toStdString()));
    this->CMakeInstance->SetGeneratorPlatform(this->Platform.toStdString());
    this->CMakeInstance->SetGeneratorToolset(this->Toolset.toStdString());
    this->CMakeInstance->LoadCache();
    this->CMakeInstance->SetWarnUninitialized(this->WarnUninitializedMode);
    this->CMakeInstance->PreLoadCMakeFiles();

    InterruptFlag = 0;
    cmSystemTools::ResetErrorOccurredFlag();

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
    cmSystemTools::ResetErrorOccurredFlag();

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
  cmSystemTools::ResetErrorOccurredFlag();

  auto successful =
    this->CMakeInstance->Open(this->BinaryDirectory.toStdString(), false);

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
    prop.Key = QString::fromStdString(key);
    cm_qsizetype idx = props.indexOf(prop);
    if (idx == -1) {
      toremove.append(QString::fromStdString(key));
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
        state->SetCacheEntryValue(key, prop.Value.toString().toStdString());
      }
      props.removeAt(idx);
    }
  }

  // remove some properties
  foreach (QString const& s, toremove) {
    this->CMakeInstance->UnwatchUnusedCli(s.toStdString());

    state->RemoveCacheEntry(s.toStdString());
  }

  // add some new properties
  foreach (QCMakeProperty const& s, props) {
    this->CMakeInstance->WatchUnusedCli(s.Key.toStdString());

    if (s.Type == QCMakeProperty::BOOL) {
      this->CMakeInstance->AddCacheEntry(
        s.Key.toStdString(), s.Value.toBool() ? "ON" : "OFF",
        s.Help.toStdString(), cmStateEnums::BOOL);
    } else if (s.Type == QCMakeProperty::STRING) {
      this->CMakeInstance->AddCacheEntry(
        s.Key.toStdString(), s.Value.toString().toStdString(),
        s.Help.toStdString(), cmStateEnums::STRING);
    } else if (s.Type == QCMakeProperty::PATH) {
      this->CMakeInstance->AddCacheEntry(
        s.Key.toStdString(), s.Value.toString().toStdString(),
        s.Help.toStdString(), cmStateEnums::PATH);
    } else if (s.Type == QCMakeProperty::FILEPATH) {
      this->CMakeInstance->AddCacheEntry(
        s.Key.toStdString(), s.Value.toString().toStdString(),
        s.Help.toStdString(), cmStateEnums::FILEPATH);
    }
  }

  this->CMakeInstance->SaveCache(this->BinaryDirectory.toStdString());
}

namespace {
template <typename T>
QCMakeProperty cache_to_property(const T& v)
{
  QCMakeProperty prop;
  prop.Key = QString::fromStdString(v.first);
  prop.Value = QString::fromStdString(v.second->Value);
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
  return prop;
}

void add_to_property_list(QCMakePropertyList& list, QCMakeProperty&& prop)
{
  // QCMakeCacheModel prefers variables earlier in the list rather than
  // later, so overwrite them if they already exist rather than simply
  // appending
  bool found = false;
  for (auto& orig : list) {
    if (orig.Key == prop.Key) {
      orig = prop;
      found = true;
      break;
    }
  }
  if (!found) {
    list.append(prop);
  }
}
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
    prop.Key = QString::fromStdString(key);
    if (cmValue hs = state->GetCacheEntryProperty(key, "HELPSTRING")) {
      prop.Help = QString(hs->c_str());
    }
    prop.Value = QString(cachedValue->c_str());
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
        prop.Strings = QString(stringsProperty->c_str()).split(";");
      }
    }

    ret.append(prop);
  }

  if (!this->PresetName.isNull()) {
    std::string presetName(this->PresetName.toStdString());
    auto const& p =
      this->CMakePresetsGraph.ConfigurePresets.at(presetName).Expanded;
    if (p) {
      if (!p->ToolchainFile.empty()) {
        using CacheVariable = cmCMakePresetsGraph::CacheVariable;
        CacheVariable var{ "FILEPATH", p->ToolchainFile };
        std::pair<std::string, cm::optional<CacheVariable>> value = {
          "CMAKE_TOOLCHAIN_FILE", var
        };
        auto prop = cache_to_property(value);
        add_to_property_list(ret, std::move(prop));
      }
      for (auto const& v : p->CacheVariables) {
        if (!v.second) {
          continue;
        }
        auto prop = cache_to_property(v);
        add_to_property_list(ret, std::move(prop));
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
    cmSystemTools::UnsetEnv(key.toStdString().c_str());
  }

  for (auto const& var : this->Environment.toStringList()) {
    cmSystemTools::PutEnv(var.toStdString());
  }
}

void QCMake::loadPresets()
{
  auto result = this->CMakePresetsGraph.ReadProjectPresets(
    this->SourceDirectory.toStdString(), true);
  if (result != this->LastLoadPresetsResult && !result) {
    emit this->presetLoadError(
      this->SourceDirectory,
      QString::fromStdString(
        this->CMakePresetsGraph.parseState.GetErrorMessage(false)));
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
    preset.name = QString::fromStdString(p.Name);
    preset.displayName = QString::fromStdString(p.DisplayName);
    preset.description = QString::fromStdString(p.Description);
    preset.generator = QString::fromStdString(p.Generator);
    preset.architecture = QString::fromStdString(p.Architecture);
    preset.setArchitecture = !p.ArchitectureStrategy ||
      p.ArchitectureStrategy == cmCMakePresetsGraph::ArchToolsetStrategy::Set;
    preset.toolset = QString::fromStdString(p.Toolset);
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
  this->CMakeInstance->DeleteCache(this->BinaryDirectory.toStdString());
  // reload to make our cache empty
  this->CMakeInstance->LoadCache(this->BinaryDirectory.toStdString());
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
  this->CMakeInstance->LoadCache(this->BinaryDirectory.toStdString());
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
  std::string data = this->BinaryDirectory.toStdString();
  auto possible = this->CMakeInstance->Open(data, true);
  emit openPossible(possible);
}
