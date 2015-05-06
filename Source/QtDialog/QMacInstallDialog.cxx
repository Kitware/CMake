#include "QMacInstallDialog.h"
#include <QMessageBox>
#include "cmSystemTools.h"
#include <iostream>
#include <QFileDialog>
#include "ui_MacInstallDialog.h"
#include <Security/Authorization.h>
#include <Security/AuthorizationTags.h>

class QMacInstallDialog::QMacInstallDialogInternals : public Ui::Dialog
{
public:
};

QMacInstallDialog::QMacInstallDialog(QWidget*w)
  :QDialog(w)
{
  this->Internals = new QMacInstallDialogInternals;
  this->Internals->setupUi(this);
  QObject::connect(this->Internals->choosePathButton, SIGNAL(clicked(bool)),
                   this, SLOT(ShowBrowser()));
  QObject::connect(this->Internals->skipInstallButton, SIGNAL(clicked(bool)),
                   this, SLOT(SkipInstall()));
  QObject::connect(this->Internals->doInstallButton, SIGNAL(clicked(bool)),
                   this, SLOT(DoInstall()));
  this->Internals->InstallPrefix->setText("/usr/bin/");

}

QMacInstallDialog::~QMacInstallDialog()
{
  delete this->Internals;
}

void QMacInstallDialog::DoInstall()
{
  std::string cmd;
  QDir installDir(this->Internals->InstallPrefix->text());
  QString installTo = installDir.path();
  if(!cmSystemTools::FileExists(installTo.toLocal8Bit().data()))
    {
    QString message = tr("Build install does not exist, "
                         "should I create it?\n\n"
                         "Directory: ");
    message += installDir.path();
    QString title = tr("Create Directory");
    QMessageBox::StandardButton btn;
    btn = QMessageBox::information(this, title, message,
                                   QMessageBox::Yes | QMessageBox::No);
    if(btn == QMessageBox::Yes)
      {
      cmSystemTools::MakeDirectory(installTo.toLocal8Bit().data());
      }
    }
  QDir cmExecDir(QApplication::applicationDirPath());
  cmExecDir.cd("../bin");
  QFileInfoList list = cmExecDir.entryInfoList();
  cmd.append("osascript -e 'do shell script \"ln -sfh ");
  for (int i = 0; i < list.size(); ++i)
    {
    QFileInfo fileInfo = list.at(i);
    QString filename = fileInfo.fileName();
    if(filename.size() && filename[0] == '.')
      {
      continue;
      }
    QString file = fileInfo.absoluteFilePath();
    cmd.append("\\\"").append(file.toLocal8Bit().data()).append("\\\" ");
    }
    cmd.append("\\\"").append(installTo.toLocal8Bit().data()).append("\\\" ");
    cmd.append("\" with administrator privileges'");
    std::cout << cmd << "\n";
    system(cmd.c_str());

  this->done(0);
}

void QMacInstallDialog::SkipInstall()
{
  this->done(0);
}


void QMacInstallDialog::ShowBrowser()
{
  QString dir = QFileDialog::getExistingDirectory(this,
    tr("Enter Install Prefix"), this->Internals->InstallPrefix->text());
  if(!dir.isEmpty())
    {
    this->Internals->InstallPrefix->setText(dir);
    }
}
