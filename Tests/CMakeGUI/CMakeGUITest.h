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

private slots:
  void sourceBinaryArgs();
  void sourceBinaryArgs_data();
};
