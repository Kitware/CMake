#ifndef CFG_OTHER
#  error "This source should only be compiled in a non-Debug configuration."
#endif
#ifdef CFG_DEBUG
#  error "This source should not be compiled in a Debug configuration."
#endif

#include "iface.h"

int main(int argc, char** argv)
{
  return iface_src() + iface_other();
}
