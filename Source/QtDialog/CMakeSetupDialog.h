/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#pragma once

#include <memory>

#include "QCMake.h"
#include "QCMakePreset.h"
#include <QEventLoop>
#include <QMainWindow>
#include <QThread>
#include <QVector>

#include "ui_CMakeSetupDialog.h"

class QCMakePresetItemModel;
class QCMakeThread;
class CMakeCacheModel;
class QProgressBar;
class QToolButton;

#ifdef QT_WINEXTRAS
class QWinTaskbarButton;
#endif

/// Qt user interface for CMake
class CMakeSetupDialog
  : public QMainWindow
  , public Ui::CMakeSetupDialog
{
  Q_OBJECT
public:
  CMakeSetupDialog();
  ~CMakeSetupDialog();

public slots:
  void setBinaryDirectory(QString const& dir);
  void setSourceDirectory(QString const& dir);
  void setDeferredPreset(QString const& preset);
  void setStartupBinaryDirectory(bool startup);

protected slots:
  void initialize();
  void doConfigure();
  void doGenerate();
  void doOpenProject();
  void doInstallForCommandLine();
  void doHelp();
  void doAbout();
  void doInterrupt();
  void error(QString const& message);
  void message(QString const& message);

  void doSourceBrowse();
  void doBinaryBrowse();
  void doReloadCache();
  void doDeleteCache();
  void updateSourceDirectory(QString const& dir);
  void updateBinaryDirectory(QString const& dir);
  void updatePresets(QVector<QCMakePreset> const& presets);
  void updatePreset(QString const& name);
  void showPresetLoadError(QString const& dir, QString const& message);
  void showProgress(QString const& msg, float percent);
  void setEnabledState(bool);
  bool setupFirstConfigure();
  void updateGeneratorLabel(QString const& gen);
  void setExitAfterGenerate(bool);
  void addBinaryPath(QString const&);
  QStringList loadBuildPaths();
  void saveBuildPaths(QStringList const&);
  void onBinaryDirectoryChanged(QString const& dir);
  void onSourceDirectoryChanged(QString const& dir);
  void onBuildPresetChanged(QString const& name);
  void setCacheModified();
  void removeSelectedCacheEntries();
  void selectionChanged();
  void editEnvironment();
  void addCacheEntry();
  void startSearch();
  void setDebugOutput(bool);
  void setAdvancedView(bool);
  void setGroupedView(bool);
  void showUserChanges();
  void setSearchFilter(QString const& str);
  bool prepareConfigure();
  bool doConfigureInternal();
  bool doGenerateInternal();
  void exitLoop(int);
  void doOutputContextMenu(QPoint pt);
  void doOutputFindDialog();
  void doOutputFindNext(bool directionForward = true);
  void doOutputFindPrev();
  void doOutputErrorNext();
  void doRegexExplorerDialog();
  /// display the modal warning messages dialog window
  void doWarningMessagesDialog();

protected:
  enum State
  {
    Interrupting,
    ReadyConfigure,
    ReadyGenerate,
    Configuring,
    Generating
  };
  void enterState(State s);

  void closeEvent(QCloseEvent*);
  void dragEnterEvent(QDragEnterEvent*);
  void dropEvent(QDropEvent*);

  QCMakeThread* CMakeThread;
  bool ExitAfterGenerate;
  bool CacheModified;
  bool ConfigureNeeded;
  QAction* ReloadCacheAction;
  QAction* DeleteCacheAction;
  QAction* ExitAction;
  QAction* ConfigureAction;
  QAction* GenerateAction;
  QAction* WarnUninitializedAction;
  QAction* InstallForCommandLineAction;
  State CurrentState;
  QString DeferredPreset;
  bool StartupBinaryDirectory = false;

  QTextCharFormat ErrorFormat;
  QTextCharFormat MessageFormat;

  QStringList AddVariableNames;
  QStringList AddVariableTypes;
  QStringList FindHistory;

  QEventLoop LocalLoop;

#ifdef QT_WINEXTRAS
  QWinTaskbarButton* TaskbarButton;
#endif

  float ProgressOffset;
  float ProgressFactor;
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

protected:
  virtual void run();
  std::unique_ptr<QCMake> CMakeInstance;
};
