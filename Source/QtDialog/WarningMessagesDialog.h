/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2015 Kitware, Inc., Gregor Jasny

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/

#ifndef WarningMessagesDialog_h
#define WarningMessagesDialog_h

#include <QDialog>
#include <QWidget>

#include "ui_WarningMessagesDialog.h"
#include "QCMake.h"

/**
 * Dialog window for setting the warning message related options.
 */
class WarningMessagesDialog : public QDialog, public Ui_MessagesDialog
{
  Q_OBJECT

public:
  WarningMessagesDialog(QWidget* prnt, QCMake* instance);

private slots:
  /**
   * Handler for the accept event of the ok/cancel button box.
   */
  void doAccept();

  /**
   * Handler for checked state changed event of the suppress developer warnings
   * checkbox.
   */
  void doSuppressDeveloperWarningsChanged(int state);
  /**
   * Handler for checked state changed event of the suppress deprecated
   * warnings checkbox.
   */
  void doSuppressDeprecatedWarningsChanged(int state);

  /**
   * Handler for checked state changed event of the developer warnings as
   * errors checkbox.
   */
  void doDeveloperWarningsAsErrorsChanged(int state);
  /**
   * Handler for checked state changed event of the deprecated warnings as
   * errors checkbox.
   */
  void doDeprecatedWarningsAsErrorsChanged(int state);

private:
  QCMake* cmakeInstance;

  /**
   * Set the initial values of the widgets on this dialog window, using the
   * current state of the cache.
   */
  void setInitialValues();

  /**
   * Setup the signals for the widgets on this dialog window.
   */
  void setupSignals();
};

#endif /* MessageDialog_h */
