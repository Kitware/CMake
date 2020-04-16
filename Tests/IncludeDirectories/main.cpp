#include "Flags.h"
#include "IncDir.h"
#include "SrcProp.h"
#include "TarProp.h"

#ifdef INCLUDE_SPECIAL_DIR
#  include "SpecialDir.h"
#  ifndef SPECIAL_DIR_H
#    error "SPECIAL_DIR_H not defined"
#  endif
#endif

#ifdef INCLUDE_SPECIAL_SPACE_DIR
#  include "SpecialSpaceDir.h"
#  ifndef SPECIAL_SPACE_DIR_H
#    error "SPECIAL_SPACE_DIR_H not defined"
#  endif
#endif

int main(int argc, char** argv)
{
  return 0;
}
