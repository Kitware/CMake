/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include "QCMake.h"
#include <QCheckBox>
#include <QStringList>
#include <QWidget>

#include "ui_AddCacheEntry.h"

class AddCacheEntry
  : public QWidget
  , public Ui::AddCacheEntry
{
  Q_OBJECT
public:
  AddCacheEntry(QWidget* p, QStringList const& varNames,
                QStringList const& varTypes);

  QString name() const;
  QVariant value() const;
  QString description() const;
  QCMakeProperty::PropertyType type() const;
  QString typeString() const;

private slots:
  void onCompletionActivated(QString const& text);

private:
  QStringList const& VarNames;
  QStringList const& VarTypes;
};
