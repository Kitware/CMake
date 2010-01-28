#if defined(__PATHSCALE__)
/* PathScale Fortran wants mymodule_ when calling any mymodule symbol,
   but module symbols use '.in.' so we cannot provide them anyway.  */
void pathscale_mymodule_(void) {}
#else
/* PGI Fortran wants mymodule_ when calling any mymodule symbol.  */
void mymodule_(void) {}
#endif
