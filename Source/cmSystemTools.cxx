/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmSystemTools.h"   
#include "errno.h"
#include <stdio.h>
#include <sys/stat.h>
#include "cmRegularExpression.h"
#include <ctype.h>
#include "cmDirectory.h"
#include "cmMakefile.h"

// support for realpath call
#ifndef _WIN32
#include <limits.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/wait.h>
#endif

#if defined(_WIN32) && (defined(_MSC_VER) || defined(__BORLANDC__))
#include <string.h>
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

bool cmSystemTools::s_DisableRunCommandOutput = false;
bool cmSystemTools::s_ErrorOccured = false;
bool cmSystemTools::s_DisableMessages = false;

void (*cmSystemTools::s_ErrorCallback)(const char*, const char*, bool&, void*);
void* cmSystemTools::s_ErrorCallbackClientData = 0;

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
#if defined(_WIN32) || defined(__CYGWIN__)
  return ".exe";
#else
  return "";
#endif  
}


bool cmSystemTools::MakeDirectory(const char* path)
{
  if(cmSystemTools::FileExists(path))
    {
    return true;
    }
  std::string dir = path;
  if(dir.size() == 0)
    {
    return false;
    }
  cmSystemTools::ConvertToUnixSlashes(dir);

  std::string::size_type pos = dir.find(':');
  if(pos == std::string::npos)
    {
    pos = 0;
    }
  std::string topdir;
  while((pos = dir.find('/', pos)) != std::string::npos)
    {
    topdir = dir.substr(0, pos);
    Mkdir(topdir.c_str());
    pos++;
    }
  if(dir[dir.size()-1] == '/')
    {
    topdir = dir.substr(0, dir.size());
    }
  else
    {
    topdir = dir;
    }
  if(Mkdir(topdir.c_str()) != 0)
    {
    // There is a bug in the Borland Run time library which makes MKDIR
    // return EACCES when it should return EEXISTS
    // if it is some other error besides directory exists
    // then return false
    if( (errno != EEXIST) 
#ifdef __BORLANDC__
        && (errno != EACCES) 
#endif      
      )
      {
      cmSystemTools::Error("Faild to create directory:", path);
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
  std::string::size_type lengthReplace = strlen(replace);
  std::string::size_type lengthWith = strlen(with);
  std::string rest;
  std::string::size_type start = source.find(replace);
  while(start != std::string::npos)
    {
    rest = source.substr(start+lengthReplace);
    source = source.substr(0, start);
    source += with;
    source += rest;
    start = source.find(replace, start + lengthWith );
    }
}

// Read a registry value.
// Example : 
//      HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath
//      =>  will return the data of the "default" value of the key
//      HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root
//      =>  will return the data of the "Root" value of the key

#if defined(_WIN32) && !defined(__CYGWIN__)
bool cmSystemTools::ReadRegistryValue(const char *key, std::string &value)
{

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
  if(RegOpenKeyEx(primaryKey, 
                  second.c_str(), 
		  0, 
                  KEY_READ, 
                  &hKey) != ERROR_SUCCESS)
    {
    return false;
    }
  else
    {
    DWORD dwType, dwSize;
    dwSize = 1023;
    char data[1024];
    if(RegQueryValueEx(hKey, 
                       (LPTSTR)valuename.c_str(), 
                       NULL, 
                       &dwType, 
                       (BYTE *)data, 
                       &dwSize) == ERROR_SUCCESS)
      {
      if (dwType == REG_SZ)
        {
        value = data;
        return true;
        }
      }
    }
  return false;
}
#else
bool cmSystemTools::ReadRegistryValue(const char *, std::string &)
{
  return false;
}
#endif


// Write a registry value.
// Example : 
//      HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath
//      =>  will set the data of the "default" value of the key
//      HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root
//      =>  will set the data of the "Root" value of the key

#if defined(_WIN32) && !defined(__CYGWIN__)
bool cmSystemTools::WriteRegistryValue(const char *key, const char *value)
{
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
  DWORD dwDummy;
  if(RegCreateKeyEx(primaryKey, 
                    second.c_str(), 
                    0, 
                    "",
                    REG_OPTION_NON_VOLATILE,
                    KEY_WRITE,
                    NULL,
                    &hKey,
                    &dwDummy) != ERROR_SUCCESS)
    {
    return false;
    }

  if(RegSetValueEx(hKey, 
                   (LPTSTR)valuename.c_str(), 
                   0, 
                   REG_SZ, 
                   (CONST BYTE *)value, 
                   (DWORD)(strlen(value) + 1)) == ERROR_SUCCESS)
    {
    return true;
    }
  return false;
}
#else
bool cmSystemTools::WriteRegistryValue(const char *, const char *)
{
  return false;
}
#endif

// Delete a registry value.
// Example : 
//      HKEY_LOCAL_MACHINE\SOFTWARE\Python\PythonCore\2.1\InstallPath
//      =>  will delete the data of the "default" value of the key
//      HKEY_LOCAL_MACHINE\SOFTWARE\Scriptics\Tcl\8.4;Root
//      =>  will delete  the data of the "Root" value of the key

#if defined(_WIN32) && !defined(__CYGWIN__)
bool cmSystemTools::DeleteRegistryValue(const char *key)
{
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
  if(RegOpenKeyEx(primaryKey, 
                  second.c_str(), 
		  0, 
                  KEY_WRITE, 
                  &hKey) != ERROR_SUCCESS)
    {
    return false;
    }
  else
    {
    if(RegDeleteValue(hKey, 
                      (LPTSTR)valuename.c_str()) == ERROR_SUCCESS)
      {
      return true;
      }
    }
  return false;
}
#else
bool cmSystemTools::DeleteRegistryValue(const char *)
{
  return false;
}
#endif

// replace replace with with as many times as it shows up in source.
// write the result into source.
#if defined(_WIN32) && !defined(__CYGWIN__)
void cmSystemTools::ExpandRegistryValues(std::string& source)
{
  // Regular expression to match anything inside [...] that begins in HKEY.
  // Note that there is a special rule for regular expressions to match a
  // close square-bracket inside a list delimited by square brackets.
  // The "[^]]" part of this expression will match any character except
  // a close square-bracket.  The ']' character must be the first in the
  // list of characters inside the [^...] block of the expression.
  cmRegularExpression regEntry("\\[(HKEY[^]]*)\\]");
  
  // check for black line or comment
  while (regEntry.find(source))
    {
    // the arguments are the second match
    std::string key = regEntry.match(1);
    std::string val;
    if (ReadRegistryValue(key.c_str(), val))
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
}
#else
void cmSystemTools::ExpandRegistryValues(std::string&)
{
}
#endif  


std::string cmSystemTools::EscapeQuotes(const char* str)
{
  std::string result = "";
  for(const char* ch = str; *ch != '\0'; ++ch)
    {
    if(*ch == '"')
      {
      result += '\\';
      }
    result += *ch;
    }
  return result;
}

bool cmSystemTools::SameFile(const char* file1, const char* file2)
{
#ifdef _WIN32
  HANDLE hFile1, hFile2;

  hFile1 = CreateFile( file1, 
                      GENERIC_READ, 
                      FILE_SHARE_READ ,
                      NULL,
                      OPEN_EXISTING,
                      FILE_FLAG_BACKUP_SEMANTICS,
                      NULL
    );
  hFile2 = CreateFile( file2, 
                      GENERIC_READ, 
                      FILE_SHARE_READ, 
                      NULL,
                      OPEN_EXISTING,
                      FILE_FLAG_BACKUP_SEMANTICS,
                      NULL
    );
  if( hFile1 == INVALID_HANDLE_VALUE || hFile2 == INVALID_HANDLE_VALUE)
    {
    if(hFile1 != INVALID_HANDLE_VALUE)
      {
      CloseHandle(hFile1);
      }
    if(hFile2 != INVALID_HANDLE_VALUE)
      {
      CloseHandle(hFile2);
      }
    return false;
    }

   BY_HANDLE_FILE_INFORMATION fiBuf1;
   BY_HANDLE_FILE_INFORMATION fiBuf2;
   GetFileInformationByHandle( hFile1, &fiBuf1 );
   GetFileInformationByHandle( hFile2, &fiBuf2 );
   CloseHandle(hFile1);
   CloseHandle(hFile2);
   return (fiBuf1.nFileIndexHigh == fiBuf2.nFileIndexHigh &&
           fiBuf1.nFileIndexLow == fiBuf2.nFileIndexLow);
#else
  struct stat fileStat1, fileStat2;
  if (stat(file1, &fileStat1) == 0 && stat(file2, &fileStat2) == 0)
    {
    // see if the files are the same file
    // check the device inode and size
    if(memcmp(&fileStat2.st_dev, &fileStat1.st_dev, sizeof(fileStat1.st_dev)) == 0 && 
       memcmp(&fileStat2.st_ino, &fileStat1.st_ino, sizeof(fileStat1.st_ino)) == 0 &&
       fileStat2.st_size == fileStat1.st_size 
      ) 
      {
      return true;
      }
    }
  return false;
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


// Return a lower case string 
std::string cmSystemTools::LowerCase(const std::string& s)
{
  std::string n;
  n.resize(s.size());
  for (size_t i = 0; i < s.size(); i++)
    {
    n[i] = tolower(s[i]);
    }
  return n;
}

// Return a lower case string 
std::string cmSystemTools::UpperCase(const std::string& s)
{
  std::string n;
  n.resize(s.size());
  for (size_t i = 0; i < s.size(); i++)
    {
    n[i] = toupper(s[i]);
    }
  return n;
}


// convert windows slashes to unix slashes 
void cmSystemTools::ConvertToUnixSlashes(std::string& path)
{
  std::string::size_type pos = 0;
  while((pos = path.find('\\', pos)) != std::string::npos)
    {
    path[pos] = '/';
    pos++;
    }
  // remove any trailing slash
  if(path.size() && path[path.size()-1] == '/')
    {
    path = path.substr(0, path.size()-1);
    }

  // if there is a tilda ~ then replace it with HOME
  if(path.find("~") == 0)
    {
    if (getenv("HOME"))
      {
      path = std::string(getenv("HOME")) + path.substr(1);
      }
    }
  
  // if there is a /tmp_mnt in a path get rid of it!
  // stupid sgi's 
  if(path.find("/tmp_mnt") == 0)
    {
    path = path.substr(8);
    }
}


// change // to /, and escape any spaces in the path
std::string cmSystemTools::ConvertToUnixOutputPath(const char* path)
{
  std::string ret = path;
  
  // remove // except at the beginning might be a cygwin drive
  std::string::size_type pos = 1;
  while((pos = ret.find("//", pos)) != std::string::npos)
    {
    ret.erase(pos, 1);
    }
  // now escape spaces if there is a space in the path
  if(ret.find(" ") != std::string::npos)
    {
    std::string result = "";
    char lastch = 1;
    for(const char* ch = ret.c_str(); *ch != '\0'; ++ch)
      {
	// if it is already escaped then don't try to escape it again
      if(*ch == ' ' && lastch != '\\')
        {
        result += '\\';
        }
      result += *ch;
      lastch = *ch;
      }
    ret = result;
    }
  return ret;
}



std::string cmSystemTools::EscapeSpaces(const char* str)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  std::string result;
  
  // if there are spaces
  std::string temp = str;
  if (temp.find(" ") != std::string::npos && 
      temp.find("\"")==std::string::npos)
    {
    result = "\"";
    result += str;
    result += "\"";
    return result;
    }
  return str;
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

std::string cmSystemTools::ConvertToOutputPath(const char* path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  return cmSystemTools::ConvertToWindowsOutputPath(path);
#else
  return cmSystemTools::ConvertToUnixOutputPath(path);
#endif
}


// remove double slashes not at the start
std::string cmSystemTools::ConvertToWindowsOutputPath(const char* path)
{  
  std::string ret = path;
  std::string::size_type pos = 0;
  // first convert all of the slashes
  while((pos = ret.find('/', pos)) != std::string::npos)
    {
    ret[pos] = '\\';
    pos++;
    }
  // check for really small paths
  if(ret.size() < 2)
    {
    return ret;
    }
  // now clean up a bit and remove double slashes
  // Only if it is not the first position in the path which is a network
  // path on windows
  pos = 1; // start at position 1
  while((pos = ret.find("\\\\", pos)) != std::string::npos)
    {
    ret.erase(pos, 1);
    }
  // now double quote the path if it has spaces in it
  // and is not already double quoted
  if(ret.find(" ") != std::string::npos
     && ret[0] != '\"')
    {
    std::string result;
    result = "\"" + ret + "\"";
    ret = result;
    }
  return ret;
}

inline void RemoveComments(char* ptr)
{
  while(*ptr)
    {
    if(*ptr == '#')
      {
      *ptr = 0;
      break;
      }
    ++ptr;
    }
}

bool cmSystemTools::ParseFunction(std::ifstream& fin,
                                  std::string& name,
                                  std::vector<std::string>& arguments,
                                  const char* filename,
				  bool& parseError)
{
  parseError = false;
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
    RemoveComments(inbuffer);
    cmRegularExpression blankLine("^[ \t\r]*$");
//    cmRegularExpression comment("^[ \t]*#.*$");
    cmRegularExpression oneLiner("^[ \t]*([A-Za-z_0-9]*)[ \t]*\\((.*)\\)[ \t\r]*$");
    cmRegularExpression multiLine("^[ \t]*([A-Za-z_0-9]*)[ \t]*\\((.*)$");
    cmRegularExpression lastLine("^(.*)\\)[ \t\r]*$");

    // check for blank line or comment
    if(blankLine.find(inbuffer) )
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
          RemoveComments(inbuffer);
          // Check for comment lines and ignore them.
          if(blankLine.find(inbuffer))
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
	  parseError = true;
	  cmSystemTools::Error("Parse error in read function missing end )\nIn File: ",
			       filename, "\nCurrent line:", inbuffer);
	  return false;
	  }
        }
      return true;
      }
    else
      {
      parseError = true;
      cmSystemTools::Error("Parse error in read function\nIn file:", 
                           filename, "\nCurrent line:", inbuffer);
      return false;
      }
    }
  return false;

}

void cmSystemTools::GetArguments(std::string& line,
                                 std::vector<std::string>& arguments)
{
  // Match a normal argument (not quoted, no spaces).
  cmRegularExpression normalArgument("[ \t]*(([^ \t\r\\]|[\\].)+)[ \t\r]*");
  // Match a quoted argument (surrounded by double quotes, spaces allowed).
  cmRegularExpression quotedArgument("[ \t]*(\"([^\"\\]|[\\].)*\")[ \t\r]*");

  bool done = false;
  while(!done)
    {
    std::string arg;
    std::string::size_type endpos=0;
    bool foundQuoted = quotedArgument.find(line.c_str());
    bool foundNormal = normalArgument.find(line.c_str());

    if(foundQuoted && foundNormal)
      {
      // Both matches were found.  Take the earlier one.
      // Favor double-quoted version if there is a tie.
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
      arguments.push_back(cmSystemTools::RemoveEscapes(arg.c_str()));
      line = line.substr(endpos, line.length() - endpos);
      }
    }
}


std::string cmSystemTools::RemoveEscapes(const char* s)
{
  std::string result = "";
  for(const char* ch = s; *ch; ++ch)
    {
    if(*ch == '\\' && *(ch+1) != ';')
      {
      ++ch;
      switch (*ch)
        {
        case '\\': result.insert(result.end(), '\\'); break;
        case '"': result.insert(result.end(), '"'); break;
        case ' ': result.insert(result.end(), ' '); break;
        case 't': result.insert(result.end(), '\t'); break;
        case 'n': result.insert(result.end(), '\n'); break;
        case 'r': result.insert(result.end(), '\r'); break;
        case '0': result.insert(result.end(), '\0'); break;
        case '\0':
          {
          cmSystemTools::Error("Trailing backslash in argument:\n", s);
          return result;
          }
        default:
          {
          std::string chStr(1, *ch);
          cmSystemTools::Error("Invalid escape sequence \\", chStr.c_str(),
                               "\nin argument ", s);
          }
        }
      }
    else
      {
      result.insert(result.end(), *ch);
      }
    }
  return result;
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


void cmSystemTools::SetErrorCallback(ErrorCallback f, void* clientData)
{
  s_ErrorCallback = f;
  s_ErrorCallbackClientData = clientData;
}

void cmSystemTools::Message(const char* m1, const char *title)
{
  if(s_DisableMessages)
    {
    return;
    }
  if(s_ErrorCallback)
    {
    (*s_ErrorCallback)(m1, title, s_DisableMessages, s_ErrorCallbackClientData);
    return;
    }
  else
    {
    std::cerr << m1 << std::endl;
    }
  
}


bool cmSystemTools::CopyFileIfDifferent(const char* source,
                                        const char* destination)
{
  if(cmSystemTools::FilesDiffer(source, destination))
    {
    cmSystemTools::cmCopyFile(source, destination);
    return true;
    }
  return false;
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

  if(statSource.st_size == 0)
    {
    return false;
    }

#if defined(_WIN32) || defined(__CYGWIN__)
  std::ifstream finSource(source, 
                          std::ios::binary | std::ios::in);
  std::ifstream finDestination(destination, 
                               std::ios::binary | std::ios::in);
#else
  std::ifstream finSource(source);
  std::ifstream finDestination(destination);
#endif
  if(!finSource || !finDestination)
    {
    return true;
    }

  char* source_buf = new char[statSource.st_size];
  char* dest_buf = new char[statSource.st_size];

  finSource.read(source_buf, statSource.st_size);
  finDestination.read(dest_buf, statSource.st_size);

  if(statSource.st_size != static_cast<long>(finSource.gcount()) ||
     statSource.st_size != static_cast<long>(finDestination.gcount()))
    {
    cmStringStream msg;
    msg << "FilesDiffer failed to read files (allocated: " 
        << statSource.st_size << ", read source: " <<  finSource.gcount() 
        << ", read dest: " << finDestination.gcount();
    cmSystemTools::Error(msg.str().c_str());
    delete [] source_buf;
    delete [] dest_buf;
    return false;
    }
  int ret = memcmp((const void*)source_buf, 
                   (const void*)dest_buf, 
                   statSource.st_size);

  delete [] dest_buf;
  delete [] source_buf;

  return ret != 0;
}


/**
 * Copy a file named by "source" to the file named by "destination".
 */
void cmSystemTools::cmCopyFile(const char* source,
                               const char* destination)
{
  const int bufferSize = 4096;
  char buffer[bufferSize];

  // If destination is a directory, try to create a file with the same
  // name as the source in that directory.

  std::string new_destination;
  if(cmSystemTools::FileExists(destination) &&
     cmSystemTools::FileIsDirectory(destination))
    {
    new_destination = destination;
    cmSystemTools::ConvertToUnixSlashes(new_destination);
    new_destination += '/';
    std::string source_name = source;
    new_destination += cmSystemTools::GetFilenameName(source_name);
    destination = new_destination.c_str();
    }

  // Create destination directory

  std::string destination_dir = destination;
  destination_dir = cmSystemTools::GetFilenamePath(destination_dir);
  cmSystemTools::MakeDirectory(destination_dir.c_str());

  // Open files

#if defined(_WIN32) || defined(__CYGWIN__)
  std::ifstream fin(source, 
                    std::ios::binary | std::ios::in);
#else
  std::ifstream fin(source);
#endif
  if(!fin)
    {
    cmSystemTools::Error("CopyFile failed to open input file \"",
                         source, "\"");
    return;
    }

#if defined(_WIN32) || defined(__CYGWIN__)
  std::ofstream fout(destination, 
                     std::ios::binary | std::ios::out | std::ios::trunc);
#else
  std::ofstream fout(destination, 
                     std::ios::out | std::ios::trunc);
#endif
  if(!fout)
    {
    cmSystemTools::Error("CopyFile failed to open output file \"",
                         destination, "\"");
    return;
    }
  
  // This copy loop is very sensitive on certain platforms with
  // slightly broken stream libraries (like HPUX).  Normally, it is
  // incorrect to not check the error condition on the fin.read()
  // before using the data, but the fin.gcount() will be zero if an
  // error occurred.  Therefore, the loop should be safe everywhere.
  while(fin)
    {
    fin.read(buffer, bufferSize);
    if(fin.gcount())
      {
      fout.write(buffer, fin.gcount());
      }
    }

  fin.close();
  fout.close();

  // More checks

  struct stat statSource, statDestination;
  if (stat(source, &statSource) != 0 ||
      stat(destination, &statDestination) != 0)
    {
    cmSystemTools::Error("CopyFile failed to copy files!");
    }

  if (statSource.st_size != statDestination.st_size)
    {
    cmStringStream msg;
    msg << "CopyFile failed to copy files (sizes differ, source: " 
        << statSource.st_size << " , dest: " << statDestination.st_size;
    cmSystemTools::Error(msg.str().c_str());
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


  
bool cmSystemTools::RemoveFile(const char* source)
{
  return unlink(source) != 0 ? false : true;
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
  if (!val || strlen(val) == 0)
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
	  v == "N" || v == "NOTFOUND" || v == "IGNORE");
}


bool cmSystemTools::RunCommand(const char* command, 
                               std::string& output,
                               const char* dir,
                               bool verbose)
{
  int foo;
  return cmSystemTools::RunCommand(command, output, foo, dir, verbose);
}

#if defined(WIN32) && !defined(__CYGWIN__)
// use this for shell commands like echo and dir
bool RunCommandViaSystem(const char* command,
                         const char* dir,
                         std::string& output,
                         int& retVal,
                         bool verbose)
{  
  const int BUFFER_SIZE = 4096;
  char buffer[BUFFER_SIZE];
  std::string commandInDir;
  if(dir)
    {
    commandInDir = "cd ";
    commandInDir += cmSystemTools::ConvertToOutputPath(dir);
    commandInDir += " && ";
    commandInDir += command;
    }
  else
    {
    commandInDir = command;
    }
  command = commandInDir.c_str();
  std::string commandToFile = command;
  commandToFile += " > ";
  std::string tempFile;
  tempFile += _tempnam(0, "cmake");

  commandToFile += tempFile;
  retVal = system(commandToFile.c_str());
  std::ifstream fin(tempFile.c_str());
  if(!fin)
    {
    if(verbose)
      {
      std::string errormsg = "RunCommand produced no output: command: \"";
      errormsg += command;
      errormsg += "\"";
      errormsg += "\nOutput file: ";
      errormsg += tempFile;
      cmSystemTools::Error(errormsg.c_str());
      }
    fin.close();
    cmSystemTools::RemoveFile(tempFile.c_str());
    return false;
    }
  bool multiLine = false;
  while(fin)
    {
    fin.getline(buffer, BUFFER_SIZE);
    output += buffer;
    if(multiLine)
      {
      output += "\n";
      }
    multiLine = true;
    }
  fin.close();
  cmSystemTools::RemoveFile(tempFile.c_str());
  return true;
}


#endif  // endif WIN32 not CYGWIN


// run a command unix uses popen (easy)
// windows uses system and ShortPath
bool cmSystemTools::RunCommand(const char* command, 
                               std::string& output,
                               int &retVal, 
                               const char* dir,
                               bool verbose)
{
  if(s_DisableRunCommandOutput)
    {
    verbose = false;
    }
  
#if defined(WIN32) && !defined(__CYGWIN__)
  // if the command does not start with a quote, then
  // try to find the program, and if the program can not be
  // found use system to run the command as it must be a built in
  // shell command like echo or dir
  int count = 0;
  if(command[0] == '\"')
    {
    // count the number of quotes
    for(const char* s = command; *s != 0; ++s)
      {
      if(*s == '\"')
        {
        count++;
        if(count > 2)
          {
          break;
          }
        }      
      }
    // if there are more than two double quotes use 
    // GetShortPathName, the cmd.exe program in windows which
    // is used by system fails to execute if there are more than
    // one set of quotes in the arguments
    if(count > 2)
      {
      cmRegularExpression quoted("^\"([^\"]*)\"[ \t](.*)");
      if(quoted.find(command))
        {
        std::string shortCmd;
        std::string cmd = quoted.match(1);
        std::string args = quoted.match(2);
        if(!cmSystemTools::GetShortPath(cmd.c_str(), shortCmd))
          {
          cmSystemTools::Error("GetShortPath failed for " , cmd.c_str());
          return false;
          }
        shortCmd += " ";
        shortCmd += args;
        return RunCommandViaSystem(shortCmd.c_str(), dir, 
                                   output, retVal, verbose);
        }
      else
        {
        cmSystemTools::Error("Could not parse command line with quotes ", 
                             command);
        }
      }
    }
  // if there is only one set of quotes or no quotes then just run the command
  return RunCommandViaSystem(command, dir, output, retVal, verbose);
#else
  // if only popen worked on windows.....
  std::string commandInDir;
  if(dir)
    {
    commandInDir = "cd ";
    commandInDir += dir;
    commandInDir += " && ";
    commandInDir += command;
    }
  else
    {
    commandInDir = command;
    }
  commandInDir += " 2>&1";
  command = commandInDir.c_str();
  const int BUFFER_SIZE = 4096;
  char buffer[BUFFER_SIZE];
  if(verbose)
    {
    std::cout << "running " << command << std::endl;
    }
  fflush(stdout);
  fflush(stderr);
  FILE* cpipe = popen(command, "r");
  if(!cpipe)
    {
    return false;
    }
  fgets(buffer, BUFFER_SIZE, cpipe);
  while(!feof(cpipe))
    {
    if(verbose)
      {
      std::cout << buffer << std::flush;
      }
    output += buffer;
    fgets(buffer, BUFFER_SIZE, cpipe);
    }

  retVal = pclose(cpipe);
  if (WIFEXITED(retVal))
    {
    retVal = WEXITSTATUS(retVal);
    return true;
    }
  if (WIFSIGNALED(retVal))
    {
    retVal = WTERMSIG(retVal);
    cmStringStream error;
    error << "\nProcess terminated due to ";
    switch (retVal)
      {
#ifdef SIGKILL
      case SIGKILL:
	error << "SIGKILL";
	break;
#endif
#ifdef SIGFPE
      case SIGFPE:
	error << "SIGFPE";
	break;
#endif
#ifdef SIGBUS
      case SIGBUS:
	error << "SIGBUS";
	break;
#endif
#ifdef SIGSEGV
      case SIGSEGV:
	error << "SIGSEGV";
	break;
#endif
      default:
	error << "signal " << retVal;
	break;
      }
    output += error.str();
    }
  return false;
#endif
}

/**
 * Find the file the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the file if it is
 * found.  Otherwise, the empty string is returned.
 */
std::string cmSystemTools::FindFile(const char* name, 
				       const std::vector<std::string>& userPaths)
{
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
    if(cmSystemTools::FileExists(tryPath.c_str()) &&
      !cmSystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return cmSystemTools::CollapseFullPath(tryPath.c_str());
      }
    }
  // Couldn't find the file.
  return "";
}

/**
 * Find the executable with the given name.  Searches the given path and then
 * the system search path.  Returns the full path to the executable if it is
 * found.  Otherwise, the empty string is returned.
 */
std::string cmSystemTools::FindProgram(const char* name,
				       const std::vector<std::string>& userPaths,
                                       bool no_system_path)
{
  // See if the executable exists as written.
  if(cmSystemTools::FileExists(name) &&
      !cmSystemTools::FileIsDirectory(name))
    {
    return cmSystemTools::CollapseFullPath(name);
    }
  std::string tryPath = name;
  tryPath += cmSystemTools::GetExecutableExtension();
  if(cmSystemTools::FileExists(tryPath.c_str()) &&
     !cmSystemTools::FileIsDirectory(tryPath.c_str()))
    {
    return cmSystemTools::CollapseFullPath(tryPath.c_str());
    }

  // Add the system search path to our path.
  std::vector<std::string> path = userPaths;
  if (!no_system_path)
    {
    cmSystemTools::GetPath(path);
    }

  for(std::vector<std::string>::const_iterator p = path.begin();
      p != path.end(); ++p)
    {
    tryPath = *p;
    tryPath += "/";
    tryPath += name;
    if(cmSystemTools::FileExists(tryPath.c_str()) &&
      !cmSystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return cmSystemTools::CollapseFullPath(tryPath.c_str());
      }
#ifdef _WIN32
    tryPath += ".com";
    if(cmSystemTools::FileExists(tryPath.c_str()) &&
       !cmSystemTools::FileIsDirectory(tryPath.c_str()))
      {
      return cmSystemTools::CollapseFullPath(tryPath.c_str());
      }
    tryPath = *p;
    tryPath += "/";
    tryPath += name;
#endif
    tryPath += cmSystemTools::GetExecutableExtension();
    if(cmSystemTools::FileExists(tryPath.c_str()) &&
       !cmSystemTools::FileIsDirectory(tryPath.c_str()))
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
                                       const std::vector<std::string>& userPaths,
                                       const cmMakefile *makefile)
{
  // See if the executable exists as written.
  if(cmSystemTools::FileExists(name))
    {
    return cmSystemTools::CollapseFullPath(name);
    }
    
  // Add the system search path to our path.
  std::vector<std::string> path = userPaths;
  cmSystemTools::GetPath(path);

  // Add some lib directories specific to compilers, depending on the
  // current generator, so that library that might have been stored here
  // can be found too.
  // i.e. Microsoft Visual Studio or .Net: path to compiler/../Lib
  //      Borland: path to compiler/../Lib
  if (makefile)
    {
    const char* genName = makefile->GetDefinition("CMAKE_GENERATOR");
    if (genName)
      {
      if (!strcmp(genName, "NMake Makefiles") ||
          !strcmp(genName, "Visual Studio 6"))
        {
        const char* compiler = makefile->GetDefinition("CMAKE_CXX_COMPILER");
        if (compiler)
          {
          std::string compiler_path = cmSystemTools::FindProgram(compiler);
          if (compiler_path.size())
            {
            std::string lib_path = 
              cmSystemTools::GetFilenamePath(
                cmSystemTools::GetFilenamePath(compiler_path)) + "/Lib";
            path.push_back(lib_path);
            }
          }
        }
      else if (!strcmp(genName, "Visual Studio 7"))
        {
        // It is likely that the compiler won't be in the path for .Net, but
        // we know where devenv is.
        const char* devenv = makefile->GetDefinition("MICROSOFT_DEVENV");
        if (devenv)
          {
          std::string devenv_path = cmSystemTools::FindProgram(devenv);
          if (devenv_path.size())
            {
            std::string vc7_path = 
              cmSystemTools::GetFilenamePath(
                cmSystemTools::GetFilenamePath(
                  cmSystemTools::GetFilenamePath(devenv_path))) + "/Vc7";
            path.push_back(vc7_path + "/lib");
            path.push_back(vc7_path + "/PlatformSDK/lib");
            }
          }
        }
      else if (!strcmp(genName, "Borland Makefiles"))
        {
        const char* bcb_bin_path = makefile->GetDefinition("BCB_BIN_PATH");
        if (bcb_bin_path)
          {
          std::string lib_path = 
            cmSystemTools::GetFilenamePath(bcb_bin_path) + "/Lib";
          path.push_back(lib_path);
          }
        }
      }
    }
  
  std::string tryPath;
  for(std::vector<std::string>::const_iterator p = path.begin();
      p != path.end(); ++p)
    {
#if defined(_WIN32) && !defined(__CYGWIN__)
    tryPath = *p;
    tryPath += "/";
    tryPath += name;
    tryPath += ".lib";
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      return cmSystemTools::CollapseFullPath(tryPath.c_str());
      }
#else
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
    tryPath = *p;
    tryPath += "/lib";
    tryPath += name;
    tryPath += ".dylib";
    if(cmSystemTools::FileExists(tryPath.c_str()))
      {
      return cmSystemTools::CollapseFullPath(tryPath.c_str());
      }
#endif
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

int cmSystemTools::ChangeDirectory(const char *dir)
{
  return Chdir(dir);
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
#ifdef _WIN32
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
#else
# ifdef MAXPATHLEN
  char resolved_name[MAXPATHLEN];
# else
#  ifdef PATH_MAX
  char resolved_name[PATH_MAX];
#  else
  char resolved_name[5024];
#  endif
# endif
  if(dir != "")
    {
    realpath(dir.c_str(), resolved_name);
    dir = resolved_name;
    }
  else
    {
    dir = cmSystemTools::GetCurrentWorkingDirectory();
    }
  if(file == "")
    {
    return dir;
    }
  return dir + "/" + file;
#endif
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
std::string cmSystemTools::GetFilenameWithoutExtension(const std::string& filename)
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


/**
 * Return file name without extension of a full filename (i.e. without path).
 * Warning: it considers the last extension (for example: removes .gz
 * from .tar.gz)
 */
std::string
cmSystemTools::GetFilenameWithoutLastExtension(const std::string& filename)
{
  std::string name = cmSystemTools::GetFilenameName(filename);
  std::string::size_type dot_pos = name.rfind(".");
  if(dot_pos != std::string::npos)
    {
    return name.substr(0, dot_pos);
    }
  else
    {
    return name;
    }
}

bool cmSystemTools::FileIsFullPath(const char* in_name)
{
  std::string name = in_name;
#if defined(_WIN32)
  // On Windows, the name must be at least two characters long.
  if(name.length() < 2)
    {
    return false;
    }
  if(name[1] == ':')
    {
    return true;
    }
  if(name[0] == '\\')
    {
    return true;
    }
#else
  // On UNIX, the name must be at least one character long.
  if(name.length() < 1)
    {
    return false;
    }
#endif
  // On UNIX, the name must begin in a '/'.
  // On Windows, if the name begins in a '/', then it is a full
  // network path.
  if(name[0] == '/')
    {
    return true;
    }
  return false;
}

void cmSystemTools::Glob(const char *directory, const char *regexp,
                         std::vector<std::string>& files)
{
  cmDirectory d;
  cmRegularExpression reg(regexp);
  
  if (d.Load(directory))
    {
    size_t numf;
	unsigned int i;
    numf = d.GetNumberOfFiles();
    for (i = 0; i < numf; i++)
      {
      std::string fname = d.GetFile(i);
      if (reg.find(fname))
        {
        files.push_back(fname);
        }
      }
    }
}


void cmSystemTools::GlobDirs(const char *fullPath,
                             std::vector<std::string>& files)
{
  std::string path = fullPath;
  std::string::size_type pos = path.find("/*");
  if(pos == std::string::npos)
    {
    files.push_back(fullPath);
    return;
    }
  std::string startPath = path.substr(0, pos);
  std::string finishPath = path.substr(pos+2);

  cmDirectory d;
  if (d.Load(startPath.c_str()))
    {
    for (unsigned int i = 0; i < d.GetNumberOfFiles(); ++i)
      {
      if((std::string(d.GetFile(i)) != ".")
         && (std::string(d.GetFile(i)) != ".."))
        {
        std::string fname = startPath;
        fname +="/";
        fname += d.GetFile(i);
        if(cmSystemTools::FileIsDirectory(fname.c_str()))
          {
          fname += finishPath;
          cmSystemTools::GlobDirs(fname.c_str(), files);
          }
        }
      }
    }
}


void cmSystemTools::ExpandListArguments(std::vector<std::string> const& arguments, 
                                        std::vector<std::string>& newargs)
{
  std::vector<std::string>::const_iterator i;
  std::string newarg;
  for(i = arguments.begin();i != arguments.end(); ++i)
    {
    // if there are no ; in the name then just copy the current string
    if(i->find(';') == std::string::npos)
      {
      newargs.push_back(*i);
      }
    else
      {
      std::string::size_type start = 0;
      std::string::size_type endpos = 0;
      const std::string::size_type size = i->size();
      // break up ; separated sections of the string into separate strings
      while(endpos != size)
        {
        endpos = i->find(';', start); 
        if(endpos == std::string::npos)
          {
          endpos = i->size();
          }
        else
          {
          // skip right over escaped ; ( \; )
          while((endpos != std::string::npos)
                && (endpos > 0) 
                && ((*i)[endpos-1] == '\\') )
            {
            endpos = i->find(';', endpos+1);
            }
          if(endpos == std::string::npos)
            {
            endpos = i->size();
            }
          }
        std::string::size_type len = endpos - start;
        if (len > 0)
          {
          // check for a closing ] after the start position
          if(i->find('[', start) == std::string::npos)
            {
            // if there is no [ in the string then keep it
            newarg = i->substr(start, len);
            }
          else
            {
            int opencount = 0;
            int closecount = 0;
            for(std::string::size_type j = start; j < endpos; ++j)
              {
              if(i->at(j) == '[')
                {
                ++opencount;
                }
              else if (i->at(j) == ']')
                {
                ++closecount;
                }
              }
            if(opencount != closecount)
              {
              // skip this one
              endpos = i->find(';', endpos+1);  
              if(endpos == std::string::npos)
                {
                endpos = i->size();
                }
              len = endpos - start;
              }
            newarg = i->substr(start, len);
            }
          std::string::size_type pos = newarg.find("\\;");
          if(pos != std::string::npos)
            {
            newarg.erase(pos, 1);
            }
          newargs.push_back(newarg);
          }
        start = endpos+1;
        }
      }
    }
}

bool cmSystemTools::GetShortPath(const char* path, std::string& shortPath)
{
#if defined(WIN32) && !defined(__CYGWIN__)  
  const int size = int(strlen(path)) +1; // size of return
  char *buffer = new char[size];  // create a buffer
  char *tempPath = new char[size];  // create a buffer
  int ret;
  
  // if the path passed in has quotes around it, first remove the quotes
  if (path[0] == '"' && path[strlen(path)-1] == '"')
    {
    strcpy(tempPath,path+1);
    tempPath[strlen(tempPath)-1] = '\0';
    }
  else
    {
    strcpy(tempPath,path);
    }
  
  buffer[0] = 0;
  ret = GetShortPathName(tempPath, buffer, size);

  if(buffer[0] == 0 || ret > size)
    {
    if(ret < size)
      {
      LPVOID lpMsgBuf;
      FormatMessage( 
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM | 
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
        (LPTSTR) &lpMsgBuf,
        0,
        NULL 
        );
      cmSystemTools::Error((LPCTSTR)lpMsgBuf);
      LocalFree( lpMsgBuf );
      }
    cmSystemTools::Error("Unable to get a short path: ", path);
    delete [] tempPath;
    return false;
    }
  else
    {
    shortPath = buffer;
    delete [] buffer;
    delete [] tempPath;
    return true;
    }
#else
  shortPath = path;
  return true;
#endif
}
