/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmSystemTools.h"   
#include <ctype.h>
#include <errno.h>
#include <time.h>

#include <cmsys/RegularExpression.hxx>
#include <cmsys/Directory.hxx>
#include <cmsys/Process.h>

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
#else
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#endif


bool cmSystemTools::s_RunCommandHideConsole = false;
bool cmSystemTools::s_DisableRunCommandOutput = false;
bool cmSystemTools::s_ErrorOccured = false;
bool cmSystemTools::s_FatalErrorOccured = false;
bool cmSystemTools::s_DisableMessages = false;
bool cmSystemTools::s_ForceUnixPaths = false;

std::string cmSystemTools::s_Windows9xComspecSubstitute = "command.com";
void cmSystemTools::SetWindows9xComspecSubstitute(const char* str)
{
  if ( str )
    {
    cmSystemTools::s_Windows9xComspecSubstitute = str;
    }
}
const char* cmSystemTools::GetWindows9xComspecSubstitute()
{
  return cmSystemTools::s_Windows9xComspecSubstitute.c_str();
}

void (*cmSystemTools::s_ErrorCallback)(const char*, const char*, bool&, void*);
void (*cmSystemTools::s_StdoutCallback)(const char*, int len, void*);
void* cmSystemTools::s_ErrorCallbackClientData = 0;
void* cmSystemTools::s_StdoutCallbackClientData = 0;

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
  cmsys::RegularExpression regEntry("\\[(HKEY[^]]*)\\]");
  
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
        case '#': result.insert(result.end(), '#'); break;
        case '(': result.insert(result.end(), '('); break;
        case ')': result.insert(result.end(), ')'); break;
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

void cmSystemTools::SetStdoutCallback(StdoutCallback f, void* clientData)
{
  s_StdoutCallback = f;
  s_StdoutCallbackClientData = clientData;
}

void cmSystemTools::Stdout(const char* s)
{
  if(s_StdoutCallback)
    {
    (*s_StdoutCallback)(s, strlen(s), s_StdoutCallbackClientData);
    }
  else
    {
    std::cout << s;
    std::cout.flush();
    }
}

void cmSystemTools::Stdout(const char* s, int length)
{
  if(s_StdoutCallback)
    {
    (*s_StdoutCallback)(s, length, s_StdoutCallbackClientData);
    }
  else
    {
    std::cout.write(s, length);
    std::cout.flush();
    }
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
    std::cerr << m1 << std::endl << std::flush;
    }
  
}


void cmSystemTools::ReportLastSystemError(const char* msg)
{
  std::string m = msg;
  m += ": System Error: ";
  m += Superclass::GetLastSystemError();
  cmSystemTools::Error(m.c_str());
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

bool cmSystemTools::IsNOTFOUND(const char* val)
{
  cmsys::RegularExpression reg("-NOTFOUND$");
  if(reg.find(val))
    {
    return true;
    }
  return std::string("NOTFOUND") == val;
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
          v == "N" || cmSystemTools::IsNOTFOUND(v.c_str()) || v == "IGNORE");
}

std::vector<cmStdString> cmSystemTools::ParseArguments(const char* command)
{
  std::vector<cmStdString> args;
  std::string arg;

  bool win_path = false;

  if ( command[0] != '/' && command[1] == ':' && command[2] == '\\' ||
       command[0] == '\"' && command[1] != '/' && command[2] == ':' && command[3] == '\\' || 
       command[0] == '\\' && command[1] == '\\')
    {
    win_path = true;
    }
  // Split the command into an argv array.
  for(const char* c = command; *c;)
    {
    // Skip over whitespace.
    while(*c == ' ' || *c == '\t')
      {
      ++c;
      }
    arg = "";
    if(*c == '"')
      {
      // Parse a quoted argument.
      ++c;
      while(*c && *c != '"')
        {
        arg.append(1, *c);
        ++c;
        }
      if(*c)
        {
        ++c;
        }
      args.push_back(arg);
      }
    else if(*c)
      {
      // Parse an unquoted argument.
      while(*c && *c != ' ' && *c != '\t')
        {
        if(*c == '\\' && !win_path)
          {
          ++c;
          if(*c)
            {
            arg.append(1, *c);
            ++c;
            }
          }
        else
          {
          arg.append(1, *c);
          ++c;
          }
        }
      args.push_back(arg);
      }
    }
  
  return args;
}

bool cmSystemTools::RunSingleCommand(
  const char* command, 
  std::string* output,
  int *retVal, 
  const char* dir,
  bool verbose,
  int timeout)
{
  if(s_DisableRunCommandOutput)
    {
    verbose = false;
    }

  std::vector<cmStdString> args = cmSystemTools::ParseArguments(command);

  if(args.size() < 1)
    {
    return false;
    }
  
  std::vector<const char*> argv;
  for(std::vector<cmStdString>::const_iterator a = args.begin();
      a != args.end(); ++a)
    {
    argv.push_back(a->c_str());
    }
  argv.push_back(0);

  if ( output )
    {
    *output = "";
    }

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_SetCommand(cp, &*argv.begin());
  cmsysProcess_SetWorkingDirectory(cp, dir);
  if(cmSystemTools::GetRunCommandHideConsole())
    {
    cmsysProcess_SetOption(cp, cmsysProcess_Option_HideWindow, 1);
    }
  cmsysProcess_SetTimeout(cp, timeout);
  cmsysProcess_Execute(cp);
  
  std::vector<char> tempOutput;
  char* data;
  int length;
  while(cmsysProcess_WaitForData(cp, &data, &length, 0))
    {
    if ( output )
      {
      tempOutput.insert(tempOutput.end(), data, data+length);
      }
    if(verbose)
      {
      cmSystemTools::Stdout(data, length);
      }
    }
  
  cmsysProcess_WaitForExit(cp, 0);
  if ( output )
    {
    output->append(&*tempOutput.begin(), tempOutput.size());
    }
  
  bool result = true;
  if(cmsysProcess_GetState(cp) == cmsysProcess_State_Exited)
    {
    if ( retVal )
      {
      *retVal = cmsysProcess_GetExitValue(cp);
      }
    else
      {
      if ( cmsysProcess_GetExitValue(cp) !=  0 )
        {
        result = false;
        }
      }
    }
  else if(cmsysProcess_GetState(cp) == cmsysProcess_State_Exception)
    {
    std::cerr << cmsysProcess_GetExceptionString(cp) << "\n";
    result = false;
    }
  else if(cmsysProcess_GetState(cp) == cmsysProcess_State_Error)
    {
    std::cerr << cmsysProcess_GetErrorString(cp) << "\n";
    result = false;
    }
  else if(cmsysProcess_GetState(cp) == cmsysProcess_State_Expired)
    {
    std::cerr << "Process terminated due to timeout\n";
    result = false;
    }
  
  cmsysProcess_Delete(cp);
  
  return result;
}
bool cmSystemTools::RunCommand(const char* command, 
                               std::string& output,
                               const char* dir,
                               bool verbose,
                               int timeout)
{
  int dummy;
  return cmSystemTools::RunCommand(command, output, dummy, 
                                   dir, verbose, timeout);
}

#if defined(WIN32) && !defined(__CYGWIN__)
#include "cmWin32ProcessExecution.h"
// use this for shell commands like echo and dir
bool RunCommandViaWin32(const char* command,
                        const char* dir,
                        std::string& output,
                        int& retVal,
                        bool verbose,
                        int timeout)
{
#if defined(__BORLANDC__)
  return cmWin32ProcessExecution::BorlandRunCommand(command, dir, output, 
                                                    retVal, 
                                                    verbose, timeout, 
                                                    cmSystemTools::GetRunCommandHideConsole());
#else // Visual studio
  ::SetLastError(ERROR_SUCCESS);
  if ( ! command )
    {
    cmSystemTools::Error("No command specified");
    return false;
    }

  cmWin32ProcessExecution resProc;
  if(cmSystemTools::GetRunCommandHideConsole())
    {
    resProc.SetHideWindows(true);
    }
  
  if ( cmSystemTools::GetWindows9xComspecSubstitute() )
    {
    resProc.SetConsoleSpawn(cmSystemTools::GetWindows9xComspecSubstitute() );
    }
  if ( !resProc.StartProcess(command, dir, verbose) )
    {
    return false;
    }
  resProc.Wait(timeout);
  output = resProc.GetOutput();
  retVal = resProc.GetExitValue();
  return true;
#endif
}

// use this for shell commands like echo and dir
bool RunCommandViaSystem(const char* command,
                         const char* dir,
                         std::string& output,
                         int& retVal,
                         bool verbose)
{  
  std::cout << "@@ " << command << std::endl;

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
  std::string line;
  while(cmSystemTools::GetLineFromStream(fin, line))
    {
    output += line;
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

#else // We have popen

bool RunCommandViaPopen(const char* command,
                        const char* dir,
                        std::string& output,
                        int& retVal,
                        bool verbose,
                        int /*timeout*/)
{
  // if only popen worked on windows.....
  std::string commandInDir;
  if(dir)
    {
    commandInDir = "cd \"";
    commandInDir += dir;
    commandInDir += "\" && ";
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
    cmSystemTools::Stdout("running ");
    cmSystemTools::Stdout(command);
    cmSystemTools::Stdout("\n");
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
      cmSystemTools::Stdout(buffer);
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
    cmOStringStream error;
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
}

#endif  // endif WIN32 not CYGWIN


// run a command unix uses popen (easy)
// windows uses system and ShortPath
bool cmSystemTools::RunCommand(const char* command, 
                               std::string& output,
                               int &retVal, 
                               const char* dir,
                               bool verbose,
                               int timeout)
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
      cmsys::RegularExpression quoted("^\"([^\"]*)\"[ \t](.*)");
      if(quoted.find(command))
        {
        std::string shortCmd;
        std::string cmd = quoted.match(1);
        std::string args = quoted.match(2);
        if(! cmSystemTools::FileExists(cmd.c_str()) )
          {
          shortCmd = cmd;
          }
        else if(!cmSystemTools::GetShortPath(cmd.c_str(), shortCmd))
          {
         cmSystemTools::Error("GetShortPath failed for " , cmd.c_str());
          return false;
          }
        shortCmd += " ";
        shortCmd += args;

        //return RunCommandViaSystem(shortCmd.c_str(), dir, 
        //                           output, retVal, verbose);
        //return WindowsRunCommand(shortCmd.c_str(), dir, 
        //output, retVal, verbose);
        return RunCommandViaWin32(shortCmd.c_str(), dir, 
                                  output, retVal, verbose, timeout);
        }
      else
        {
        cmSystemTools::Error("Could not parse command line with quotes ", 
                             command);
        }
      }
    }
  // if there is only one set of quotes or no quotes then just run the command
  //return RunCommandViaSystem(command, dir, output, retVal, verbose);
  //return WindowsRunCommand(command, dir, output, retVal, verbose);
  return ::RunCommandViaWin32(command, dir, output, retVal, verbose, timeout);
#else
  return ::RunCommandViaPopen(command, dir, output, retVal, verbose, timeout);
#endif
}

bool cmSystemTools::DoesFileExistWithExtensions(
  const char* name,
  const std::vector<std::string>& headerExts)
{
  std::string hname;

  for( std::vector<std::string>::const_iterator ext = headerExts.begin();
       ext != headerExts.end(); ++ext )
    {
    hname = name;
    hname += ".";
    hname += *ext;
    if(cmSystemTools::FileExists(hname.c_str()))
      {
      return true;
      }
    }
  return false;
}

bool cmSystemTools::cmCopyFile(const char* source, const char* destination)
{
  return Superclass::CopyFileAlways(source, destination);
}

void cmSystemTools::Glob(const char *directory, const char *regexp,
                         std::vector<std::string>& files)
{
  cmsys::Directory d;
  cmsys::RegularExpression reg(regexp);
  
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

  cmsys::Directory d;
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


void cmSystemTools::ExpandList(std::vector<std::string> const& arguments, 
                               std::vector<std::string>& newargs)
{
  std::vector<std::string>::const_iterator i;
  for(i = arguments.begin();i != arguments.end(); ++i)
    {
    cmSystemTools::ExpandListArgument(*i, newargs);
    }
}

void cmSystemTools::ExpandListArgument(const std::string& arg,
                                       std::vector<std::string>& newargs)
{
  std::string newarg;
  // If argument is empty, it is an empty list.
  if(arg.length() == 0)
    {
    return;
    }
  // if there are no ; in the name then just copy the current string
  if(arg.find(';') == std::string::npos)
    {
    newargs.push_back(arg);
    return;
    }
  // Break the string at non-escaped semicolons not nested in [].
  int squareNesting = 0;
  for(const char* c = arg.c_str(); *c; ++c)
    {
    switch(*c)
      {
      case '\\':
        {
        // We only want to allow escaping of semicolons.  Other
        // escapes should not be processed here.
        ++c;
        if(*c == ';')
          {
          newarg += ';';
          }
        else
          {
          newarg += '\\';
          if(*c)
            {
            newarg += *c;
            }
          }
        } break;
      case '[':
        {
        ++squareNesting;
        newarg += '[';
        } break;
      case ']':
        {
        --squareNesting;
        newarg += ']';
        } break;
      case ';':
        {
        // Break the string here if we are not nested inside square
        // brackets.
        if(squareNesting == 0)
          {
          if(newarg.length())
            {
            // Add an argument if the string is not empty.
            newargs.push_back(newarg);
            newarg = "";
            }
          }
        else
          {
          newarg += ';';
          }
        } break;
      default:
        {
        // Just append this character.
        newarg += *c;
        } break;
      }
    }
  if(newarg.length())
    {
    // Add the last argument if the string is not empty.
    newargs.push_back(newarg);
    }
}

bool cmSystemTools::SimpleGlob(const cmStdString& glob, 
                               std::vector<cmStdString>& files, 
                               int type /* = 0 */)
{
  files.clear();
  if ( glob[glob.size()-1] != '*' )
    {
    return false;
    }
  std::string path = cmSystemTools::GetFilenamePath(glob);
  std::string ppath = cmSystemTools::GetFilenameName(glob);
  ppath = ppath.substr(0, ppath.size()-1);
  if ( path.size() == 0 )
    {
    path = "/";
    }

  bool res = false;
  cmsys::Directory d;
  if (d.Load(path.c_str()))
    {
    for (unsigned int i = 0; i < d.GetNumberOfFiles(); ++i)
      {
      if((std::string(d.GetFile(i)) != ".")
         && (std::string(d.GetFile(i)) != ".."))
        {
        std::string fname = path;
        if ( path[path.size()-1] != '/' )
          {
          fname +="/";
          }
        fname += d.GetFile(i);
        std::string sfname = d.GetFile(i);
        if ( type > 0 && cmSystemTools::FileIsDirectory(fname.c_str()) )
          {
          continue;
          }
        if ( type < 0 && !cmSystemTools::FileIsDirectory(fname.c_str()) )
          {
          continue;
          }
        if ( sfname.size() >= ppath.size() && 
             sfname.substr(0, ppath.size()) == 
             ppath )
          {
          files.push_back(fname);
          res = true;
          }
        }
      }
    }
  return res;
}

cmSystemTools::FileFormat cmSystemTools::GetFileFormat(const char* cext)
{
  if ( ! cext || *cext == 0 )
    {
    return cmSystemTools::NO_FILE_FORMAT;
    }
  //std::string ext = cmSystemTools::LowerCase(cext);
  std::string ext = cext;
  if ( ext == "c" || ext == ".c" ) { return cmSystemTools::C_FILE_FORMAT; }
  if ( 
    ext == "C" || ext == ".C" ||
    ext == "M" || ext == ".M" ||
    ext == "c++" || ext == ".c++" ||
    ext == "cc" || ext == ".cc" ||
    ext == "cpp" || ext == ".cpp" ||
    ext == "cxx" || ext == ".cxx" ||
    ext == "m" || ext == ".m" ||
    ext == "mm" || ext == ".mm"
    ) { return cmSystemTools::CXX_FILE_FORMAT; }
  if ( ext == "java" || ext == ".java" ) { return cmSystemTools::JAVA_FILE_FORMAT; }
  if ( 
    ext == "H" || ext == ".H" || 
    ext == "h" || ext == ".h" || 
    ext == "h++" || ext == ".h++" ||
    ext == "hm" || ext == ".hm" || 
    ext == "hpp" || ext == ".hpp" || 
    ext == "hxx" || ext == ".hxx" ||
    ext == "in" || ext == ".in" ||
    ext == "txx" || ext == ".txx"
    ) { return cmSystemTools::HEADER_FILE_FORMAT; }
  if ( ext == "rc" || ext == ".rc" ) { return cmSystemTools::RESOURCE_FILE_FORMAT; }
  if ( ext == "def" || ext == ".def" ) { return cmSystemTools::DEFINITION_FILE_FORMAT; }
  if ( ext == "lib" || ext == ".lib" ||
       ext == "a" || ext == ".a") { return cmSystemTools::STATIC_LIBRARY_FILE_FORMAT; }
  if ( ext == "o" || ext == ".o" ||
       ext == "obj" || ext == ".obj") { return cmSystemTools::OBJECT_FILE_FORMAT; }
#ifdef __APPLE__
  if ( ext == "dylib" || ext == ".dylib" ) 
    { return cmSystemTools::SHARED_LIBRARY_FILE_FORMAT; }
  if ( ext == "so" || ext == ".so" || 
       ext == "bundle" || ext == ".bundle" ) 
    { return cmSystemTools::MODULE_FILE_FORMAT; } 
#else // __APPLE__
  if ( ext == "so" || ext == ".so" || 
       ext == "sl" || ext == ".sl" || 
       ext == "dll" || ext == ".dll" ) 
    { return cmSystemTools::SHARED_LIBRARY_FILE_FORMAT; }
#endif // __APPLE__
  return cmSystemTools::UNKNOWN_FILE_FORMAT;
}

bool cmSystemTools::Split(const char* s, std::vector<cmStdString>& l)
{
  std::vector<std::string> temp;
  if(!Superclass::Split(s, temp))
    {
    return false;
    }
  for(std::vector<std::string>::const_iterator i = temp.begin();
      i != temp.end(); ++i)
    {
    l.push_back(*i);
    }
  return true;
}

std::string cmSystemTools::ConvertToOutputPath(const char* path)
{
#if defined(_WIN32) && !defined(__CYGWIN__)
  if(s_ForceUnixPaths)
    {
    return cmSystemTools::ConvertToUnixOutputPath(path);
    }
  return cmSystemTools::ConvertToWindowsOutputPath(path);
#else
  return cmSystemTools::ConvertToUnixOutputPath(path);
#endif
}

bool cmSystemTools::StringEndsWith(const char* str1, const char* str2)
{
  if ( !str1 || !str2 || strlen(str1) < strlen(str2) )
    {
    return 0;
    }
  return !strncmp(str1 + (strlen(str1)-strlen(str2)), str2, strlen(str2));
}

#if defined(_WIN32) && !defined(__CYGWIN__)
bool cmSystemTools::CreateSymlink(const char*, const char*)
{
  // Should we create a copy here?
  return false;
}
#else
bool cmSystemTools::CreateSymlink(const char* origName, const char* newName)
{
  return (symlink(origName, newName) >= 0);
}
#endif


std::vector<cmStdString> cmSystemTools::SplitString(const char* p, char sep)
{
  std::string path = p;
  std::vector<cmStdString> paths;
  std::string::size_type pos1 = 0;
  std::string::size_type pos2 = path.find(sep, pos1+1);
  while(pos2 != std::string::npos)
    {
    paths.push_back(path.substr(pos1, pos2-pos1));
    pos1 = pos2+1;
    pos2 = path.find(sep, pos1+1);
    } 
  paths.push_back(path.substr(pos1, pos2-pos1));
  
  return paths;
}


// compute the relative path from here to there
std::string cmSystemTools::RelativePath(const char* local, const char* remote)
{
#ifdef _WIN32
  std::string lowerCaseLocal = cmSystemTools::LowerCase(std::string(local));
  std::string lowerCaseRemote = cmSystemTools::LowerCase(std::string(remote));
  remote = lowerCaseRemote.c_str();
  local = lowerCaseLocal.c_str();
#endif
  // check for driveletter: as the start of the path
  // if not on the same drive then full path to remote must be used.
  if(local[0] && local[0] != '/')
    {
    if(remote[0] && local[0] != remote[0])
      {
      return remote;
      }
    }
  std::string relativePath;     // result string
  // split up both paths into arrays of strings using / as a separator
  std::vector<cmStdString> fileSplit = cmSystemTools::SplitString(local);
  std::vector<cmStdString> relativeSplit = cmSystemTools::SplitString(remote);
  // count up how many mathing directory names there are from the start
  unsigned int sameCount = 0;
  while(sameCount < fileSplit.size()-1 && sameCount < relativeSplit.size()-1 && 
        fileSplit[sameCount] == relativeSplit[sameCount])
    {
    sameCount++;
    }
  if(sameCount == 0)
    {
    return std::string(remote);
    }
  // put in sameCount number of ../ into the path
  unsigned int i;
  for(i = sameCount; i < fileSplit.size(); ++i)
    {
    relativePath += "../";
    }
  // now put the rest of path that did not match back
  for(i = sameCount; i < relativeSplit.size()-1; ++i)
    {
    relativePath += relativeSplit[i] + "/";
    }
  relativePath += relativeSplit[i];
  return relativePath;
}
