
#ifndef FOO_DEFINE
#error Expected FOO_DEFINE
#endif

#ifndef BAR_DEFINE
#error Expected Bar_DEFINE
#endif

#include "commandoutput.h"

#ifndef COMMANDOUTPUT_DEFINE
#error Expected COMMANDOUTPUT_DEFINE
#endif

#include "targetoutput.h"

#ifndef TARGETOUTPUT_DEFINE
#error Expected TARGETOUTPUT_DEFINE
#endif

int bar()
{
  return 0;
}
