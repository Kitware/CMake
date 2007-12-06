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
#include <QCompleter>
#include <QDirModel>
#include <QSettings>
#include <QMenu>
#include <QMenuBar>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QUrl>

#include "QCMake.h"
#include "QCMakeCacheView.h"
#include "AddCacheEntry.h"

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
  : ExitAfterGenerate(true), CacheModified(false), CurrentState(Interrupting)
{
  // create the GUI
  QSettings settings;
  settings.beginGroup("Settings/StartPath");
  int h = settings.value("Height", 500).toInt();
  int w = settings.value("Width", 700).toInt();
  this->resize(w, h);

  QWidget* cont = new QWidget(this);
  this->setupUi(cont);
  this->Splitter->setStretchFactor(0, 3);
  this->Splitter->setStretchFactor(1, 1);
  this->setCentralWidget(cont);
  this->ProgressBar->reset();
  this->RemoveEntry->setEnabled(false);
  this->AddEntry->setEnabled(false);

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
  // prevent merging with Preferences menu item on Mac OS X
  this->ConfigureAction->setMenuRole(QAction::NoRole);
  QObject::connect(this->ConfigureAction, SIGNAL(triggered(bool)), 
                   this, SLOT(doConfigure()));
  this->GenerateAction = ToolsMenu->addAction(tr("&Generate"));
  QObject::connect(this->GenerateAction, SIGNAL(triggered(bool)), 
                   this, SLOT(doGenerate()));

  QMenu* HelpMenu = this->menuBar()->addMenu(tr("&Help"));
  QAction* a = HelpMenu->addAction(tr("About"));
  QObject::connect(a, SIGNAL(triggered(bool)),
                   this, SLOT(doAbout()));
  a = HelpMenu->addAction(tr("Help"));
  QObject::connect(a, SIGNAL(triggered(bool)),
                   this, SLOT(doHelp()));
  
  this->setAcceptDrops(true);
  
  // get the saved binary directories
  QStringList buildPaths = this->loadBuildPaths();
  this->BinaryDirectory->addItems(buildPaths);
 
  QCompleter* compBinaryDir = new QCompleter(this);
  QDirModel* modelBinaryDir = new QDirModel(compBinaryDir);
  modelBinaryDir->setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
  compBinaryDir->setModel(modelBinaryDir);
  this->BinaryDirectory->setCompleter(compBinaryDir);
  QCompleter* compSourceDir = new QCompleter(this);
  QDirModel* modelSourceDir = new QDirModel(compSourceDir);
  modelSourceDir->setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
  compSourceDir->setModel(modelSourceDir);
  this->SourceDirectory->setCompleter(compSourceDir);


  // start the cmake worker thread
  this->CMakeThread = new QCMakeThread(this);
  QObject::connect(this->CMakeThread, SIGNAL(cmakeInitialized()),
                   this, SLOT(initialize()), Qt::QueuedConnection);  
  this->CMakeThread->start();
  
  this->enterState(ReadyConfigure);
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
                   this, SLOT(onBinaryDirectoryChanged(QString)));
  QObject::connect(this->SourceDirectory, SIGNAL(textChanged(QString)),
                   this, SLOT(onSourceDirectoryChanged(QString)));

  QObject::connect(this->CMakeThread->cmakeInstance(),
                   SIGNAL(sourceDirChanged(QString)),
                   this, SLOT(updateSourceDirectory(QString)));
 
  QObject::connect(this->CMakeThread->cmakeInstance(),
                   SIGNAL(progressChanged(QString, float)),
                   this, SLOT(showProgress(QString,float)));
  
  QObject::connect(this->CMakeThread->cmakeInstance(),
                   SIGNAL(errorMessage(QString)),
                   this, SLOT(error(QString)));

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
                   SIGNAL(dataChanged(QModelIndex,QModelIndex)), 
                   this, SLOT(setCacheModified()));
  
  QObject::connect(this->CacheValues->selectionModel(),
                   SIGNAL(selectionChanged(QItemSelection,QItemSelection)), 
                   this, SLOT(selectionChanged()));
  QObject::connect(this->RemoveEntry, SIGNAL(clicked(bool)), 
                   this, SLOT(removeSelectedCacheEntries()));
  QObject::connect(this->AddEntry, SIGNAL(clicked(bool)), 
                   this, SLOT(addCacheEntry()));

  
  if(!this->SourceDirectory->text().isEmpty() ||
     !this->BinaryDirectory->lineEdit()->text().isEmpty())
    {
    this->onSourceDirectoryChanged(this->SourceDirectory->text());
    this->onBinaryDirectoryChanged(this->BinaryDirectory->lineEdit()->text());
    }
  else
    {
    this->onBinaryDirectoryChanged(this->BinaryDirectory->lineEdit()->text());
    }
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
  if(this->CurrentState == Configuring)
    {
    // stop configure
    doInterrupt();
    return;
    }

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
    
  this->enterState(Configuring);

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
  if(0 == err && 0 == this->CacheValues->cacheModel()->newCount())
    {
    this->enterState(ReadyGenerate);
    }
  else
    {
    this->enterState(ReadyConfigure);
    this->CacheValues->scrollToTop();
    }
  
  if(err != 0)
    {
    QMessageBox::critical(this, tr("Error"), 
      tr("Error in configuration process, project files may be invalid"), 
      QMessageBox::Ok);
    }
}

void CMakeSetupDialog::finishGenerate(int err)
{
  this->enterState(ReadyGenerate);
  if(err != 0)
    {
    QMessageBox::critical(this, tr("Error"), 
      tr("Error in generation process, project files may be invalid"),
      QMessageBox::Ok);
    }
}

void CMakeSetupDialog::doGenerate()
{
  if(this->CurrentState == Generating)
    {
    // stop generate
    doInterrupt();
    return;
    }
  this->enterState(Generating);
  QMetaObject::invokeMethod(this->CMakeThread->cmakeInstance(),
    "generate", Qt::QueuedConnection);
}
  
void CMakeSetupDialog::closeEvent(QCloseEvent* e)
{
  // don't close if we're busy
  if(this->CurrentState == Configuring || this->CurrentState == Generating)
    {
    e->ignore();
    }
  
  // prompt for close if there are unsaved changes
  if(this->CacheModified)
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
    "should click the Generate button.  This will write the build files to the build "
    "directory.");

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
  this->enterState(Interrupting);
  QMetaObject::invokeMethod(this->CMakeThread->cmakeInstance(),
    "interrupt", Qt::QueuedConnection);
}

void CMakeSetupDialog::doSourceBrowse()
{
  QString dir = QFileDialog::getExistingDirectory(this, 
    tr("Enter Path to Source"), this->SourceDirectory->text());
  if(!dir.isEmpty())
    {
    this->setSourceDirectory(dir);
    }
}

void CMakeSetupDialog::updateSourceDirectory(const QString& dir)
{
  if(this->SourceDirectory->text() != dir)
    {
    this->setSourceDirectory(dir);
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
  this->BinaryDirectory->setEditText(dir);
}

void CMakeSetupDialog::onSourceDirectoryChanged(const QString& dir)
{
  QMetaObject::invokeMethod(this->CMakeThread->cmakeInstance(),
    "setSourceDirectory", Qt::QueuedConnection, Q_ARG(QString, dir));
}

void CMakeSetupDialog::onBinaryDirectoryChanged(const QString& dir)
{
  this->CacheModified = false;
  this->CacheValues->cacheModel()->clear();
  this->Output->clear();
  QMetaObject::invokeMethod(this->CMakeThread->cmakeInstance(),
    "setBinaryDirectory", Qt::QueuedConnection, Q_ARG(QString, dir));
}

void CMakeSetupDialog::setSourceDirectory(const QString& dir)
{
  this->SourceDirectory->setText(dir);
}

void CMakeSetupDialog::showProgress(const QString& /*msg*/, float percent)
{
  this->ProgressBar->setValue(qRound(percent * 100));
}
  
void CMakeSetupDialog::error(const QString& message)
{
  this->Output->append(QString("<b><font color=red>%1</font></b>").arg(message));
}

void CMakeSetupDialog::setEnabledState(bool enabled)
{
  // disable parts of the GUI during configure/generate
  this->CacheValues->cacheModel()->setEditEnabled(enabled);
  this->SourceDirectory->setEnabled(enabled);
  this->BrowseSourceDirectoryButton->setEnabled(enabled);
  this->BinaryDirectory->setEnabled(enabled);
  this->BrowseBinaryDirectoryButton->setEnabled(enabled);
  this->ReloadCacheAction->setEnabled(enabled);
  this->DeleteCacheAction->setEnabled(enabled);
  this->ExitAction->setEnabled(enabled);
  this->ConfigureAction->setEnabled(enabled);
  this->AddEntry->setEnabled(enabled);
  this->RemoveEntry->setEnabled(false);  // let selection re-enable it
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
}

void CMakeSetupDialog::addBinaryPath(const QString& path)
{
  QString cleanpath = QDir::cleanPath(path);
  
  // update UI
  this->BinaryDirectory->blockSignals(true);
  int idx = this->BinaryDirectory->findText(cleanpath);
  if(idx != -1)
    {
    this->BinaryDirectory->removeItem(idx);
    }
  this->BinaryDirectory->insertItem(0, cleanpath);
  this->BinaryDirectory->setCurrentIndex(0);
  this->BinaryDirectory->blockSignals(false);
  
  // save to registry
  QStringList buildPaths = this->loadBuildPaths();
  buildPaths.removeAll(cleanpath);
  buildPaths.prepend(cleanpath);
  this->saveBuildPaths(buildPaths);
}

void CMakeSetupDialog::dragEnterEvent(QDragEnterEvent* e)
{
  if(this->CurrentState != ReadyConfigure || 
     this->CurrentState != ReadyGenerate)
    {
    e->ignore();
    return;
    }

  const QMimeData* dat = e->mimeData();
  QList<QUrl> urls = dat->urls();
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
  if(this->CurrentState != ReadyConfigure || 
     this->CurrentState != ReadyGenerate)
    {
    return;
    }

  const QMimeData* dat = e->mimeData();
  QList<QUrl> urls = dat->urls();
  QString file = urls.count() ? urls[0].toLocalFile() : QString();
  if(file.endsWith("CMakeCache.txt", Qt::CaseInsensitive))
    {
    QFileInfo info(file);
    if(this->CMakeThread->cmakeInstance()->binaryDirectory() != info.absolutePath())
      {
      this->setBinaryDirectory(info.absolutePath());
      }
    }
  else if(file.endsWith("CMakeLists.txt", Qt::CaseInsensitive))
    {
    QFileInfo info(file);
    if(this->CMakeThread->cmakeInstance()->binaryDirectory() != info.absolutePath())
      {
      this->setSourceDirectory(info.absolutePath());
      this->setBinaryDirectory(info.absolutePath());
      }
    }
}

QStringList CMakeSetupDialog::loadBuildPaths()
{
  QSettings settings;
  settings.beginGroup("Settings/StartPath");

  QStringList buildPaths;
  for(int i=0; i<10; i++)
    { 
      QString p = settings.value(QString("WhereBuild%1").arg(i)).toString();
      if(!p.isEmpty())
        {
        buildPaths.append(p);
        }
    }
  return buildPaths;
}

void CMakeSetupDialog::saveBuildPaths(const QStringList& paths)
{
  QSettings settings;
  settings.beginGroup("Settings/StartPath");

  int num = paths.count();
  if(num > 10)
    {
    num = 10;
    }

  for(int i=0; i<num; i++)
    { 
    settings.setValue(QString("WhereBuild%1").arg(i), paths[i]);
    }
}
  
void CMakeSetupDialog::setCacheModified()
{
  this->CacheModified = true;
  this->enterState(ReadyConfigure);
}

void CMakeSetupDialog::removeSelectedCacheEntries()
{
  QModelIndexList idxs = this->CacheValues->selectionModel()->selectedRows();
  QList<QPersistentModelIndex> pidxs;
  foreach(QModelIndex i, idxs)
    {
    pidxs.append(i);
    }
  foreach(QPersistentModelIndex pi, pidxs)
    {
    this->CacheValues->model()->removeRow(pi.row());
    }
}

void CMakeSetupDialog::selectionChanged()
{
  QModelIndexList idxs = this->CacheValues->selectionModel()->selectedRows();
  if(idxs.count() && 
      (this->CurrentState == ReadyConfigure || 
       this->CurrentState == ReadyGenerate) )
    {
    this->RemoveEntry->setEnabled(true);
    }
  else
    {
    this->RemoveEntry->setEnabled(false);
    }
}
  
void CMakeSetupDialog::enterState(CMakeSetupDialog::State s)
{
  if(s == this->CurrentState)
    {
    return;
    }

  this->CurrentState = s;

  if(s == Interrupting)
    {
    this->ConfigureButton->setEnabled(false);
    this->GenerateButton->setEnabled(false);
    }
  else if(s == Configuring)
    {
    this->Output->clear();
    this->setEnabledState(false);
    this->GenerateButton->setEnabled(false);
    this->GenerateAction->setEnabled(false);
    this->ConfigureButton->setText(tr("Stop"));
    }
  else if(s == Generating)
    {
    this->CacheModified = false;
    this->Output->clear();
    this->setEnabledState(false);
    this->ConfigureButton->setEnabled(false);
    this->GenerateAction->setEnabled(false);
    this->GenerateButton->setText(tr("Stop"));
    }
  else if(s == ReadyConfigure)
    {
    this->ProgressBar->reset();
    this->setEnabledState(true);
    this->GenerateButton->setEnabled(false);
    this->GenerateAction->setEnabled(false);
    this->ConfigureButton->setEnabled(true);
    this->ConfigureButton->setText(tr("Configure"));
    this->GenerateButton->setText(tr("Generate"));
    }
  else if(s == ReadyGenerate)
    {
    this->ProgressBar->reset();
    this->setEnabledState(true);
    this->GenerateButton->setEnabled(true);
    this->GenerateAction->setEnabled(true);
    this->ConfigureButton->setEnabled(true);
    this->ConfigureButton->setText(tr("Configure"));
    this->GenerateButton->setText(tr("Generate"));
    }
}

void CMakeSetupDialog::addCacheEntry()
{
  QDialog dialog(this);
  dialog.resize(400, 200);
  dialog.setWindowTitle(tr("CMakeSetup Help"));
  QVBoxLayout* l = new QVBoxLayout(&dialog);
  AddCacheEntry* w = new AddCacheEntry(&dialog);
  QDialogButtonBox* btns = new QDialogButtonBox(
      QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
      Qt::Horizontal, &dialog);
  QObject::connect(btns, SIGNAL(accepted()), &dialog, SLOT(accept()));
  QObject::connect(btns, SIGNAL(rejected()), &dialog, SLOT(reject()));
  l->addWidget(w);
  l->addStretch();
  l->addWidget(btns);
  if(QDialog::Accepted == dialog.exec())
    {
    QCMakeCacheModel* m = this->CacheValues->cacheModel();
    m->insertRows(0, 1, QModelIndex());
    m->setData(m->index(0, 0), w->type(), QCMakeCacheModel::TypeRole);
    m->setData(m->index(0, 0), w->name(), Qt::DisplayRole);
    m->setData(m->index(0, 0), w->description(), QCMakeCacheModel::HelpRole);
    m->setData(m->index(0, 0), 0, QCMakeCacheModel::AdvancedRole);
    if(w->type() == QCMakeCacheProperty::BOOL)
      {
      m->setData(m->index(0, 1), w->value().toBool() ? 
          Qt::Checked : Qt::Unchecked, Qt::CheckStateRole);
      }
    else
      {
      m->setData(m->index(0, 1), w->value(), Qt::DisplayRole);
      }
    }
}

