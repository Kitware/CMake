/* This is a hack to work around a bug in MIPSpro 7.4 on some SGIs.
   We need to include stdarg.h before the stdio core to avoid putting
   va_list in the std namespace and never in the global namespace.  */
#include <stdarg.h>
#include_next <internal/stdio_core.h>
