/*
**  Copyright 2002 University of Illinois Board of Trustees
**  Copyright 2002 Mark D. Roth
**  All rights reserved.
**
**  gethostbyname_r.c - gethostbyname_r() function for compatibility library
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
compat_gethostbyname_r(const char *name, struct hostent *hp,
           char *buf, size_t buflen,
           struct hostent **hpp, int *herr)
{
#if GETHOSTBYNAME_R_NUM_ARGS == 5
  *hpp = gethostbyname_r(name, hp, buf, buflen, herr);

  if (*hpp == NULL)
    return -1;
  return 0;
#elif GETHOSTBYNAME_R_NUM_ARGS == 3
  struct hostent_data hdata;

  if (gethostbyname_r(name, hp, &hdata) == -1)
    return -1;
  *hpp = hp;
  return 0;
#endif /* GETHOSTBYNAME_R_NUM_ARGS == 5 */
}


