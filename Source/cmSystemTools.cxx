#include "cmSystemTools.h"
#include "errno.h"
#include <sys/stat.h>
#include "cmRegularExpression.h"

#ifdef _MSC_VER
#include <windows.h>
#include <direct.h>
inline int Mkdir(const char* dir)
{
  return _mkdir(dir);
}
#else
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
inline int Mkdir(const char* dir)
{
  return mkdir(dir, 00700);
}
#endif

// remove extra spaces and the "\" character from the name
// of the class as it is in the CMakeLists.txt
std::string cmSystemTools::CleanUpName(const char* name)
{
  std::string className = name;
  size_t i =0;
  while(className[i] == ' ' || className[i] == '\t')
    {
    i++;
    }
  if(i)
    {
    className = className.substr(i, className.size());
    } 
  size_t pos = className.find('\\');
  if(pos != std::string::npos)
    {
    className = className.substr(0, pos);
    }
  
  pos = className.find(' ');
  if(pos != std::string::npos)
    {
    className = className.substr(0, pos);
    }
  return className;
}


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
    Mkdir(topdir.c_str());
    pos++;
    }
  if(Mkdir(path) != 0)
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


// replace replace with with as many times as it shows up in source.
// write the result into source.
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

// return true if the file exists
bool cmSystemTools::FileExists(const char* filename)
{
  struct stat fs;
  if (stat(filename, &fs) != 0) 
  {
    return false;
  }
  else
  {
    return true;
  }
}

// Read a list from a CMakeLists.txt file open stream.
// assume the stream has just read "VAR = \"
// read until there is not a "\" at the end of the line.
void cmSystemTools::ReadList(std::vector<std::string>& stringList, 
                             std::ifstream& fin)
{
  char inbuffer[2048];
  bool done = false;
  while ( !done )
    {
    fin.getline(inbuffer, sizeof(inbuffer) );
    std::string inname = inbuffer;
    if(inname.find('\\') == std::string::npos)
      {
      done = true;
      }
    if(inname.size())
      {
      stringList.push_back(cmSystemTools::CleanUpName(inname.c_str()));
      }
    }
}


// convert windows slashes to unix slashes \ with /
void cmSystemTools::ConvertToUnixSlashes(std::string& path)
{
  std::string::size_type pos = path.find('\\');
  while(pos != std::string::npos)
    {
    path[pos] = '/';
    pos = path.find('\\');
    }
  // remove any trailing slash
  if(path[path.size()-1] == '/')
    {
    path = path.substr(0, path.size()-1);
    }
}


int cmSystemTools::Grep(const char* dir, const char* file, const char* expression)
{
  std::string path = dir;
  path += "/";
  path += file;
  std::ifstream fin(path.c_str());
  char buffer[2056];
  int count = 0;
  cmRegularExpression reg(expression);
  while(fin)
    {
    fin.getline(buffer, sizeof(buffer));
    count += reg.find(buffer);
    }
  return count;
}

std::string cmSystemTools::ExtractVariable(const char* variable,
                                           const char* l)
{
  std::string line = l;
  size_t varstart = line.find(variable);
  size_t start = line.find("=");
  if(start != std::string::npos && start > varstart )
    {
    start++;
    while(line[start] == ' ' && start < line.size())
      {
      start++;
      }
    size_t end = line.size()-1;
    while(line[end] == ' ' && end > start)
      {
      end--;
      }
    return line.substr(start, end).c_str();
    }
  return std::string("");
}

