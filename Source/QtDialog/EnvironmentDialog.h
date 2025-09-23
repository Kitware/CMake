/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <QDialog>
#include <QObject>
#include <QProcessEnvironment>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>

#include "ui_EnvironmentDialog.h"

class EnvironmentItemModel : public QStandardItemModel
{
  Q_OBJECT
public:
  EnvironmentItemModel(QProcessEnvironment const& environment,
                       QObject* parent = nullptr);

  QProcessEnvironment environment() const;
  void clear();

  QModelIndex buddy(QModelIndex const& index) const override;

public slots:
  void appendVariable(QString const& key, QString const& value);
  void insertVariable(int row, QString const& key, QString const& value);
};

class EnvironmentSearchFilter : public QSortFilterProxyModel
{
  Q_OBJECT
public:
  EnvironmentSearchFilter(QObject* parent = nullptr);

protected:
  bool filterAcceptsRow(int row, QModelIndex const& parent) const override;
};

class EnvironmentDialog
  : public QDialog
  , public Ui::EnvironmentDialog
{
  Q_OBJECT
public:
  EnvironmentDialog(QProcessEnvironment const& environment,
                    QWidget* parent = nullptr);

  QProcessEnvironment environment() const;

protected slots:
  void addEntry();
  void removeSelectedEntries();
  void selectionChanged();

private:
  EnvironmentItemModel* m_model;
  EnvironmentSearchFilter* m_filter;
};
