/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "CMakeGUITest.h"

#include <QApplication>
#include <QEventLoop>
#include <QSettings>
#include <QString>
#include <QStringList>
#include <QTimer>
#include <QtGlobal>
#include <QtTest>

#include "CMakeSetupDialog.h"

namespace {
void loopSleep(int msecs = 100)
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
