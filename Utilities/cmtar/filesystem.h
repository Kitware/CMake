#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#ifdef _MSC_VER
#include <io.h>

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
#ifndef MAXPATHLEN
#define MAXPATHLEN _MAX_PATH
#endif
typedef struct _KWDIRENTRY
{
  char d_name[MAXPATHLEN];
}kwDirEntry;

typedef struct _KWDIR kwDirectory;
kwDirectory * kwOpenDir(const char* name);
kwDirEntry * kwReadDir(kwDirectory * dir);
int kwCloseDir(kwDirectory * dir);
#else

#endif //MSC


#endif
