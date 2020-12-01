/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <QObject>

class EnvironmentDialogTest : public QObject
{
  Q_OBJECT
public:
  EnvironmentDialogTest(QObject* parent = nullptr);

private slots:
  void environmentDialog();
};
