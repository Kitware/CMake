/*================================================================
** Copyright 2000, Clark Cooper
** All rights reserved.
**
** This is free software. You are permitted to copy, distribute, or modify
** it under the terms of the MIT/X license (contained in the COPYING file
** with this distribution.)
*/

#ifndef WINCONFIG_H
#define WINCONFIG_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#undef WIN32_LEAN_AND_MEAN

#include <memory.h>
#include <string.h>

#include "expat_config.h"

#if defined(_MSC_VER)
# pragma warning(push,1)
# pragma warning(disable:4311)   /* pointer truncation */
#endif

#endif /* ndef WINCONFIG_H */
