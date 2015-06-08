/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmOutputConverter.h"

#include "cmAlgorithms.h"
#include "cmake.h"

#include <cmsys/System.h>

#include <assert.h>

cmOutputConverter::cmOutputConverter(cmState::Snapshot snapshot)
  : StateSnapshot(snapshot), LinkScriptShell(false)
{
}

//----------------------------------------------------------------------------
std::string
cmOutputConverter::ConvertToOutputForExistingCommon(const std::string& remote,
                                                    std::string const& result,
                                                    OutputFormat format)
{
  // If this is a windows shell, the result has a space, and the path
  // already exists, we can use a short-path to reference it without a
  // space.
  if(this->GetState()->UseWindowsShell() && result.find(' ') != result.npos &&
     cmSystemTools::FileExists(remote.c_str()))
    {
    std::string tmp;
    if(cmSystemTools::GetShortPath(remote, tmp))
      {
      return this->ConvertToOutputFormat(tmp, format);
      }
    }

  // Otherwise, leave it unchanged.
  return result;
}

//----------------------------------------------------------------------------
std::string
cmOutputConverter::ConvertToOutputForExisting(const std::string& remote,
                                              RelativeRoot local,
                                              OutputFormat format)
{
  static_cast<void>(local);

  // Perform standard conversion.
  std::string result = this->ConvertToOutputFormat(remote, format);

  // Consider short-path.
  return this->ConvertToOutputForExistingCommon(remote, result, format);
}

//----------------------------------------------------------------------------
std::string
cmOutputConverter::ConvertToOutputForExisting(RelativeRoot remote,
                                              const std::string& local,
                                              OutputFormat format)
{
  // Perform standard conversion.
  std::string result = this->Convert(remote, local, format, true);

  // Consider short-path.
  const char* remotePath = this->GetRelativeRootPath(remote);
  return this->ConvertToOutputForExistingCommon(remotePath, result, format);
}

//----------------------------------------------------------------------------
const char* cmOutputConverter::GetRelativeRootPath(RelativeRoot relroot)
{
  switch (relroot)
    {
    case HOME:         return this->GetState()->GetSourceDirectory();
    case START:        return this->StateSnapshot.GetCurrentSourceDirectory();
    case HOME_OUTPUT:  return this->GetState()->GetBinaryDirectory();
    case START_OUTPUT: return this->StateSnapshot.GetCurrentBinaryDirectory();
    default: break;
    }
  return 0;
}

std::string cmOutputConverter::Convert(const std::string& source,
                                       RelativeRoot relative,
                                       OutputFormat output)
{
  // Convert the path to a relative path.
  std::string result = source;

  switch (relative)
    {
  case HOME:
    result = this->ConvertToRelativePath(
          this->GetState()->GetSourceDirectoryComponents(), result);
    break;
  case START:
    result = this->ConvertToRelativePath(
          this->StateSnapshot.GetCurrentSourceDirectoryComponents(), result);
    break;
  case HOME_OUTPUT:
    result = this->ConvertToRelativePath(
          this->GetState()->GetBinaryDirectoryComponents(), result);
    break;
  case START_OUTPUT:
    result = this->ConvertToRelativePath(
          this->StateSnapshot.GetCurrentBinaryDirectoryComponents(), result);
    break;
  case FULL:
    result = cmSystemTools::CollapseFullPath(result);
    break;
  case NONE:
    break;
    }
  return this->ConvertToOutputFormat(result, output);
}

//----------------------------------------------------------------------------
std::string cmOutputConverter::ConvertToOutputFormat(const std::string& source,
                                                     OutputFormat output)
{
  std::string result = source;
  // Convert it to an output path.
  if (output == MAKERULE)
    {
    result = cmSystemTools::ConvertToOutputPath(result.c_str());
    }
  else if(output == SHELL || output == WATCOMQUOTE)
    {
        // For the MSYS shell convert drive letters to posix paths, so
    // that c:/some/path becomes /c/some/path.  This is needed to
    // avoid problems with the shell path translation.
    if(this->GetState()->UseMSYSShell() && !this->LinkScriptShell)
      {
      if(result.size() > 2 && result[1] == ':')
        {
        result[1] = result[0];
        result[0] = '/';
        }
      }
    if(this->GetState()->UseWindowsShell())
      {
      std::replace(result.begin(), result.end(), '/', '\\');
      }
    result = this->EscapeForShell(result, true, false, output == WATCOMQUOTE);
    }
  else if(output == RESPONSE)
    {
    result = this->EscapeForShell(result, false, false, false);
    }
  return result;
}

//----------------------------------------------------------------------------
std::string cmOutputConverter::Convert(RelativeRoot remote,
                                      const std::string& local,
                                      OutputFormat output,
                                      bool optional)
{
  const char* remotePath = this->GetRelativeRootPath(remote);

  // The relative root must have a path (i.e. not FULL or NONE)
  assert(remotePath != 0);

  if(!local.empty() && !optional)
    {
    std::vector<std::string> components;
    cmSystemTools::SplitPath(local, components);
    std::string result = this->ConvertToRelativePath(components, remotePath);
    return this->ConvertToOutputFormat(result, output);
    }

  return this->ConvertToOutputFormat(remotePath, output);
}

//----------------------------------------------------------------------------
static bool cmOutputConverterNotAbove(const char* a, const char* b)
{
  return (cmSystemTools::ComparePath(a, b) ||
          cmSystemTools::IsSubDirectory(a, b));
}

//----------------------------------------------------------------------------
std::string
cmOutputConverter::ConvertToRelativePath(const std::vector<std::string>& local,
                                        const std::string& in_remote,
                                        bool force)
{
  // The path should never be quoted.
  assert(in_remote[0] != '\"');

  // The local path should never have a trailing slash.
  assert(!local.empty() && !(local[local.size()-1] == ""));

  // If the path is already relative then just return the path.
  if(!cmSystemTools::FileIsFullPath(in_remote.c_str()))
    {
    return in_remote;
    }

  if(!force)
    {
    // Skip conversion if the path and local are not both in the source
    // or both in the binary tree.
    std::string local_path = cmSystemTools::JoinPath(local);
    if(!((cmOutputConverterNotAbove(local_path.c_str(),
              this->StateSnapshot.GetRelativePathTopBinary()) &&
          cmOutputConverterNotAbove(in_remote.c_str(),
              this->StateSnapshot.GetRelativePathTopBinary())) ||
         (cmOutputConverterNotAbove(local_path.c_str(),
              this->StateSnapshot.GetRelativePathTopSource()) &&
          cmOutputConverterNotAbove(in_remote.c_str(),
              this->StateSnapshot.GetRelativePathTopSource()))))
      {
      return in_remote;
      }
    }

  // Identify the longest shared path component between the remote
  // path and the local path.
  std::vector<std::string> remote;
  cmSystemTools::SplitPath(in_remote, remote);
  unsigned int common=0;
  while(common < remote.size() &&
        common < local.size() &&
        cmSystemTools::ComparePath(remote[common],
                                   local[common]))
    {
    ++common;
    }

  // If no part of the path is in common then return the full path.
  if(common == 0)
    {
    return in_remote;
    }

  // If the entire path is in common then just return a ".".
  if(common == remote.size() &&
     common == local.size())
    {
    return ".";
    }

  // If the entire path is in common except for a trailing slash then
  // just return a "./".
  if(common+1 == remote.size() &&
     remote[common].empty() &&
     common == local.size())
    {
    return "./";
    }

  // Construct the relative path.
  std::string relative;

  // First add enough ../ to get up to the level of the shared portion
  // of the path.  Leave off the trailing slash.  Note that the last
  // component of local will never be empty because local should never
  // have a trailing slash.
  for(unsigned int i=common; i < local.size(); ++i)
    {
    relative += "..";
    if(i < local.size()-1)
      {
      relative += "/";
      }
    }

  // Now add the portion of the destination path that is not included
  // in the shared portion of the path.  Add a slash the first time
  // only if there was already something in the path.  If there was a
  // trailing slash in the input then the last iteration of the loop
  // will add a slash followed by an empty string which will preserve
  // the trailing slash in the output.

  if(!relative.empty() && !remote.empty())
    {
    relative += "/";
    }
  relative += cmJoin(cmRange(remote).advance(common), "/");

  // Finally return the path.
  return relative;
}

//----------------------------------------------------------------------------
static bool cmOutputConverterIsShellOperator(const std::string& str)
{
  static std::set<std::string> shellOperators;
  if(shellOperators.empty())
    {
    shellOperators.insert("<");
    shellOperators.insert(">");
    shellOperators.insert("<<");
    shellOperators.insert(">>");
    shellOperators.insert("|");
    shellOperators.insert("||");
    shellOperators.insert("&&");
    shellOperators.insert("&>");
    shellOperators.insert("1>");
    shellOperators.insert("2>");
    shellOperators.insert("2>&1");
    shellOperators.insert("1>&2");
    }
  return shellOperators.count(str) > 0;
}

//----------------------------------------------------------------------------
std::string cmOutputConverter::EscapeForShell(const std::string& str,
                                             bool makeVars,
                                             bool forEcho,
                                             bool useWatcomQuote)
{
  // Do not escape shell operators.
  if(cmOutputConverterIsShellOperator(str))
    {
    return str;
    }

  // Compute the flags for the target shell environment.
  int flags = 0;
  if(this->GetState()->UseWindowsVSIDE())
    {
    flags |= cmsysSystem_Shell_Flag_VSIDE;
    }
  else if(!this->LinkScriptShell)
    {
    flags |= cmsysSystem_Shell_Flag_Make;
    }
  if(makeVars)
    {
    flags |= cmsysSystem_Shell_Flag_AllowMakeVariables;
    }
  if(forEcho)
    {
    flags |= cmsysSystem_Shell_Flag_EchoWindows;
    }
  if(useWatcomQuote)
    {
    flags |= cmsysSystem_Shell_Flag_WatcomQuote;
    }
  if(this->GetState()->UseWatcomWMake())
    {
    flags |= cmsysSystem_Shell_Flag_WatcomWMake;
    }
  if(this->GetState()->UseMinGWMake())
    {
    flags |= cmsysSystem_Shell_Flag_MinGWMake;
    }
  if(this->GetState()->UseNMake())
    {
    flags |= cmsysSystem_Shell_Flag_NMake;
    }

  // Compute the buffer size needed.
  int size = (this->GetState()->UseWindowsShell() ?
              cmsysSystem_Shell_GetArgumentSizeForWindows(str.c_str(), flags) :
              cmsysSystem_Shell_GetArgumentSizeForUnix(str.c_str(), flags));

  // Compute the shell argument itself.
  std::vector<char> arg(size);
  if(this->GetState()->UseWindowsShell())
    {
    cmsysSystem_Shell_GetArgumentForWindows(str.c_str(), &arg[0], flags);
    }
  else
    {
    cmsysSystem_Shell_GetArgumentForUnix(str.c_str(), &arg[0], flags);
    }
  return std::string(&arg[0]);
}

//----------------------------------------------------------------------------
std::string cmOutputConverter::EscapeForCMake(const std::string& str)
{
  // Always double-quote the argument to take care of most escapes.
  std::string result = "\"";
  for(const char* c = str.c_str(); *c; ++c)
    {
    if(*c == '"')
      {
      // Escape the double quote to avoid ending the argument.
      result += "\\\"";
      }
    else if(*c == '$')
      {
      // Escape the dollar to avoid expanding variables.
      result += "\\$";
      }
    else if(*c == '\\')
      {
      // Escape the backslash to avoid other escapes.
      result += "\\\\";
      }
    else
      {
      // Other characters will be parsed correctly.
      result += *c;
      }
    }
  result += "\"";
  return result;
}

//----------------------------------------------------------------------------
cmOutputConverter::FortranFormat
cmOutputConverter::GetFortranFormat(const char* value)
{
  FortranFormat format = FortranFormatNone;
  if(value && *value)
    {
    std::vector<std::string> fmt;
    cmSystemTools::ExpandListArgument(value, fmt);
    for(std::vector<std::string>::iterator fi = fmt.begin();
        fi != fmt.end(); ++fi)
      {
      if(*fi == "FIXED")
        {
        format = FortranFormatFixed;
        }
      if(*fi == "FREE")
        {
        format = FortranFormatFree;
        }
      }
    }
  return format;
}

void cmOutputConverter::SetLinkScriptShell(bool linkScriptShell)
{
  this->LinkScriptShell = linkScriptShell;
}

cmState* cmOutputConverter::GetState() const
{
  return this->StateSnapshot.GetState();
}
