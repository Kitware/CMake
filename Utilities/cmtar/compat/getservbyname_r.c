/*
**  Copyright 2002 University of Illinois Board of Trustees
**  Copyright 2002 Mark D. Roth
**  All rights reserved.
**
**  getservbyname_r.c - getservbyname_r() function for compatibility library
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <config.h>

#include <stdio.h>
#include <sys/types.h>
#include <netdb.h>


int
compat_getservbyname_r(const char *name, const char *proto,
           struct servent *sp, char *buf, size_t buflen,
           struct servent **spp)
{
#if GETSERVBYNAME_R_NUM_ARGS == 5
  *spp = getservbyname_r(name, proto, sp, buf, buflen);

  if (*spp == NULL)
    return -1;
  return 0;
#elif GETSERVBYNAME_R_NUM_ARGS == 4
  struct servent_data sdata;

  if (getservbyname_r(name, proto, sp, &sdata) == -1)
    return -1;
  *spp = sp;
  return 0;
#endif /* GETSERVBYNAME_R_NUM_ARGS == 5 */
}


