/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "QCMakeCacheView.h"
#include <QObject>

class QCMakeCacheModelTest : public QObject
{
  Q_OBJECT
private slots:
  void setNewProperties();
  void setNewProperties_data();
};
