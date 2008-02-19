#include "QMacInstallDialog.h"
#include "cmSystemTools.h"
#include <iostream>
#include <QFileDialog>
#include "ui_MacInstallDialog.h"

class QMacInstallDialog::QMacInstallDialogInternals : public Ui::Dialog
{
public:
};

QMacInstallDialog::QMacInstallDialog(QWidget*w)
  :QDialog(w)
{
  this->Internals = new QMacInstallDialogInternals;
  this->Internals->setupUi(this);
  QObject::connect(this->Internals->choosePathButton, SIGNAL(pressed()), 
                   this, SLOT(ShowBrowser()));
  QObject::connect(this->Internals->skipInstallButton, SIGNAL(pressed()), 
                   this, SLOT(SkipInstall()));
  QObject::connect(this->Internals->doInstallButton, SIGNAL(pressed()), 
                   this, SLOT(DoInstall()));
  this->Internals->InstallPrefix->setText("/usr/bin/");

}

QMacInstallDialog::~QMacInstallDialog()
{
  delete this->Internals;
}

void QMacInstallDialog::DoInstall()
{  
  QDir installDir(this->Internals->InstallPrefix->text());
  std::string installTo = installDir.path().toStdString();
  QDir cmExecDir(QApplication::applicationDirPath());
  cmExecDir.cd("../bin");
  QFileInfoList list = cmExecDir.entryInfoList();
  for (int i = 0; i < list.size(); ++i) 
    {
    QFileInfo fileInfo = list.at(i);
    std::string filename = fileInfo.fileName().toStdString();
    std::string file = fileInfo.absoluteFilePath().toStdString();
    std::string newName = installTo;
    newName += "/";
    newName += filename;
    std::cout << "ln -s [" << file << "] [";
    std::cout << newName << "]\n";
    cmSystemTools::CreateSymlink(file.c_str(),
                                 newName.c_str());
    }
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
