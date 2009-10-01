/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef AddCacheEntry_h
#define AddCacheEntry_h

#include <QWidget>
#include <QCheckBox>

#include "QCMake.h"
#include "ui_AddCacheEntry.h"

class AddCacheEntry : public QWidget, public Ui::AddCacheEntry
{
  Q_OBJECT
public:
  AddCacheEntry(QWidget* p);

  QString name() const;
  QVariant value() const;
  QString description() const;
  QCMakeProperty::PropertyType type() const;
};

#endif

