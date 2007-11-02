
#include "QCMake.h"

#include <QCoreApplication>
#include <QDir>

#include "cmake.h"
#include "cmCacheManager.h"
#include "cmSystemTools.h"

QCMake::QCMake(QObject* p)
  : QObject(p)
{
  static int metaId = qRegisterMetaType<QCMakeCacheProperty>();
  static int metaIdList = qRegisterMetaType<QCMakeCachePropertyList>();
  
  QDir appDir(QCoreApplication::applicationDirPath());
#if defined(Q_OS_WIN)
  this->CMakeExecutable = appDir.filePath("cmake.exe");
#elif defined(Q_OS_MAC)
# error "need to implement for Mac OS X"
#else
  this->CMakeExecutable = appDir.filePath("cmake");
#endif
  // TODO: check for existence?

  cmSystemTools::DisableRunCommandOutput();
  cmSystemTools::SetRunCommandHideConsole(true);
  cmSystemTools::SetErrorCallback(QCMake::errorCallback, this);

  this->CMakeInstance = new cmake;
  this->CMakeInstance->SetProgressCallback(QCMake::progressCallback, this);
}

QCMake::~QCMake()
{
  delete this->CMakeInstance;
  //cmDynamicLoader::FlushCache();
}

void QCMake::loadCache(const QString& dir)
{
  this->setBinaryDirectory(dir);
}

void QCMake::setSourceDirectory(const QString& dir)
{
  this->SourceDirectory = dir;
  emit this->sourceDirChanged(dir);
}

void QCMake::setBinaryDirectory(const QString& dir)
{
  cmCacheManager *cachem = this->CMakeInstance->GetCacheManager();
  this->BinaryDirectory = dir;
  this->CMakeInstance->GetCacheManager()->LoadCache(dir.toLocal8Bit().data());
  QCMakeCachePropertyList props = this->properties();
  emit this->propertiesChanged(props);
  cmCacheManager::CacheIterator itm = cachem->NewIterator();
  if ( itm.Find("CMAKE_HOME_DIRECTORY"))
    {
    setSourceDirectory(itm.GetValue());
    }
}


void QCMake::setGenerator(const QString& generator)
{
}

void QCMake::configure()
{
  this->CMakeInstance->SetHomeDirectory(this->SourceDirectory.toAscii().data());
  this->CMakeInstance->SetStartDirectory(this->SourceDirectory.toAscii().data());
  this->CMakeInstance->SetHomeOutputDirectory(this->BinaryDirectory.toAscii().data());
  this->CMakeInstance->SetStartOutputDirectory(this->BinaryDirectory.toAscii().data());
  this->CMakeInstance->SetGlobalGenerator(
    this->CMakeInstance->CreateGlobalGenerator("Unix Makefiles"));  // TODO
  this->CMakeInstance->SetCMakeCommand(this->CMakeExecutable.toAscii().data());
  this->CMakeInstance->LoadCache();

  cmSystemTools::ResetErrorOccuredFlag();

  int error = this->CMakeInstance->Configure();

  emit this->propertiesChanged(this->properties());
  emit this->configureDone(error);
}

void QCMake::generate()
{
  cmSystemTools::ResetErrorOccuredFlag();
  int error = this->CMakeInstance->Generate();
  emit this->generateDone(error);
}
  
void QCMake::setProperties(const QCMakeCachePropertyList& props)
{
  cmCacheManager *cachem = this->CMakeInstance->GetCacheManager();
  cmCacheManager::CacheIterator it = cachem->NewIterator();
  foreach(QCMakeCacheProperty prop, props)
    {
    if ( it.Find(prop.Key.toAscii().data()) )
      {
      it.SetValue(prop.Value.toAscii().data());
      }
    }
}

QCMakeCachePropertyList QCMake::properties()
{
  QCMakeCachePropertyList ret;

  cmCacheManager *cachem = this->CMakeInstance->GetCacheManager();
  for(cmCacheManager::CacheIterator i = cachem->NewIterator();
      !i.IsAtEnd(); i.Next())
    {

    if(i.GetType() == cmCacheManager::INTERNAL ||
       i.GetType() == cmCacheManager::STATIC)
      {
      continue;
      }

    QCMakeCacheProperty prop;
    prop.Key = i.GetName();
    prop.Help = i.GetProperty("HELPSTRING");
    prop.Value = i.GetValue();
    prop.Advanced = i.GetPropertyAsBool("ADVANCED");

    if(i.GetType() == cmCacheManager::BOOL)
      {
      prop.Type = QCMakeCacheProperty::BOOL;
      if(cmSystemTools::IsOn(prop.Value.toAscii().data()))
        {
        prop.Value = QString("ON");
        }
      else
        {
        prop.Value = QString("OFF");
        }
      }
    else if(i.GetType() == cmCacheManager::PATH)
      {
      prop.Type = QCMakeCacheProperty::PATH;
      }
    else if(i.GetType() == cmCacheManager::FILEPATH)
      {
      prop.Type = QCMakeCacheProperty::FILEPATH;
      }
    else if(i.GetType() == cmCacheManager::STRING)
      {
      prop.Type = QCMakeCacheProperty::STRING;
      }

    ret.append(prop);
    }

  return ret;
}
  
void QCMake::interrupt()
{
  cmSystemTools::SetFatalErrorOccured();
}

void QCMake::progressCallback(const char* msg, float percent, void* cd)
{
  QCMake* self = reinterpret_cast<QCMake*>(cd);
  emit self->progressChanged(msg, percent);
  QCoreApplication::processEvents();
}

void QCMake::errorCallback(const char* msg, const char* title, bool& stop, void* cd)
{
  QCMake* self = reinterpret_cast<QCMake*>(cd);
  emit self->error(title, msg, &stop);
}

