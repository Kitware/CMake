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
#include "cmFileCommand.h"
#include "cmake.h"
#include "cmHexFileConverter.h"
#include "cmFileTimeComparison.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_curl.h"
#endif

#undef GetCurrentDirectory
#include <sys/types.h>
#include <sys/stat.h>

#include <cmsys/Directory.hxx>
#include <cmsys/Glob.hxx>
#include <cmsys/RegularExpression.hxx>

// Table of permissions flags.
#if defined(_WIN32) && !defined(__CYGWIN__)
static mode_t mode_owner_read = S_IREAD;
static mode_t mode_owner_write = S_IWRITE;
static mode_t mode_owner_execute = S_IEXEC;
static mode_t mode_group_read = 0;
static mode_t mode_group_write = 0;
static mode_t mode_group_execute = 0;
static mode_t mode_world_read = 0;
static mode_t mode_world_write = 0;
static mode_t mode_world_execute = 0;
static mode_t mode_setuid = 0;
static mode_t mode_setgid = 0;
#else
static mode_t mode_owner_read = S_IRUSR;
static mode_t mode_owner_write = S_IWUSR;
static mode_t mode_owner_execute = S_IXUSR;
static mode_t mode_group_read = S_IRGRP;
static mode_t mode_group_write = S_IWGRP;
static mode_t mode_group_execute = S_IXGRP;
static mode_t mode_world_read = S_IROTH;
static mode_t mode_world_write = S_IWOTH;
static mode_t mode_world_execute = S_IXOTH;
static mode_t mode_setuid = S_ISUID;
static mode_t mode_setgid = S_ISGID;
#endif

// cmLibraryCommand
bool cmFileCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("must be called with at least two arguments.");
    return false;
    }
  std::string subCommand = args[0];
  if ( subCommand == "WRITE" )
    {
    return this->HandleWriteCommand(args, false);
    }
  else if ( subCommand == "APPEND" )
    {
    return this->HandleWriteCommand(args, true);
    }
  else if ( subCommand == "DOWNLOAD" )
    {
    return this->HandleDownloadCommand(args);
    }
  else if ( subCommand == "READ" )
    {
    return this->HandleReadCommand(args);
    }
  else if ( subCommand == "STRINGS" )
    {
    return this->HandleStringsCommand(args);
    }
  else if ( subCommand == "GLOB" )
    {
    return this->HandleGlobCommand(args, false);
    }
  else if ( subCommand == "GLOB_RECURSE" )
    {
    return this->HandleGlobCommand(args, true);
    }
  else if ( subCommand == "MAKE_DIRECTORY" )
    {
    return this->HandleMakeDirectoryCommand(args);
    }
  else if ( subCommand == "RENAME" )
    {
    return this->HandleRename(args);
    }
  else if ( subCommand == "REMOVE" )
    {
    return this->HandleRemove(args, false);
    }
  else if ( subCommand == "REMOVE_RECURSE" )
    {
    return this->HandleRemove(args, true);
    }
  else if ( subCommand == "INSTALL" )
    {
    return this->HandleInstallCommand(args);
    }
  else if ( subCommand == "DIFFERENT" )
    {
    return this->HandleDifferentCommand(args);
    }
  else if ( subCommand == "RPATH_CHANGE" || subCommand == "CHRPATH" )
    {
    return this->HandleRPathChangeCommand(args);
    }
  else if ( subCommand == "RPATH_CHECK" )
    {
    return this->HandleRPathCheckCommand(args);
    }
  else if ( subCommand == "RPATH_REMOVE" )
    {
    return this->HandleRPathRemoveCommand(args);
    }
  else if ( subCommand == "RELATIVE_PATH" )
    {
    return this->HandleRelativePathCommand(args);
    }
  else if ( subCommand == "TO_CMAKE_PATH" )
    {
    return this->HandleCMakePathCommand(args, false);
    }
  else if ( subCommand == "TO_NATIVE_PATH" )
    {
    return this->HandleCMakePathCommand(args, true);
    }

  std::string e = "does not recognize sub-command "+subCommand;
  this->SetError(e.c_str());
  return false;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleWriteCommand(std::vector<std::string> const& args,
  bool append)
{
  std::string message;
  std::vector<std::string>::const_iterator i = args.begin();

  i++; // Get rid of subcommand

  std::string fileName = *i;
  if ( !cmsys::SystemTools::FileIsFullPath(i->c_str()) )
    {
    fileName = this->Makefile->GetCurrentDirectory();
    fileName += "/" + *i;
    }

  i++;

  for(;i != args.end(); ++i)
    {
    message += *i;
    }
  if ( !this->Makefile->CanIWriteThisFile(fileName.c_str()) )
    {
    std::string e
      = "attempted to write a file: " + fileName +
      " into a source directory.";
    this->SetError(e.c_str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }
  std::string dir = cmSystemTools::GetFilenamePath(fileName);
  cmSystemTools::MakeDirectory(dir.c_str());

  mode_t mode =
#if defined( _MSC_VER ) || defined( __MINGW32__ )
    S_IREAD | S_IWRITE
#elif defined( __BORLANDC__ )
    S_IRUSR | S_IWUSR
#else
    S_IRUSR | S_IWUSR |
    S_IRGRP |
    S_IROTH
#endif
    ;

  // Set permissions to writable
  if ( cmSystemTools::GetPermissions(fileName.c_str(), mode) )
    {
    cmSystemTools::SetPermissions(fileName.c_str(),
#if defined( _MSC_VER ) || defined( __MINGW32__ )
      S_IREAD | S_IWRITE
#else
      S_IRUSR | S_IWUSR
#endif
    );
    }
  // If GetPermissions fails, pretend like it is ok. File open will fail if
  // the file is not writable
  std::ofstream file(fileName.c_str(), append?std::ios::app: std::ios::out);
  if ( !file )
    {
    std::string error = "Internal CMake error when trying to open file: ";
    error += fileName.c_str();
    error += " for writing.";
    this->SetError(error.c_str());
    return false;
    }
  file << message;
  file.close();
  cmSystemTools::SetPermissions(fileName.c_str(), mode);
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleReadCommand(std::vector<std::string> const& args)
{
  if ( args.size() < 3 )
    {
    this->SetError("READ must be called with at least two additional "
                   "arguments");
    return false;
    }

  cmCommandArgumentsHelper argHelper;
  cmCommandArgumentGroup group;

  cmCAString readArg    (&argHelper, "READ");
  cmCAString fileNameArg    (&argHelper, 0);
  cmCAString resultArg      (&argHelper, 0);

  cmCAString offsetArg      (&argHelper, "OFFSET", &group);
  cmCAString limitArg       (&argHelper, "LIMIT", &group);
  cmCAEnabler hexOutputArg  (&argHelper, "HEX", &group);
  readArg.Follows(0);
  fileNameArg.Follows(&readArg);
  resultArg.Follows(&fileNameArg);
  group.Follows(&resultArg);
  argHelper.Parse(&args, 0);

  std::string fileName = fileNameArg.GetString();
  if ( !cmsys::SystemTools::FileIsFullPath(fileName.c_str()) )
    {
    fileName = this->Makefile->GetCurrentDirectory();
    fileName += "/" + fileNameArg.GetString();
    }

  std::string variable = resultArg.GetString();

  // Open the specified file.
#if defined(_WIN32) || defined(__CYGWIN__)
  std::ifstream file(fileName.c_str(), std::ios::in | 
               (hexOutputArg.IsEnabled() ? std::ios::binary : std::ios::in));
#else
  std::ifstream file(fileName.c_str(), std::ios::in);
#endif

  if ( !file )
    {
    std::string error = "Internal CMake error when trying to open file: ";
    error += fileName.c_str();
    error += " for reading.";
    this->SetError(error.c_str());
    return false;
    }

  // is there a limit?
  long sizeLimit = -1;
  if (limitArg.GetString().size() > 0)
    {
    sizeLimit = atoi(limitArg.GetCString());
    }

  // is there an offset?
  long offset = 0;
  if (offsetArg.GetString().size() > 0)
    {
    offset = atoi(offsetArg.GetCString());
    }

  file.seekg(offset);

  std::string output;

  if (hexOutputArg.IsEnabled())
    {
    // Convert part of the file into hex code
    int c;
    while((sizeLimit != 0) && (c = file.get(), file))
      {
      char hex[4];
      sprintf(hex, "%x", c&0xff);
      output += hex;
      if (sizeLimit > 0)
        {
        sizeLimit--;
        }
      }
    }
  else
    {
    std::string line;
    bool has_newline = false;
    while (sizeLimit != 0 &&
          cmSystemTools::GetLineFromStream(file, line, &has_newline,
                                            sizeLimit) )
      {
      if (sizeLimit > 0)
        {
        sizeLimit = sizeLimit - static_cast<long>(line.size());
        if (has_newline)
          {
          sizeLimit--;
          }
        if (sizeLimit < 0)
          {
          sizeLimit = 0;
          }
        }
      output += line;
      if ( has_newline )
        {
        output += "\n";
        }
      }
    }
  this->Makefile->AddDefinition(variable.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleStringsCommand(std::vector<std::string> const& args)
{
  if(args.size() < 3)
    {
    this->SetError("STRINGS requires a file name and output variable");
    return false;
    }

  // Get the file to read.
  std::string fileName = args[1];
  if(!cmsys::SystemTools::FileIsFullPath(fileName.c_str()))
    {
    fileName = this->Makefile->GetCurrentDirectory();
    fileName += "/" + args[1];
    }

  // Get the variable in which to store the results.
  std::string outVar = args[2];

  // Parse the options.
  enum { arg_none,
         arg_limit_input,
         arg_limit_output,
         arg_limit_count,
         arg_length_minimum,
         arg_length_maximum,
         arg__maximum,
         arg_regex };
  unsigned int minlen = 0;
  unsigned int maxlen = 0;
  int limit_input = -1;
  int limit_output = -1;
  unsigned int limit_count = 0;
  cmsys::RegularExpression regex;
  bool have_regex = false;
  bool newline_consume = false;
  bool hex_conversion_enabled = true;
  int arg_mode = arg_none;
  for(unsigned int i=3; i < args.size(); ++i)
    {
    if(args[i] == "LIMIT_INPUT")
      {
      arg_mode = arg_limit_input;
      }
    else if(args[i] == "LIMIT_OUTPUT")
      {
      arg_mode = arg_limit_output;
      }
    else if(args[i] == "LIMIT_COUNT")
      {
      arg_mode = arg_limit_count;
      }
    else if(args[i] == "LENGTH_MINIMUM")
      {
      arg_mode = arg_length_minimum;
      }
    else if(args[i] == "LENGTH_MAXIMUM")
      {
      arg_mode = arg_length_maximum;
      }
    else if(args[i] == "REGEX")
      {
      arg_mode = arg_regex;
      }
    else if(args[i] == "NEWLINE_CONSUME")
      {
      newline_consume = true;
      arg_mode = arg_none;
      }
    else if(args[i] == "NO_HEX_CONVERSION")
      {
      hex_conversion_enabled = false;
      arg_mode = arg_none;
      }
    else if(arg_mode == arg_limit_input)
      {
      if(sscanf(args[i].c_str(), "%d", &limit_input) != 1 ||
         limit_input < 0)
        {
        cmOStringStream e;
        e << "STRINGS option LIMIT_INPUT value \""
          << args[i] << "\" is not an unsigned integer.";
        this->SetError(e.str().c_str());
        return false;
        }
      arg_mode = arg_none;
      }
    else if(arg_mode == arg_limit_output)
      {
      if(sscanf(args[i].c_str(), "%d", &limit_output) != 1 ||
         limit_output < 0)
        {
        cmOStringStream e;
        e << "STRINGS option LIMIT_OUTPUT value \""
          << args[i] << "\" is not an unsigned integer.";
        this->SetError(e.str().c_str());
        return false;
        }
      arg_mode = arg_none;
      }
    else if(arg_mode == arg_limit_count)
      {
      int count;
      if(sscanf(args[i].c_str(), "%d", &count) != 1 || count < 0)
        {
        cmOStringStream e;
        e << "STRINGS option LIMIT_COUNT value \""
          << args[i] << "\" is not an unsigned integer.";
        this->SetError(e.str().c_str());
        return false;
        }
      limit_count = count;
      arg_mode = arg_none;
      }
    else if(arg_mode == arg_length_minimum)
      {
      int len;
      if(sscanf(args[i].c_str(), "%d", &len) != 1 || len < 0)
        {
        cmOStringStream e;
        e << "STRINGS option LENGTH_MINIMUM value \""
          << args[i] << "\" is not an unsigned integer.";
        this->SetError(e.str().c_str());
        return false;
        }
      minlen = len;
      arg_mode = arg_none;
      }
    else if(arg_mode == arg_length_maximum)
      {
      int len;
      if(sscanf(args[i].c_str(), "%d", &len) != 1 || len < 0)
        {
        cmOStringStream e;
        e << "STRINGS option LENGTH_MAXIMUM value \""
          << args[i] << "\" is not an unsigned integer.";
        this->SetError(e.str().c_str());
        return false;
        }
      maxlen = len;
      arg_mode = arg_none;
      }
    else if(arg_mode == arg_regex)
      {
      if(!regex.compile(args[i].c_str()))
        {
        cmOStringStream e;
        e << "STRINGS option REGEX value \""
          << args[i] << "\" could not be compiled.";
        this->SetError(e.str().c_str());
        return false;
        }
      have_regex = true;
      arg_mode = arg_none;
      }
    else
      {
      cmOStringStream e;
      e << "STRINGS given unknown argument \""
        << args[i] << "\"";
      this->SetError(e.str().c_str());
      return false;
      }
    }

  if (hex_conversion_enabled)
    {
    // TODO: should work without temp file, but just on a memory buffer
    std::string binaryFileName = this->Makefile->GetCurrentOutputDirectory();
    binaryFileName += cmake::GetCMakeFilesDirectory();
    binaryFileName += "/FileCommandStringsBinaryFile";
    if(cmHexFileConverter::TryConvert(fileName.c_str(),binaryFileName.c_str()))
      {
      fileName = binaryFileName;
      }
    }

  // Open the specified file.
#if defined(_WIN32) || defined(__CYGWIN__)
  std::ifstream fin(fileName.c_str(), std::ios::in | std::ios::binary);
#else
  std::ifstream fin(fileName.c_str(), std::ios::in);
#endif
  if(!fin)
    {
    cmOStringStream e;
    e << "STRINGS file \"" << fileName << "\" cannot be read.";
    this->SetError(e.str().c_str());
    return false;
    }

  // Parse strings out of the file.
  int output_size = 0;
  std::vector<std::string> strings;
  std::string s;
  int c;
  while((!limit_count || strings.size() < limit_count) &&
        (limit_input < 0 || static_cast<int>(fin.tellg()) < limit_input) &&
        (c = fin.get(), fin))
    {
    if(c == '\0')
      {
      // A terminating null character has been found.  Check if the
      // current string matches the requirements.  Since it was
      // terminated by a null character, we require that the length be
      // at least one no matter what the user specified.
      if(s.length() >= minlen && s.length() >= 1 &&
         (!have_regex || regex.find(s.c_str())))
        {
        output_size += static_cast<int>(s.size()) + 1;
        if(limit_output >= 0 && output_size >= limit_output)
          {
          s = "";
          break;
          }
        strings.push_back(s);
        }

      // Reset the string to empty.
      s = "";
      }
    else if(c == '\n' && !newline_consume)
      {
      // The current line has been terminated.  Check if the current
      // string matches the requirements.  The length may now be as
      // low as zero since blank lines are allowed.
      if(s.length() >= minlen &&
         (!have_regex || regex.find(s.c_str())))
        {
        output_size += static_cast<int>(s.size()) + 1;
        if(limit_output >= 0 && output_size >= limit_output)
          {
          s = "";
          break;
          }
        strings.push_back(s);
        }

      // Reset the string to empty.
      s = "";
      }
    else if(c == '\r')
      {
      // Ignore CR character to make output always have UNIX newlines.
      }
    else if(c >= 0x20 && c < 0x7F || c == '\t' || c == '\f' ||
            (c == '\n' && newline_consume))
      {
      // This is an ASCII character that may be part of a string.
      s += c;
      }
    else
      {
      // This is a non-string character.  Reset the string to emtpy.
      s = "";
      }

    // Terminate a string if the maximum length is reached.
    if(maxlen > 0 && s.size() == maxlen)
      {
      if(s.length() >= minlen &&
         (!have_regex || regex.find(s.c_str())))
        {
        output_size += static_cast<int>(s.size()) + 1;
        if(limit_output >= 0 && output_size >= limit_output)
          {
          s = "";
          break;
          }
        strings.push_back(s);
        }
      s = "";
      }
    }

  // If there is a non-empty current string we have hit the end of the
  // input file or the input size limit.  Check if the current string
  // matches the requirements.
  if((!limit_count || strings.size() < limit_count) &&
     !s.empty() && s.length() >= minlen &&
     (!have_regex || regex.find(s.c_str())))
    {
    output_size += static_cast<int>(s.size()) + 1;
    if(limit_output < 0 || output_size < limit_output)
      {
      strings.push_back(s);
      }
    }

  // Encode the result in a CMake list.
  const char* sep = "";
  std::string output;
  for(std::vector<std::string>::const_iterator si = strings.begin();
      si != strings.end(); ++si)
    {
    // Separate the strings in the output to make it a list.
    output += sep;
    sep = ";";

    // Store the string in the output, but escape semicolons to
    // make sure it is a list.
    std::string const& sr = *si;
    for(unsigned int i=0; i < sr.size(); ++i)
      {
      if(sr[i] == ';')
        {
        output += '\\';
        }
      output += sr[i];
      }
    }

  // Save the output in a makefile variable.
  this->Makefile->AddDefinition(outVar.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleGlobCommand(std::vector<std::string> const& args,
  bool recurse)
{
  if ( args.size() < 2 )
    {
    this->SetError("GLOB requires at least a variable name");
    return false;
    }

  std::vector<std::string>::const_iterator i = args.begin();

  i++; // Get rid of subcommand

  std::string variable = *i;
  i++;
  cmsys::Glob g;
  g.SetRecurse(recurse);

  bool explicitFollowSymlinks = false;
  cmPolicies::PolicyStatus status =
    this->Makefile->GetPolicyStatus(cmPolicies::CMP0009);
  if(recurse)
    {
    switch(status)
      {
      case cmPolicies::NEW:
        g.RecurseThroughSymlinksOff();
        break;
      case cmPolicies::OLD:
      case cmPolicies::WARN:
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        g.RecurseThroughSymlinksOn();
        break;
      }
    }

  std::string output = "";
  bool first = true;
  for ( ; i != args.end(); ++i )
    {
    if ( recurse && (*i == "FOLLOW_SYMLINKS") )
      {
      explicitFollowSymlinks = true;
      g.RecurseThroughSymlinksOn();
      ++i;
      if ( i == args.end() )
        {
        this->SetError(
          "GLOB_RECURSE requires a glob expression after FOLLOW_SYMLINKS");
        return false;
        }
      }

    if ( *i == "RELATIVE" )
      {
      ++i; // skip RELATIVE
      if ( i == args.end() )
        {
        this->SetError("GLOB requires a directory after the RELATIVE tag");
        return false;
        }
      g.SetRelative(i->c_str());
      ++i;
      if(i == args.end())
        {
        this->SetError("GLOB requires a glob expression after the directory");
        return false;
        }
      }

    if ( !cmsys::SystemTools::FileIsFullPath(i->c_str()) )
      {
      std::string expr = this->Makefile->GetCurrentDirectory();
      // Handle script mode
      if ( expr.size() > 0 )
        {
        expr += "/" + *i;
        g.FindFiles(expr);
        }
      else
        {
        g.FindFiles(*i);
        }
      }
    else
      {
      g.FindFiles(*i);
      }

    std::vector<std::string>::size_type cc;
    std::vector<std::string>& files = g.GetFiles();
    for ( cc = 0; cc < files.size(); cc ++ )
      {
      if ( !first )
        {
        output += ";";
        }
      output += files[cc];
      first = false;
      }
    }

  if(recurse && !explicitFollowSymlinks)
    {
    switch (status)
      {
      case cmPolicies::NEW:
        // Correct behavior, yay!
        break;
      case cmPolicies::OLD:
        // Probably not really the expected behavior, but the author explicitly
        // asked for the old behavior... no warning.
      case cmPolicies::WARN:
        // Possibly unexpected old behavior *and* we actually traversed
        // symlinks without being explicitly asked to: warn the author.
        if(g.GetFollowedSymlinkCount() != 0)
          {
          this->Makefile->IssueMessage(cmake::AUTHOR_WARNING,
            this->Makefile->GetPolicies()->
              GetPolicyWarning(cmPolicies::CMP0009));
          }
        break;
      case cmPolicies::REQUIRED_IF_USED:
      case cmPolicies::REQUIRED_ALWAYS:
        this->SetError("policy CMP0009 error");
        this->Makefile->IssueMessage(cmake::FATAL_ERROR,
          this->Makefile->GetPolicies()->
            GetRequiredPolicyError(cmPolicies::CMP0009));
        return false;
      }
    }

  this->Makefile->AddDefinition(variable.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleMakeDirectoryCommand(
  std::vector<std::string> const& args)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::vector<std::string>::const_iterator i = args.begin();

  i++; // Get rid of subcommand

  std::string expr;
  for ( ; i != args.end(); ++i )
    {
    const std::string* cdir = &(*i);
    if ( !cmsys::SystemTools::FileIsFullPath(i->c_str()) )
      {
      expr = this->Makefile->GetCurrentDirectory();
      expr += "/" + *i;
      cdir = &expr;
      }
    if ( !this->Makefile->CanIWriteThisFile(cdir->c_str()) )
      {
      std::string e = "attempted to create a directory: " + *cdir
        + " into a source directory.";
      this->SetError(e.c_str());
      cmSystemTools::SetFatalErrorOccured();
      return false;
      }
    if ( !cmSystemTools::MakeDirectory(cdir->c_str()) )
      {
      std::string error = "problem creating directory: " + *cdir;
      this->SetError(error.c_str());
      return false;
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool
cmFileCommand::HandleDifferentCommand(std::vector<std::string> const& args)
{
  /*
    FILE(DIFFERENT <variable> FILES <lhs> <rhs>)
   */

  // Evaluate arguments.
  const char* file_lhs = 0;
  const char* file_rhs = 0;
  const char* var = 0;
  enum Doing { DoingNone, DoingVar, DoingFileLHS, DoingFileRHS };
  Doing doing = DoingVar;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "FILES")
      {
      doing = DoingFileLHS;
      }
    else if(doing == DoingVar)
      {
      var = args[i].c_str();
      doing = DoingNone;
      }
    else if(doing == DoingFileLHS)
      {
      file_lhs = args[i].c_str();
      doing = DoingFileRHS;
      }
    else if(doing == DoingFileRHS)
      {
      file_rhs = args[i].c_str();
      doing = DoingNone;
      }
    else
      {
      cmOStringStream e;
      e << "DIFFERENT given unknown argument " << args[i];
      this->SetError(e.str().c_str());
      return false;
      }
    }
  if(!var)
    {
    this->SetError("DIFFERENT not given result variable name.");
    return false;
    }
  if(!file_lhs || !file_rhs)
    {
    this->SetError("DIFFERENT not given FILES option with two file names.");
    return false;
    }

  // Compare the files.
  const char* result =
    cmSystemTools::FilesDiffer(file_lhs, file_rhs)? "1" : "0";
  this->Makefile->AddDefinition(var, result);
  return true;
}

//----------------------------------------------------------------------------
// File installation helper class.
struct cmFileInstaller
{
  // Methods to actually install files.
  bool InstallFile(const char* fromFile, const char* toFile, bool always);
  bool InstallDirectory(const char* source, const char* destination,
                        bool always);

  // All instances need the file command and makefile using them.
  cmFileInstaller(cmFileCommand* command):
    FileCommand(command), Makefile(command->GetMakefile()),
    DestDirLength(0), MatchlessFiles(true)
    {
    // Get the current manifest.
    this->Manifest =
      this->Makefile->GetSafeDefinition("CMAKE_INSTALL_MANIFEST_FILES");
    }
  ~cmFileInstaller()
    {
    // Save the updated install manifest.
    this->Makefile->AddDefinition("CMAKE_INSTALL_MANIFEST_FILES",
                                  this->Manifest.c_str());
    }

private:
  cmFileCommand* FileCommand;
  cmMakefile* Makefile;
  cmFileTimeComparison FileTimes;
public:

  // The length of the destdir setting.
  int DestDirLength;

  // Whether to install a file not matching any expression.
  bool MatchlessFiles;

  // The current file manifest (semicolon separated list).
  std::string Manifest;

  // Permissions for files and directories installed by this object.
  mode_t FilePermissions;
  mode_t DirPermissions;

  // Properties set by pattern and regex match rules.
  struct MatchProperties
  {
    bool Exclude;
    mode_t Permissions;
    MatchProperties(): Exclude(false), Permissions(0) {}
  };
  struct MatchRule
  {
    cmsys::RegularExpression Regex;
    MatchProperties Properties;
    std::string RegexString;
    MatchRule(std::string const& regex):
      Regex(regex.c_str()), RegexString(regex) {}
  };
  std::vector<MatchRule> MatchRules;

  // Get the properties from rules matching this input file.
  MatchProperties CollectMatchProperties(const char* file,
                                         bool isDirectory)
    {
    // Match rules are case-insensitive on some platforms.
#if defined(_WIN32) || defined(__APPLE__) || defined(__CYGWIN__)
    std::string lower = cmSystemTools::LowerCase(file);
    file = lower.c_str();
#endif

    // Collect properties from all matching rules.
    bool matched = false;
    MatchProperties result;
    for(std::vector<MatchRule>::iterator mr = this->MatchRules.begin();
        mr != this->MatchRules.end(); ++mr)
      {
      if(mr->Regex.find(file))
        {
        matched = true;
        result.Exclude |= mr->Properties.Exclude;
        result.Permissions |= mr->Properties.Permissions;
        }
      }
    if(!matched && !this->MatchlessFiles && !isDirectory)
      {
      result.Exclude = true;
      }
    return result;
    }

  // Append a file to the installation manifest.
  void ManifestAppend(std::string const& file)
    {
    this->Manifest += ";";
    this->Manifest += file.substr(this->DestDirLength);
    }

  // Translate an argument to a permissions bit.
  bool CheckPermissions(std::string const& arg, mode_t& permissions)
    {
    if(arg == "OWNER_READ")         { permissions |= mode_owner_read; }
    else if(arg == "OWNER_WRITE")   { permissions |= mode_owner_write; }
    else if(arg == "OWNER_EXECUTE") { permissions |= mode_owner_execute; }
    else if(arg == "GROUP_READ")    { permissions |= mode_group_read; }
    else if(arg == "GROUP_WRITE")   { permissions |= mode_group_write; }
    else if(arg == "GROUP_EXECUTE") { permissions |= mode_group_execute; }
    else if(arg == "WORLD_READ")    { permissions |= mode_world_read; }
    else if(arg == "WORLD_WRITE")   { permissions |= mode_world_write; }
    else if(arg == "WORLD_EXECUTE") { permissions |= mode_world_execute; }
    else if(arg == "SETUID")        { permissions |= mode_setuid; }
    else if(arg == "SETGID")        { permissions |= mode_setgid; }
    else
      {
      cmOStringStream e;
      e << "INSTALL given invalid permission \"" << arg << "\".";
      this->FileCommand->SetError(e.str().c_str());
      return false;
      }
    return true;
    }

private:
  bool InstallSymlink(const char* fromFile, const char* toFile, bool always);
};

//----------------------------------------------------------------------------
bool cmFileInstaller::InstallSymlink(const char* fromFile, const char* toFile,
                                     bool always)
{
  // Read the original symlink.
  std::string symlinkTarget;
  if(!cmSystemTools::ReadSymlink(fromFile, symlinkTarget))
    {
    cmOStringStream e;
    e << "INSTALL cannot read symlink \"" << fromFile
      << "\" to duplicate at \"" << toFile << "\".";
    this->FileCommand->SetError(e.str().c_str());
    return false;
    }

  // Compare the symlink value to that at the destination if not
  // always installing.
  bool copy = true;
  if(!always)
    {
    std::string oldSymlinkTarget;
    if(cmSystemTools::ReadSymlink(toFile, oldSymlinkTarget))
      {
      if(symlinkTarget == oldSymlinkTarget)
        {
        copy = false;
        }
      }
    }

  // Inform the user about this file installation.
  std::string message = (copy? "Installing: " : "Up-to-date: ");
  message += toFile;
  this->Makefile->DisplayStatus(message.c_str(), -1);

  if(copy)
    {
    // Remove the destination file so we can always create the symlink.
    cmSystemTools::RemoveFile(toFile);

    // Create the symlink.
    if(!cmSystemTools::CreateSymlink(symlinkTarget.c_str(), toFile))
      {
      cmOStringStream e;
      e << "INSTALL cannot duplicate symlink \"" << fromFile
        << "\" at \"" << toFile << "\".";
      this->FileCommand->SetError(e.str().c_str());
      return false;
      }
    }

  // Add the file to the manifest.
  this->ManifestAppend(toFile);

  return true;
}

//----------------------------------------------------------------------------
bool cmFileInstaller::InstallFile(const char* fromFile, const char* toFile,
                                  bool always)
{
  // Collect any properties matching this file name.
  MatchProperties match_properties =
    this->CollectMatchProperties(fromFile, false);

  // Skip the file if it is excluded.
  if(match_properties.Exclude)
    {
    return true;
    }

  // Short-circuit for symbolic links.
  if(cmSystemTools::FileIsSymlink(fromFile))
    {
    return this->InstallSymlink(fromFile, toFile, always);
    }

  // Determine whether we will copy the file.
  bool copy = true;
  if(!always)
    {
    // If both files exist with the same time do not copy.
    if(!this->FileTimes.FileTimesDiffer(fromFile, toFile))
      {
      copy = false;
      }
    }

  // Inform the user about this file installation.
  std::string message = (copy? "Installing: " : "Up-to-date: ");
  message += toFile;
  this->Makefile->DisplayStatus(message.c_str(), -1);

  // Copy the file.
  if(copy && !cmSystemTools::CopyAFile(fromFile, toFile, true, false))
    {
    cmOStringStream e;
    e << "INSTALL cannot copy file \"" << fromFile
      << "\" to \"" << toFile << "\".";
    this->FileCommand->SetError(e.str().c_str());
    return false;
    }

  // Add the file to the manifest.
  this->ManifestAppend(toFile);

  // Set the file modification time of the destination file.
  if(copy && !always)
    {
    if (!cmSystemTools::CopyFileTime(fromFile, toFile))
      {
      cmOStringStream e;
      e << "Problem setting modification time on file \"" << toFile << "\"";
      this->FileCommand->SetError(e.str().c_str());
      return false;
      }
    }

  // Set permissions of the destination file.
  mode_t permissions = (match_properties.Permissions?
                        match_properties.Permissions : this->FilePermissions);
  if(!permissions)
    {
    // No permissions were explicitly provided but the user requested
    // that the source file permissions be used.
    cmSystemTools::GetPermissions(fromFile, permissions);
    }
  if(permissions && !cmSystemTools::SetPermissions(toFile, permissions))
    {
    cmOStringStream e;
    e << "Problem setting permissions on file \"" << toFile << "\"";
    this->FileCommand->SetError(e.str().c_str());
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmFileInstaller::InstallDirectory(const char* source,
                                       const char* destination,
                                       bool always)
{
  // Collect any properties matching this directory name.
  MatchProperties match_properties =
    this->CollectMatchProperties(source, true);

  // Skip the directory if it is excluded.
  if(match_properties.Exclude)
    {
    return true;
    }

  // Short-circuit for symbolic links.
  if(cmSystemTools::FileIsSymlink(source))
    {
    return this->InstallSymlink(source, destination, always);
    }

  // Inform the user about this directory installation.
  std::string message = "Installing: ";
  message += destination;
  this->Makefile->DisplayStatus(message.c_str(), -1);

  // Make sure the destination directory exists.
  if(!cmSystemTools::MakeDirectory(destination))
    {
    return false;
    }

  // Compute the requested permissions for the destination directory.
  mode_t permissions = (match_properties.Permissions?
                        match_properties.Permissions : this->DirPermissions);
  if(!permissions)
    {
    // No permissions were explicitly provided but the user requested
    // that the source directory permissions be used.
    cmSystemTools::GetPermissions(source, permissions);
    }

  // Compute the set of permissions required on this directory to
  // recursively install files and subdirectories safely.
  mode_t required_permissions =
    mode_owner_read | mode_owner_write | mode_owner_execute;

  // If the required permissions are specified it is safe to set the
  // final permissions now.  Otherwise we must add the required
  // permissions temporarily during file installation.
  mode_t permissions_before = 0;
  mode_t permissions_after = 0;
  if((permissions & required_permissions) == required_permissions)
    {
    permissions_before = permissions;
    }
  else
    {
    permissions_before = permissions | required_permissions;
    permissions_after = permissions;
    }

  // Set the required permissions of the destination directory.
  if(permissions_before &&
     !cmSystemTools::SetPermissions(destination, permissions_before))
    {
    cmOStringStream e;
    e << "Problem setting permissions on directory \""
      << destination << "\"";
    this->FileCommand->SetError(e.str().c_str());
    return false;
    }

  // Load the directory contents to traverse it recursively.
  cmsys::Directory dir;
  if(source && *source)
    {
    dir.Load(source);
    }
  unsigned long numFiles = static_cast<unsigned long>(dir.GetNumberOfFiles());
  for(unsigned long fileNum = 0; fileNum < numFiles; ++fileNum)
    {
    if(!(strcmp(dir.GetFile(fileNum), ".") == 0 ||
         strcmp(dir.GetFile(fileNum), "..") == 0))
      {
      cmsys_stl::string fromPath = source;
      fromPath += "/";
      fromPath += dir.GetFile(fileNum);
      if(cmSystemTools::FileIsDirectory(fromPath.c_str()))
        {
        cmsys_stl::string toDir = destination;
        toDir += "/";
        toDir += dir.GetFile(fileNum);
        if(!this->InstallDirectory(fromPath.c_str(), toDir.c_str(), always))
          {
          return false;
          }
        }
      else
        {
        // Install this file.
        std::string toFile = destination;
        toFile += "/";
        toFile += dir.GetFile(fileNum);
        if(!this->InstallFile(fromPath.c_str(), toFile.c_str(), always))
          {
          return false;
          }
        }
      }
    }

  // Set the requested permissions of the destination directory.
  if(permissions_after &&
     !cmSystemTools::SetPermissions(destination, permissions_after))
    {
    cmOStringStream e;
    e << "Problem setting permissions on directory \"" << destination << "\"";
    this->FileCommand->SetError(e.str().c_str());
    return false;
    }

  return true;
}

//----------------------------------------------------------------------------
void cmFileCommand::HandleInstallPermissions(cmFileInstaller& installer,
                              mode_t& permissions_file,
                              mode_t& permissions_dir,
                              int itype,
                              bool use_given_permissions_file,
                              bool use_given_permissions_dir,
                              bool use_source_permissions) const
{
  // Choose a default for shared library permissions.
  bool install_so_no_exe = this->Makefile->IsOn("CMAKE_INSTALL_SO_NO_EXE");
  // If file permissions were not specified set default permissions
  // for this target type.
  if(!use_given_permissions_file && !use_source_permissions)
    {
    switch(itype)
      {
      case cmTarget::SHARED_LIBRARY:
      case cmTarget::MODULE_LIBRARY:
        if(install_so_no_exe)
          {
          // Use read/write permissions.
          permissions_file = 0;
          permissions_file |= mode_owner_read;
          permissions_file |= mode_owner_write;
          permissions_file |= mode_group_read;
          permissions_file |= mode_world_read;
          break;
          }
      case cmTarget::EXECUTABLE:
      case cmTarget::INSTALL_PROGRAMS:
        // Use read/write/executable permissions.
        permissions_file = 0;
        permissions_file |= mode_owner_read;
        permissions_file |= mode_owner_write;
        permissions_file |= mode_owner_execute;
        permissions_file |= mode_group_read;
        permissions_file |= mode_group_execute;
        permissions_file |= mode_world_read;
        permissions_file |= mode_world_execute;
        break;
      default:
        // Use read/write permissions.
        permissions_file = 0;
        permissions_file |= mode_owner_read;
        permissions_file |= mode_owner_write;
        permissions_file |= mode_group_read;
        permissions_file |= mode_world_read;
        break;
      }
    }

  // If directory permissions were not specified set default permissions.
  if(!use_given_permissions_dir && !use_source_permissions)
    {
    // Use read/write/executable permissions.
    permissions_dir = 0;
    permissions_dir |= mode_owner_read;
    permissions_dir |= mode_owner_write;
    permissions_dir |= mode_owner_execute;
    permissions_dir |= mode_group_read;
    permissions_dir |= mode_group_execute;
    permissions_dir |= mode_world_read;
    permissions_dir |= mode_world_execute;
    }
  // Set the installer permissions.
  installer.FilePermissions = permissions_file;
  installer.DirPermissions = permissions_dir;
}

//----------------------------------------------------------------------------
void cmFileCommand
::GetTargetTypeFromString(const std::string& stype, int& itype) const
{
  if ( stype == "EXECUTABLE" )
    {
    itype = cmTarget::EXECUTABLE;
    }
  else if ( stype == "PROGRAM" )
    {
    itype = cmTarget::INSTALL_PROGRAMS;
    }
  else if ( stype == "STATIC_LIBRARY" )
    {
    itype = cmTarget::STATIC_LIBRARY;
    }
  else if ( stype == "SHARED_LIBRARY" )
    {
    itype = cmTarget::SHARED_LIBRARY;
    }
  else if ( stype == "MODULE" )
    {
    itype = cmTarget::MODULE_LIBRARY;
    }
  else if ( stype == "DIRECTORY" )
    {
    itype = cmTarget::INSTALL_DIRECTORY;
    }
}


//----------------------------------------------------------------------------
bool cmFileCommand::HandleInstallDestination(cmFileInstaller& installer,
                                             std::string& destination)
{
  // allow for / to be a valid destination
  if ( destination.size() < 2 && destination != "/" )
    {
    this->SetError("called with inapropriate arguments. "
        "No DESTINATION provided or .");
    return false;
    }

  const char* destdir = cmSystemTools::GetEnv("DESTDIR");
  if ( destdir && *destdir )
    {
    std::string sdestdir = destdir;
    cmSystemTools::ConvertToUnixSlashes(sdestdir);
    char ch1 = destination[0];
    char ch2 = destination[1];
    char ch3 = 0;
    if ( destination.size() > 2 )
      {
      ch3 = destination[2];
      }
    int skip = 0;
    if ( ch1 != '/' )
      {
      int relative = 0;
      if ( ( ch1 >= 'a' && ch1 <= 'z' || ch1 >= 'A' && ch1 <= 'Z' ) &&
             ch2 == ':' )
        {
        // Assume windows
        // let's do some destdir magic:
        skip = 2;
        if ( ch3 != '/' )
          {
          relative = 1;
          }
        }
      else
        {
        relative = 1;
        }
      if ( relative )
        {
        // This is relative path on unix or windows. Since we are doing
        // destdir, this case does not make sense.
        this->SetError("called with relative DESTINATION. This "
            "does not make sense when using DESTDIR. Specify "
            "absolute path or remove DESTDIR environment variable.");
        return false;
        }
      }
    else
      {
      if ( ch2 == '/' )
        {
        // looks like a network path.
        std::string message = "called with network path DESTINATION. This "
          "does not make sense when using DESTDIR. Specify local "
          "absolute path or remove DESTDIR environment variable."
          "\nDESTINATION=\n";
        message += destination;
        this->SetError(message.c_str());
        return false;
        }
      }
    destination = sdestdir + (destination.c_str() + skip);
    installer.DestDirLength = int(sdestdir.size());
    }

  if ( !cmSystemTools::FileExists(destination.c_str()) )
    {
    if ( !cmSystemTools::MakeDirectory(destination.c_str()) )
      {
      std::string errstring = "cannot create directory: " + destination +
          ". Maybe need administrative privileges.";
      this->SetError(errstring.c_str());
      return false;
      }
    }
  if ( !cmSystemTools::FileIsDirectory(destination.c_str()) )
    {
    std::string errstring = "INSTALL destination: " + destination +
        " is not a directory.";
    this->SetError(errstring.c_str());
    return false;
    }
  return true;
}

//----------------------------------------------------------------------------
bool
cmFileCommand::HandleRPathChangeCommand(std::vector<std::string> const& args)
{
  // Evaluate arguments.
  const char* file = 0;
  const char* oldRPath = 0;
  const char* newRPath = 0;
  enum Doing { DoingNone, DoingFile, DoingOld, DoingNew };
  Doing doing = DoingNone;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "OLD_RPATH")
      {
      doing = DoingOld;
      }
    else if(args[i] == "NEW_RPATH")
      {
      doing = DoingNew;
      }
    else if(args[i] == "FILE")
      {
      doing = DoingFile;
      }
    else if(doing == DoingFile)
      {
      file = args[i].c_str();
      doing = DoingNone;
      }
    else if(doing == DoingOld)
      {
      oldRPath = args[i].c_str();
      doing = DoingNone;
      }
    else if(doing == DoingNew)
      {
      newRPath = args[i].c_str();
      doing = DoingNone;
      }
    else
      {
      cmOStringStream e;
      e << "RPATH_CHANGE given unknown argument " << args[i];
      this->SetError(e.str().c_str());
      return false;
      }
    }
  if(!file)
    {
    this->SetError("RPATH_CHANGE not given FILE option.");
    return false;
    }
  if(!oldRPath)
    {
    this->SetError("RPATH_CHANGE not given OLD_RPATH option.");
    return false;
    }
  if(!newRPath)
    {
    this->SetError("RPATH_CHANGE not given NEW_RPATH option.");
    return false;
    }
  if(!cmSystemTools::FileExists(file, true))
    {
    cmOStringStream e;
    e << "RPATH_CHANGE given FILE \"" << file << "\" that does not exist.";
    this->SetError(e.str().c_str());
    return false;
    }
  bool success = true;
  cmSystemToolsFileTime* ft = cmSystemTools::FileTimeNew();
  bool have_ft = cmSystemTools::FileTimeGet(file, ft);
  std::string emsg;
  bool changed;
  if(!cmSystemTools::ChangeRPath(file, oldRPath, newRPath, &emsg, &changed))
    {
    cmOStringStream e;
    e << "RPATH_CHANGE could not write new RPATH:\n"
      << "  " << newRPath << "\n"
      << "to the file:\n"
      << "  " << file << "\n"
      << emsg;
    this->SetError(e.str().c_str());
    success = false;
    }
  if(success)
    {
    if(changed)
      {
      std::string message = "Set runtime path of \"";
      message += file;
      message += "\" to \"";
      message += newRPath;
      message += "\"";
      this->Makefile->DisplayStatus(message.c_str(), -1);
      }
    if(have_ft)
      {
      cmSystemTools::FileTimeSet(file, ft);
      }
    }
  cmSystemTools::FileTimeDelete(ft);
  return success;
}

//----------------------------------------------------------------------------
bool
cmFileCommand::HandleRPathRemoveCommand(std::vector<std::string> const& args)
{
  // Evaluate arguments.
  const char* file = 0;
  enum Doing { DoingNone, DoingFile };
  Doing doing = DoingNone;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "FILE")
      {
      doing = DoingFile;
      }
    else if(doing == DoingFile)
      {
      file = args[i].c_str();
      doing = DoingNone;
      }
    else
      {
      cmOStringStream e;
      e << "RPATH_REMOVE given unknown argument " << args[i];
      this->SetError(e.str().c_str());
      return false;
      }
    }
  if(!file)
    {
    this->SetError("RPATH_REMOVE not given FILE option.");
    return false;
    }
  if(!cmSystemTools::FileExists(file, true))
    {
    cmOStringStream e;
    e << "RPATH_REMOVE given FILE \"" << file << "\" that does not exist.";
    this->SetError(e.str().c_str());
    return false;
    }
  bool success = true;
  cmSystemToolsFileTime* ft = cmSystemTools::FileTimeNew();
  bool have_ft = cmSystemTools::FileTimeGet(file, ft);
  std::string emsg;
  bool removed;
  if(!cmSystemTools::RemoveRPath(file, &emsg, &removed))
    {
    cmOStringStream e;
    e << "RPATH_REMOVE could not remove RPATH from file:\n"
      << "  " << file << "\n"
      << emsg;
    this->SetError(e.str().c_str());
    success = false;
    }
  if(success)
    {
    if(removed)
      {
      std::string message = "Removed runtime path from \"";
      message += file;
      message += "\"";
      this->Makefile->DisplayStatus(message.c_str(), -1);
      }
    if(have_ft)
      {
      cmSystemTools::FileTimeSet(file, ft);
      }
    }
  cmSystemTools::FileTimeDelete(ft);
  return success;
}

//----------------------------------------------------------------------------
bool
cmFileCommand::HandleRPathCheckCommand(std::vector<std::string> const& args)
{
  // Evaluate arguments.
  const char* file = 0;
  const char* rpath = 0;
  enum Doing { DoingNone, DoingFile, DoingRPath };
  Doing doing = DoingNone;
  for(unsigned int i=1; i < args.size(); ++i)
    {
    if(args[i] == "RPATH")
      {
      doing = DoingRPath;
      }
    else if(args[i] == "FILE")
      {
      doing = DoingFile;
      }
    else if(doing == DoingFile)
      {
      file = args[i].c_str();
      doing = DoingNone;
      }
    else if(doing == DoingRPath)
      {
      rpath = args[i].c_str();
      doing = DoingNone;
      }
    else
      {
      cmOStringStream e;
      e << "RPATH_CHECK given unknown argument " << args[i];
      this->SetError(e.str().c_str());
      return false;
      }
    }
  if(!file)
    {
    this->SetError("RPATH_CHECK not given FILE option.");
    return false;
    }
  if(!rpath)
    {
    this->SetError("RPATH_CHECK not given RPATH option.");
    return false;
    }

  // If the file exists but does not have the desired RPath then
  // delete it.  This is used during installation to re-install a file
  // if its RPath will change.
  if(cmSystemTools::FileExists(file, true) &&
     !cmSystemTools::CheckRPath(file, rpath))
    {
    cmSystemTools::RemoveFile(file);
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleInstallCommand(std::vector<std::string> const& args)
{
  if ( args.size() < 6 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Construct a file installer object.
  cmFileInstaller installer(this);

  std::string rename = "";
  std::string destination = "";

  std::vector<std::string> files;
  int itype = cmTarget::INSTALL_FILES;

  bool optional = false;
  bool result = this->ParseInstallArgs(args, installer,
                                       itype, rename, destination, files,
                                       optional);
  if (result == true)
    {
    result = this->DoInstall(installer,
                             itype, rename, destination, files, optional);
    }
  return result;
}

//----------------------------------------------------------------------------
bool cmFileCommand::ParseInstallArgs(std::vector<std::string> const& args,
                                cmFileInstaller& installer,
                                int& itype,
                                std::string& rename,
                                std::string& destination,
                                std::vector<std::string>& files,
                                bool& optional)
{
    std::string stype = "FILES";
    enum Doing { DoingNone, DoingFiles,
                 DoingPermissionsFile, DoingPermissionsDir,
                 DoingPermissionsMatch, DoingSelf24 };
    Doing doing = DoingNone;
    bool use_given_permissions_file = false;
    bool use_given_permissions_dir = false;
    bool use_source_permissions = false;
    mode_t permissions_file = 0;
    mode_t permissions_dir = 0;

    cmFileInstaller::MatchRule* current_match_rule = 0;
    std::vector<std::string>::size_type i = 0;
    i++; // Get rid of subcommand
    for ( ; i != args.size(); ++i )
      {
      const std::string* cstr = &args[i];
      if ( *cstr == "DESTINATION" && i < args.size()-1 )
        {
        if(current_match_rule)
          {
          cmOStringStream e;
          e << "INSTALL does not allow \"" << *cstr << "\" after REGEX.";
          this->SetError(e.str().c_str());
          return false;
          }

        i++;
        destination = args[i];
        doing = DoingNone;
        }
      else if ( *cstr == "TYPE" && i < args.size()-1 )
        {
        if(current_match_rule)
          {
          cmOStringStream e;
          e << "INSTALL does not allow \"" << *cstr << "\" after REGEX.";
          this->SetError(e.str().c_str());
          return false;
          }

        i++;
        stype = args[i];
        if ( args[i+1] == "OPTIONAL" )
          {
          i++;
          optional = true;
          }
        doing = DoingNone;
        }
      else if ( *cstr == "RENAME" && i < args.size()-1 )
        {
        if(current_match_rule)
          {
          cmOStringStream e;
          e << "INSTALL does not allow \"" << *cstr << "\" after REGEX.";
          this->SetError(e.str().c_str());
          return false;
          }

        i++;
        rename = args[i];
        doing = DoingNone;
        }
      else if ( *cstr == "REGEX" && i < args.size()-1 )
        {
        i++;
        installer.MatchRules.push_back(cmFileInstaller::MatchRule(args[i]));
        current_match_rule = &*(installer.MatchRules.end()-1);
        if(!current_match_rule->Regex.is_valid())
          {
          cmOStringStream e;
          e << "INSTALL could not compile REGEX \"" << args[i] << "\".";
          this->SetError(e.str().c_str());
          return false;
          }
        doing = DoingNone;
        }
      else if ( *cstr == "EXCLUDE"  )
        {
      // Add this property to the current match rule.
        if(!current_match_rule)
          {
          cmOStringStream e;
          e << "INSTALL does not allow \""
              << *cstr << "\" before a REGEX is given.";
          this->SetError(e.str().c_str());
          return false;
          }
        current_match_rule->Properties.Exclude = true;
        doing = DoingPermissionsMatch;
        }
      else if ( *cstr == "PROPERTIES"  )
        {
        cmOStringStream e;
        e << "INSTALL called with old-style PROPERTIES argument.  "
          << "This script was generated with an older version of CMake.  "
          << "Re-run this cmake version on your build tree.";
        this->SetError(e.str().c_str());
        return false;
        }
      else if ( *cstr == "PERMISSIONS" )
        {
        if(current_match_rule)
          {
          doing = DoingPermissionsMatch;
          }
        else
          {
          doing = DoingPermissionsFile;
          use_given_permissions_file = true;
          }
        }
      else if ( *cstr == "DIR_PERMISSIONS" )
        {
        if(current_match_rule)
          {
          cmOStringStream e;
          e << "INSTALL does not allow \"" << *cstr << "\" after REGEX.";
          this->SetError(e.str().c_str());
          return false;
          }

        use_given_permissions_dir = true;
        doing = DoingPermissionsDir;
        }
      else if ( *cstr == "USE_SOURCE_PERMISSIONS" )
        {
        if(current_match_rule)
          {
          cmOStringStream e;
          e << "INSTALL does not allow \"" << *cstr << "\" after REGEX.";
          this->SetError(e.str().c_str());
          return false;
          }

        doing = DoingNone;
        use_source_permissions = true;
        }
      else if ( *cstr == "FILES_MATCHING" )
        {
        if(current_match_rule)
          {
          cmOStringStream e;
          e << "INSTALL does not allow \"" << *cstr << "\" after REGEX.";
          this->SetError(e.str().c_str());
          return false;
          }

        doing = DoingNone;
        installer.MatchlessFiles = false;
        }
      else if ( *cstr == "COMPONENTS"  )
        {
        if(this->Makefile->IsOn("CMAKE_INSTALL_SELF_2_4"))
          {
          // When CMake 2.4 builds this CMake version we need to support
          // the install scripts it generates since it asks this CMake
          // to install itself using the rules it generated.
          doing = DoingSelf24;
          continue;
          }
        cmOStringStream e;
        e << "INSTALL called with old-style COMPONENTS argument.  "
          << "This script was generated with an older version of CMake.  "
          << "Re-run this cmake version on your build tree.";
        this->SetError(e.str().c_str());
        return false;
        }
      else if ( *cstr == "CONFIGURATIONS"  )
        {
        cmOStringStream e;
        e << "INSTALL called with old-style CONFIGURATIONS argument.  "
          << "This script was generated with an older version of CMake.  "
          << "Re-run this cmake version on your build tree.";
        this->SetError(e.str().c_str());
        return false;
        }
      else if(*cstr == "FILES" && doing != DoingFiles)
        {
        if(current_match_rule)
          {
          cmOStringStream e;
          e << "INSTALL does not allow \"" << *cstr << "\" after REGEX.";
          this->SetError(e.str().c_str());
          return false;
          }

        doing = DoingFiles;
        }
      else if(doing == DoingFiles)
        {
        files.push_back(*cstr);
        }
      else if(doing == DoingPermissionsFile)
        {
        if(!installer.CheckPermissions(args[i], permissions_file))
          {
          return false;
          }
        }
      else if(doing == DoingPermissionsDir)
        {
        if(!installer.CheckPermissions(args[i], permissions_dir))
          {
          return false;
          }
        }
      else if(doing == DoingPermissionsMatch)
        {
        if(!installer.CheckPermissions(
            args[i], current_match_rule->Properties.Permissions))
          {
          return false;
          }
        }
      else if(doing == DoingSelf24)
        {
        // Ignore these arguments for compatibility.  This should be
        // reached only when CMake 2.4 is installing the current
        // CMake.  It can be removed when CMake 2.6 or higher is
        // required to build CMake.
        }
      else
        {
        this->SetError("called with inappropriate arguments");
        return false;
        }
      }

    // now check and postprocess what has been parsed
    if ( files.size() == 0 )
      {
      // nothing to do, no files were listed.
      // if this is handled as error, INSTALL_FILES() creates an invalid
      // cmake_install.cmake script with no FILES() arguments if no files were
      // given to INSTALL_FILES(). This was accepted with CMake 2.4.x.
      return true;
      }

    // Check rename form.
    if(!rename.empty())
      {
      if(itype != cmTarget::INSTALL_FILES &&
         itype != cmTarget::INSTALL_PROGRAMS)
        {
        this->SetError("INSTALL option RENAME may be used only with "
            "FILES or PROGRAMS.");
        return false;
        }
      if(files.size() > 1)
        {
        this->SetError("INSTALL option RENAME may be used only with "
                       "one file.");
        return false;
        }
      }

      if (this->HandleInstallDestination(installer, destination) == false)
      {
      return false;
      }

    this->GetTargetTypeFromString(stype, itype);

    this->HandleInstallPermissions(installer,
                             permissions_file,
                             permissions_dir,
                             itype,
                             use_given_permissions_file,
                             use_given_permissions_dir,
                             use_source_permissions);

  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::DoInstall( cmFileInstaller& installer,
                              const int itype,
                              const std::string& rename,
                              const std::string& destination,
                              const std::vector<std::string>& files,
                              const bool optional)
{
  typedef std::set<cmStdString>::const_iterator iter_type;

  // Check whether files should be copied always or only if they have
  // changed.
  bool copy_always =
    cmSystemTools::IsOn(cmSystemTools::GetEnv("CMAKE_INSTALL_ALWAYS"));

  // Handle each file listed.
  for (std::vector<std::string>::size_type i = 0; i < files.size(); i ++ )
    {
    // Split the input file into its directory and name components.
    std::vector<std::string> fromPathComponents;
    cmSystemTools::SplitPath(files[i].c_str(), fromPathComponents);
    std::string fromName = *(fromPathComponents.end()-1);
    std::string fromDir = cmSystemTools::JoinPath(fromPathComponents.begin(),
                                                  fromPathComponents.end()-1);

    // Compute the full path to the destination file.
    std::string toFile = destination;
    std::string const& toName = rename.empty()? fromName : rename;
    if(!toName.empty())
      {
      toFile += "/";
      toFile += toName;
      }

    // Construct the full path to the source file.  The file name may
    // have been changed above.
    std::string fromFile = fromDir;
    if(!fromName.empty())
      {
      fromFile += "/";
      fromFile += fromName;
      }

    std::string message;
    if(!cmSystemTools::SameFile(fromFile.c_str(), toFile.c_str()))
      {
      if(itype == cmTarget::INSTALL_DIRECTORY &&
         (fromFile.empty() ||
          cmSystemTools::FileIsDirectory(fromFile.c_str())))
        {
        // Try installing this directory.
        if(!installer.InstallDirectory(fromFile.c_str(), toFile.c_str(),
                                       copy_always))
          {
          return false;
          }
        }
      else if(cmSystemTools::FileExists(fromFile.c_str()))
        {
        // Install this file.
        if(!installer.InstallFile(fromFile.c_str(), toFile.c_str(),
                                  copy_always))
          {
          return false;
          }
        }
      else if(!optional)
        {
        // The input file does not exist and installation is not optional.
        cmOStringStream e;
        e << "INSTALL cannot find file \"" << fromFile << "\" to install.";
        this->SetError(e.str().c_str());
        return false;
        }
      }
    }

  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleRelativePathCommand(
  std::vector<std::string> const& args)
{
  if(args.size() != 4 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  const std::string& outVar = args[1];
  const std::string& directoryName = args[2];
  const std::string& fileName = args[3];

  if(!cmSystemTools::FileIsFullPath(directoryName.c_str()))
    {
    std::string errstring =
      "RelativePath must be passed a full path to the directory: "
      + directoryName;
    this->SetError(errstring.c_str());
    return false;
    }
  if(!cmSystemTools::FileIsFullPath(fileName.c_str()))
    {
    std::string errstring =
      "RelativePath must be passed a full path to the file: "
      + fileName;
    this->SetError(errstring.c_str());
    return false;
    }

  std::string res = cmSystemTools::RelativePath(directoryName.c_str(),
                                                fileName.c_str());
  this->Makefile->AddDefinition(outVar.c_str(),
    res.c_str());
  return true;
}


//----------------------------------------------------------------------------
bool cmFileCommand::HandleRename(std::vector<std::string> const& args)
{
  if(args.size() != 3)
    {
    this->SetError("given incorrect number of arguments.");
    return false;
    }

  // Compute full path for old and new names.
  std::string oldname = args[1];
  if(!cmsys::SystemTools::FileIsFullPath(oldname.c_str()))
    {
    oldname = this->Makefile->GetCurrentDirectory();
    oldname += "/" + args[1];
    }
  std::string newname = args[2];
  if(!cmsys::SystemTools::FileIsFullPath(newname.c_str()))
    {
    newname = this->Makefile->GetCurrentDirectory();
    newname += "/" + args[2];
    }

  if(!cmSystemTools::RenameFile(oldname.c_str(), newname.c_str()))
    {
    std::string err = cmSystemTools::GetLastSystemError();
    cmOStringStream e;
    e << "RENAME failed to rename\n"
      << "  " << oldname << "\n"
      << "to\n"
      << "  " << newname << "\n"
      << "because: " << err << "\n";
    this->SetError(e.str().c_str());
    return false;
    }
  return true;
}


//----------------------------------------------------------------------------
bool cmFileCommand::HandleRemove(std::vector<std::string> const& args,
                                 bool recurse)
{

  std::string message;
  std::vector<std::string>::const_iterator i = args.begin();

  i++; // Get rid of subcommand
  for(;i != args.end(); ++i)
    {
    std::string fileName = *i;
    if(!cmsys::SystemTools::FileIsFullPath(fileName.c_str()))
      {
      fileName = this->Makefile->GetCurrentDirectory();
      fileName += "/" + *i;
      }

    if(cmSystemTools::FileIsDirectory(fileName.c_str()) && recurse)
      {
      cmSystemTools::RemoveADirectory(fileName.c_str());
      }
    else
      {
      cmSystemTools::RemoveFile(fileName.c_str());
      }
    }
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleCMakePathCommand(std::vector<std::string>
                                           const& args,
                                           bool nativePath)
{
  std::vector<std::string>::const_iterator i = args.begin();
  if(args.size() != 3)
    {
    this->SetError("FILE(SYSTEM_PATH ENV result) must be called with "
                   "only three arguments.");
    return false;
    }
  i++; // Get rid of subcommand
#if defined(_WIN32) && !defined(__CYGWIN__)
  char pathSep = ';';
#else
  char pathSep = ':';
#endif
  std::vector<cmsys::String> path = cmSystemTools::SplitString(i->c_str(),
                                                             pathSep);
  i++;
  const char* var =  i->c_str();
  std::string value;
  for(std::vector<cmsys::String>::iterator j = path.begin();
      j != path.end(); ++j)
    {
    if(j != path.begin())
      {
      value += ";";
      }
    if(!nativePath)
      {
      cmSystemTools::ConvertToUnixSlashes(*j);
      }
    else
      {
      *j = cmSystemTools::ConvertToOutputPath(j->c_str());
      // remove double quotes in the path
      cmsys::String& s = *j;

      if(s.size() > 1 && s[0] == '\"' && s[s.size()-1] == '\"')
        {
        s = s.substr(1,s.size()-2);
        }
      }
    value += *j;
    }
  this->Makefile->AddDefinition(var, value.c_str());
  return true;
}
#if defined(CMAKE_BUILD_WITH_CMAKE)

// Stuff for curl download
typedef std::vector<char> cmFileCommandVectorOfChar;
namespace{
  size_t
  cmFileCommandWriteMemoryCallback(void *ptr, size_t size, size_t nmemb,
                                          void *data)
  { 
    register int realsize = (int)(size * nmemb);
    std::ofstream* fout = static_cast<std::ofstream*>(data);
    const char* chPtr = static_cast<char*>(ptr);
    fout->write(chPtr, realsize);
    return realsize;
  }
  
  static size_t
  cmFileCommandCurlDebugCallback(CURL *, curl_infotype, char *chPtr,
                                        size_t size, void *data)
  {
    cmFileCommandVectorOfChar *vec
      = static_cast<cmFileCommandVectorOfChar*>(data);
    vec->insert(vec->end(), chPtr, chPtr + size);
    
    return size;
  }
  
  
}

#endif

bool 
cmFileCommand::HandleDownloadCommand(std::vector<std::string> 
                                     const& args)
{
#if defined(CMAKE_BUILD_WITH_CMAKE)
  std::vector<std::string>::const_iterator i = args.begin();
  if(args.size() < 3)
    {
    this->SetError("FILE(DOWNLOAD url file) must be called with "
                   "at least three arguments.");
    return false;
    }
  i++; // Get rid of subcommand
  std::string url = *i;
  i++;
  std::string file = *i;
  i++;
  double timeout = 0;
  std::string verboseLog;
  std::string statusVar;
  while(i != args.end())
    {
    if(*i == "TIMEOUT")
      {
      i++;
      if(i != args.end())
        {
        timeout = atof(i->c_str());
        }
      else
        { 
        this->SetError("FILE(DOWNLOAD url file TIMEOUT time) missing "
                       "time for TIMEOUT.");
        return false;
        }
      }
    else if(*i == "LOG")
      {
      i++;
      if( i == args.end())
        {
        this->SetError("FILE(DOWNLOAD url file LOG VAR) missing "
                       "VAR for LOG.");
        return false;
        }
      verboseLog = *i;
      }
    else if(*i == "STATUS")
      {
      i++;
      if( i == args.end())
        {
        this->SetError("FILE(DOWNLOAD url file STATUS VAR) missing "
                       "VAR for STATUS.");
        return false;
        }
      statusVar = *i;
      }
    i++;
    }

  std::string dir = cmSystemTools::GetFilenamePath(file.c_str());
  if(!cmSystemTools::FileExists(dir.c_str()) &&
     !cmSystemTools::MakeDirectory(dir.c_str()))
    {
    std::string errstring = "FILE(DOWNLOAD ) error; cannot create directory: "
      + dir + ". Maybe need administrative privileges.";
    this->SetError(errstring.c_str());
    return false;
    }

  std::ofstream fout(file.c_str(), std::ios::binary);
  if(!fout)
    {
    this->SetError("FILE(DOWNLOAD url file TIMEOUT time) can not open "
                       "file for write.");
    return false;
    }
  CURL *curl;
  curl_global_init(CURL_GLOBAL_DEFAULT);
  curl = curl_easy_init();
  if(!curl)
    {
    this->SetError("FILE(DOWNLOAD ) error "
                   "initializing curl.");
    return false;
    }
  
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, 
                   cmFileCommandWriteMemoryCallback);
  curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION,
                   cmFileCommandCurlDebugCallback);
  cmFileCommandVectorOfChar chunkDebug;
  ::curl_easy_setopt(curl, CURLOPT_FILE, (void *)&fout);
  ::curl_easy_setopt(curl, CURLOPT_DEBUGDATA, (void *)&chunkDebug);
  if(verboseLog.size())
    {
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
    }
  if(timeout > 0)
    {
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout ); 
    }
  CURLcode res = curl_easy_perform(curl);
  /* always cleanup */
  curl_easy_cleanup(curl);
  if(statusVar.size())
    {
    cmOStringStream result;
    result << (int)res << ";\"" << curl_easy_strerror(res) << "\"";
    this->Makefile->AddDefinition(statusVar.c_str(), 
                                  result.str().c_str());
    }
  curl_global_cleanup();
  if(chunkDebug.size())
    {
    chunkDebug.push_back(0);
    if(CURLE_OPERATION_TIMEOUTED == res)
      { 
      std::string output = &*chunkDebug.begin();
      
      if(verboseLog.size())
        {
        this->Makefile->AddDefinition(verboseLog.c_str(),
                                      &*chunkDebug.begin());
        }
      }

    this->Makefile->AddDefinition(verboseLog.c_str(),
                                  &*chunkDebug.begin());
    }
  return true;
#else 
  this->SetError("FILE(DOWNLOAD ) "
                 "not supported in bootstrap cmake ");
  return false;
#endif  
}
