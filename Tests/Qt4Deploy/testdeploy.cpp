#include <QCoreApplication>
#include <QImageReader>
#include <QSqlDatabase>
#include <QLibraryInfo>
#include <QDebug>

int main(int argc, char** argv)
{
  QCoreApplication app(argc, argv);

  qDebug() << "App path:" << app.applicationDirPath();
  qDebug() << "Plugin path:" << QLibraryInfo::location(QLibraryInfo::PluginsPath);

  bool foundSqlite = false;

  qDebug() << "Supported Database Drivers:";
  foreach(const QString &sqlDriver, QSqlDatabase::drivers())
  {
    qDebug() << " " << sqlDriver;
    if(sqlDriver == "QSQLITE")
      foundSqlite = true;
  }

  if(foundSqlite)
    qDebug() << "Found sqlite support from plugin.";
  else
    qDebug() << "Could not find sqlite support from plugin.";
  return foundSqlite ? 0 : 1;
}
