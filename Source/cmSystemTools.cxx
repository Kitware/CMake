/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmSystemTools.h"
#include "errno.h"
#include "stdio.h"
#include <sys/stat.h>
#include "cmRegularExpression.h"

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <windows.h>
#include <direct.h>
#define _unlink unlink
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
  return mkdir(dir, 00777);
}
#endif

bool cmSystemTools::s_ErrorOccured = false;

// adds the elements of the env variable path to the arg passed in
void cmSystemTools::GetPath(std::vector<std::string>& path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  const char* pathSep = ";";
#else
  const char* pathSep = ":";
#endif
  std::string pathEnv = getenv("PATH");
  std::string::size_type start =0;
  bool done = false;
  while(!done)
    {
    std::string::size_type endpos = pathEnv.find(pathSep, start);
    if(endpos != std::string::npos)
      {
      path.push_back(pathEnv.substr(start, endpos-start));
      start = endpos+1;
      }
    else
      {
      done = true;
      }
    }
}


const char* cmSystemTools::GetExecutableExtension()
{
#if defined(_WIN32)
  return ".exe";
#else
  return "";
#endif  
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


std::string cmSystemTools::EscapeSpaces(const char* str)
{
  std::string result = "";
  for(const char* ch = str; *ch != '\0'; ++ch)
    {
    if(*ch == ' ')
      {
      result += '\\';
      }
    result += *ch;
    }
  return result;
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


int cmSystemTools::Grep(const char* dir, const char* file, 
                        const char* expression)
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
    cmRegularExpression blankLine("^[ \t]*$");
    cmRegularExpression comment("^[ \t]*#.*$");
    cmRegularExpression oneLiner("^[ \t]*([A-Za-z_0-9]*)[ \t]*\\((.*)\\)[ \t]*$");
    cmRegularExpression multiLine("^[ \t]*([A-Za-z_0-9]*)[ \t]*\\((.*)$");
    cmRegularExpression lastLine("^(.*)\\)[ \t]*$");

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
          // Check for comment lines and ignore them.
          if(blankLine.find(inbuffer) || comment.find(inbuffer))
            { continue; }
          // Is this the last line?
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
	else
	  {
	  cmSystemTools::Error("Parse error in read function missing end )",
			       inbuffer);
	  return false;
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
  // Match a normal argument (not quoted, no spaces).
  cmRegularExpression normalArgument("[\t ]*([^\" \t]+)[\t ]*");
  // Match a quoted argument (surrounded by double quotes, spaces allowed).
  cmRegularExpression quotedArgument("[\t ]*(\"[^\"]*\")[\t ]*");

  bool done = false;
  while(!done)
    {
    std::string arg;
    long endpos;
    bool foundQuoted = quotedArgument.find(line.c_str());
    bool foundNormal = normalArgument.find(line.c_str());

    if(foundQuoted && foundNormal)
      {
      // Both matches were found.  Take the earlier one.
      if(normalArgument.start(1) < quotedArgument.start(1))
        {
        arg = normalArgument.match(1);
        endpos = normalArgument.end(1);
        }
      else
        {
        arg = quotedArgument.match(1);
        endpos = quotedArgument.end(1);
        // Strip off the double quotes on the ends.
        arg = arg.substr(1, arg.length()-2);
        }
      }    
    else if (foundQuoted)
      {
      arg = quotedArgument.match(1);
      endpos = quotedArgument.end(1);
      // Strip off the double quotes on the ends.
      arg = arg.substr(1, arg.length()-2);
      }
    else if(foundNormal)
      {
      arg = normalArgument.match(1);
      endpos = normalArgument.end(1);
      }
    else
      {
      done = true;
      }
    if(!done)
      {
      arguments.push_back(arg);
      line = line.substr(endpos, line.length() - endpos);
      }
    }
}

void cmSystemTools::Error(const char* m1, const char* m2,
                          const char* m3, const char* m4)
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
  if(m3)
    {
    message += m3;
    }
  if(m4)
    {
    message += m4;
    }
  cmSystemTools::s_ErrorOccured = true;
#if defined(_WIN32) && !defined(__CYGWIN__)
  ::MessageBox(0, message.c_str(), 0, MB_OK);
  std::cerr << message.c_str() << std::endl;
#else
  std::cerr << message.c_str() << std::endl;
#endif
}



void cmSystemTools::CopyFileIfDifferent(const char* source,
                                        const char* destination)
{
  if(cmSystemTools::FilesDiffer(source, destination))
    {
    cmSystemTools::cmCopyFile(source, destination);
    }
}

  
bool cmSystemTools::FilesDiffer(const char* source,
                                const char* destination)
{
  struct stat statSource;
  if (stat(source, &statSource) != 0) 
    {
    return true;
    }
  struct stat statDestination;
  if (stat(destination, &statDestination) != 0) 
    {
    return true;
    }
  if(statSource.st_size != statDestination.st_size)
    {
    return true;
    }
  std::ifstream finSource(source);
  std::ifstream finDestination(destination);
  if(!finSource || !finDestination)
    {
    return true;
    }
  
  while(finSource && finDestination)
    {
    char s, d;
    finSource >> s;
    finDestination >> d;
    if(s != d)
      {
      return true;
      }
    }
  return false;
}



void cmSystemTools::cmCopyFile(const char* source,
                             const char* destination)
{
  std::ifstream fin(source);
  char buff[4096];
  std::ofstream fout(destination);
  if(!fout )
    {
    cmSystemTools::Error("CopyFile failed to open input file", source);
    }
  if(!fin)
    {
    cmSystemTools::Error("CopyFile failed to open output file", destination);
    }
  while(fin)
    {
    fin.getline(buff, 4096);
    if(fin)
      {
      fout << buff << "\n";
      }
    }
}

// return true if the file exists
long int cmSystemTools::ModifiedTime(const char* filename)
{
  struct stat fs;
  if (stat(filename, &fs) != 0) 
    {
    return 0;
    }
  else
    {
    return (long int)fs.st_mtime;
    }
}


  
void cmSystemTools::RemoveFile(const char* source)
{
  unlink(source);
}
