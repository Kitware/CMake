/*
**  Copyright 2002 University of Illinois Board of Trustees
**  Copyright 2002 Mark D. Roth
**  All rights reserved.
**
**  inet_aton.c - inet_aton() function for compatibility library
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


int
inet_aton(const char *cp, struct in_addr *inp)
{
  inp->s_addr = inet_addr(cp);
  if (inp->s_addr == -1)
    return 0;
  return 1;
}


