/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "cmSystemTools.h"
#include "errno.h"
#include "stdio.h"
#include <sys/stat.h>
#include "cmRegularExpression.h"
#include <ctype.h>

#if defined(_MSC_VER) || defined(__BORLANDC__)
#include <windows.h>
#include <direct.h>
#define _unlink unlink
inline int Mkdir(const char* dir)
{
  return _mkdir(dir);
}
inline const char* Getcwd(char* buf, unsigned int len)
{
  return _getcwd(buf, len);
}
inline int Chdir(const char* dir)
{
  #if defined(__BORLANDC__)
  return chdir(dir);
  #else
  return _chdir(dir);
  #endif
}
#else
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
inline int Mkdir(const char* dir)
{
  return mkdir(dir, 00777);
}
inline const char* Getcwd(char* buf, unsigned int len)
{
  return getcwd(buf, len);
}
inline int Chdir(const char* dir)
{
  return chdir(dir);
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
  // A hack to make the below algorithm work.  
  if(pathEnv[pathEnv.length()-1] != ':')
    {
    pathEnv += pathSep;
    }
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
  for(std::vector<std::string>::iterator i = path.begin();
      i != path.end(); ++i)
    {
    cmSystemTools::ConvertToUnixSlashes(*i);
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

  cmSystemTools::ConvertToUnixSlashes(dir);

  std::string::size_type pos = dir.find(':');
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
    // There is a bug in the Borland Run time library which makes MKDIR
    // return EACCES when it should return EEXISTS
    #ifdef __BORLANDC__
    if( (errno != EEXIST) && (errno != EACCES) )
      {
      return false;
      }
    #else
    // if it is some other error besides directory exists
    // then return false
    if(errno != EEXIST)
      {
      return false;
      }
    #endif
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

#if defined(_WIN32) && !defined(__CYGWIN__)

// Get the data of key value.
// Example : 
//      HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath
//      =>  will return the data of the "default" value of the key
//      HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root
//      =>  will return the data of the "Root" value of the key
bool ReadAValue(std::string &res, const char *key)
{
  // find the primary key
  std::string primary = key;
  std::string second;
  std::string valuename;
 
  size_t start = primary.find("\\");
  if (start == std::string::npos)
    {
    return false;
    }
  size_t valuenamepos = primary.find(";");
  if (valuenamepos != std::string::npos)
    {
    valuename = primary.substr(valuenamepos+1);
    }
  second = primary.substr(start+1, valuenamepos-start-1);
  primary = primary.substr(0, start);
  
  HKEY primaryKey;
  if (primary == "HKEY_CURRENT_USER")
    {
    primaryKey = HKEY_CURRENT_USER;
    }
  if (primary == "HKEY_CURRENT_CONFIG")
    {
    primaryKey = HKEY_CURRENT_CONFIG;
    }
  if (primary == "HKEY_CLASSES_ROOT")
    {
    primaryKey = HKEY_CLASSES_ROOT;
    }
  if (primary == "HKEY_LOCAL_MACHINE")
    {
    primaryKey = HKEY_LOCAL_MACHINE;
    }
  if (primary == "HKEY_USERS")
    {
    primaryKey = HKEY_USERS;
    }
  
  HKEY hKey;
  if(RegOpenKeyEx(primaryKey, second.c_str(), 
		  0, KEY_READ, &hKey) != ERROR_SUCCESS)
    {
    return false;
    }
  else
    {
    DWORD dwType, dwSize;
    dwSize = 1023;
    char data[1024];
    if(RegQueryValueEx(hKey, (LPTSTR)valuename.c_str(), NULL, &dwType, 
                       (BYTE *)data, &dwSize) == ERROR_SUCCESS)
      {
      if (dwType == REG_SZ)
        {
        res = data;
        return true;
        }
      }
    }
  return false;
}
#endif

// replace replace with with as many times as it shows up in source.
// write the result into source.
void cmSystemTools::ExpandRegistryValues(std::string& source)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  cmRegularExpression regEntry("\\[(HKEY[A-Za-z0-9_~\\:\\-\\(\\)\\.]*)\\]");
  
  // check for black line or comment
  while (regEntry.find(source))
    {
    // the arguments are the second match
    std::string key = regEntry.match(1);
    std::string val;
    if (ReadAValue(val,key.c_str()))
      {
      std::string reg = "[";
      reg += key + "]";
      cmSystemTools::ReplaceString(source, reg.c_str(), val.c_str());
      }
    else
      {
      std::string reg = "[";
      reg += key + "]";
      cmSystemTools::ReplaceString(source, reg.c_str(), "/registry");
      }
    }
#endif  
}


std::string cmSystemTools::EscapeSpaces(const char* str)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  std::string result = str;
  return "\""+result+"\"";
#else
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
#endif
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


// Return a capitalized string (i.e the first letter is uppercased, all other
// are lowercased)
std::string cmSystemTools::Capitalized(const std::string& s)
{
  std::string n;
  n.resize(s.size());
  n[0] = toupper(s[0]);
  for (size_t i = 1; i < s.size(); i++)
    {
    n[i] = tolower(s[i]);
    }
  return n;
}


// convert windows slashes to unix slashes \ with /
void cmSystemTools::ConvertToUnixSlashes(std::string& path)
{
  std::string::size_type pos = 0;
  while((pos = path.find('\\', pos)) != std::string::npos)
    {
    path[pos] = '/';
    pos++;
    }
  // remove any trailing slash
  if(path[path.size()-1] == '/')
    {
    path = path.substr(0, path.size()-1);
    }
}

// convert windows slashes to unix slashes \ with /
void cmSystemTools::CleanUpWindowsSlashes(std::string& path)
{
  std::string::size_type pos = 0;
  while((pos = path.find('/', pos)) != std::string::npos)
    {
    path[pos] = '\\';
    pos++;
    }
  // remove any trailing slash
  if(path[path.size()-1] == '\\')
    {
    path = path.substr(0, path.size()-1);
    }
  // remove any duplicate // slashes
  pos = 0;
  while((pos = path.find("\\\\", pos)) != std::string::npos)
    {
    path.erase(pos, 1);
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
  cmSystemTools::Message(message.c_str(),"Error");
}

void cmSystemTools::Message(const char* m1, const char *title)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  ::MessageBox(0, m1, title, MB_OK);
#endif
  std::cerr << m1 << std::endl;
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

bool cmSystemTools::IsOn(const char* val)
{
  if (!val)
    {
    return false;
    }
  std::basic_string<char> v = val;
  
  for(std::basic_string<char>::iterator c = v.begin();
      c != v.end(); c++)
    {
    *c = toupper(*c);
    }
  return (v == "ON" || v == "1" || v == "YES" || v == "TRUE" || v == "Y");
}

bool cmSystemTools::IsOff(const char* val)
{
  if (!val)
    {
    return true;
    }
  std::basic_string<char> v = val;
  
  for(std::basic_string<char>::iterator c = v.begin();
      c != v.end(); c++)
    {
    *c = toupper(*c);
    }
  return (v == "OFF" || v == "0" || v == "NO" || v == "FALSE" || 
	  v == "N" || v == "NOTFOUND");
}


bool cmSystemTools::RunCommand(const char* command, 
                               std::string& output)
{
  const int BUFFER_SIZE = 4096;
  char buffer[BUFFER_SIZE];

#if defined(WIN32) && !defined(__CYGWIN__)
  std::string commandToFile = command;
  commandToFile += " > ";
  std::string tempFile;
  tempFile += cmSystemTools::TemporaryFileName();
  commandToFile += tempFile;
  system(commandToFile.c_str());
  std::ifstream fin(tempFile.c_str());
  if(!fin)
    {
    cmSystemTools::Error(command, " from RunCommand Faild to create output file",
                         tempFile.c_str());
    return false;
    }
  while(fin)
    {
    fin.getline(buffer, BUFFER_SIZE);
    output += buffer;
    }
  cmSystemTools::RemoveFile(tempFile.c_str());
  return true;
#else
  std::cout << "runing " << command << std::endl;
  FILE* cpipe = popen(command, "r");
  if(!cpipe)
    {
    return false;
    }
  fgets(buffer, BUFFER_SIZE, cpipe);
  while(!feof(cpipe))
    {
    std::cout << buffer << std::flush;
    output += buffer;
    fgets(buffer, BUFFER_SIZE, cpipe);
    }
  fclose(cpipe);
  return true;
#endif
}

#ifdef _MSC_VER
#define tempnam _tempnam
#endif

std::string cmSystemTools::TemporaryFileName()
{
  return tempnam(0, "cmake");
}
  


/**
 * Find the executable with the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the executable if it is
 * found.  Otherwise, the empty string is returned.
 */
std::string cmSystemTools::FindProgram(const char* name,
				       const std::vector<std::string>& userPaths)
{
  // See if the executable exists as written.
  if(cmSystemTools::FileExists(name))
    {
    return cmSystemTools::CollapseFullPath(name);
    }
  std::string tryPath = name;
  tryPath += cmSystemTools::GetExecutableExtension();
  if(cmSystemTools::FileExists(tryPath.c_str()))
    {
    return cmSystemTools::CollapseFullPath(tryPath.c_str());
    }
  
  // Add the system search path to our path.
  std::vector<std::string> path = userPaths;
  cmSystemTools::GetPath(path);
  
  for(std::vector<std::string>::const_iterator p = path.begin();
      p != path.end(); ++p)
    {
    tryPath = *p;
    tryPath += "/";
    tryPath += name;
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      return cmSystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath += cmSystemTools::GetExecutableExtension();
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      return cmSystemTools::CollapseFullPath(tryPath.c_str());
      }
    }
  
  // Couldn't find the program.
  return "";
}


/**
 * Find the library with the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the library if it is
 * found.  Otherwise, the empty string is returned.
 */
std::string cmSystemTools::FindLibrary(const char* name,
				       const std::vector<std::string>& userPaths)
{
  // See if the executable exists as written.
  if(cmSystemTools::FileExists(name))
    {
    return cmSystemTools::CollapseFullPath(name);
    }
    
  // Add the system search path to our path.
  std::vector<std::string> path = userPaths;
  cmSystemTools::GetPath(path);
  std::string tryPath;
  for(std::vector<std::string>::const_iterator p = path.begin();
      p != path.end(); ++p)
    {
    tryPath = *p;
    tryPath += "/";
    tryPath += name;
    tryPath += ".lib";
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      return cmSystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".so";
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      return cmSystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".a";
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      return cmSystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".sl";
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      return cmSystemTools::CollapseFullPath(tryPath.c_str());
      }
    }
  
  // Couldn't find the library.
  return "";
}

bool cmSystemTools::FileIsDirectory(const char* name)
{  
  struct stat fs;
  if(stat(name, &fs) == 0)
    {
#if _WIN32
    return ((fs.st_mode & _S_IFDIR) != 0);
#else
    return S_ISDIR(fs.st_mode);
#endif
    }
  else
    {
    return false;
    }
}


std::string cmSystemTools::GetCurrentWorkingDirectory()
{
  char buf[2048];
  std::string path = Getcwd(buf, 2048);
  return path;
}

/**
 * Given the path to a program executable, get the directory part of the path with the
 * file stripped off.  If there is no directory part, the empty string is returned.
 */
std::string cmSystemTools::GetProgramPath(const char* in_name)
{
  std::string dir, file;
  cmSystemTools::SplitProgramPath(in_name, dir, file);
  return dir;
}

/**
 * Given the path to a program executable, get the directory part of the path
 * with the file stripped off.  If there is no directory part, the empty
 * string is returned.
 */
void cmSystemTools::SplitProgramPath(const char* in_name,
				     std::string& dir,
				     std::string& file)
{
  dir = in_name;
  file = "";
  cmSystemTools::ConvertToUnixSlashes(dir);
  
  if(!cmSystemTools::FileIsDirectory(dir.c_str()))
    {
    std::string::size_type slashPos = dir.rfind("/");
    if(slashPos != std::string::npos)
      {
      file = dir.substr(slashPos+1);
      dir = dir.substr(0, slashPos);
      }
    else
      {
      file = dir;
      dir = "";
      }
    }
  
  if((dir != "") && !cmSystemTools::FileIsDirectory(dir.c_str()))
    {
    std::string oldDir = in_name;
    cmSystemTools::ConvertToUnixSlashes(oldDir);
    cmSystemTools::Error("Error splitting file name off end of path:\n",
                         oldDir.c_str(), "\nDirectory not found: ", 
                         dir.c_str());
    dir = in_name;
    return;
    }
}

/**
 * Given a path to a file or directory, convert it to a full path.
 * This collapses away relative paths.  The full path is returned.
 */
std::string cmSystemTools::CollapseFullPath(const char* in_name)
{
  std::string dir, file;
  cmSystemTools::SplitProgramPath(in_name, dir, file);
  // Ultra-hack warning:
  // This changes to the target directory, saves the working directory,
  // and then changes back to the original working directory.
  std::string cwd = cmSystemTools::GetCurrentWorkingDirectory();
  if(dir != "") { Chdir(dir.c_str()); }
  std::string newDir = cmSystemTools::GetCurrentWorkingDirectory();
  Chdir(cwd.c_str());

  cmSystemTools::ConvertToUnixSlashes(newDir);
  std::string newPath = newDir+"/"+file;
  
  return newPath;
}

/**
 * Return path of a full filename (no trailing slashes).
 * Warning: returned path is converted to Unix slashes format.
 */
std::string cmSystemTools::GetFilenamePath(const std::string& filename)
{
  std::string fn = filename;
  cmSystemTools::ConvertToUnixSlashes(fn);
  
  std::string::size_type slash_pos = fn.rfind("/");
  if(slash_pos != std::string::npos)
    {
    return fn.substr(0, slash_pos);
    }
  else
    {
    return "";
    }
}


/**
 * Return file name of a full filename (i.e. file name without path).
 */
std::string cmSystemTools::GetFilenameName(const std::string& filename)
{
  std::string fn = filename;
  cmSystemTools::ConvertToUnixSlashes(fn);
  
  std::string::size_type slash_pos = fn.rfind("/");
  if(slash_pos != std::string::npos)
    {
    return fn.substr(slash_pos + 1);
    }
  else
    {
    return filename;
    }
}


/**
 * Return file extension of a full filename (dot included).
 * Warning: this is the longest extension (for example: .tar.gz)
 */
std::string cmSystemTools::GetFilenameExtension(const std::string& filename)
{
  std::string name = cmSystemTools::GetFilenameName(filename);
  std::string::size_type dot_pos = name.find(".");
  if(dot_pos != std::string::npos)
    {
    return name.substr(dot_pos);
    }
  else
    {
    return "";
    }
}


/**
 * Return file name without extension of a full filename (i.e. without path).
 * Warning: it considers the longest extension (for example: .tar.gz)
 */
std::string cmSystemTools::GetFilenameNameWithoutExtension(const std::string& filename)
{
  std::string name = cmSystemTools::GetFilenameName(filename);
  std::string::size_type dot_pos = name.find(".");
  if(dot_pos != std::string::npos)
    {
    return name.substr(0, dot_pos);
    }
  else
    {
    return name;
    }
}

