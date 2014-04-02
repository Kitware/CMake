
#include "resourcetester.h"

#include <QDebug>
#include <QApplication>
#include <QFile>
#include <QTimer>

ResourceTester::ResourceTester(QObject *parent)
  : QObject(parent)
{

}

void ResourceTester::doTest()
{
  if (!QFile::exists(":/CMakeLists.txt"))
      qApp->exit(EXIT_FAILURE);
  if (!QFile::exists(":/main.cpp"))
      qApp->exit(EXIT_FAILURE);

  QTimer::singleShot(0, qApp, SLOT(quit()));
}
