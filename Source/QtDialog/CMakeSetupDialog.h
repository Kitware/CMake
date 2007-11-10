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

public slots:
  void setBinaryDirectory(const QString& dir);
  void setSourceDirectory(const QString& dir);

protected slots: 
  void initialize();
  void doConfigure();
  void doGenerate();
  void doHelp();
  void doAbout();
  void doInterrupt();
  void finishConfigure(int error);
  void finishGenerate(int error);
  void error(const QString& message);
  
  void doSourceBrowse();
  void doBinaryBrowse();
  void doReloadCache();
  void doDeleteCache();
  void updateSourceDirectory(const QString& dir);
  void showProgress(const QString& msg, float percent);
  void setEnabledState(bool);
  void promptForGenerator();
  void updateGeneratorLabel(const QString& gen);
  void setExitAfterGenerate(bool);
  void setGenerateEnabled(bool);
  void addBinaryPath(const QString&);
  QStringList loadBuildPaths();
  void saveBuildPaths(const QStringList&);
  void onBinaryDirectoryChanged(const QString& dir);
  void onSourceDirectoryChanged(const QString& dir);
  void setCacheModified();
  void removeSelectedCacheEntries();
  void selectionChanged();

protected:
  void closeEvent(QCloseEvent*);
  void dragEnterEvent(QDragEnterEvent*);
  void dropEvent(QDropEvent*);

  QCMakeThread* CMakeThread;
  bool ExitAfterGenerate;
  bool CacheModified;
  QAction* ReloadCacheAction;
  QAction* DeleteCacheAction;
  QAction* ExitAction;
  QAction* ConfigureAction;
  QAction* GenerateAction;
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
