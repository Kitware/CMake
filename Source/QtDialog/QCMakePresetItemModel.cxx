/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "QCMakePresetItemModel.h"

#include <QFont>

QCMakePresetItemModel::QCMakePresetItemModel(QObject* parent)
  : QAbstractItemModel(parent)
{
}

QVariant QCMakePresetItemModel::data(const QModelIndex& index, int role) const
{
  switch (role) {
    case Qt::AccessibleDescriptionRole:
      // Separators have to return "separator" for the
      // AccessibleDescriptionRole. This was determined by looking at
      // QComboBoxDelegate::isSeparator() (located in qcombobox_p.h.)
      if (index.internalId() == SEPARATOR_INDEX) {
        return QString("separator");
      }
      return QString{};
    case Qt::DisplayRole: {
      if (index.internalId() == CUSTOM_INDEX) {
        return QString("<custom>");
      }
      if (index.internalId() == SEPARATOR_INDEX) {
        return QVariant{};
      }
      auto const& preset = this->m_presets[index.internalId()];
      return preset.displayName.isEmpty() ? preset.name : preset.displayName;
    }
    case Qt::ToolTipRole:
      if (index.internalId() == CUSTOM_INDEX) {
        return QString("Specify all settings manually");
      }
      if (index.internalId() == SEPARATOR_INDEX) {
        return QVariant{};
      }
      return this->m_presets[index.internalId()].description;
    case Qt::UserRole:
      if (index.internalId() == CUSTOM_INDEX) {
        return QVariant{};
      }
      if (index.internalId() == SEPARATOR_INDEX) {
        return QVariant{};
      }
      return QVariant::fromValue(this->m_presets[index.internalId()]);
    case Qt::FontRole:
      if (index.internalId() == CUSTOM_INDEX) {
        QFont font;
        font.setItalic(true);
        return font;
      }
      return QFont{};
    default:
      return QVariant{};
  }
}

Qt::ItemFlags QCMakePresetItemModel::flags(const QModelIndex& index) const
{
  Qt::ItemFlags flags =
    Qt::ItemIsEditable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
  if (index.internalId() != SEPARATOR_INDEX &&
      (index.internalId() == CUSTOM_INDEX ||
       this->m_presets[index.internalId()].enabled)) {
    flags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
  }
  return flags;
}

int QCMakePresetItemModel::rowCount(const QModelIndex& parent) const
{
  if (parent.isValid()) {
    return 0;
  }
  if (this->m_presets.empty()) {
    return 1;
  }
  return this->m_presets.size() + 2;
}

int QCMakePresetItemModel::columnCount(const QModelIndex& parent) const
{
  if (parent.isValid()) {
    return 0;
  }
  return 1;
}

QModelIndex QCMakePresetItemModel::index(int row, int column,
                                         const QModelIndex& parent) const
{
  if (parent.isValid() || column != 0 || row < 0 ||
      row >= this->rowCount(QModelIndex{})) {
    return QModelIndex{};
  }

  if (this->m_presets.empty() || row == this->m_presets.size() + 1) {
    return this->createIndex(row, column, CUSTOM_INDEX);
  }

  if (row == this->m_presets.size()) {
    return this->createIndex(row, column, SEPARATOR_INDEX);
  }

  return this->createIndex(row, column, static_cast<quintptr>(row));
}

QModelIndex QCMakePresetItemModel::parent(const QModelIndex& /*index*/) const
{
  return QModelIndex{};
}

QVector<QCMakePreset> const& QCMakePresetItemModel::presets() const
{
  return this->m_presets;
}

void QCMakePresetItemModel::setPresets(QVector<QCMakePreset> const& presets)
{
  this->beginResetModel();
  this->m_presets = presets;
  this->endResetModel();
}

int QCMakePresetItemModel::presetNameToRow(const QString& name) const
{
  if (this->m_presets.empty()) {
    return 0;
  }

  int index = 0;
  for (auto const& preset : this->m_presets) {
    if (preset.name == name) {
      return index;
    }
    index++;
  }

  return this->m_presets.size() + 1;
}
