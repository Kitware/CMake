#include "cmSystemTools.h"
#include <direct.h>
#include "errno.h"
#include <windows.h>

bool cmSystemTools::MakeDirectory(const char* path)
{
  std::string dir = path;
  // replace all of the \ with /
  size_t pos = 0;
  while((pos = dir.find('\\', pos)) != std::string::npos)
    {
    dir[pos] = '/';
    pos++;
    }
  pos =  dir.find(':');
  if(pos == std::string::npos)
    {
    pos = 0;
    }
  while((pos = dir.find('/', pos)) != std::string::npos)
    {
    std::string topdir = dir.substr(0, pos);
    _mkdir(topdir.c_str());
    pos++;
    }
  if(_mkdir(path) != 0)
    {
    // if it is some other error besides directory exists
    // then return false
    if(errno != EEXIST)
      {
      return false;
      }
    }
  return true;
}

void cmSystemTools::ReplaceString(std::string& source,
                                   const char* replace,
                                   const char* with)
{
  int lengthReplace = strlen(replace);
  std::string rest;
  size_t start = source.find(replace);
  while(start != std::string::npos)
    {
    rest = source.substr(start+lengthReplace);
    source = source.substr(0, start);
    source += with;
    source += rest;
    start = source.find(replace, start + lengthReplace );
    }
}
