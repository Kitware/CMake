#include <qapplication.h>
#include "qtwrapping.h"

int main(int argc, char *argv[])
{

QApplication app(argc,argv);

qtwrapping qtw;
app.setMainWidget(&qtw);
qtw.show();

int ret=app.exec();
return ret;
}
