/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#pragma once

#include "QCMakePreset.h"
#include <QComboBox>
#include <QObject>
#include <QString>
#include <QVector>

class QCMakePresetItemModel;

class QCMakePresetComboBox : public QComboBox
{
  Q_OBJECT
public:
  QCMakePresetComboBox(QWidget* parent = nullptr);

  const QVector<QCMakePreset>& presets() const;
  QString presetName() const;

public slots:
  void setPresets(const QVector<QCMakePreset>& presets);
  void setPresetName(const QString& name);

signals:
  void presetChanged(const QString& name);

private:
  QCMakePresetItemModel* m_model;
  bool m_resetting = false;
  QString m_lastPreset;

  void emitPresetChanged();
};
