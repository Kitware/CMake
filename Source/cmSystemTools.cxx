#include "cmSystemTools.h"
#include "errno.h"
#include <sys/stat.h>
#include "cmRegularExpression.h"

#if defined(_MSC_VER) || defined(__BORLANDC__)
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

  
void cmSystemTools::ConvertCygwinPath(std::string& pathname)
{
  if(pathname.find("/cygdrive/") != std::string::npos)
    {
    std::string cygStuff = pathname.substr(0, 11);
    std::string replace;
    replace += cygStuff.at(10);
    replace += ":";
    cmSystemTools::ReplaceString(pathname, cygStuff.c_str(), replace.c_str());
    }
}


bool cmSystemTools::ParseFunction(std::ifstream& fin,
                                  std::string& name,
                                  std::vector<std::string>& arguments)
{
  name = "";
  arguments = std::vector<std::string>();
  const int BUFFER_SIZE = 4096;
  char inbuffer[BUFFER_SIZE];
  if(!fin)
    {
    return false;
    }
  
  if(fin.getline(inbuffer, BUFFER_SIZE ) )
    {
    cmRegularExpression blankLine("^$");
    cmRegularExpression comment("^#.*");
    cmRegularExpression oneLiner("[ \t]*([A-Za-z_0-9]*).*\\((.*)\\)");
    cmRegularExpression multiLine("[ \t]*([A-Za-z_0-9]*).*\\((.*)");
    cmRegularExpression lastLine("(.*)\\)");

    // BEGIN VERBATIM JUNK SHOULD BE REMOVED
    cmRegularExpression verbatim("BEGIN MAKE VERBATIM");
    if(verbatim.find(inbuffer))
      {
      cmRegularExpression endVerbatim("END MAKE VERBATIM");
      name = "VERBATIM";
      bool done = false;
      while(!done)
        {
        if(fin.getline(inbuffer, BUFFER_SIZE))
          {
          if(endVerbatim.find(inbuffer))
            {
            done = true;
            }
          else
            {
            arguments.push_back(inbuffer);
            }
          }
        else
          {
          done = true;
          }
        }
      return true;
      }
    // END VERBATIM JUNK SHOULD BE REMOVED

    // check for black line or comment
    if(blankLine.find(inbuffer) || comment.find(inbuffer))
      {
      return false;
      }
    // look for a oneline fun(arg arg2) 
    else if(oneLiner.find(inbuffer))
      {
      // the arguments are the second match
      std::string args = oneLiner.match(2);
      name = oneLiner.match(1);
      // break up the arguments
      cmSystemTools::GetArguments(args, arguments);
      return true;
      }
    // look for a start of a multiline with no trailing ")"  fun(arg arg2 
    else if(multiLine.find(inbuffer))
      {
      name = multiLine.match(1);
      std::string args = multiLine.match(2);
      cmSystemTools::GetArguments(args, arguments);
      // Read lines until the closing paren is hit
      bool done = false;
      while(!done)
        {
        // read lines until the end paren is found
        if(fin.getline(inbuffer, BUFFER_SIZE ) )
          {
          if(lastLine.find(inbuffer))
            {
            done = true;
            std::string args = lastLine.match(1);
            cmSystemTools::GetArguments(args, arguments);
            }
          else
            {
            std::string line = inbuffer;
            cmSystemTools::GetArguments(line, arguments);
            }
          }
        }
      return true;
      }
    else
      {
      cmSystemTools::Error("Parse error in read function ", inbuffer);
      return false;
      }
    }
  return false;

}

void cmSystemTools::GetArguments(std::string& line,
                                 std::vector<std::string>& arguments)
{
  cmRegularExpression argument("[\t ]*([-/\\\\{}\\$A-Za-z_0-9]+)[\t ]*");
  cmRegularExpression argumentWithSpaces("[\t ]*\"([- /\\\\{}\\$A-Za-z_0-9]+)\"[\t ]*");
  std::string arg(" ");
  while(arg.length() )
    {
    arg = "";
    long endpos;

    if (argumentWithSpaces.find(line.c_str()))
      {
      arg = argumentWithSpaces.match(1);
      endpos = argumentWithSpaces.end(1);
      }
    else if(argument.find(line.c_str()))
      {
      arg = argument.match(1);
      endpos = argument.end(1);
      }
    if(arg.length())
      {
      arguments.push_back(arg);
      line = line.substr(endpos, line.length() - endpos);
      }
    }
}

void cmSystemTools::Error(const char* m1, const char* m2)
{
  std::string message = "CMake Error: ";
  if(m1)
    {
    message += m1;
    }
  if(m2)
    {
    message += m2;
    }
#ifdef _WIN32
//  MessageBox(0, message.c_str(), 0, MB_OK);
  std::cerr << message.c_str() << std::endl;
#else
  std::cerr << message.c_str() << std::endl;
#endif
}
