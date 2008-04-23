#include "QMacInstallDialog.h"
#include <QMessageBox>
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
  if(!cmSystemTools::FileExists(installTo.c_str()))
    {
    QString message = tr("Build install does not exist, "
                         "should I create it?")
                      + "\n\n"
                      + tr("Directory: ");
    message += installDir.path();
    QString title = tr("Create Directory");
    QMessageBox::StandardButton btn;
    btn = QMessageBox::information(this, title, message, 
                                   QMessageBox::Yes | QMessageBox::No);
    if(btn == QMessageBox::Yes)
      {
      cmSystemTools::MakeDirectory(installTo.c_str());
      }
    }
  QDir cmExecDir(QApplication::applicationDirPath());
  cmExecDir.cd("../bin");
  QFileInfoList list = cmExecDir.entryInfoList();
  for (int i = 0; i < list.size(); ++i) 
    {
    QFileInfo fileInfo = list.at(i);
    std::string filename = fileInfo.fileName().toStdString();
    if(filename.size() && filename[0] == '.')
      {
      continue;
      }
    std::string file = fileInfo.absoluteFilePath().toStdString();
    std::string newName = installTo;
    newName += "/";
    newName += filename;
    // Remove the old files
    if(cmSystemTools::FileExists(newName.c_str()))
      {
      std::cout << "rm [" << newName << "]\n";
      if(!cmSystemTools::RemoveFile(newName.c_str()))
        {
        QString message = tr("Failed to remove file "
                             "installation may be incomplete: ");
        message += newName.c_str();
        QString title = tr("Error Removing file");
        QMessageBox::StandardButton btn =
          QMessageBox::critical(this, title, message, 
                                QMessageBox::Ok|QMessageBox::Abort);
        if(btn == QMessageBox::Abort)
          {
          return;
          }
        }
      }
    std::cout << "ln -s [" << file << "] [";
    std::cout << newName << "]\n";
    if(!cmSystemTools::CreateSymlink(file.c_str(),
                                     newName.c_str()))
      {
      QString message = tr("Failed create symlink "
                           "installation may be incomplete: ");
      message += newName.c_str();
      QString title = tr("Error Creating Symlink");
      QMessageBox::StandardButton btn =
        QMessageBox::critical(this, title, message, 
                              QMessageBox::Ok|QMessageBox::Abort);
      if(btn == QMessageBox::Abort)
        {
        return;
        }
      }
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
