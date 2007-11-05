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

#include "QCMake.h"

#include <QDir>
#include <QCoreApplication>

#include "cmake.h"
#include "cmCacheManager.h"
#include "cmSystemTools.h"
#include "cmExternalMakefileProjectGenerator.h"

QCMake::QCMake(QObject* p)
  : QObject(p)
{
  qRegisterMetaType<QCMakeCacheProperty>();
  qRegisterMetaType<QCMakeCachePropertyList>();
  
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

  std::vector<std::string> generators;
  this->CMakeInstance->GetRegisteredGenerators(generators);
  std::vector<std::string>::iterator iter;
  for(iter = generators.begin(); iter != generators.end(); ++iter)
    {
    this->AvailableGenerators.append(QString::fromStdString(*iter));
    }
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
  if(this->SourceDirectory != dir)
    {
    this->SourceDirectory = dir;
    emit this->sourceDirChanged(dir);
    }
}

void QCMake::setBinaryDirectory(const QString& dir)
{
  if(this->BinaryDirectory != dir)
    {
    cmCacheManager *cachem = this->CMakeInstance->GetCacheManager();
    this->BinaryDirectory = dir;
    this->setGenerator(QString());
    if(!this->CMakeInstance->GetCacheManager()->LoadCache(dir.toLocal8Bit().data()))
      {
      QDir testDir(dir);
      if(testDir.exists("CMakeCache.txt"))
        {
        cmSystemTools::Error("There is a CMakeCache.txt file for the current binary "
            "tree but cmake does not have permission to read it.  "
            "Please check the permissions of the directory you are trying to run CMake on.");
        }
      }
    QCMakeCachePropertyList props = this->properties();
    emit this->propertiesChanged(props);
    cmCacheManager::CacheIterator itm = cachem->NewIterator();
    if ( itm.Find("CMAKE_HOME_DIRECTORY"))
      {
      setSourceDirectory(itm.GetValue());
      }
    if ( itm.Find("CMAKE_GENERATOR"))
      {
      const char* extraGen = cachem->GetCacheValue("CMAKE_EXTRA_GENERATOR");
      std::string curGen = cmExternalMakefileProjectGenerator::
                              CreateFullGeneratorName(itm.GetValue(), extraGen);
      this->setGenerator(QString::fromStdString(curGen));
      }
    }
}


void QCMake::setGenerator(const QString& gen)
{
  if(this->Generator != gen)
    {
    this->Generator = gen;
    emit this->generatorChanged(this->Generator);
    }
}

void QCMake::configure()
{
  this->CMakeInstance->SetHomeDirectory(this->SourceDirectory.toAscii().data());
  this->CMakeInstance->SetStartDirectory(this->SourceDirectory.toAscii().data());
  this->CMakeInstance->SetHomeOutputDirectory(this->BinaryDirectory.toAscii().data());
  this->CMakeInstance->SetStartOutputDirectory(this->BinaryDirectory.toAscii().data());
  this->CMakeInstance->SetGlobalGenerator(
    this->CMakeInstance->CreateGlobalGenerator(this->Generator.toAscii().data()));
  this->CMakeInstance->SetCMakeCommand(this->CMakeExecutable.toAscii().data());
  this->CMakeInstance->LoadCache();

  cmSystemTools::ResetErrorOccuredFlag();

  int err = this->CMakeInstance->Configure();

  emit this->propertiesChanged(this->properties());
  emit this->configureDone(err);
}

void QCMake::generate()
{
  cmSystemTools::ResetErrorOccuredFlag();
  int err = this->CMakeInstance->Generate();
  emit this->generateDone(err);
}
  
void QCMake::setProperties(const QCMakeCachePropertyList& props)
{
  cmCacheManager *cachem = this->CMakeInstance->GetCacheManager();
  cmCacheManager::CacheIterator it = cachem->NewIterator();
  foreach(QCMakeCacheProperty prop, props)
    {
    if ( it.Find(prop.Key.toAscii().data()) )
      {
      if(prop.Value.type() == QVariant::Bool)
        {
        it.SetValue(prop.Value.toBool() ? "ON" : "OFF");
        }
      else
        {
        it.SetValue(prop.Value.toString().toAscii().data());
        }
      }
    }
  cachem->SaveCache(this->BinaryDirectory.toAscii().data());
}

QCMakeCachePropertyList QCMake::properties() const
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
      prop.Value = cmSystemTools::IsOn(i.GetValue());
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
  if(percent >= 0)
    {
    emit self->progressChanged(msg, percent);
    }
  else
    {
    emit self->outputMessage(msg);
    }
}

void QCMake::errorCallback(const char* msg, const char* title,
                           bool& stop, void* cd)
{
  QCMake* self = reinterpret_cast<QCMake*>(cd);
  emit self->error(title, msg, &stop);
}

QString QCMake::generator() const
{
  return this->Generator;
}

QStringList QCMake::availableGenerators() const
{
  return this->AvailableGenerators;
}

