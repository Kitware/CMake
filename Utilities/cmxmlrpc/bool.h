#ifndef BOOL_H_INCLUDED
#define BOOL_H_INCLUDED

#ifndef TRUE
#define TRUE (1)
#endif
#ifndef FALSE
#define FALSE (0)
#endif

#ifndef __cplusplus
#ifndef HAVE_BOOL
#define HAVE_BOOL
typedef int bool;
#endif
#endif

#endif
