/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "QCMakePresetComboBox.h"

#include "QCMakePresetItemModel.h"

QCMakePresetComboBox::QCMakePresetComboBox(QWidget* parent)
  : QComboBox(parent)
{
  this->m_model = new QCMakePresetItemModel(this);
  this->setModel(this->m_model);

  QObject::connect(this->m_model, &QCMakePresetItemModel::modelAboutToBeReset,
                   this, [this]() { this->m_resetting = true; });
  QObject::connect(this->m_model, &QCMakePresetItemModel::modelReset, this,
                   [this]() {
                     this->setPresetName(this->m_lastPreset);
                     this->m_resetting = false;
                     this->emitPresetChanged();
                   });
  QObject::connect(
    this,
    static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
    this, [this](int /*row*/) {
      if (!this->m_resetting) {
        this->emitPresetChanged();
      }
    });
}

const QVector<QCMakePreset>& QCMakePresetComboBox::presets() const
{
  return this->m_model->presets();
}

QString QCMakePresetComboBox::presetName() const
{
  auto preset = this->currentData();
  if (preset.canConvert<QCMakePreset>()) {
    return preset.value<QCMakePreset>().name;
  }
  return QString{};
}

void QCMakePresetComboBox::setPresets(const QVector<QCMakePreset>& presets)
{
  this->m_model->setPresets(presets);
}

void QCMakePresetComboBox::setPresetName(const QString& name)
{
  this->setCurrentIndex(this->m_model->presetNameToRow(name));
  if (this->signalsBlocked()) {
    this->m_lastPreset = this->presetName();
  }
}

void QCMakePresetComboBox::emitPresetChanged()
{
  if (this->presetName() != this->m_lastPreset) {
    emit this->presetChanged(this->presetName());
    this->m_lastPreset = this->presetName();
  }
}
