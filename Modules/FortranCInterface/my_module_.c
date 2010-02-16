#if defined(__PATHSCALE__)
/* PathScale Fortran wants my_module_ when calling any my_module symbol,
   but module symbols use '.in.' so we cannot provide them anyway.  */
void pathscale_my_module_(void) {}
#else
/* PGI Fortran wants my_module_ when calling any my_module symbol.  */
void my_module_(void) {}
#endif
