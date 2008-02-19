#ifndef QMacInstallDialog_h
#define QMacInstallDialog_h
#include <QDialog>

class QMacInstallDialog : public QDialog
{
  Q_OBJECT;
public:
  QMacInstallDialog(QWidget*w);
  ~QMacInstallDialog();
private slots:
  void ShowBrowser();
  void SkipInstall();
  void DoInstall();
private:
  class QMacInstallDialogInternals;
  QMacInstallDialogInternals* Internals;
};

#endif
