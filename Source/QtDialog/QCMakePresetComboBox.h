/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <QComboBox>
#include <QObject>
#include <QString>
#include <QVector>

#include "QCMakePreset.h"

class QCMakePresetItemModel;

class QCMakePresetComboBox : public QComboBox
{
  Q_OBJECT
public:
  QCMakePresetComboBox(QWidget* parent = nullptr);

  QVector<QCMakePreset> const& presets() const;
  QString presetName() const;

public Q_SLOTS:
  void setPresets(QVector<QCMakePreset> const& presets);
  void setPresetName(QString const& name);

Q_SIGNALS:
  void presetChanged(QString const& name);

private:
  QCMakePresetItemModel* m_model;
  bool m_resetting = false;
  QString m_lastPreset;

  void emitPresetChanged();
};
