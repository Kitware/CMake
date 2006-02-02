


// First microsoft compilers

#include <windows.h>
#include <io.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <libtarint/filesystem.h>


kwDirectory * kwOpenDir(const char* name)
{
//  struct _KWDIR ssss;
  char* buf;
  size_t n = strlen(name);
  kwDirectory * dir = (kwDirectory *)malloc(sizeof (kwDirectory));
  if(dir==NULL)
    {
      return NULL;
    }
  dir->EOD=0; //not the end of directory
  if ( name[n - 1] == '/' ) 
    {
    buf = (char*) malloc(n + 1 + 1);
//    buf = new char[n + 1 + 1];
    sprintf(buf, "%s*", name);
    } 
  else
    {
    buf = (char*)malloc(n + 2 + 1);
//    buf = new char[n + 2 + 1];
    sprintf(buf, "%s/*", name);
    }
  
  // Now put them into the file array
  dir->SrchHandle = _findfirst(buf, &dir->Entry);
  free(buf);
  
  if ( dir->SrchHandle == -1 )
    {
    free(dir);
    return NULL;
    }
  return dir;  
}

kwDirEntry * kwReadDir(kwDirectory * dir)
{
  static kwDirEntry entry;
  if(!dir || dir->EOD ==1)
    {
    return NULL;
    }
  strncpy(entry.d_name,dir->Entry.name,TAR_MAXPATHLEN-1);
  if(_findnext(dir->SrchHandle, &dir->Entry) == -1)
    {
      dir->EOD=1;
    }

  // It is both stupid and dangerous to return a pointer to a static like this.
  // This can only be called by one caller at a time: i.e., it's not thread safe.
  // On the other hand, it mimics the documented behavior of "readdir" which is
  // what it's implemented to replace for platforms that do not have readdir.
  // Memory leaks are also stupid and dangerous... perhaps this is less so.
  //
  return &entry;
}

int kwCloseDir(kwDirectory * dir)
{
  int r=-1;
  if(dir)
    {
    r=_findclose(dir->SrchHandle);
    free(dir);
    }
  if(r==-1) return 0;
  return 1;
}
