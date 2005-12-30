/*
**  Copyright 2002-2003 University of Illinois Board of Trustees
**  Copyright 2002-2003 Mark D. Roth
**  All rights reserved.
**
**  internal.h - internal header file for libtar
**
**  Mark D. Roth <roth@uiuc.edu>
**  Campus Information Technologies and Educational Services
**  University of Illinois at Urbana-Champaign
*/

#include <libtar/config.h>
#include <libtar/compat.h>

#include <libtar/libtar.h>

#ifndef major
# define major(dev) ((int)(((dev) >> 8) & 0xff))
#endif
#ifndef minor
# define minor(dev) ((int)((dev) & 0xff))
#endif
