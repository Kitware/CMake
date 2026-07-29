#ifndef _CSE_DUMMY_H
#define _CSE_DUMMY_H

int non_existent_function_for_symbol_test();

#ifdef CSE_REQUIRED_FLAG
#  define required_flag_symbol 1
#endif

#endif
