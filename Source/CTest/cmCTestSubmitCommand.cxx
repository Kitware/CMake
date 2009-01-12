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
#include "cmCTestSubmitCommand.h"

#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmCTestSubmitHandler.h"

cmCTestGenericHandler* cmCTestSubmitCommand::InitializeHandler()
{
  const char* ctestDropMethod
    = this->Makefile->GetDefinition("CTEST_DROP_METHOD");
  const char* ctestDropSite
    = this->Makefile->GetDefinition("CTEST_DROP_SITE");
  const char* ctestDropLocation
    = this->Makefile->GetDefinition("CTEST_DROP_LOCATION");
  const char* ctestTriggerSite
    = this->Makefile->GetDefinition("CTEST_TRIGGER_SITE");

  if ( !ctestDropMethod )
    {
    ctestDropMethod = "http";
    }
  if ( !ctestDropSite )
    {
    ctestDropSite = "public.kitware.com";
    }
  if ( !ctestDropLocation )
    {
    ctestDropLocation = "/cgi-bin/HTTPUploadDartFile.cgi";
    }
  if ( !ctestTriggerSite )
    {
    ctestTriggerSite
      = "http://public.kitware.com/cgi-bin/Submit-Random-TestingResults.cgi";
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "* Use default trigger site: "
      << ctestTriggerSite << std::endl;);
    }

  this->CTest->SetCTestConfiguration("DropMethod",   ctestDropMethod);
  this->CTest->SetCTestConfiguration("DropSite",     ctestDropSite);
  this->CTest->SetCTestConfiguration("DropLocation", ctestDropLocation);
  this->CTest->SetCTestConfiguration("TriggerSite",  ctestTriggerSite);

  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "DropSiteUser", "CTEST_DROP_SITE_USER");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "DropSitePassword", "CTEST_DROP_SITE_PASSWORD");
  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "ScpCommand", "CTEST_SCP_COMMAND");

  const char* notesFilesVariable
    = this->Makefile->GetDefinition("CTEST_NOTES_FILES");
  if (notesFilesVariable)
    {
    std::vector<std::string> notesFiles;
    std::vector<cmStdString> newNotesFiles;
    cmSystemTools::ExpandListArgument(notesFilesVariable,notesFiles);
    std::vector<std::string>::iterator it;
    for ( it = notesFiles.begin();
      it != notesFiles.end();
      ++ it )
      {
      newNotesFiles.push_back(*it);
      }
    this->CTest->GenerateNotesFile(newNotesFiles);
    }
  const char* extraFilesVariable
    = this->Makefile->GetDefinition("CTEST_EXTRA_SUBMIT_FILES");
  if (extraFilesVariable)
    {
    std::vector<std::string> extraFiles;
    std::vector<cmStdString> newExtraFiles;
    cmSystemTools::ExpandListArgument(extraFilesVariable,extraFiles);
    std::vector<std::string>::iterator it;
    for ( it = extraFiles.begin();
      it != extraFiles.end();
      ++ it )
      {
      newExtraFiles.push_back(*it);
      }
    if ( !this->CTest->SubmitExtraFiles(newExtraFiles))
      {
      this->SetError("problem submitting extra files.");
      return 0;
      }
    }

  cmCTestGenericHandler* handler
    = this->CTest->GetInitializedHandler("submit");
  if ( !handler )
    {
    this->SetError("internal CTest error. Cannot instantiate submit handler");
    return 0;
    }

  // If a PARTS option was given, select only the named parts for submission.
  if(!this->Parts.empty())
    {
    static_cast<cmCTestSubmitHandler*>(handler)->SelectParts(this->Parts);
    }
  return handler;
}



//----------------------------------------------------------------------------
bool cmCTestSubmitCommand::CheckArgumentKeyword(std::string const& arg)
{
  // Look for arguments specific to this command.
  if(arg == "PARTS")
    {
    this->ArgumentDoing = ArgumentDoingParts;
    return true;
    }

  // Look for other arguments.
  return this->Superclass::CheckArgumentKeyword(arg);
}

//----------------------------------------------------------------------------
bool cmCTestSubmitCommand::CheckArgumentValue(std::string const& arg)
{
  // Handle states specific to this command.
  if(this->ArgumentDoing == ArgumentDoingParts)
    {
    cmCTest::Part p = this->CTest->GetPartFromName(arg.c_str());
    if(p != cmCTest::PartCount)
      {
      this->Parts.insert(p);
      }
    else
      {
      cmOStringStream e;
      e << "Part name \"" << arg << "\" is invalid.";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      this->ArgumentDoing = ArgumentDoingError;
      }
    return true;
    }

  // Look for other arguments.
  return this->Superclass::CheckArgumentValue(arg);
}
