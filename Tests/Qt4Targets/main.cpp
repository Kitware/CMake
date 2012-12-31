
#include <QApplication>

#include <QDeclarativeView>

#include <QUrl>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  QDeclarativeView v;
  v.setSource(QUrl("http://www.example.com"));
  v.show();

  return 0;
}
