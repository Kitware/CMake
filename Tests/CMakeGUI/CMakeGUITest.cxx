/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "CMakeGUITest.h"

#include "QCMake.h"
#include <QApplication>
#include <QEventLoop>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QtGlobal>
#include <QtTest>

#include "CMakeSetupDialog.h"

#include "CatchShow.h"
#include "FirstConfigure.h"

using WindowSetupHelper = std::function<void(CMakeSetupDialog*)>;
Q_DECLARE_METATYPE(WindowSetupHelper)

namespace {
void loopSleep(int msecs = 500)
{
  QEventLoop loop;
  QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
  loop.exec();
}
}

CMakeGUITest::CMakeGUITest(CMakeSetupDialog* window, QObject* parent)
  : QObject(parent)
  , m_window(window)
{
}

void CMakeGUITest::tryConfigure(int expectedResult, int timeout)
{
  auto* cmake = this->m_window->findChild<QCMakeThread*>()->cmakeInstance();

  bool done = false;
  CatchShow catchConfigure;
  catchConfigure.setCallback<FirstConfigure>([&done](FirstConfigure* dialog) {
    if (done) {
      return;
    }
    done = true;

    dialog->findChild<StartCompilerSetup*>()->setCurrentGenerator(
      CMAKE_GENERATOR);
    dialog->accept();
  });

  CatchShow catchMessages;
  catchMessages.setCallback<QMessageBox>([](QMessageBox* box) {
    if (box->text().contains("Build directory does not exist")) {
      box->accept();
    }

    if (box->text().contains("Error in configuration process")) {
      box->accept();
    }
  });

  QSignalSpy configureDoneSpy(cmake, &QCMake::configureDone);
  QVERIFY(configureDoneSpy.isValid());
  QMetaObject::invokeMethod(
    this->m_window, [this]() { this->m_window->ConfigureButton->click(); },
    Qt::QueuedConnection);
  QVERIFY(configureDoneSpy.wait(timeout));

  QCOMPARE(configureDoneSpy, { { expectedResult } });
}

void CMakeGUITest::sourceBinaryArgs()
{
  QFETCH(QString, sourceDir);
  QFETCH(QString, binaryDir);

  // Wait a bit for everything to update
  loopSleep();

  QCOMPARE(this->m_window->SourceDirectory->text(), sourceDir);
  QCOMPARE(this->m_window->BinaryDirectory->currentText(), binaryDir);
}

void CMakeGUITest::sourceBinaryArgs_data()
{
  QTest::addColumn<QString>("sourceDir");
  QTest::addColumn<QString>("binaryDir");

  QTest::newRow("sourceAndBinaryDir")
    << CMakeGUITest_BINARY_DIR "/sourceBinaryArgs-sourceAndBinaryDir/src"
    << CMakeGUITest_BINARY_DIR "/sourceBinaryArgs-sourceAndBinaryDir/build";
  QTest::newRow("sourceAndBinaryDirRelative") << CMakeGUITest_BINARY_DIR
    "/sourceBinaryArgs-sourceAndBinaryDirRelative/src"
                                              << CMakeGUITest_BINARY_DIR
    "/sourceBinaryArgs-sourceAndBinaryDirRelative/build";
  QTest::newRow("sourceDir")
    << CMakeGUITest_BINARY_DIR "/sourceBinaryArgs-sourceDir/src"
    << CMakeGUITest_BINARY_DIR "/sourceBinaryArgs-sourceDir";
  QTest::newRow("binaryDir")
    << CMakeGUITest_BINARY_DIR "/sourceBinaryArgs-binaryDir/src"
    << CMakeGUITest_BINARY_DIR "/sourceBinaryArgs-binaryDir/build";
  QTest::newRow("noExist") << ""
                           << "";
  QTest::newRow("noExistConfig")
    << ""
    << CMakeGUITest_BINARY_DIR "/sourceBinaryArgs-noExistConfig/oldbuild";
  QTest::newRow("noExistConfigExists")
    << CMakeGUITest_BINARY_DIR "/sourceBinaryArgs-noExistConfigExists/src"
    << CMakeGUITest_BINARY_DIR "/sourceBinaryArgs-noExistConfigExists/build";
}

void CMakeGUITest::simpleConfigure()
{
  QFETCH(QString, sourceDir);
  QFETCH(QString, binaryDir);
  QFETCH(int, expectedResult);

  this->m_window->SourceDirectory->setText(sourceDir);
  this->m_window->BinaryDirectory->setCurrentText(binaryDir);

  // Wait a bit for everything to update
  loopSleep();

  this->tryConfigure(expectedResult, 1000);
}

void CMakeGUITest::simpleConfigure_data()
{
  QTest::addColumn<QString>("sourceDir");
  QTest::addColumn<QString>("binaryDir");
  QTest::addColumn<int>("expectedResult");

  QTest::newRow("success") << CMakeGUITest_BINARY_DIR
    "/simpleConfigure-success/src"
                           << CMakeGUITest_BINARY_DIR
    "/simpleConfigure-success/build"
                           << 0;
  QTest::newRow("fail") << CMakeGUITest_BINARY_DIR "/simpleConfigure-fail/src"
                        << CMakeGUITest_BINARY_DIR
    "/simpleConfigure-fail/build"
                        << -1;
}

void CMakeGUITest::environment()
{
  auto* cmake = this->m_window->findChild<QCMakeThread*>()->cmakeInstance();

  this->m_window->SourceDirectory->setText(CMakeGUITest_BINARY_DIR
                                           "/environment/src");
  this->m_window->BinaryDirectory->setCurrentText(CMakeGUITest_BINARY_DIR
                                                  "/environment/build");

  // We are already testing EnvironmentDialog, so just trust that it's
  // connected correctly and modify the environment directly.
  auto env = cmake->environment();
  env.insert("ADDED_VARIABLE", "Added variable");
  env.insert("CHANGED_VARIABLE", "Changed variable");
  env.remove("REMOVED_VARIABLE");
  cmake->setEnvironment(env);

  // Wait a bit for everything to update
  loopSleep();

  this->tryConfigure();

  auto penv = QProcessEnvironment::systemEnvironment();
  QVERIFY(!penv.contains("ADDED_VARIABLE"));
  QCOMPARE(penv.value("KEPT_VARIABLE"), "Kept variable");
  QCOMPARE(penv.value("CHANGED_VARIABLE"), "This variable will be changed");
  QCOMPARE(penv.value("REMOVED_VARIABLE"), "Removed variable");
}

void CMakeGUITest::presetArg()
{
  QFETCH(WindowSetupHelper, setupFunction);
  QFETCH(QString, presetName);
  QFETCH(QString, sourceDir);
  QFETCH(QString, binaryDir);
  QFETCH(QCMakePropertyList, properties);

  if (setupFunction) {
    setupFunction(this->m_window);
  }

  // Wait a bit for everything to update
  loopSleep();

  QCOMPARE(this->m_window->Preset->presetName(), presetName);
  QCOMPARE(this->m_window->SourceDirectory->text(), sourceDir);
  QCOMPARE(this->m_window->BinaryDirectory->currentText(), binaryDir);

  auto actualProperties =
    this->m_window->CacheValues->cacheModel()->properties();
  QCOMPARE(actualProperties.size(), properties.size());
  for (int i = 0; i < actualProperties.size(); ++i) {
    // operator==() only compares Key, we need to compare Value and Type too
    QCOMPARE(actualProperties[i].Key, properties[i].Key);
    QCOMPARE(actualProperties[i].Value, properties[i].Value);
    QCOMPARE(actualProperties[i].Type, properties[i].Type);
  }
}

namespace {
QCMakePropertyList makePresetProperties(const QString& name)
{
  return QCMakePropertyList{
    QCMakeProperty{
      /*Key=*/"FALSE_VARIABLE",
      /*Value=*/false,
      /*Strings=*/{},
      /*Help=*/"",
      /*Type=*/QCMakeProperty::BOOL,
      /*Advanced=*/false,
    },
    QCMakeProperty{
      /*Key=*/"FILEPATH_VARIABLE",
      /*Value=*/
      QString::fromLocal8Bit(CMakeGUITest_BINARY_DIR "/%1/src/CMakeLists.txt")
        .arg(name),
      /*Strings=*/{},
      /*Help=*/"",
      /*Type=*/QCMakeProperty::FILEPATH,
      /*Advanced=*/false,
    },
    QCMakeProperty{
      /*Key=*/"ON_VARIABLE",
      /*Value=*/true,
      /*Strings=*/{},
      /*Help=*/"",
      /*Type=*/QCMakeProperty::BOOL,
      /*Advanced=*/false,
    },
    QCMakeProperty{
      /*Key=*/"PATH_VARIABLE",
      /*Value=*/
      QString::fromLocal8Bit(CMakeGUITest_BINARY_DIR "/%1/src").arg(name),
      /*Strings=*/{},
      /*Help=*/"",
      /*Type=*/QCMakeProperty::PATH,
      /*Advanced=*/false,
    },
    QCMakeProperty{
      /*Key=*/"STRING_VARIABLE",
      /*Value=*/"String value",
      /*Strings=*/{},
      /*Help=*/"",
      /*Type=*/QCMakeProperty::STRING,
      /*Advanced=*/false,
    },
    QCMakeProperty{
      /*Key=*/"UNINITIALIZED_VARIABLE",
      /*Value=*/"Uninitialized value",
      /*Strings=*/{},
      /*Help=*/"",
      /*Type=*/QCMakeProperty::STRING,
      /*Advanced=*/false,
    },
  };
}
}

void CMakeGUITest::presetArg_data()
{
  QTest::addColumn<WindowSetupHelper>("setupFunction");
  QTest::addColumn<QString>("presetName");
  QTest::addColumn<QString>("sourceDir");
  QTest::addColumn<QString>("binaryDir");
  QTest::addColumn<QCMakePropertyList>("properties");

  QTest::newRow("preset") << WindowSetupHelper{} << "ninja"
                          << CMakeGUITest_BINARY_DIR "/presetArg-preset/src"
                          << CMakeGUITest_BINARY_DIR
    "/presetArg-preset/src/build"
                          << makePresetProperties("presetArg-preset");
  QTest::newRow("presetBinary")
    << WindowSetupHelper{} << "ninja"
    << CMakeGUITest_BINARY_DIR "/presetArg-presetBinary/src"
    << CMakeGUITest_BINARY_DIR "/presetArg-presetBinary/build"
    << makePresetProperties("presetArg-presetBinary");
  QTest::newRow("presetBinaryChange")
    << WindowSetupHelper{ [](CMakeSetupDialog* window) {
         loopSleep();
         window->Preset->setPresetName("ninja2");
       } }
    << "ninja2" << CMakeGUITest_BINARY_DIR "/presetArg-presetBinaryChange/src"
    << CMakeGUITest_BINARY_DIR "/presetArg-presetBinaryChange/src/build"
    << makePresetProperties("presetArg-presetBinaryChange");
  QTest::newRow("noPresetBinaryChange")
    << WindowSetupHelper{ [](CMakeSetupDialog* window) {
         loopSleep();
         window->Preset->setPresetName("ninja");
       } }
    << "ninja" << CMakeGUITest_BINARY_DIR "/presetArg-noPresetBinaryChange/src"
    << CMakeGUITest_BINARY_DIR "/presetArg-noPresetBinaryChange/src/build"
    << makePresetProperties("presetArg-noPresetBinaryChange");
  QTest::newRow("presetConfigExists")
    << WindowSetupHelper{} << "ninja"
    << CMakeGUITest_BINARY_DIR "/presetArg-presetConfigExists/src"
    << CMakeGUITest_BINARY_DIR "/presetArg-presetConfigExists/src/build"
    << makePresetProperties("presetArg-presetConfigExists");
  QTest::newRow("noExist") << WindowSetupHelper{} << QString{}
                           << CMakeGUITest_BINARY_DIR "/presetArg-noExist/src"
                           << "" << QCMakePropertyList{};
}

namespace {
void writePresets(const QString& buildDir, const QStringList& names)
{
  QJsonArray presets{
    QJsonObject{
      { "name", "base" },
      { "generator", "Ninja" },
      { "binaryDir",
        QString::fromLocal8Bit("${sourceDir}/%1/${presetName}")
          .arg(buildDir) },
      { "hidden", true },
    },
  };

  for (auto const& name : names) {
    presets.append(QJsonObject{
      { "name", name },
      { "inherits", QJsonArray{ "base" } },
    });
  }

  QJsonDocument doc{ QJsonObject{
    { "version", 1 },
    { "configurePresets", presets },
  } };

  QFile presetsFile(CMakeGUITest_BINARY_DIR
                    "/changingPresets/src/CMakePresets.json");
  bool open = presetsFile.open(QIODevice::WriteOnly);
  Q_ASSERT(open);
  presetsFile.write(doc.toJson());
}
}

void CMakeGUITest::changingPresets()
{
  QDir::root().mkpath(CMakeGUITest_BINARY_DIR "/changingPresets/src");

  this->m_window->SourceDirectory->setText(CMakeGUITest_BINARY_DIR
                                           "/changingPresets/src");
  loopSleep();
  QCOMPARE(this->m_window->Preset->presetName(), QString{});
  QCOMPARE(this->m_window->Preset->presets().size(), 0);
  QCOMPARE(this->m_window->BinaryDirectory->currentText(), "");
  QCOMPARE(this->m_window->Preset->isEnabled(), false);

  writePresets("build1", { "preset" });
  loopSleep(1500);
  QCOMPARE(this->m_window->Preset->presetName(), QString{});
  QCOMPARE(this->m_window->Preset->presets().size(), 1);
  QCOMPARE(this->m_window->BinaryDirectory->currentText(), "");
  QCOMPARE(this->m_window->Preset->isEnabled(), true);

  this->m_window->Preset->setPresetName("preset");
  loopSleep();
  QCOMPARE(this->m_window->Preset->presetName(), "preset");
  QCOMPARE(this->m_window->Preset->presets().size(), 1);
  QCOMPARE(this->m_window->BinaryDirectory->currentText(),
           CMakeGUITest_BINARY_DIR "/changingPresets/src/build1/preset");
  QCOMPARE(this->m_window->Preset->isEnabled(), true);

  writePresets("build2", { "preset2", "preset" });
  loopSleep(1500);
  QCOMPARE(this->m_window->Preset->presetName(), "preset");
  QCOMPARE(this->m_window->Preset->presets().size(), 2);
  QCOMPARE(this->m_window->BinaryDirectory->currentText(),
           CMakeGUITest_BINARY_DIR "/changingPresets/src/build1/preset");
  QCOMPARE(this->m_window->Preset->isEnabled(), true);

  writePresets("build3", { "preset2" });
  loopSleep(1500);
  QCOMPARE(this->m_window->Preset->presetName(), QString{});
  QCOMPARE(this->m_window->Preset->presets().size(), 1);
  QCOMPARE(this->m_window->BinaryDirectory->currentText(),
           CMakeGUITest_BINARY_DIR "/changingPresets/src/build1/preset");
  QCOMPARE(this->m_window->Preset->isEnabled(), true);

  this->m_window->Preset->setPresetName("preset2");
  loopSleep();
  QCOMPARE(this->m_window->Preset->presetName(), "preset2");
  QCOMPARE(this->m_window->Preset->presets().size(), 1);
  QCOMPARE(this->m_window->BinaryDirectory->currentText(),
           CMakeGUITest_BINARY_DIR "/changingPresets/src/build3/preset2");
  QCOMPARE(this->m_window->Preset->isEnabled(), true);

  QDir::root().mkpath(CMakeGUITest_BINARY_DIR "/changingPresets/src2");
  QFile::copy(CMakeGUITest_BINARY_DIR "/changingPresets/src/CMakePresets.json",
              CMakeGUITest_BINARY_DIR
              "/changingPresets/src2/CMakePresets.json");
  this->m_window->SourceDirectory->setText(CMakeGUITest_BINARY_DIR
                                           "/changingPresets/src2");
  loopSleep();
  QCOMPARE(this->m_window->Preset->presetName(), QString{});
  QCOMPARE(this->m_window->Preset->presets().size(), 1);
  QCOMPARE(this->m_window->BinaryDirectory->currentText(),
           CMakeGUITest_BINARY_DIR "/changingPresets/src/build3/preset2");
  QCOMPARE(this->m_window->Preset->isEnabled(), true);

  this->m_window->Preset->setPresetName("preset2");
  loopSleep();
  QCOMPARE(this->m_window->Preset->presetName(), "preset2");
  QCOMPARE(this->m_window->Preset->presets().size(), 1);
  QCOMPARE(this->m_window->BinaryDirectory->currentText(),
           CMakeGUITest_BINARY_DIR "/changingPresets/src2/build3/preset2");
  QCOMPARE(this->m_window->Preset->isEnabled(), true);

  QFile(CMakeGUITest_BINARY_DIR "/changingPresets/src2/CMakePresets.json")
    .remove();
  loopSleep(1500);
  QCOMPARE(this->m_window->Preset->presetName(), QString{});
  QCOMPARE(this->m_window->Preset->presets().size(), 0);
  QCOMPARE(this->m_window->BinaryDirectory->currentText(),
           CMakeGUITest_BINARY_DIR "/changingPresets/src2/build3/preset2");
  QCOMPARE(this->m_window->Preset->isEnabled(), false);
}

void SetupDefaultQSettings()
{
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QSettings::setPath(QSettings::IniFormat, QSettings::UserScope,
                     QString::fromLocal8Bit(qgetenv("CMake_GUI_CONFIG_DIR")));
}

int CMakeGUIExec(CMakeSetupDialog* window)
{
  auto nameArray = qgetenv("CMake_GUI_TEST_NAME");
  auto name = QString::fromLocal8Bit(nameArray);
  if (name.isEmpty()) {
    return QApplication::exec();
  }

  QStringList args{ "CMakeGUITest", name };
  CMakeGUITest obj(window);
  return QTest::qExec(&obj, args);
}
