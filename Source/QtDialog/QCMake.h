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

#ifndef __QCMake_h
#define __QCMake_h
#ifdef _MSC_VER
#pragma warning ( disable : 4127 )
#pragma warning ( disable : 4512 )
#endif

#include <QObject>
#include <QString>
#include <QVariant>
#include <QList>
#include <QMetaType>

class cmake;

/// struct to represent cache properties in Qt
/// Value is of type String or Bool
struct QCMakeCacheProperty
{
  enum PropertyType { BOOL, PATH, FILEPATH, STRING };
  QString Key;
  QVariant Value;
  QString Help;
  PropertyType Type;
  bool Advanced;
};

// make types usable with QVariant
Q_DECLARE_METATYPE(QCMakeCacheProperty)
typedef QList<QCMakeCacheProperty> QCMakeCachePropertyList;
Q_DECLARE_METATYPE(QCMakeCachePropertyList)

/// Qt API for CMake library.
/// Wrapper like class allows for easier integration with 
/// Qt features such as, signal/slot connections, multi-threading, etc..
class QCMake : public QObject
{
  Q_OBJECT
public:
  QCMake(QObject* p=0);
  ~QCMake();

public slots:
  /// load the cache file in a directory
  void loadCache(const QString& dir);
  /// set the source directory containing the source
  void setSourceDirectory(const QString& dir);
  /// set the binary directory to build in
  void setBinaryDirectory(const QString& dir);
  /// set the desired generator to use
  void setGenerator(const QString& generator);
  /// do the configure step
  void configure();
  /// generate the files
  void generate();
  /// set the property values
  void setProperties(const QCMakeCachePropertyList&);
  /// interrupt the configure or generate process
  void interrupt();

public:
  /// get the list of cache properties
  QCMakeCachePropertyList properties();
  /// get the current binary directory
  QString binaryDirectory();
  /// get the current source directory
  QString sourceDirectory();
  /// get the current generator
  QString generator();
  /// get the available generators
  QStringList availableGenerators();

signals:
  /// signal when properties change (during read from disk or configure process)
  void propertiesChanged(const QCMakeCachePropertyList& vars);
  /// signal when the generator changes
  void generatorChanged(const QString& gen);
  /// signal when there is an error message
  void error(const QString& title, const QString& message, bool*);
  /// signal when the source directory changes (binary directory already
  /// containing a CMakeCache.txt file)
  void sourceDirChanged(const QString& dir);
  /// signal for progress events
  void progressChanged(const QString& msg, float percent);
  /// signal when configure is done
  void configureDone(int error);
  /// signal when generate is done
  void generateDone(int error);
  /// signal when there is an output message
  void outputMessage(const QString& msg);

protected:
  cmake* CMakeInstance;

  static void progressCallback(const char* msg, float percent, void* cd);
  static void errorCallback(const char* msg, const char* title, bool&, void* cd);

  QString SourceDirectory;
  QString BinaryDirectory;
  QString Generator;
  QString CMakeExecutable;
};

#endif // __QCMake_h

