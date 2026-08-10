/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <QObject>

#include "QCMakeCacheView.h"

class QCMakeCacheModelTest : public QObject
{
  Q_OBJECT
private slots:
  void setNewProperties();
  void setNewProperties_data();
};
