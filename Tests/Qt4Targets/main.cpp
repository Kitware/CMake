
#include <QApplication>
#include <QWidget>

#include <QString>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  QWidget w;
  w.setWindowTitle(QString::fromLatin1("SomeTitle"));
  w.show();

  return 0;
}
