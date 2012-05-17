
#if defined(__ELF__)
#  if !defined(__PIC__)
#    error "The POSITION_INDEPENDENT_CODE property should cause __PIC__ to be defined on ELF platforms."
#  endif
#endif
