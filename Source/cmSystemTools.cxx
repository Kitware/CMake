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
  std::string line = source;
  size_t start = line.find(replace);
  while(start != std::string::npos)
    {
    source = line.substr(0, start);
    source += with;
    source += line.substr(start + strlen(replace));
    start = line.find(replace, start + strlen(replace) );
    }
}
