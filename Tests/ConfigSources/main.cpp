#if !defined(CFG_DEBUG) && !defined(CFG_OTHER)
#  error "Neither CFG_DEBUG or CFG_OTHER is defined."
#endif
#ifdef CFG_DEBUG
#  include "main_debug.cpp"
#endif
#ifdef CFG_OTHER
#  include "main_other.cpp"
#endif
