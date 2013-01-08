
#include "common.h"
#include "publicinclude.h"
#include "interfaceinclude.h"
#include "importedinterfaceinclude.h"

#ifdef PRIVATEINCLUDE_DEFINE
#error Unexpected PRIVATEINCLUDE_DEFINE
#endif

#ifndef PUBLICINCLUDE_DEFINE
#error Expected PUBLICINCLUDE_DEFINE
#endif

#ifndef INTERFACEINCLUDE_DEFINE
#error Expected INTERFACEINCLUDE_DEFINE
#endif

#ifndef IMPORTEDINTERFACEINCLUDE_DEFINE
#error Expected IMPORTEDINTERFACEINCLUDE_DEFINE
#endif

#ifndef CURE_DEFINE
#error Expected CURE_DEFINE
#endif

int main() { return 0; }
