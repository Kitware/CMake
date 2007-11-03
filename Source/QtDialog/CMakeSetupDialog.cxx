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

#include "CMakeSetupDialog.h"

#include <QFileDialog>
#include <QThread>
#include <QProgressBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolButton>
#include <QDialogButtonBox>

#include "QCMake.h"
#include "QCMakeCacheView.h"

// QCMake instance on a thread
class QCMakeThread : public QThread
{
public:
  QCMakeThread(QObject* p) : QThread(p) { }
  QCMake* CMakeInstance;

protected:
  virtual void run()
  {
    this->CMakeInstance = new QCMake;
    this->exec();
    delete this->CMakeInstance;
  }
};

CMakeSetupDialog::CMakeSetupDialog()
{
  // create the GUI
  this->resize(700, 500);
  QWidget* cont = new QWidget(this);
  this->setupUi(cont);
  this->setCentralWidget(cont);
  this->ProgressBar = new QProgressBar();
  this->ProgressBar->setRange(0,100);
  this->InterruptButton = new QToolButton();
  this->InterruptButton->setEnabled(false);
  this->InterruptButton->setIcon(
    this->style()->standardPixmap(QStyle::SP_DialogCancelButton));
  this->statusBar()->addPermanentWidget(this->InterruptButton);
  this->statusBar()->addPermanentWidget(this->ProgressBar);
  
  // start the cmake worker thread
  this->CMakeThread = new QCMakeThread(this);
  // TODO does this guarantee the QCMake instance is created before initialize is called?
  QObject::connect(this->CMakeThread, SIGNAL(started()),
                   this, SLOT(initialize()));  
  this->CMakeThread->start();
}

void CMakeSetupDialog::initialize()
{
  // now the cmake worker thread is running, lets make our connections to it
  QObject::connect(this->CMakeThread->CMakeInstance, 
      SIGNAL(propertiesChanged(const QCMakeCachePropertyList&)),
      this->CacheValues->cacheModel(),
      SLOT(setProperties(const QCMakeCachePropertyList&)));

  QObject::connect(this->ConfigureButton, SIGNAL(clicked(bool)),
                   this, SLOT(doConfigure()));
  QObject::connect(this->CMakeThread->CMakeInstance, SIGNAL(configureDone(int)),
                   this, SLOT(finishConfigure(int)));
  QObject::connect(this->CMakeThread->CMakeInstance, SIGNAL(generateDone(int)),
                   this, SLOT(finishGenerate(int)));

  QObject::connect(this->GenerateButton, SIGNAL(clicked(bool)),
                   this, SLOT(doOk()));
  
  QObject::connect(this->CancelButton, SIGNAL(clicked(bool)),
                   this, SLOT(doCancel()));
  
  QObject::connect(this->BrowseSourceDirectoryButton, SIGNAL(clicked(bool)),
                   this, SLOT(doSourceBrowse()));
  QObject::connect(this->BrowseBinaryDirectoryButton, SIGNAL(clicked(bool)),
                   this, SLOT(doBinaryBrowse()));
  
  QObject::connect(this->BinaryDirectory, SIGNAL(editTextChanged(QString)),
                   this->CMakeThread->CMakeInstance, SLOT(setBinaryDirectory(QString)));

  QObject::connect(this->CMakeThread->CMakeInstance, SIGNAL(sourceDirChanged(QString)),
                   this, SLOT(updateSourceDirectory(QString)));
 
  QObject::connect(this->CMakeThread->CMakeInstance, SIGNAL(progressChanged(QString, float)),
                   this, SLOT(showProgress(QString,float)));
  
  QObject::connect(this->CMakeThread->CMakeInstance, SIGNAL(error(QString, QString, bool*)),
                   this, SLOT(error(QString,QString,bool*)), Qt::BlockingQueuedConnection);

  QObject::connect(this->InterruptButton, SIGNAL(clicked(bool)),
                   this->CMakeThread->CMakeInstance, SLOT(interrupt()));
  QObject::connect(this->InterruptButton, SIGNAL(clicked(bool)),
                   this, SLOT(doInterrupt()));
  
  QObject::connect(this->CMakeThread->CMakeInstance, SIGNAL(outputMessage(QString)),
                   this->Output, SLOT(append(QString)));
  
  QObject::connect(this->HelpButton, SIGNAL(clicked(bool)),
                   this, SLOT(doHelp()));
}

CMakeSetupDialog::~CMakeSetupDialog()
{
  // wait for thread to stop
  this->CMakeThread->quit();
  this->CMakeThread->wait();
}
  
void CMakeSetupDialog::doConfigure()
{
  this->InterruptButton->setEnabled(true);
  this->setEnabledState(false);
  this->Output->clear();
  QMetaObject::invokeMethod(this->CMakeThread->CMakeInstance,
    "setProperties", Qt::QueuedConnection, 
    Q_ARG(QCMakeCachePropertyList,
      this->CacheValues->cacheModel()->properties()));
  QMetaObject::invokeMethod(this->CMakeThread->CMakeInstance,
    "configure", Qt::QueuedConnection);
}

void CMakeSetupDialog::finishConfigure(int error)
{
  this->InterruptButton->setEnabled(false);
  this->setEnabledState(true);
  this->ProgressBar->reset();
  this->statusBar()->showMessage(tr("Configure Done"), 2000);
  if(error != 0)
    {
    QMessageBox::critical(this, tr("Error"), 
      tr("Error in configuration process, project files may be invalid"), 
      QMessageBox::Ok);
    }
}

void CMakeSetupDialog::finishGenerate(int error)
{
  this->InterruptButton->setEnabled(false);
  this->setEnabledState(true);
  this->ProgressBar->reset();
  this->statusBar()->showMessage(tr("Generate Done"), 2000);
  if(error != 0)
    {
    QMessageBox::critical(this, tr("Error"), 
      tr("Error in generation process, project files may be invalid"),
      QMessageBox::Ok);
    }
  else
    {
    QApplication::quit();
    }
}

void CMakeSetupDialog::doOk()
{
  this->InterruptButton->setEnabled(true);
  this->setEnabledState(false);
  this->Output->clear();
  QMetaObject::invokeMethod(this->CMakeThread->CMakeInstance,
    "generate", Qt::QueuedConnection);
}

void CMakeSetupDialog::doCancel()
{
  if(this->CacheValues->cacheModel()->isDirty())
    {
    QString message = tr("You have changed options but not rebuilt, "
                    "are you sure you want to exit?");
    QString title = tr("Confirm Exit");
    QMessageBox::StandardButton btn =
      QMessageBox::critical(this, title, message, QMessageBox::Ok | QMessageBox::Cancel);
    if(btn == QMessageBox::Cancel)
      {
      return;
      }
    }

  QApplication::quit();
}

void CMakeSetupDialog::doHelp()
{
  QString msg = tr("CMake is used to configure and generate build files for"
    "software projects.   The basic steps for configuring a project are as"
    "follows:\r\n\r\n1. Select the source directory for the project.  This should"
    "contain the CMakeLists.txt files for the project.\r\n\r\n2. Select the build"
    "directory for the project.   This is the directory where the project will be"
    "built.  It can be the same or a different directory than the source"
    "directory.   For easy clean up, a separate build directory is recommended."
    "CMake will create the directory if it does not exist.\r\n\r\n3. Once the"
    "source and binary directories are selected, it is time to press the"
    "Configure button.  This will cause CMake to read all of the input files and"
    "discover all the variables used by the project.   The first time a variable"
    "is displayed it will be in Red.   Users should inspect red variables making"
    "sure the values are correct.   For some projects the Configure process can"
    "be iterative, so continue to press the Configure button until there are no"
    "longer red entries.\r\n\r\n4. Once there are no longer red entries, you"
    "should click the OK button.  This will write the build files to the build"
    "directory and exit CMake.");

  QDialog dialog;
  QVBoxLayout* l = new QVBoxLayout(&dialog);
  QLabel* label = new QLabel(&dialog);
  l->addWidget(label);
  label->setText(msg);
  label->setWordWrap(true);
  QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok,
                                                Qt::Horizontal, &dialog);
  QObject::connect(btns, SIGNAL(accepted()), &dialog, SLOT(accept()));
  l->addWidget(btns);
  dialog.exec();
}

void CMakeSetupDialog::doInterrupt()
{
  this->InterruptButton->setEnabled(false);
  this->statusBar()->showMessage(tr("Interrupting..."));
}

void CMakeSetupDialog::doSourceBrowse()
{
  QString dir = QFileDialog::getExistingDirectory(this, 
    tr("Enter Path to Source"), this->SourceDirectory->text());
  if(!dir.isEmpty())
    {
    this->updateSourceDirectory(dir);
    }
}

void CMakeSetupDialog::updateSourceDirectory(const QString& dir)
{
  this->SourceDirectory->setText(dir);
}

void CMakeSetupDialog::doBinaryBrowse()
{
  QString dir = QFileDialog::getExistingDirectory(this, 
    tr("Enter Path to Build"), this->BinaryDirectory->currentText());
  if(!dir.isEmpty())
    {
    this->setBinaryDirectory(dir);
    }
}

void CMakeSetupDialog::setBinaryDirectory(const QString& dir)
{
  if(dir != this->BinaryDirectory->currentText())
    {
    this->Output->clear();
    this->BinaryDirectory->setEditText(dir);
    }
}

void CMakeSetupDialog::showProgress(const QString& msg, float percent)
{
  this->statusBar()->showMessage(msg);
  this->ProgressBar->setValue(qRound(percent * 100));
}
  
void CMakeSetupDialog::error(const QString& title, const QString& message, bool* cancel)
{
  QMessageBox::StandardButton btn =
    QMessageBox::critical(this, title, message, QMessageBox::Ok | QMessageBox::Cancel);
  if(btn == QMessageBox::Cancel)
    {
    *cancel = false;
    }
}

void CMakeSetupDialog::setEnabledState(bool enabled)
{
  this->CacheValues->setEnabled(enabled);
  this->SourceDirectory->setEnabled(enabled);
  this->BrowseSourceDirectoryButton->setEnabled(enabled);
  this->BinaryDirectory->setEnabled(enabled);
  this->BrowseBinaryDirectoryButton->setEnabled(enabled);
  this->ConfigureButton->setEnabled(enabled);
  this->GenerateButton->setEnabled(enabled);
  this->CancelButton->setEnabled(enabled);
  this->HelpButton->setEnabled(enabled);
}


