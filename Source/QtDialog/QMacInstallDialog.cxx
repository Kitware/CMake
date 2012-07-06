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
  for (int i = 0; i < list.size(); ++i) 
    {
    QFileInfo fileInfo = list.at(i);
    QString filename = fileInfo.fileName();
    if(filename.size() && filename[0] == '.')
      {
      continue;
      }
    QString file = fileInfo.absoluteFilePath();
    QString newName = installTo;
    newName += "/";
    newName += filename;
    // Remove the old files
    if(cmSystemTools::FileExists(newName.toLocal8Bit().data()))
      {
      std::cout << "rm [" << newName.toLocal8Bit().data() << "]\n";
      if(!cmSystemTools::RemoveFile(newName.toLocal8Bit().data()))
        {
        QString message = tr("Failed to remove file "
                             "installation may be incomplete: ");
        message += newName;
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
    std::cout << "ln -s [" << file.toLocal8Bit().data() << "] [";
    std::cout << newName.toLocal8Bit().data() << "]\n";
    if(!cmSystemTools::CreateSymlink(file.toLocal8Bit().data(),
                                     newName.toLocal8Bit().data()))
      {
      QString message = tr("Failed create symlink "
                           "installation may be incomplete: ");
      message += newName;
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
