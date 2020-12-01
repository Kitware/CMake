/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <QObject>

class CMakeSetupDialog;

class CMakeGUITest : public QObject
{
  Q_OBJECT
public:
  CMakeGUITest(CMakeSetupDialog* window, QObject* parent = nullptr);

private:
  CMakeSetupDialog* m_window = nullptr;

  void tryConfigure(int expectedResult = 0, int timeout = 60000);

private slots:
  void sourceBinaryArgs();
  void sourceBinaryArgs_data();
  void simpleConfigure();
  void simpleConfigure_data();
  void environment();
  void presetArg();
  void presetArg_data();
  void changingPresets();
};
