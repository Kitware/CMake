
#include "CMakeSetupDialog.h"

#include <QFileDialog>
#include <QThread>
#include <QProgressBar>
#include <QMessageBox>

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
  this->setupUi(this);
  this->ProgressBar = new QProgressBar();
  this->ProgressBar->setRange(0,100);
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
  QObject::connect(this,
      SIGNAL(propertiesChanged(const QCMakeCachePropertyList&)),
      this->CMakeThread->CMakeInstance,
      SLOT(setProperties(const QCMakeCachePropertyList&)));

  QObject::connect(this->configureButton, SIGNAL(clicked(bool)),
                   this, SLOT(doConfigure()));
  QObject::connect(this, SIGNAL(configure()),
                   this->CMakeThread->CMakeInstance, SLOT(configure()));
  QObject::connect(this->CMakeThread->CMakeInstance, SIGNAL(configureDone(int)),
                   this, SLOT(finishConfigure(int)));
  QObject::connect(this->CMakeThread->CMakeInstance, SIGNAL(generateDone(int)),
                   this, SLOT(finishGenerate(int)));

  QObject::connect(this->generateButton, SIGNAL(clicked(bool)),
                   this, SLOT(doOk()));
  QObject::connect(this, SIGNAL(ok()),
                   this->CMakeThread->CMakeInstance, SLOT(generate()));
  
  QObject::connect(this->cancelButton, SIGNAL(clicked(bool)),
                   this, SLOT(doCancel()));
  QObject::connect(this, SIGNAL(cancel()),
                   this->CMakeThread->CMakeInstance, SLOT(interrupt()));
  
  QObject::connect(this->BrowseSourceDirectoryButton, SIGNAL(clicked(bool)),
                   this, SLOT(doSourceBrowse()));
  QObject::connect(this->BrowseBinaryDirectoryButton, SIGNAL(clicked(bool)),
                   this, SLOT(doBinaryBrowse()));
  
  QObject::connect(this->BinaryDirectory, SIGNAL(textChanged(QString)),
                   this->CMakeThread->CMakeInstance, SLOT(setBinaryDirectory(QString)));

  QObject::connect(this->CMakeThread->CMakeInstance, SIGNAL(sourceDirChanged(QString)),
                   this, SLOT(updateSourceDirectory(QString)));
 
  QObject::connect(this->CMakeThread->CMakeInstance, SIGNAL(progressChanged(QString, float)),
                   this, SLOT(showProgress(QString,float)));
  
  QObject::connect(this->CMakeThread->CMakeInstance, SIGNAL(error(QString, QString, bool*)),
                   this, SLOT(error(QString,QString,bool*)), Qt::BlockingQueuedConnection);

}

CMakeSetupDialog::~CMakeSetupDialog()
{
  // wait for thread to stop
  this->CMakeThread->quit();
  this->CMakeThread->wait();
}
  
void CMakeSetupDialog::doConfigure()
{
  emit this->propertiesChanged(this->CacheValues->cacheModel()->properties());
  emit this->configure();
}

void CMakeSetupDialog::finishConfigure(int error)
{
  this->ProgressBar->reset();
  this->statusBar()->showMessage("Configure Done", 2000);
  if(error != 0)
  {
    bool dummy;
    this->error("Error", "Error in configuration process, project files may be invalid", &dummy);
  }
}

void CMakeSetupDialog::finishGenerate(int error)
{
  this->ProgressBar->reset();
  this->statusBar()->showMessage("Generate Done", 2000);
  if(error != 0)
  {
    bool dummy;
    this->error("Error", "Error in generation process, project files may be invalid", &dummy);
  }
}

void CMakeSetupDialog::doOk()
{
  emit this->ok();
}

void CMakeSetupDialog::doCancel()
{
  emit this->cancel();
}

void CMakeSetupDialog::doHelp()
{
}

void CMakeSetupDialog::doSourceBrowse()
{
  QString dir = QFileDialog::getExistingDirectory(this, "TODO", this->SourceDirectory->text());
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
  QString dir = QFileDialog::getExistingDirectory(this, "TODO", this->BinaryDirectory->currentText());
  if(!dir.isEmpty())
    {
    this->setBinaryDirectory(dir);
    }
}

void CMakeSetupDialog::setBinaryDirectory(const QString& dir)
{
  if(dir != this->BinaryDirectory->currentText())
  {
    this->BinaryDirectory->setEditText(dir);
  }
}

void CMakeSetupDialog::showProgress(const QString& msg, float percent)
{
  if(percent >= 0)
  {
    this->statusBar()->showMessage(msg);
    this->ProgressBar->setValue(qRound(percent * 100));
  }
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


