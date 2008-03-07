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
#include "cmConfigureFileCommand.h"

#include <cmsys/RegularExpression.hxx>

// cmConfigureFileCommand
bool cmConfigureFileCommand
::InitialPass(std::vector<std::string> const& args, cmExecutionStatus &)
{
  if(args.size() < 2 )
    {
    this->SetError("called with incorrect number of arguments, expected 2");
    return false;
    }
  this->InputFile = args[0];
  this->OuputFile = args[1];
  if ( !this->Makefile->CanIWriteThisFile(this->OuputFile.c_str()) )
    {
    std::string e = "attempted to configure a file: " + this->OuputFile 
      + " into a source directory.";
    this->SetError(e.c_str());
    cmSystemTools::SetFatalErrorOccured();
    return false;
    }
  this->CopyOnly = false;
  this->EscapeQuotes = false;

  // for CMake 2.0 and earlier CONFIGURE_FILE defaults to the FinalPass,
  // after 2.0 it only does InitialPass
  this->Immediate = !this->Makefile->NeedBackwardsCompatibility(2,0);

  this->AtOnly = false;
  for(unsigned int i=2;i < args.size();++i)
    {
    if(args[i] == "COPYONLY")
      {
      this->CopyOnly = true;
      }
    else if(args[i] == "ESCAPE_QUOTES")
      {
      this->EscapeQuotes = true;
      }
    else if(args[i] == "@ONLY")
      {
      this->AtOnly = true;
      }
    else if(args[i] == "IMMEDIATE")
      {
      this->Immediate = true;
      }
    }
  
  // If we were told to copy the file immediately, then do it on the
  // first pass (now).
  if(this->Immediate)
    {
    if ( !this->ConfigureFile() )
      {
      this->SetError("Problem configuring file");
      return false;
      }
    }
  
  return true;
}

void cmConfigureFileCommand::FinalPass()
{
  if(!this->Immediate)
    {
    this->ConfigureFile();
    }
}

int cmConfigureFileCommand::ConfigureFile()
{
  std::string inFile = this->InputFile;
  if (!cmSystemTools::FileIsFullPath(inFile.c_str()))
    {
    inFile = this->Makefile->GetStartDirectory();
    inFile += "/";
    inFile += this->InputFile;
    }
  return this->Makefile->ConfigureFile(inFile.c_str(),
    this->OuputFile.c_str(),
    this->CopyOnly,
    this->AtOnly,
    this->EscapeQuotes);
}


