/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "QCMakePresetItemModel.h"
#include <QObject>

class QCMakePresetItemModelTest : public QObject
{
  Q_OBJECT
private slots:
  void initTestCase();
  void initTestCase_data();

  void data();
  void data_data();
};
