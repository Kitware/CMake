/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
/**
 * Include header files as a function of the build process, compiler,
 * and operating system.
 */
#ifndef cmStandardIncludes_h
#define cmStandardIncludes_h

#include <cmConfigure.h>

// Provide fixed-size integer types.
#include <cm_kwiml.h>

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

// we must have stl with the standard include style
#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <set>
#include <string>
#include <vector>

// include the "c" string header
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_MSC_VER)
typedef unsigned short mode_t;
#else
#include <sys/types.h>
#endif

// use this class to shrink the size of symbols in .o files
// std::string is really basic_string<....lots of stuff....>
// when combined with a map or set, the symbols can be > 2000 chars!
#include <cmsys/String.hxx>
// typedef cmsys::String std::string;

/* Poison this operator to avoid common mistakes.  */
extern void operator<<(std::ostream&, const std::ostringstream&);

#include "cmCustomCommandLines.h"
#include "cmDocumentationEntry.h"
#include "cmTargetLinkLibraryType.h"

#endif
