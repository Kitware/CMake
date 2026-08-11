/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <QObject>

#include "QCMakePresetComboBox.h"

class QCMakePresetComboBoxTest : public QObject
{
  Q_OBJECT
private slots:
  void changePresets();
};
