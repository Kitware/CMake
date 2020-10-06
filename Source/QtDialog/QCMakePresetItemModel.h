/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include <cm/optional>

#include "QCMakePreset.h"
#include <QAbstractItemModel>
#include <QModelIndex>
#include <QString>
#include <QVariant>
#include <QVector>
#include <QtGlobal>

class QObject;

class QCMakePresetItemModel : public QAbstractItemModel
{
  Q_OBJECT
public:
  QCMakePresetItemModel(QObject* parent = nullptr);

  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

  int rowCount(const QModelIndex& parent = QModelIndex{}) const override;
  int columnCount(const QModelIndex& parent = QModelIndex{}) const override;

  QModelIndex index(int row, int column,
                    const QModelIndex& parent = QModelIndex{}) const override;
  QModelIndex parent(const QModelIndex& index) const override;

  QVector<QCMakePreset> const& presets() const;

  int presetNameToRow(const QString& name) const;

public slots:
  void setPresets(QVector<QCMakePreset> const& presets);

private:
  QVector<QCMakePreset> m_presets;

  static constexpr quintptr SEPARATOR_INDEX = static_cast<quintptr>(-2);
  static constexpr quintptr CUSTOM_INDEX = static_cast<quintptr>(-1);
};
