#ifndef cmStandardIncludes_h
#define cmStandardIncludes_h


#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif

#ifndef CMAKE_NO_ANSI_STREAM_HEADERS
#include <fstream>
#include <iostream>
#else
#include <fsream.h>
#include <iostream.h>
#endif

#include <vector>
#include <string>
#include <iterator>
#include <algorithm>
#include <functional>
#include <map>

#ifdef CMAKE_NO_STD_NAMESPACE
#define std 
#endif


#endif
