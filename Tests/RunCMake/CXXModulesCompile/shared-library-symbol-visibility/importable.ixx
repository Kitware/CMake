export module importable;

#include "library_export.h"

#ifdef CHECK_IMPORT_INTERFACE
#  ifdef library_EXPORTS
#    error "library_EXPORTS defined but should NOT be defined when importing module"
#  endif
#else
#  ifndef library_EXPORTS
#    error "library_EXPORTS NOT defined but should be defined when building module"
#  endif
#endif

export LIBRARY_EXPORT int from_import();
