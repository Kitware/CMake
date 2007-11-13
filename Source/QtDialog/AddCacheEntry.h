/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

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
  QCMakeCacheProperty::PropertyType type() const;
};

#endif

