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
#include <QProgressBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolButton>
#include <QDialogButtonBox>
#include <QCloseEvent>
#include <QCoreApplication>
#include <QSettings>
#include <QMenu>
#include <QMenuBar>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>

#include "QCMake.h"
#include "QCMakeCacheView.h"

QCMakeThread::QCMakeThread(QObject* p) 
  : QThread(p), CMakeInstance(NULL)
{
}

QCMake* QCMakeThread::cmakeInstance() const
{
  return this->CMakeInstance;
}

void QCMakeThread::processEvents()
{
  QCoreApplication::processEvents();
}

void QCMakeThread::run()
{
  this->CMakeInstance = new QCMake;
  // make the cmake thread to process events it receives from the GUI thread
  QObject::connect(this->CMakeInstance, SIGNAL(progressChanged(QString, float)),
                   this, SLOT(processEvents()), Qt::DirectConnection);
  QObject::connect(this->CMakeInstance, SIGNAL(outputMessage(QString)),
                   this, SLOT(processEvents()), Qt::DirectConnection);
  // emit that this cmake thread is ready for use
  emit this->cmakeInitialized();
  this->exec();
  delete this->CMakeInstance;
  this->CMakeInstance = NULL;
}

CMakeSetupDialog::CMakeSetupDialog()
  : ExitAfterGenerate(true)
{
  // create the GUI
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  int h = settings.value("Height", 500).toInt();
  int w = settings.value("Width", 700).toInt();
  this->resize(w, h);

  QWidget* cont = new QWidget(this);
  this->setupUi(cont);
  this->Splitter->setStretchFactor(0, 2);
  this->Splitter->setStretchFactor(1, 1);
  this->setCentralWidget(cont);
  this->ProgressBar = new QProgressBar();
  this->ProgressBar->setRange(0,100);
  this->InterruptButton = new QToolButton();
  this->InterruptButton->setEnabled(false);
  this->InterruptButton->setIcon(
    this->style()->standardPixmap(QStyle::SP_DialogCancelButton));
  this->statusBar()->addPermanentWidget(this->InterruptButton);
  this->statusBar()->addPermanentWidget(this->ProgressBar);

  QMenu* FileMenu = this->menuBar()->addMenu(tr("&File"));
  this->ReloadCacheAction = FileMenu->addAction(tr("&Reload Cache"));
  QObject::connect(this->ReloadCacheAction, SIGNAL(triggered(bool)), 
                   this, SLOT(doReloadCache()));
  this->DeleteCacheAction = FileMenu->addAction(tr("&Delete Cache"));
  QObject::connect(this->DeleteCacheAction, SIGNAL(triggered(bool)), 
                   this, SLOT(doDeleteCache()));
  this->ExitAction = FileMenu->addAction(tr("&Exit"));
  QObject::connect(this->ExitAction, SIGNAL(triggered(bool)), 
                   this, SLOT(close()));

  QMenu* ToolsMenu = this->menuBar()->addMenu(tr("&Tools"));
  this->ConfigureAction = ToolsMenu->addAction(tr("&Configure"));
  QObject::connect(this->ConfigureAction, SIGNAL(triggered(bool)), 
                   this, SLOT(doConfigure()));
  this->GenerateAction = ToolsMenu->addAction(tr("&Generate"));
  QObject::connect(this->GenerateAction, SIGNAL(triggered(bool)), 
                   this, SLOT(doGenerate()));
  
  QMenu* OptionsMenu = this->menuBar()->addMenu(tr("&Options"));
  QAction* a = OptionsMenu->addAction(tr("Exit after Generation"));
  a->setCheckable(true);
  this->ExitAfterGenerate = settings.value("ExitAfterGenerate", true).toBool();
  a->setChecked(this->ExitAfterGenerate);
  QObject::connect(a, SIGNAL(triggered(bool)),
                   this, SLOT(setExitAfterGenerate(bool)));

  QMenu* HelpMenu = this->menuBar()->addMenu(tr("&Help"));
  a = HelpMenu->addAction(tr("About"));
  QObject::connect(a, SIGNAL(triggered(bool)),
                   this, SLOT(doAbout()));
  a = HelpMenu->addAction(tr("Help"));
  QObject::connect(a, SIGNAL(triggered(bool)),
                   this, SLOT(doHelp()));
  
  this->setGenerateEnabled(false);

  this->setAcceptDrops(true);

  // start the cmake worker thread
  this->CMakeThread = new QCMakeThread(this);
  QObject::connect(this->CMakeThread, SIGNAL(cmakeInitialized()),
                   this, SLOT(initialize()), Qt::QueuedConnection);  
  this->CMakeThread->start();
}

void CMakeSetupDialog::initialize()
{
  // now the cmake worker thread is running, lets make our connections to it
  QObject::connect(this->CMakeThread->cmakeInstance(), 
      SIGNAL(propertiesChanged(const QCMakeCachePropertyList&)),
      this->CacheValues->cacheModel(),
      SLOT(setProperties(const QCMakeCachePropertyList&)));

  QObject::connect(this->ConfigureButton, SIGNAL(clicked(bool)),
                   this, SLOT(doConfigure()));
  QObject::connect(this->CMakeThread->cmakeInstance(), 
                   SIGNAL(configureDone(int)),
                   this, SLOT(finishConfigure(int)));
  QObject::connect(this->CMakeThread->cmakeInstance(),
                   SIGNAL(generateDone(int)),
                   this, SLOT(finishGenerate(int)));

  QObject::connect(this->GenerateButton, SIGNAL(clicked(bool)),
                   this, SLOT(doGenerate()));
  
  QObject::connect(this->BrowseSourceDirectoryButton, SIGNAL(clicked(bool)),
                   this, SLOT(doSourceBrowse()));
  QObject::connect(this->BrowseBinaryDirectoryButton, SIGNAL(clicked(bool)),
                   this, SLOT(doBinaryBrowse()));
  
  QObject::connect(this->BinaryDirectory, SIGNAL(editTextChanged(QString)),
                   this, SLOT(setBinaryDirectory(QString)));
  QObject::connect(this->SourceDirectory, SIGNAL(textChanged(QString)),
                   this->CMakeThread->cmakeInstance(),
                   SLOT(setSourceDirectory(QString)));

  QObject::connect(this->CMakeThread->cmakeInstance(),
                   SIGNAL(sourceDirChanged(QString)),
                   this, SLOT(updateSourceDirectory(QString)));
 
  QObject::connect(this->CMakeThread->cmakeInstance(),
                   SIGNAL(progressChanged(QString, float)),
                   this, SLOT(showProgress(QString,float)));
  
  QObject::connect(this->CMakeThread->cmakeInstance(),
                   SIGNAL(error(QString, QString, bool*)),
                   this, SLOT(error(QString,QString,bool*)),
                   Qt::BlockingQueuedConnection);

  QObject::connect(this->InterruptButton, SIGNAL(clicked(bool)),
                   this->CMakeThread->cmakeInstance(), SLOT(interrupt()));
  QObject::connect(this->InterruptButton, SIGNAL(clicked(bool)),
                   this, SLOT(doInterrupt()));
  
  QObject::connect(this->CMakeThread->cmakeInstance(),
                   SIGNAL(outputMessage(QString)),
                   this->Output, SLOT(append(QString)));

  QObject::connect(this->Advanced, SIGNAL(clicked(bool)), 
                   this->CacheValues, SLOT(setShowAdvanced(bool)));
  QObject::connect(this->Search, SIGNAL(textChanged(QString)), 
                   this->CacheValues, SLOT(setSearchFilter(QString)));
  
  QObject::connect(this->CMakeThread->cmakeInstance(),
                   SIGNAL(generatorChanged(QString)),
                   this, SLOT(updateGeneratorLabel(QString)));
  this->updateGeneratorLabel(QString());
  
  QObject::connect(this->CacheValues->cacheModel(),
                   SIGNAL(dataChanged(QModelIndex, QModelIndex)),
                   this, SLOT(cacheModelDirty()));
  QObject::connect(this->CacheValues->cacheModel(), SIGNAL(modelReset()),
                   this, SLOT(cacheModelDirty()));

  // get the saved binary directories
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  QStringList buildPaths = settings.value("WhereBuild").toStringList();
  this->BinaryDirectory->addItems(buildPaths);
}

CMakeSetupDialog::~CMakeSetupDialog()
{
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  settings.setValue("Height", this->height());
  settings.setValue("Width", this->width());

  // wait for thread to stop
  this->CMakeThread->quit();
  this->CMakeThread->wait();
}
  
void CMakeSetupDialog::doConfigure()
{
  QString bindir = this->CMakeThread->cmakeInstance()->binaryDirectory();
  QDir dir(bindir);
  if(!dir.exists())
    {
    QString message = tr("Build directory does not exist, "
                         "should I create it?\n\n"
                         "Directory: ");
    message += bindir;
    QString title = tr("Create Directory");
    QMessageBox::StandardButton btn;
    btn = QMessageBox::information(this, title, message, 
                                   QMessageBox::Yes | QMessageBox::No);
    if(btn == QMessageBox::No)
      {
      return;
      }
    dir.mkpath(".");
    }

  // prompt for generator if one doesn't exist
  if(this->CMakeThread->cmakeInstance()->generator().isEmpty())
    {
    this->promptForGenerator();
    }

  // remember path
  this->addBinaryPath(dir.absolutePath());

  this->InterruptButton->setEnabled(true);
  this->setEnabledState(false);
  this->Output->clear();
  QMetaObject::invokeMethod(this->CMakeThread->cmakeInstance(),
    "setProperties", Qt::QueuedConnection, 
    Q_ARG(QCMakeCachePropertyList,
      this->CacheValues->cacheModel()->properties()));
  QMetaObject::invokeMethod(this->CMakeThread->cmakeInstance(),
    "configure", Qt::QueuedConnection);
}

void CMakeSetupDialog::finishConfigure(int err)
{
  this->InterruptButton->setEnabled(false);
  this->setEnabledState(true);
  this->ProgressBar->reset();
  this->statusBar()->showMessage(tr("Configure Done"), 2000);
  if(err != 0)
    {
    QMessageBox::critical(this, tr("Error"), 
      tr("Error in configuration process, project files may be invalid"), 
      QMessageBox::Ok);
    }
  if(!this->CacheValues->cacheModel()->modifiedValues())
    {
    this->setGenerateEnabled(true);
    }
}

void CMakeSetupDialog::finishGenerate(int err)
{
  this->InterruptButton->setEnabled(false);
  this->setEnabledState(true);
  this->setGenerateEnabled(true);
  this->ProgressBar->reset();
  this->statusBar()->showMessage(tr("Generate Done"), 2000);
  if(err != 0)
    {
    QMessageBox::critical(this, tr("Error"), 
      tr("Error in generation process, project files may be invalid"),
      QMessageBox::Ok);
    }
  else if(this->ExitAfterGenerate)
    {
    QApplication::quit();
    }
}

void CMakeSetupDialog::doGenerate()
{
  this->InterruptButton->setEnabled(true);
  this->setEnabledState(false);
  this->setGenerateEnabled(false);
  this->Output->clear();
  QMetaObject::invokeMethod(this->CMakeThread->cmakeInstance(),
    "generate", Qt::QueuedConnection);
}
  
void CMakeSetupDialog::closeEvent(QCloseEvent* e)
{
  // don't close if we're busy
  if(this->InterruptButton->isEnabled())
    {
    e->ignore();
    }
  
  // prompt for close if there are unsaved changes
  if(this->CacheValues->cacheModel()->modifiedValues())
    {
    QString message = tr("You have changed options but not rebuilt, "
                    "are you sure you want to exit?");
    QString title = tr("Confirm Exit");
    QMessageBox::StandardButton btn;
    btn = QMessageBox::critical(this, title, message,
                                QMessageBox::Yes | QMessageBox::No);
    if(btn == QMessageBox::No)
      {
      e->ignore();
      }
    }
}

void CMakeSetupDialog::doHelp()
{
  QString msg = tr("CMake is used to configure and generate build files for "
    "software projects.   The basic steps for configuring a project are as "
    "follows:\r\n\r\n1. Select the source directory for the project.  This should "
    "contain the CMakeLists.txt files for the project.\r\n\r\n2. Select the build "
    "directory for the project.   This is the directory where the project will be "
    "built.  It can be the same or a different directory than the source "
    "directory.   For easy clean up, a separate build directory is recommended. "
    "CMake will create the directory if it does not exist.\r\n\r\n3. Once the "
    "source and binary directories are selected, it is time to press the "
    "Configure button.  This will cause CMake to read all of the input files and "
    "discover all the variables used by the project.   The first time a variable "
    "is displayed it will be in Red.   Users should inspect red variables making "
    "sure the values are correct.   For some projects the Configure process can "
    "be iterative, so continue to press the Configure button until there are no "
    "longer red entries.\r\n\r\n4. Once there are no longer red entries, you "
    "should click the OK button.  This will write the build files to the build "
    "directory and exit CMake.");

  QDialog dialog;
  dialog.setWindowTitle(tr("CMakeSetup Help"));
  QVBoxLayout* l = new QVBoxLayout(&dialog);
  QLabel* lab = new QLabel(&dialog);
  l->addWidget(lab);
  lab->setText(msg);
  lab->setWordWrap(true);
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
    this->SourceDirectory->setText(dir);
    }
}

void CMakeSetupDialog::updateSourceDirectory(const QString& dir)
{
  if(this->SourceDirectory->text() != dir)
    {
    this->SourceDirectory->setText(dir);
    }
}

void CMakeSetupDialog::doBinaryBrowse()
{
  QString dir = QFileDialog::getExistingDirectory(this, 
    tr("Enter Path to Build"), this->BinaryDirectory->currentText());
  if(!dir.isEmpty() && dir != this->BinaryDirectory->currentText())
    {
    this->setBinaryDirectory(dir);
    }
}

void CMakeSetupDialog::setBinaryDirectory(const QString& dir)
{
  this->CacheValues->cacheModel()->clear();
  this->Output->clear();
  this->BinaryDirectory->setEditText(dir);
  QMetaObject::invokeMethod(this->CMakeThread->cmakeInstance(),
    "setBinaryDirectory", Qt::QueuedConnection, Q_ARG(QString, dir));
}

void CMakeSetupDialog::showProgress(const QString& msg, float percent)
{
  this->statusBar()->showMessage(msg);
  this->ProgressBar->setValue(qRound(percent * 100));
}
  
void CMakeSetupDialog::error(const QString& title, const QString& message, 
                             bool* cancel)
{
  QMessageBox::StandardButton btn;
  QString msg = message + "\n\n" + 
    tr("(Press cancel to suppress any further messages.)");
  btn = QMessageBox::critical(this, title, msg,
                              QMessageBox::Ok | QMessageBox::Cancel);
  if(btn == QMessageBox::Cancel)
    {
    *cancel = false;
    }
}

void CMakeSetupDialog::setEnabledState(bool enabled)
{
  // disable parts of the GUI during configure/generate
  this->CacheValues->setEnabled(enabled);
  this->SourceDirectory->setEnabled(enabled);
  this->BrowseSourceDirectoryButton->setEnabled(enabled);
  this->BinaryDirectory->setEnabled(enabled);
  this->BrowseBinaryDirectoryButton->setEnabled(enabled);
  this->ConfigureButton->setEnabled(enabled);
  this->ReloadCacheAction->setEnabled(enabled);
  this->DeleteCacheAction->setEnabled(enabled);
  this->ExitAction->setEnabled(enabled);
  this->ConfigureAction->setEnabled(enabled);
  // generate button/action are handled separately
}

void CMakeSetupDialog::promptForGenerator()
{
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  QString lastGen = settings.value("LastGenerator").toString();

  QStringList gens = this->CMakeThread->cmakeInstance()->availableGenerators();
  QDialog dialog;
  dialog.setWindowTitle(tr("CMakeSetup choose generator"));
  QLabel* lab = new QLabel(&dialog);
  lab->setText(tr("Please select what build system you want CMake to generate files for.\n"
                    "You should select the tool that you will use to build the project.\n"
                    "Press OK once you have made your selection."));
  QComboBox* combo = new QComboBox(&dialog);
  combo->addItems(gens);
  int idx = combo->findText(lastGen);
  if(idx != -1)
    {
    combo->setCurrentIndex(idx);
    }
  QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok,
                                                Qt::Horizontal, &dialog);
  QObject::connect(btns, SIGNAL(accepted()), &dialog, SLOT(accept()));
  
  QVBoxLayout* l = new QVBoxLayout(&dialog);
  l->addWidget(lab);
  l->addWidget(combo);
  l->addWidget(btns);
  dialog.exec();
  
  lastGen = combo->currentText();
  settings.setValue("LastGenerator", lastGen);
  this->CMakeThread->cmakeInstance()->setGenerator(combo->currentText());
}

void CMakeSetupDialog::updateGeneratorLabel(const QString& gen)
{
  QString str = tr("Current Generator: ");
  if(gen.isEmpty())
    {
    str += tr("None");
    }
  else
    {
    str += gen;
    }
  this->Generator->setText(str);
}

void CMakeSetupDialog::doReloadCache()
{
  QMetaObject::invokeMethod(this->CMakeThread->cmakeInstance(),
    "reloadCache", Qt::QueuedConnection);
}

void CMakeSetupDialog::doDeleteCache()
{
  QMetaObject::invokeMethod(this->CMakeThread->cmakeInstance(),
    "deleteCache", Qt::QueuedConnection);
}

void CMakeSetupDialog::doAbout()
{
  QString msg = "CMakeSetup\nwww.cmake.org";

  QDialog dialog;
  dialog.setWindowTitle(tr("About CMakeSetup"));
  QVBoxLayout* l = new QVBoxLayout(&dialog);
  QLabel* lab = new QLabel(&dialog);
  l->addWidget(lab);
  lab->setText(msg);
  lab->setWordWrap(true);
  QDialogButtonBox* btns = new QDialogButtonBox(QDialogButtonBox::Ok,
                                                Qt::Horizontal, &dialog);
  QObject::connect(btns, SIGNAL(accepted()), &dialog, SLOT(accept()));
  l->addWidget(btns);
  dialog.exec();
}

void CMakeSetupDialog::setExitAfterGenerate(bool b)
{
  this->ExitAfterGenerate = b;
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  settings.setValue("ExitAfterGenerate", b);
}

void CMakeSetupDialog::cacheModelDirty()
{
  if(this->CacheValues->cacheModel()->modifiedValues())
    {
    this->setGenerateEnabled(false);
    }
}

void CMakeSetupDialog::setGenerateEnabled(bool b)
{
  this->GenerateButton->setEnabled(b);
  this->GenerateAction->setEnabled(b);
}

void CMakeSetupDialog::addBinaryPath(const QString& path)
{
  QString cleanpath = QDir::cleanPath(path);
  
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  QStringList buildPaths = settings.value("WhereBuild").toStringList();
  buildPaths.removeAll(cleanpath);
  int idx = this->BinaryDirectory->findText(cleanpath);
  if(idx != -1)
    {
    this->BinaryDirectory->removeItem(idx);
    }
  this->BinaryDirectory->insertItem(0, cleanpath);
  this->BinaryDirectory->setCurrentIndex(0);
  buildPaths.prepend(cleanpath);
  while(buildPaths.count() > 10)
    {
    buildPaths.removeLast();
    }
  settings.setValue("WhereBuild", buildPaths);
}

void CMakeSetupDialog::dragEnterEvent(QDragEnterEvent* e)
{
  const QMimeData* data = e->mimeData();
  QList<QUrl> urls = data->urls();
  QString file = urls.count() ? urls[0].toLocalFile() : QString();
  if(!file.isEmpty() && 
    (file.endsWith("CMakeCache.txt", Qt::CaseInsensitive) ||
    file.endsWith("CMakeLists.txt", Qt::CaseInsensitive) ) )
    {
    e->accept();
    }
  else
    {
    e->ignore();
    }
}

void CMakeSetupDialog::dropEvent(QDropEvent* e)
{
  const QMimeData* data = e->mimeData();
  QList<QUrl> urls = data->urls();
  QString file = urls.count() ? urls[0].toLocalFile() : QString();
  if(file.endsWith("CMakeCache.txt", Qt::CaseInsensitive))
    {
    QFileInfo info(file);
    this->setBinaryDirectory(info.absolutePath());
    }
  else if(file.endsWith("CMakeLists.txt", Qt::CaseInsensitive))
    {
    QFileInfo info(file);
    this->SourceDirectory->setText(info.absolutePath());
    this->setBinaryDirectory(info.absolutePath());
    }
}

