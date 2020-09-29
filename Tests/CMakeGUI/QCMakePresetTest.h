/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "QCMakePreset.h"
#include <QObject>

class QCMakePresetTest : public QObject
{
  Q_OBJECT
private slots:
  void equality();
  void equality_data();
};
