#include <qapplication.h>
#include "qtwrapping.h"

int main(int argc, char *argv[])
{

QApplication app(argc,argv);

qtwrapping qtw;
app.setMainWidget(&qtw);

return 0;
}
