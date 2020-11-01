/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "QCMakePresetComboBox.h"
#include <QObject>

class QCMakePresetComboBoxTest : public QObject
{
  Q_OBJECT
private slots:
  void changePresets();
};
