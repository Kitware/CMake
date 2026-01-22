
#include <iostream>

#include <QCoreApplication>
#include <QFile>

#include "rcc_obj.h"

int main(int argc, char* argv[])
{
  QCoreApplication app(argc, argv);

  // Test that the resource from the OBJECT library is accessible
  QString resourcePath = RccObj::resourceContent();
  if (!QFile::exists(resourcePath)) {
    std::cerr << "Resource not found: " << resourcePath.toStdString()
              << std::endl;
    return 1;
  }

  QFile file(resourcePath);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    std::cerr << "Failed to open resource: " << resourcePath.toStdString()
              << std::endl;
    return 1;
  }

  QString content = file.readAll();
  if (content.isEmpty()) {
    std::cerr << "Resource content is empty" << std::endl;
    return 1;
  }

  std::cout << "Resource content: " << content.toStdString() << std::endl;
  std::cout << "Test passed!" << std::endl;

  return 0;
}
