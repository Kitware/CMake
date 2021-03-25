#ifndef CFG_OTHER
#  error "This source should only be compiled in a non-Debug configuration."
#endif
#ifdef CFG_DEBUG
#  error "This source should not be compiled in a Debug configuration."
#endif

#include "iface.h"

extern int custom1_other();
extern int custom2_other();
extern int custom3_other();
extern int custom4_other();
extern int custom5_other();

int main(int argc, char** argv)
{
  return iface_src() + iface_other() + custom1_other() + custom2_other() +
    custom3_other() + custom4_other() + custom5_other();
}
