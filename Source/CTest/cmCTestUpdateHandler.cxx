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

#include "cmCTestUpdateHandler.h"

#include "cmCTest.h"
#include "cmake.h"
#include "cmMakefile.h"
#include "cmLocalGenerator.h"
#include "cmGlobalGenerator.h"
#include "cmVersion.h"
#include "cmGeneratedFileStream.h"
#include "cmXMLParser.h"

//#include <cmsys/RegularExpression.hxx>
#include <cmsys/Process.h>

// used for sleep
#ifdef _WIN32
#include "windows.h"
#endif

#include <stdlib.h>
#include <math.h>
#include <float.h>

//----------------------------------------------------------------------
static const char* cmCTestUpdateHandlerUpdateStrings[] =
{
  "Unknown",
  "CVS",
  "SVN"
};

static const char* cmCTestUpdateHandlerUpdateToString(int type)
{
  if ( type < cmCTestUpdateHandler::e_UNKNOWN ||
    type >= cmCTestUpdateHandler::e_LAST )
    {
    return cmCTestUpdateHandlerUpdateStrings[cmCTestUpdateHandler::e_UNKNOWN];
    }
  return cmCTestUpdateHandlerUpdateStrings[type];
}

//----------------------------------------------------------------------
//**********************************************************************
class cmCTestUpdateHandlerSVNXMLParser : public cmXMLParser
{
public:
  struct t_CommitLog
    {
    int Revision;
    std::string Author;
    std::string Date;
    std::string Message;
    };
  cmCTestUpdateHandlerSVNXMLParser(cmCTestUpdateHandler* up)
    : cmXMLParser(), UpdateHandler(up), MinRevision(-1), MaxRevision(-1)
    {
    }

  int Parse(const char* str)
    {
    this->MinRevision = -1;
    this->MaxRevision = -1;
    int res = this->cmXMLParser::Parse(str);
    if ( this->MinRevision == -1 || this->MaxRevision == -1 )
      {
      return 0;
      }
    return res;
    }

  typedef std::vector<t_CommitLog> t_VectorOfCommits;

  t_VectorOfCommits* GetCommits() { return &this->Commits; }
  int GetMinRevision() { return this->MinRevision; }
  int GetMaxRevision() { return this->MaxRevision; }

protected:
  void StartElement(const char* name, const char** atts)
    {
    if ( strcmp(name, "logentry") == 0 )
      {
      this->CommitLog = t_CommitLog();
      const char* rev = this->FindAttribute(atts, "revision");
      if ( rev)
        {
        this->CommitLog.Revision = atoi(rev);
        if ( this->MinRevision < 0 ||
          this->MinRevision > this->CommitLog.Revision )
          {
          this->MinRevision = this->CommitLog.Revision;
          }
        if ( this->MaxRevision < 0 ||
          this->MaxRevision < this->CommitLog.Revision )
          {
          this->MaxRevision = this->CommitLog.Revision;
          }
        }
      }
    this->CharacterData.erase(
      this->CharacterData.begin(), this->CharacterData.end());
    }
  void EndElement(const char* name)
    {
    if ( strcmp(name, "logentry") == 0 )
      {
      cmCTestLog(this->UpdateHandler->GetCTestInstance(),
        HANDLER_VERBOSE_OUTPUT,
        "\tRevision: " << this->CommitLog.Revision<< std::endl
        << "\tAuthor:   " << this->CommitLog.Author.c_str() << std::endl
        << "\tDate:     " << this->CommitLog.Date.c_str() << std::endl
        << "\tMessage:  " << this->CommitLog.Message.c_str() << std::endl);
      this->Commits.push_back(this->CommitLog);
      }
    else if ( strcmp(name, "author") == 0 )
      {
      this->CommitLog.Author.assign(&(*(this->CharacterData.begin())),
        this->CharacterData.size());
      }
    else if ( strcmp(name, "date") == 0 )
      {
      this->CommitLog.Date.assign(&(*(this->CharacterData.begin())),
        this->CharacterData.size());
      }
    else if ( strcmp(name, "msg") == 0 )
      {
      this->CommitLog.Message.assign(&(*(this->CharacterData.begin())),
        this->CharacterData.size());
      }
    this->CharacterData.erase(this->CharacterData.begin(),
      this->CharacterData.end());
    }
  void CharacterDataHandler(const char* data, int length)
    {
    this->CharacterData.insert(this->CharacterData.end(), data, data+length);
    }
  const char* FindAttribute( const char** atts, const char* attribute )
    {
    if ( !atts || !attribute )
      {
      return 0;
      }
    const char **atr = atts;
    while ( *atr && **atr && **(atr+1) )
      {
      if ( strcmp(*atr, attribute) == 0 )
        {
        return *(atr+1);
        }
      atr+=2;
      }
    return 0;
    }

private:
  std::vector<char> CharacterData;
  cmCTestUpdateHandler* UpdateHandler;
  t_CommitLog CommitLog;

  t_VectorOfCommits Commits;
  int MinRevision;
  int MaxRevision;
};
//**********************************************************************
//----------------------------------------------------------------------

//----------------------------------------------------------------------
cmCTestUpdateHandler::cmCTestUpdateHandler()
{
}

//----------------------------------------------------------------------
void cmCTestUpdateHandler::Initialize()
{
  this->Superclass::Initialize();
}

//----------------------------------------------------------------------
int cmCTestUpdateHandler::DetermineType(const char* cmd, const char* type)
{
  cmCTestLog(this->CTest, DEBUG, "Determine update type from command: " << cmd
    << " and type: " << type << std::endl);
  if ( type && *type )
    {
    cmCTestLog(this->CTest, DEBUG, "Type specified: " << type << std::endl);
    std::string stype = cmSystemTools::LowerCase(type);
    if ( stype.find("cvs") != std::string::npos )
      {
      return cmCTestUpdateHandler::e_CVS;
      }
    if ( stype.find("svn") != std::string::npos )
      {
      return cmCTestUpdateHandler::e_SVN;
      }
    }
  else
    {
    cmCTestLog(this->CTest, DEBUG, "Type not specified, check command: "
      << cmd << std::endl);
    std::string stype = cmSystemTools::LowerCase(cmd);
    if ( stype.find("cvs") != std::string::npos )
      {
      return cmCTestUpdateHandler::e_CVS;
      }
    if ( stype.find("svn") != std::string::npos )
      {
      return cmCTestUpdateHandler::e_SVN;
      }
    }
  std::string sourceDirectory = this->GetOption("SourceDirectory");
  cmCTestLog(this->CTest, DEBUG, "Check directory: "
    << sourceDirectory.c_str() << std::endl);
  sourceDirectory += "/.svn";
  if ( cmSystemTools::FileExists(sourceDirectory.c_str()) )
    {
    return cmCTestUpdateHandler::e_SVN;
    }
  sourceDirectory = this->GetOption("SourceDirectory");
  sourceDirectory += "/CVS";
  if ( cmSystemTools::FileExists(sourceDirectory.c_str()) )
    {
    return cmCTestUpdateHandler::e_CVS;
    }
  return cmCTestUpdateHandler::e_UNKNOWN;
}

//----------------------------------------------------------------------
//clearly it would be nice if this were broken up into a few smaller
//functions and commented...
int cmCTestUpdateHandler::ProcessHandler()
{
  int count = 0;
  int updateType = e_CVS;
  std::string::size_type cc, kk;
  bool updateProducedError = false;
  std::string goutput;
  std::string errors;

  std::string checkoutErrorMessages;
  int retVal = 0;

  // Get source dir
  const char* sourceDirectory = this->GetOption("SourceDirectory");
  if ( !sourceDirectory )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Cannot find SourceDirectory  key in the DartConfiguration.tcl"
      << std::endl);
    return -1;
    }

  cmGeneratedFileStream ofs;
  if ( !this->CTest->GetShowOnly() )
    {
    this->StartLogFile("Update", ofs);
    }

  cmCTestLog(this->CTest, HANDLER_OUTPUT,
    "Updating the repository" << std::endl);

  const char* initialCheckoutCommand = this->GetOption("InitialCheckout");
  if ( initialCheckoutCommand )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
      "   First perform the initil checkout: " << initialCheckoutCommand
      << std::endl);
    cmStdString parent = cmSystemTools::GetParentDirectory(sourceDirectory);
    if ( parent.empty() )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "Something went wrong when trying "
        "to determine the parent directory of " << sourceDirectory
        << std::endl);
      return -1;
      }
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
      "   Perform checkout in directory: " << parent.c_str() << std::endl);
    if ( !cmSystemTools::MakeDirectory(parent.c_str()) )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "Cannot create parent directory: " << parent.c_str()
        << " of the source directory: " << sourceDirectory << std::endl);
      return -1;
      }
    ofs << "* Run initial checkout" << std::endl;
    ofs << "  Command: " << initialCheckoutCommand << std::endl;
    cmCTestLog(this->CTest, DEBUG, "   Before: "
      << initialCheckoutCommand << std::endl);
    bool retic = this->CTest->RunCommand(initialCheckoutCommand, &goutput,
      &errors, &retVal, parent.c_str(), 0 /* Timeout */);
    cmCTestLog(this->CTest, DEBUG, "   After: "
      << initialCheckoutCommand << std::endl);
    ofs << "  Output: " << goutput.c_str() << std::endl;
    ofs << "  Errors: " << errors.c_str() << std::endl;
    if ( !retic || retVal )
      {
      cmOStringStream ostr;
      ostr << "Problem running initial checkout Output [" << goutput
        << "] Errors [" << errors << "]";
      cmCTestLog(this->CTest, ERROR_MESSAGE, ostr.str().c_str() << std::endl);
      checkoutErrorMessages += ostr.str();
      updateProducedError = true;
      }
    this->CTest->InitializeFromCommand(this->Command);
    }
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Updating the repository: "
    << sourceDirectory << std::endl);

  // Get update command
  std::string updateCommand
    = this->CTest->GetCTestConfiguration("UpdateCommand");
  if ( updateCommand.empty() )
    {
    updateCommand = this->CTest->GetCTestConfiguration("CVSCommand");
    if ( updateCommand.empty() )
      {
      updateCommand = this->CTest->GetCTestConfiguration("SVNCommand");
      if ( updateCommand.empty() )
        {
        cmCTestLog(this->CTest, ERROR_MESSAGE,
          "Cannot find CVSCommand, SVNCommand, or UpdateCommand key in the "
          "DartConfiguration.tcl" << std::endl);
        return -1;
        }
      else
        {
        updateType = e_SVN;
        }
      }
    else
      {
      updateType = e_CVS;
      }
    }
  else
    {
    updateType = this->DetermineType(updateCommand.c_str(),
      this->CTest->GetCTestConfiguration("UpdateType").c_str());
    }

  cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Use "
    << cmCTestUpdateHandlerUpdateToString(updateType) << " repository type"
    << std::endl;);

  // And update options
  std::string updateOptions
    = this->CTest->GetCTestConfiguration("UpdateOptions");
  if ( updateOptions.empty() )
    {
    switch (updateType)
      {
    case cmCTestUpdateHandler::e_CVS:
      updateOptions = this->CTest->GetCTestConfiguration("CVSUpdateOptions");
      if ( updateOptions.empty() )
        {
        updateOptions = "-dP";
        }
      break;
    case cmCTestUpdateHandler::e_SVN:
      updateOptions = this->CTest->GetCTestConfiguration("SVNUpdateOptions");
      break;
      }
    }

  // Get update time
  std::string extra_update_opts;
  if ( this->CTest->GetTestModel() == cmCTest::NIGHTLY )
    {
    struct tm* t = this->CTest->GetNightlyTime(
      this->CTest->GetCTestConfiguration("NightlyStartTime"),
      this->CTest->GetTomorrowTag());
    char current_time[1024];
    sprintf(current_time, "%04d-%02d-%02d %02d:%02d:%02d",
      t->tm_year + 1900,
      t->tm_mon + 1,
      t->tm_mday,
      t->tm_hour,
      t->tm_min,
      t->tm_sec);
    std::string today_update_date = current_time;

    // TODO: SVN
    switch ( updateType )
      {
    case cmCTestUpdateHandler::e_CVS:
      extra_update_opts += "-D \"" + today_update_date +" UTC\"";
      break;
    case cmCTestUpdateHandler::e_SVN:
      extra_update_opts += "-r \"{" + today_update_date +" +0000}\"";
      break;
      }
    }

  updateCommand = "\"" + updateCommand + "\"";

  // First, check what the current state of repository is
  std::string command = "";
  switch( updateType )
    {
  case cmCTestUpdateHandler::e_CVS:
    // TODO: CVS - for now just leave empty
    break;
  case cmCTestUpdateHandler::e_SVN:
    command = updateCommand + " info";
    break;
    }

  // CVS variables
  // SVN variables
  int svn_current_revision = 0;
  int svn_latest_revision = 0;
  int svn_use_status = 0;

  bool res = true;


  //
  // Get initial repository information if that is possible. With subversion,
  // this will check the current revision.
  //
  if ( !command.empty() )
    {
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
        "* Get repository information: " << command.c_str() << std::endl);
    if ( !this->CTest->GetShowOnly() )
      {
      ofs << "* Get repository information" << std::endl;
      ofs << "  Command: " << command.c_str() << std::endl;
      res = this->CTest->RunCommand(command.c_str(), &goutput, &errors,
        &retVal, sourceDirectory, 0 /*this->TimeOut*/);

      ofs << "  Output: " << goutput.c_str() << std::endl;
      ofs << "  Errors: " << errors.c_str() << std::endl;
      if ( ofs )
        {
        ofs << "--- Update information ---" << std::endl;
        ofs << goutput << std::endl;
        }
      switch ( updateType )
        {
      case cmCTestUpdateHandler::e_CVS:
        // TODO: CVS - for now just leave empty
        break;
      case cmCTestUpdateHandler::e_SVN:
          {
          cmsys::RegularExpression current_revision_regex(
            "Revision: ([0-9]+)");
          if ( current_revision_regex.find(goutput.c_str()) )
            {
            std::string currentRevisionString
              = current_revision_regex.match(1);
            svn_current_revision = atoi(currentRevisionString.c_str());
            cmCTestLog(this->CTest, HANDLER_OUTPUT,
              "   Old revision of repository is: " << svn_current_revision
              << std::endl);
            }
          }
        break;
        }
      }
    else
      {
      cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
        "Update with command: " << command << std::endl);
      }
    }


  //
  // Now update repository and remember what files were updated
  //
  cmGeneratedFileStream os;
  if ( !this->StartResultingXML("Update", os) )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Cannot open log file"
      << std::endl);
    }
  std::string start_time = this->CTest->CurrentTime();
  double elapsed_time_start = cmSystemTools::GetTime();

  cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "* Update repository: "
    << command.c_str() << std::endl);
  if ( !this->CTest->GetShowOnly() )
    {
    command = "";
    switch( updateType )
      {
    case cmCTestUpdateHandler::e_CVS:
      command = updateCommand + " -z3 update " + updateOptions +
        " " + extra_update_opts;
      ofs << "* Update repository: " << std::endl;
      ofs << "  Command: " << command.c_str() << std::endl;
      res = this->CTest->RunCommand(command.c_str(), &goutput, &errors,
        &retVal, sourceDirectory, 0 /*this->TimeOut*/);
      ofs << "  Output: " << goutput.c_str() << std::endl;
      ofs << "  Errors: " << errors.c_str() << std::endl;
      break;
    case cmCTestUpdateHandler::e_SVN:
        {
        std::string partialOutput;
        command = updateCommand + " update " + updateOptions +
          " " + extra_update_opts;
        ofs << "* Update repository: " << std::endl;
        ofs << "  Command: " << command.c_str() << std::endl;
        bool res1 = this->CTest->RunCommand(command.c_str(), &partialOutput,
          &errors,
          &retVal, sourceDirectory, 0 /*this->TimeOut*/);
        ofs << "  Output: " << partialOutput.c_str() << std::endl;
        ofs << "  Errors: " << errors.c_str() << std::endl;
        goutput = partialOutput;
        command = updateCommand + " status";
        ofs << "* Status repository: " << std::endl;
        ofs << "  Command: " << command.c_str() << std::endl;
        res = this->CTest->RunCommand(command.c_str(), &partialOutput,
          &errors, &retVal, sourceDirectory, 0 /*this->TimeOut*/);
        ofs << "  Output: " << partialOutput.c_str() << std::endl;
        ofs << "  Errors: " << errors.c_str() << std::endl;
        goutput += partialOutput;
        res = res && res1;
        ofs << "  Total output of update: " << goutput.c_str() << std::endl;
        }
      }
    if ( ofs )
      {
      ofs << "--- Update repository ---" << std::endl;
      ofs << goutput << std::endl;
      }
    }
  if ( !res || retVal )
    {
    updateProducedError = true;
    checkoutErrorMessages += " " + goutput;
    }

  os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    << "<Update mode=\"Client\" Generator=\"ctest-"
    << cmVersion::GetCMakeVersion() << "\">\n"
    << "\t<Site>" << this->CTest->GetCTestConfiguration("Site") << "</Site>\n"
    << "\t<BuildName>" << this->CTest->GetCTestConfiguration("BuildName")
    << "</BuildName>\n"
    << "\t<BuildStamp>" << this->CTest->GetCurrentTag() << "-"
    << this->CTest->GetTestModelString() << "</BuildStamp>" << std::endl;
  os << "\t<StartDateTime>" << start_time << "</StartDateTime>\n"
    << "\t<UpdateCommand>" << this->CTest->MakeXMLSafe(command)
    << "</UpdateCommand>\n"
    << "\t<UpdateType>" << this->CTest->MakeXMLSafe(
      cmCTestUpdateHandlerUpdateToString(updateType))
    << "</UpdateType>\n";

  // Even though it failed, we may have some useful information. Try to
  // continue...
  std::vector<cmStdString> lines;
  cmSystemTools::Split(goutput.c_str(), lines);
  std::vector<cmStdString> errLines;
  cmSystemTools::Split(errors.c_str(), errLines);
  lines.insert(lines.end(), errLines.begin(), errLines.end());

  // CVS style regular expressions
  cmsys::RegularExpression cvs_date_author_regex(
    "^date: +([^;]+); +author: +([^;]+); +state: +[^;]+;");
  cmsys::RegularExpression cvs_revision_regex("^revision +([^ ]*) *$");
  cmsys::RegularExpression cvs_end_of_file_regex(
    "^=========================================="
    "===================================$");
  cmsys::RegularExpression cvs_end_of_comment_regex(
    "^----------------------------$");

  // Subversion style regular expressions
  cmsys::RegularExpression svn_status_line_regex(
    "^ *([0-9]+)  *([0-9]+)  *([^ ]+)  *([^ ][^\t\r\n]*)[ \t\r\n]*$");
  cmsys::RegularExpression svn_latest_revision_regex(
    "(Updated to|At) revision ([0-9]+)\\.");

  cmsys::RegularExpression file_removed_line(
    "cvs update: `(.*)' is no longer in the repository");
  cmsys::RegularExpression file_update_line("([A-Z])  *(.*)");
  std::string current_path = "<no-path>";
  bool first_file = true;

  cmCTestUpdateHandler::AuthorsToUpdatesMap authors_files_map;
  int numUpdated = 0;
  int numModiefied = 0;
  int numConflicting = 0;
  // In subversion, get the latest revision
  if ( updateType == cmCTestUpdateHandler::e_SVN )
    {
    for ( cc= 0; cc < lines.size(); cc ++ )
      {
      const char* line = lines[cc].c_str();
      if ( svn_latest_revision_regex.find(line) )
        {
        svn_latest_revision = atoi(
          svn_latest_revision_regex.match(2).c_str());
        }
      }
    if ( svn_latest_revision <= 0 )
      {
      cmCTestLog(this->CTest, ERROR_MESSAGE,
        "Problem determining the current "
        "revision of the repository from output:" << std::endl
        << goutput.c_str() << std::endl);
      }
    else
      {
      cmCTestLog(this->CTest, HANDLER_OUTPUT,
        "   Current revision of repository is: " << svn_latest_revision
        << std::endl);
      }
    }

  cmCTestLog(this->CTest, HANDLER_OUTPUT,
    "   Gathering version information (each . represents one updated file):"
    << std::endl);
  int file_count = 0;
  std::string removed_line;
  for ( cc= 0; cc < lines.size(); cc ++ )
    {
    const char* line = lines[cc].c_str();
    if ( file_removed_line.find(line) )
      {
      removed_line = "D " + file_removed_line.match(1);
      line = removed_line.c_str();
      }
    if ( file_update_line.find(line) )
      {
      if ( file_count == 0 )
        {
        cmCTestLog(this->CTest, HANDLER_OUTPUT, "    " << std::flush);
        }
      cmCTestLog(this->CTest, HANDLER_OUTPUT, "." << std::flush);
      std::string upChar = file_update_line.match(1);
      std::string upFile = file_update_line.match(2);
      char mod = upChar[0];
      bool modifiedOrConflict = false;
      if ( mod == 'X')
        {
        continue;
        }
      if ( mod != 'M' && mod != 'C' && mod != 'G' )
        {
        count ++;
        modifiedOrConflict = true;
        }
      const char* file = upFile.c_str();
      cmCTestLog(this->CTest, DEBUG, "Line" << cc << ": " << mod << " - "
        << file << std::endl);

      std::string output;
      if ( modifiedOrConflict )
        {
        std::string logcommand;
        switch ( updateType )
          {
        case cmCTestUpdateHandler::e_CVS:
          logcommand = updateCommand + " -z3 log -N \"" + file + "\"";
          break;
        case cmCTestUpdateHandler::e_SVN:
          if ( svn_latest_revision > 0 &&
            svn_latest_revision > svn_current_revision )
            {
            cmOStringStream logCommandStream;
            logCommandStream << updateCommand << " log -r "
              << svn_current_revision << ":" << svn_latest_revision
              << " --xml \"" << file << "\"";
            logcommand = logCommandStream.str();
            }
          else
            {
            logcommand = updateCommand +
              " status  --verbose \"" + file + "\"";
            svn_use_status = 1;
            }
          break;
          }
        cmCTestLog(this->CTest, DEBUG, "Do log: " << logcommand << std::endl);
        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
          "* Get file update information: " << logcommand.c_str()
          << std::endl);
        ofs << "* Get log information for file: " << file << std::endl;
        ofs << "  Command: " << logcommand.c_str() << std::endl;
        res = this->CTest->RunCommand(logcommand.c_str(), &output, &errors,
          &retVal, sourceDirectory, 0 /*this->TimeOut*/);
        ofs << "  Output: " << output.c_str() << std::endl;
        ofs << "  Errors: " << errors.c_str() << std::endl;
        if ( ofs )
          {
          ofs << output << std::endl;
          }
        }
      if ( res && retVal == 0)
        {
        cmCTestLog(this->CTest, DEBUG, output << std::endl);
        std::string::size_type sline = 0;
        std::string srevision1 = "Unknown";
        std::string sdate1     = "Unknown";
        std::string sauthor1   = "Unknown";
        std::string semail1    = "Unknown";
        std::string comment1   = "";
        std::string srevision2 = "Unknown";
        std::string sdate2     = "Unknown";
        std::string sauthor2   = "Unknown";
        std::string comment2   = "";
        std::string semail2    = "Unknown";
        if ( updateType == cmCTestUpdateHandler::e_CVS )
          {
          bool have_first = false;
          bool have_second = false;
          std::vector<cmStdString> ulines;
          cmSystemTools::Split(output.c_str(), ulines);
          for ( kk = 0; kk < ulines.size(); kk ++ )
            {
            const char* clp = ulines[kk].c_str();
            if ( !have_second && !sline && cvs_revision_regex.find(clp) )
              {
              if ( !have_first )
                {
                srevision1 = cvs_revision_regex.match(1);
                }
              else
                {
                srevision2 = cvs_revision_regex.match(1);
                }
              }
            else if ( !have_second && !sline &&
              cvs_date_author_regex.find(clp) )
              {
              sline = kk + 1;
              if ( !have_first )
                {
                sdate1 = cvs_date_author_regex.match(1);
                sauthor1 = cvs_date_author_regex.match(2);
                }
              else
                {
                sdate2 = cvs_date_author_regex.match(1);
                sauthor2 = cvs_date_author_regex.match(2);
                }
              }
            else if ( sline && cvs_end_of_comment_regex.find(clp) ||
              cvs_end_of_file_regex.find(clp))
              {
              if ( !have_first )
                {
                have_first = true;
                }
              else if ( !have_second )
                {
                have_second = true;
                }
              sline = 0;
              }
            else if ( sline )
              {
              if ( !have_first )
                {
                comment1 += clp;
                comment1 += "\n";
                }
              else
                {
                comment2 += clp;
                comment2 += "\n";
                }
              }
            }
          }
        else if ( updateType == cmCTestUpdateHandler::e_SVN )
          {
          if ( svn_use_status )
            {
            cmOStringStream str;
            str << svn_current_revision;
            srevision1 = str.str();
            if (!svn_status_line_regex.find(output))
              {
              cmCTestLog(this->CTest, ERROR_MESSAGE,
                "Bad output from SVN status command: " << output
                << std::endl);
              }
            else if ( svn_status_line_regex.match(4) != file )
              {
              cmCTestLog(this->CTest, ERROR_MESSAGE,
                "Bad output from SVN status command. "
                "The file name returned: \""
                << svn_status_line_regex.match(4)
                << "\" was different than the file specified: \"" << file
                << "\"" << std::endl);
              }
            else
              {
              srevision1 = svn_status_line_regex.match(2);
              int latest_revision = atoi(
                svn_status_line_regex.match(2).c_str());
              if ( svn_current_revision < latest_revision )
                {
                srevision2 = str.str();
                }
              sauthor1 = svn_status_line_regex.match(3);
              }
            }
          else
            {
            cmCTestUpdateHandlerSVNXMLParser parser(this);
            if ( parser.Parse(output.c_str()) )
              {
              int minrev = parser.GetMinRevision();
              int maxrev = parser.GetMaxRevision();
              cmCTestUpdateHandlerSVNXMLParser::
                t_VectorOfCommits::iterator it;
              for ( it = parser.GetCommits()->begin();
                it != parser.GetCommits()->end();
                ++ it )
                {
                if ( it->Revision == maxrev )
                  {
                  cmOStringStream mRevStream;
                  mRevStream << maxrev;
                  srevision1 = mRevStream.str();
                  sauthor1 = it->Author;
                  comment1 = it->Message;
                  sdate1 = it->Date;
                  }
                else if ( it->Revision == minrev )
                  {
                  cmOStringStream mRevStream;
                  mRevStream << minrev;
                  srevision2 = mRevStream.str();
                  sauthor2 = it->Author;
                  comment2 = it->Message;
                  sdate2 = it->Date;
                  }
                }
              }
            }
          }
        if ( mod == 'M' )
          {
          comment1 = "Locally modified file\n";
          sauthor1 = "Local User";
          }
        if ( mod == 'D' )
          {
          comment1 += " - Removed file\n";
          }
        if ( mod == 'C' )
          {
          comment1 = "Conflict while updating\n";
          sauthor1 = "Local User";
          }
        std::string path = cmSystemTools::GetFilenamePath(file);
        std::string fname = cmSystemTools::GetFilenameName(file);
        if ( path != current_path )
          {
          if ( !first_file )
            {
            os << "\t</Directory>" << std::endl;
            }
          else
            {
            first_file = false;
            }
          os << "\t<Directory>\n"
            << "\t\t<Name>" << path << "</Name>" << std::endl;
          }
        if ( mod == 'C' )
          {
          numConflicting ++;
          os << "\t<Conflicting>" << std::endl;
          }
        else if ( mod == 'G' )
          {
          numConflicting ++;
          os << "\t<Conflicting>" << std::endl;
          }
        else if ( mod == 'M' )
          {
          numModiefied ++;
          os << "\t<Modified>" << std::endl;
          }
        else
          {
          numUpdated ++;
          os << "\t<Updated>" << std::endl;
          }
        if ( srevision2 == "Unknown" )
          {
          srevision2 = srevision1;
          }
        cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "File: "
          << path.c_str() << " / " << fname.c_str() << " was updated by "
          << sauthor1.c_str() << " to revision: " << srevision1.c_str()
          << " from revision: " << srevision2.c_str() << std::endl);
        os << "\t\t<File Directory=\"" << cmCTest::MakeXMLSafe(path) << "\">"
          << cmCTest::MakeXMLSafe(fname)
          << "</File>\n"
          << "\t\t<Directory>" << cmCTest::MakeXMLSafe(path)
          << "</Directory>\n"
          << "\t\t<FullName>" << cmCTest::MakeXMLSafe(file) << "</FullName>\n"
          << "\t\t<CheckinDate>" << cmCTest::MakeXMLSafe(sdate1)
          << "</CheckinDate>\n"
          << "\t\t<Author>" << cmCTest::MakeXMLSafe(sauthor1) << "</Author>\n"
          << "\t\t<Email>" << cmCTest::MakeXMLSafe(semail1) << "</Email>\n"
          << "\t\t<Log>" << cmCTest::MakeXMLSafe(comment1) << "</Log>\n"
          << "\t\t<Revision>" << srevision1 << "</Revision>\n"
          << "\t\t<PriorRevision>" << srevision2 << "</PriorRevision>"
          << std::endl;
        if ( srevision2 != srevision1 )
          {
          os
            << "\t\t<Revisions>\n"
            << "\t\t\t<Revision>" << srevision1 << "</Revision>\n"
            << "\t\t\t<PreviousRevision>" << srevision2
            << "</PreviousRevision>\n"
            << "\t\t\t<Author>" << cmCTest::MakeXMLSafe(sauthor1)
            << "</Author>\n"
            << "\t\t\t<Date>" << cmCTest::MakeXMLSafe(sdate1)
            << "</Date>\n"
            << "\t\t\t<Comment>" << cmCTest::MakeXMLSafe(comment1)
            << "</Comment>\n"
            << "\t\t\t<Email>" << cmCTest::MakeXMLSafe(semail1)
            << "</Email>\n"
            << "\t\t</Revisions>\n"
            << "\t\t<Revisions>\n"
            << "\t\t\t<Revision>" << srevision2 << "</Revision>\n"
            << "\t\t\t<PreviousRevision>" << srevision2
            << "</PreviousRevision>\n"
            << "\t\t\t<Author>" << cmCTest::MakeXMLSafe(sauthor2)
            << "</Author>\n"
            << "\t\t\t<Date>" << cmCTest::MakeXMLSafe(sdate2)
            << "</Date>\n"
            << "\t\t\t<Comment>" << cmCTest::MakeXMLSafe(comment2)
            << "</Comment>\n"
            << "\t\t\t<Email>" << cmCTest::MakeXMLSafe(semail2)
            << "</Email>\n"
            << "\t\t</Revisions>" << std::endl;
          }
        if ( mod == 'C' )
          {
          os << "\t</Conflicting>" << std::endl;
          }
        else if ( mod == 'G' )
          {
          os << "\t</Conflicting>" << std::endl;
          }
        else if ( mod == 'M' )
          {
          os << "\t</Modified>" << std::endl;
          }
        else
          {
          os << "\t</Updated>" << std::endl;
          }
        cmCTestUpdateHandler::UpdateFiles *u = &authors_files_map[sauthor1];
        cmCTestUpdateHandler::StringPair p;
        p.first = path;
        p.second = fname;
        u->push_back(p);

        current_path = path;
        }
      file_count ++;
      }
    }
  if ( file_count )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, std::endl);
    }
  if ( numUpdated )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Found " << numUpdated
      << " updated files" << std::endl);
    }
  if ( numModiefied )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Found " << numModiefied
      << " locally modified files"
      << std::endl);
    }
  if ( numConflicting )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Found " << numConflicting
      << " conflicting files"
      << std::endl);
    }
  if ( numModiefied == 0 && numConflicting == 0 && numUpdated == 0 )
    {
    cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Project is up-to-date"
      << std::endl);
    }
  if ( !first_file )
    {
    os << "\t</Directory>" << std::endl;
    }

  cmCTestUpdateHandler::AuthorsToUpdatesMap::iterator it;
  for ( it = authors_files_map.begin();
    it != authors_files_map.end();
    it ++ )
    {
    os << "\t<Author>\n"
      << "\t\t<Name>" << it->first << "</Name>" << std::endl;
    cmCTestUpdateHandler::UpdateFiles *u = &(it->second);
    for ( cc = 0; cc < u->size(); cc ++ )
      {
      os << "\t\t<File Directory=\"" << (*u)[cc].first << "\">"
        << (*u)[cc].second << "</File>" << std::endl;
      }
    os << "\t</Author>" << std::endl;
    }

  cmCTestLog(this->CTest, DEBUG, "End" << std::endl);
  std::string end_time = this->CTest->CurrentTime();
  os << "\t<EndDateTime>" << end_time << "</EndDateTime>\n"
    << "<ElapsedMinutes>" <<
    static_cast<int>((cmSystemTools::GetTime() - elapsed_time_start)/6)/10.0
    << "</ElapsedMinutes>\n"
    << "\t<UpdateReturnStatus>";
  if ( numModiefied > 0 || numConflicting > 0 )
    {
    os << "Update error: There are modified or conflicting files in the "
      "repository";
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "   There are modified or conflicting files in the repository"
      << std::endl);
    }
  if ( updateProducedError )
    {
    os << "Update error: ";
    os << this->CTest->MakeXMLSafe(checkoutErrorMessages);
    cmCTestLog(this->CTest, ERROR_MESSAGE, "   Update with command: "
      << command << " failed" << std::endl);
    }
  os << "</UpdateReturnStatus>" << std::endl;
  os << "</Update>" << std::endl;

  if (! res || retVal )
    {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
      "Error(s) when updating the project" << std::endl);
    cmCTestLog(this->CTest, ERROR_MESSAGE, "Output: "
      << goutput << std::endl);
    return -1;
    }
  return count;
}
