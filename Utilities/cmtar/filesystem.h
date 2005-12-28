#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <io.h>
#include <libtarint/internal.h>

struct _KWDIR
{
#if _MSC_VER < 1300
  long SrchHandle;
#else
  intptr_t SrchHandle;
#endif
  struct _finddata_t Entry;      // data of current file
  int EOD; //end of directory

};
typedef struct _KWDIRENTRY
{
  char d_name[TAR_MAXPATHLEN];
}kwDirEntry;

typedef struct _KWDIR kwDirectory;
kwDirectory * kwOpenDir(const char* name);
kwDirEntry * kwReadDir(kwDirectory * dir);
int kwCloseDir(kwDirectory * dir);
#else

#endif //MSC

