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

#include "cmGlob.h"

#include <sys/types.h>
#include <sys/stat.h>

// cmLibraryCommand
bool cmFileCommand::InitialPass(std::vector<std::string> const& args)
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
  else if ( subCommand == "READ" )
    {
    return this->HandleReadCommand(args);
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
  else if ( subCommand == "INSTALL" )
    {
    return this->HandleInstallCommand(args);
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
    fileName = m_Makefile->GetCurrentDirectory();
    fileName += "/" + *i;
    }

  i++;

  for(;i != args.end(); ++i)
    {
    message += *i;
    }
  std::string dir = cmSystemTools::GetFilenamePath(fileName);
  cmSystemTools::MakeDirectory(dir.c_str());

  std::ofstream file(fileName.c_str(), append?std::ios::app: std::ios::out);
  if ( !file )
    {
    std::string error = "Internal CMake error when trying to open file: ";
    error += fileName.c_str();
    error += " for writting.";
    this->SetError(error.c_str());
    return false;
    }
  file << message;
  file.close();
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleReadCommand(std::vector<std::string> const& args)
{
  if ( args.size() != 3 )
    {
    this->SetError("READ must be called with two additional arguments");
    return false;
    }

  std::string fileName = args[1];
  if ( !cmsys::SystemTools::FileIsFullPath(args[1].c_str()) )
    {
    fileName = m_Makefile->GetCurrentDirectory();
    fileName += "/" + args[1];
    }

  std::string variable = args[2];
  std::ifstream file(fileName.c_str(), std::ios::in);
  if ( !file )
    {
    std::string error = "Internal CMake error when trying to open file: ";
    error += fileName.c_str();
    error += " for reading.";
    this->SetError(error.c_str());
    return false;
    }

  std::string output;
  std::string line;
  bool has_newline = false;
  while ( cmSystemTools::GetLineFromStream(file, line, &has_newline) )
    {
    output += line;
    if ( has_newline )
      {
      output += "\n";
      }
    }
  m_Makefile->AddDefinition(variable.c_str(), output.c_str());
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
  cmGlob g;
  g.SetRecurse(recurse);
  std::string output = "";
  bool first = true;
  for ( ; i != args.end(); ++i )
    {
    if ( !cmsys::SystemTools::FileIsFullPath(i->c_str()) )
      {
      std::string expr = m_Makefile->GetCurrentDirectory();
      expr += "/" + *i;
      g.FindFiles(expr);
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
  m_Makefile->AddDefinition(variable.c_str(), output.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleMakeDirectoryCommand(std::vector<std::string> const& args)
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
      expr = m_Makefile->GetCurrentDirectory();
      expr += "/" + *i;
      cdir = &expr;
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
bool cmFileCommand::HandleInstallCommand(std::vector<std::string> const& args)
{
  if ( args.size() < 6 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  std::string destination = "";
  std::string stype = "FILES";
  std::vector<std::string> files;
  int itype = cmTarget::INSTALL_FILES;

  std::vector<std::string>::size_type i = 0;
  i++; // Get rid of subcommand

  std::string expr;
  bool in_files = false;
  for ( ; i != args.size(); ++i )
    {
    const std::string* cstr = &args[i];
    if ( *cstr == "DESTINATION" && i < args.size()-1 )
      {
      i++;
      destination = args[i];
      in_files = false;
      }
    else if ( *cstr == "TYPE" && i < args.size()-1 )
      {
      i++;
      stype = args[i];
      in_files = false;
      }
    else if ( *cstr == "FILES" && !in_files)
      {
      in_files = true;
      }
    else if ( in_files )
      {
      files.push_back(*cstr);
      }
    else
      {
      this->SetError("called with inappropriate arguments");
      return false;
      }
    }

  if ( destination.size() == 0 )
    {
    this->SetError("called with inapropriate arguments. No DESTINATION provided.");
    return false;
    }
  if ( files.size() == 0 )
    {
    this->SetError("called with inapropriate arguments. No FILES provided.");
    return false;
    }
  if ( stype == "EXECUTABLE" )
    {
    itype = cmTarget::EXECUTABLE;
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

  for ( i = 0; i < files.size(); i ++ )
    {
    std::cout << " " << files[i];
    }
  std::cout << std::endl;

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
    std::string errstring = "found file: " + destination +
      " where expecting directory with the same name.";
    this->SetError(errstring.c_str());
    return false;
    }

  for ( i = 0; i < files.size(); i ++ )
    {
    std::string destfile = destination + "/" + cmSystemTools::GetFilenameName(files[i]);

    if ( !cmSystemTools::CopyFileAlways(files[i].c_str(), destination.c_str()) )
      {
      std::string errstring = "cannot copy file: " + files[i] + 
        " to directory : " + destination + ".";
      this->SetError(errstring.c_str());
      return false;
      }
    switch( itype )
      {
    case cmTarget::MODULE_LIBRARY:
    case cmTarget::SHARED_LIBRARY:
    case cmTarget::EXECUTABLE:

      if ( !cmSystemTools::SetPermissions(destfile.c_str(), 
#if defined( _MSC_VER ) || defined( __MINGW32__ )
          S_IREAD | S_IWRITE | S_IEXEC
#elif defined( __BORLANDC__ )
          S_IRUSR | S_IWUSR | S_IXUSR
#else
          S_IRUSR | S_IWUSR | S_IXUSR | 
          S_IRGRP | S_IXGRP | 
          S_IROTH | S_IXOTH 
#endif
         ) )
        {
        perror("problem doing chmod.");
        }
      }
    }

  return true;
}
