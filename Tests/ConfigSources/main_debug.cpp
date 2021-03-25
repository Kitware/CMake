#ifndef CFG_DEBUG
#  error "This source should only be compiled in a Debug configuration."
#endif
#ifdef CFG_OTHER
#  error "This source should not be compiled in a non-Debug configuration."
#endif

#include "iface.h"

extern int custom1_debug();
extern int custom2_debug();
extern int custom3_debug();
extern int custom4_debug();
extern int custom5_debug();

int main(int argc, char** argv)
{
  return iface_src() + iface_debug() + custom1_debug() + custom2_debug() +
    custom3_debug() + custom4_debug() + custom5_debug();
}
