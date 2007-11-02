
#include <QMainWindow>
#include "ui_CMakeSetupDialog.h"
#include "QCMake.h"

class QCMakeThread;
class CMakeCacheModel;
class QProgressBar;

/// Qt user interface for CMake
class CMakeSetupDialog : public QMainWindow, public Ui::CMakeSetupDialog
{
  Q_OBJECT
public:
  CMakeSetupDialog();
  ~CMakeSetupDialog();

signals:
  void configure();
  void ok();
  void cancel();
  void propertiesChanged(const QCMakeCachePropertyList&);
  
protected slots: 
  void initialize();
  void doConfigure();
  void doOk();
  void doCancel();
  void doHelp();
  void finishConfigure(int error);
  void finishGenerate(int error);
  void error(const QString& title, const QString& message, bool* cancel);
  
  void doSourceBrowse();
  void doBinaryBrowse();
  void updateSourceDirectory(const QString& dir);
  void setBinaryDirectory(const QString& dir);

  void showProgress(const QString& msg, float percent);

protected:

  QCMakeThread* CMakeThread;
  QProgressBar* ProgressBar;

};

