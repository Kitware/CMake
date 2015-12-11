/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Kitware, Inc., Gregor Jasny

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#include "WarningMessagesDialog.h"

WarningMessagesDialog::WarningMessagesDialog(QWidget* prnt, QCMake* instance)
  : QDialog(prnt), cmakeInstance(instance)
{
  this->setupUi(this);
  this->setInitialValues();
  this->setupSignals();
}

void WarningMessagesDialog::setInitialValues()
{
  this->suppressDeveloperWarnings->setChecked(
    this->cmakeInstance->getSuppressDevWarnings());
  this->suppressDeprecatedWarnings->setChecked(
    this->cmakeInstance->getSuppressDeprecatedWarnings());
}

void WarningMessagesDialog::setupSignals()
{
  QObject::connect(this->buttonBox, SIGNAL(accepted()),
                   this, SLOT(doAccept()));
}

void WarningMessagesDialog::doAccept()
{
  this->cmakeInstance->setSuppressDevWarnings(
    this->suppressDeveloperWarnings->isChecked());
  this->cmakeInstance->setSuppressDeprecatedWarnings(
    this->suppressDeprecatedWarnings->isChecked());
}
