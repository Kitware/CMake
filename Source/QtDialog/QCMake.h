
#ifndef __QCMake_h
#define __QCMake_h

#include <QObject>
#include <QString>
#include <QList>
#include <QMetaType>

class cmake;

// struct to represent cache properties in Qt
struct QCMakeCacheProperty
{
  enum PropertyType { BOOL, PATH, FILEPATH, STRING };
  QString Key;
  QString Value;
  QString Help;
  PropertyType Type;
  bool Advanced;
};

// make types usable with QVariant
Q_DECLARE_METATYPE(QCMakeCacheProperty)
typedef QList<QCMakeCacheProperty> QCMakeCachePropertyList;
Q_DECLARE_METATYPE(QCMakeCachePropertyList)

// Qt API for CMake library.
// Wrapper like class allows for easier integration with 
// Qt features such as, signal/slot connections, multi-threading, etc..
class QCMake : public QObject
{
  Q_OBJECT
public:
  QCMake(QObject* p=0);
  ~QCMake();

public slots:
  void loadCache(const QString& dir);
  void setSourceDirectory(const QString& dir);
  void setBinaryDirectory(const QString& dir);
  void setGenerator(const QString& generator);
  void configure();
  void generate();
  void setProperties(const QCMakeCachePropertyList&);
  void interrupt();

public:
  QCMakeCachePropertyList properties();
  QString binaryDirectory();
  QString sourceDirectory();
  QString generator();

signals:
  void propertiesChanged(const QCMakeCachePropertyList& vars);
  void generatorChanged(const QString& gen);
  void error(const QString& title, const QString& message, bool*);
  void sourceDirChanged(const QString& dir);
  void progressChanged(const QString& msg, float percent);
  void configureDone(int error);
  void generateDone(int error);
  void configureReady();
  void generateReady();

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

