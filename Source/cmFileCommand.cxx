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

  std::string e = "does not recognize sub-command "+subCommand;
  this->SetError(e.c_str());
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleWriteCommand(std::vector<std::string> const& args, 
  bool append)
{
  std::string message;
  std::vector<std::string>::const_iterator i = args.begin();

  i++; // Get rid of subcommand

  std::string fileName = *i;
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
  file << message << std::endl;
  file.close();
  return true;
}

//----------------------------------------------------------------------------
bool cmFileCommand::HandleReadCommand(std::vector<std::string> const& args)
{
  if ( args.size() != 3 )
    {
    this->SetError("READ must be called with two additional arguments");
    }

  std::string fileName = args[1];
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

