/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "CMakeGUITest.h"

#include "QCMake.h"
#include <QApplication>
#include <QEventLoop>
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
