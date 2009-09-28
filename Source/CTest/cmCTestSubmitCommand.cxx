/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc., Insight Software Consortium

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
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
  bool ctestDropSiteCDash
    = this->Makefile->IsOn("CTEST_DROP_SITE_CDASH");

  if ( !ctestDropMethod )
    {
    ctestDropMethod = "http";
    }

  if ( !ctestDropSite )
    {
    // error: CDash requires CTEST_DROP_SITE definition
    // in CTestConfig.cmake
    }
  if ( !ctestDropLocation )
    {
    // error: CDash requires CTEST_DROP_LOCATION definition
    // in CTestConfig.cmake
    }

  this->CTest->SetCTestConfiguration("DropMethod", ctestDropMethod);
  this->CTest->SetCTestConfiguration("DropSite", ctestDropSite);
  this->CTest->SetCTestConfiguration("DropLocation", ctestDropLocation);

  this->CTest->SetCTestConfiguration("IsCDash",
    ctestDropSiteCDash ? "TRUE" : "FALSE");

  // Only propagate TriggerSite for non-CDash projects:
  //
  if ( !ctestDropSiteCDash )
    {
    this->CTest->SetCTestConfiguration("TriggerSite",  ctestTriggerSite);
    }

  this->CTest->SetCTestConfigurationFromCMakeVariable(this->Makefile,
    "CurlOptions", "CTEST_CURL_OPTIONS");
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

  // If no FILES or PARTS given, *all* PARTS are submitted by default.
  //
  // If FILES are given, but not PARTS, only the FILES are submitted
  // and *no* PARTS are submitted.
  //  (This is why we select the empty "noParts" set in the
  //   FilesMentioned block below...)
  //
  // If PARTS are given, only the selected PARTS are submitted.
  //
  // If both PARTS and FILES are given, only the selected PARTS *and*
  // all the given FILES are submitted.

  // If given explicit FILES to submit, pass them to the handler.
  //
  if(this->FilesMentioned)
    {
    // Intentionally select *no* PARTS. (Pass an empty set.) If PARTS
    // were also explicitly mentioned, they will be selected below...
    // But FILES with no PARTS mentioned should just submit the FILES
    // without any of the default parts.
    //
    std::set<cmCTest::Part> noParts;
    static_cast<cmCTestSubmitHandler*>(handler)->SelectParts(noParts);

    static_cast<cmCTestSubmitHandler*>(handler)->SelectFiles(this->Files);
    }

  // If a PARTS option was given, select only the named parts for submission.
  //
  if(this->PartsMentioned)
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
    this->PartsMentioned = true;
    return true;
    }

  if(arg == "FILES")
    {
    this->ArgumentDoing = ArgumentDoingFiles;
    this->FilesMentioned = true;
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

  if(this->ArgumentDoing == ArgumentDoingFiles)
    {
    cmStdString filename(arg);
    if(cmSystemTools::FileExists(filename.c_str()))
      {
      this->Files.insert(filename);
      }
    else
      {
      cmOStringStream e;
      e << "File \"" << filename << "\" does not exist. Cannot submit "
          << "a non-existent file.";
      this->Makefile->IssueMessage(cmake::FATAL_ERROR, e.str());
      this->ArgumentDoing = ArgumentDoingError;
      }
    return true;
    }

  // Look for other arguments.
  return this->Superclass::CheckArgumentValue(arg);
}
