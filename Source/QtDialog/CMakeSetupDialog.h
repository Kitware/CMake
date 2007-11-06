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

#ifndef CMakeSetupDialog_h
#define CMakeSetupDialog_h

#include "QCMake.h"
#include <QMainWindow>
#include <QThread>
#include "ui_CMakeSetupDialog.h"

class QCMakeThread;
class CMakeCacheModel;
class QProgressBar;
class QToolButton;

/// Qt user interface for CMake
class CMakeSetupDialog : public QMainWindow, public Ui::CMakeSetupDialog
{
  Q_OBJECT
public:
  CMakeSetupDialog();
  ~CMakeSetupDialog();

protected slots: 
  void initialize();
  void doConfigure();
  void doOk();
  void doHelp();
  void doInterrupt();
  void finishConfigure(int error);
  void finishGenerate(int error);
  void error(const QString& title, const QString& message, bool* cancel);
  
  void doSourceBrowse();
  void doBinaryBrowse();
  void updateSourceDirectory(const QString& dir);
  void setBinaryDirectory(const QString& dir);
  void showProgress(const QString& msg, float percent);
  void setEnabledState(bool);
  void promptForGenerator();
  void updateGeneratorLabel(const QString& gen);

protected:
  void closeEvent(QCloseEvent*);

  QCMakeThread* CMakeThread;
  QProgressBar* ProgressBar;
  QToolButton* InterruptButton;
};

// QCMake instance on a thread
class QCMakeThread : public QThread
{
  Q_OBJECT
public:
  QCMakeThread(QObject* p);
  QCMake* cmakeInstance() const;
  
signals:  
  void cmakeInitialized();

protected slots:
  void processEvents();

protected:
  virtual void run();
  QCMake* CMakeInstance;
};

#endif // CMakeSetupDialog_h
