#ifndef XMLRPC_INLINE_H_INCLUDED
#define XMLRPC_INLINE_H_INCLUDED

/* Xmlrpc-c uses __inline__ to declare functions that should be
    compiled as inline code.  Some compilers, e.g. GNU, recognize the
    __inline__ keyword.
*/
#ifndef __GNUC__
#ifndef __inline__
#ifdef __sgi
#define __inline__ __inline
#else
#define __inline__
#endif
#endif
#endif


#endif
