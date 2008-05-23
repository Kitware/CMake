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

#include "AddCacheEntry.h"
#include <QMetaProperty>

static const int NumTypes = 4;
static const QString TypeStrings[NumTypes] = 
  { "BOOL", "PATH", "FILEPATH", "STRING" };
static const QCMakeProperty::PropertyType Types[NumTypes] = 
  { QCMakeProperty::BOOL, QCMakeProperty::PATH, 
    QCMakeProperty::FILEPATH, QCMakeProperty::STRING}; 

AddCacheEntry::AddCacheEntry(QWidget* p)
  : QWidget(p)
{
  this->setupUi(this);
  for(int i=0; i<NumTypes; i++)
    {
    this->Type->addItem(TypeStrings[i]);
    }
  QWidget* cb = new QCheckBox();
  QWidget* path = new QCMakePathEditor();
  QWidget* filepath = new QCMakeFilePathEditor();
  QWidget* string = new QLineEdit();
  this->StackedWidget->addWidget(cb);
  this->StackedWidget->addWidget(path);
  this->StackedWidget->addWidget(filepath);
  this->StackedWidget->addWidget(string);
  this->setTabOrder(this->Name, this->Type);
  this->setTabOrder(this->Type, cb);
  this->setTabOrder(cb, path);
  this->setTabOrder(path, filepath);
  this->setTabOrder(filepath, string);
  this->setTabOrder(string, this->Description);
}

QString AddCacheEntry::name() const
{
  return this->Name->text();
}

QVariant AddCacheEntry::value() const
{
  QWidget* w = this->StackedWidget->currentWidget();
  if(qobject_cast<QLineEdit*>(w))
    {
    return static_cast<QLineEdit*>(w)->text();
    }
  else if(qobject_cast<QCheckBox*>(w))
    {
    return static_cast<QCheckBox*>(w)->isChecked();
    }
  return QVariant();
}

QString AddCacheEntry::description() const
{
  return this->Description->text();
}

QCMakeProperty::PropertyType AddCacheEntry::type() const
{
  int idx = this->Type->currentIndex();
  if(idx >= 0 && idx < NumTypes)
    {
    return Types[idx];
    }
  return QCMakeProperty::BOOL;
}


