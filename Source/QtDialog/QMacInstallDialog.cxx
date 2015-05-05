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
  OSStatus myStatus;

	AuthorizationFlags myFlags = kAuthorizationFlagDefaults;
  AuthorizationRef myAuthorizationRef;
  myStatus = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment,
                                 myFlags, &myAuthorizationRef);
  if (myStatus != errAuthorizationSuccess) return;
  
  AuthorizationItem myItems = {kAuthorizationRightExecute, 0,
	                                 NULL, 0};
	AuthorizationRights myRights = {1, &myItems};
	myFlags = kAuthorizationFlagDefaults |
	          kAuthorizationFlagInteractionAllowed |
	          kAuthorizationFlagPreAuthorize |
	          kAuthorizationFlagExtendRights;
	myStatus =
	      AuthorizationCopyRights(myAuthorizationRef,&myRights,NULL,myFlags,NULL);
	  
	if (myStatus == errAuthorizationSuccess)
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
    char *args[list.size() + 3];
    int myArgc = 0;
    args[myArgc] = new char[4];
    strcpy(args[myArgc++],"-sfh");
    for (int i = 0; i < list.size(); ++i)
      {
      QFileInfo fileInfo = list.at(i);
      QString filename = fileInfo.fileName();
      if(filename.size() && filename[0] == '.')
        {
        continue;
        }
      QString file = fileInfo.absoluteFilePath();
      
      args[myArgc] = new char[file.toLocal8Bit().size()+1];
      strcpy(args[myArgc++] , file.toLocal8Bit().data());        
                 
      }
    args[myArgc] = new char[installTo.toLocal8Bit().size()+1];  
    args[myArgc++] = installTo.toLocal8Bit().data();
    args[myArgc] = NULL;
    std::cout << "ln";
    for (int j = 0; j < myArgc; j++) {
      std::cout << " " << args[j];
    }
    std::cout << "\n";
    myFlags = kAuthorizationFlagDefaults;
    
// AuthorizationExecuteWithPrivileges is deprecated 
// see the following link for an alternative that requires code signing
// https://bitbucket.org/sinbad/privilegedhelperexample  
  	myStatus = AuthorizationExecuteWithPrivileges (myAuthorizationRef, 
       "/bin/ln", myFlags, args,  NULL);
    if(myStatus != errAuthorizationSuccess)
      {
      QString message = tr("Failed create symlinks");
      QString title = tr("Error Creating Symlink");
      QMessageBox::StandardButton btn =
        QMessageBox::critical(this, title, message,
                              QMessageBox::Ok|QMessageBox::Abort);
      if(btn == QMessageBox::Abort)
        {
        return;
        }
      }
           
    AuthorizationFree (myAuthorizationRef, kAuthorizationFlagDefaults);
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
