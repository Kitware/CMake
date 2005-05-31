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
//**********************************************************************
class cmCTestUpdateHandlerSVNXMLParser : public cmXMLParser
{
public:
  struct t_CommitLog
    {
    int m_Revision;
    std::string m_Author;
    std::string m_Date;
    std::string m_Message;
    };
  cmCTestUpdateHandlerSVNXMLParser(cmCTestUpdateHandler* up)
    : cmXMLParser(), m_UpdateHandler(up), m_MinRevision(-1), m_MaxRevision(-1)
    {
    }

  int Parse(const char* str)
    {
    m_MinRevision = -1;
    m_MaxRevision = -1;
    int res = this->cmXMLParser::Parse(str);
    if ( m_MinRevision == -1 || m_MaxRevision == -1 )
      {
      return 0;
      }
    return res;
    }

  typedef std::vector<t_CommitLog> t_VectorOfCommits;

  t_VectorOfCommits* GetCommits() { return &m_Commits; }
  int GetMinRevision() { return m_MinRevision; }
  int GetMaxRevision() { return m_MaxRevision; }

protected:
  void StartElement(const char* name, const char** atts)
    {
    if ( strcmp(name, "logentry") == 0 )
      {
      m_CommitLog = t_CommitLog();
      const char* rev = this->FindAttribute(atts, "revision");
      if ( rev)
        {
        m_CommitLog.m_Revision = atoi(rev);
        if ( m_MinRevision < 0 || m_MinRevision > m_CommitLog.m_Revision )
          {
          m_MinRevision = m_CommitLog.m_Revision;
          }
        if ( m_MaxRevision < 0 || m_MaxRevision < m_CommitLog.m_Revision )
          {
          m_MaxRevision = m_CommitLog.m_Revision;
          }
        }
      }
    m_CharacterData.erase(m_CharacterData.begin(), m_CharacterData.end());
    }
  void EndElement(const char* name)
    {
    if ( strcmp(name, "logentry") == 0 )
      {
      cmCTestLog(m_UpdateHandler->GetCTestInstance(), HANDLER_VERBOSE_OUTPUT, "\tRevision: " << m_CommitLog.m_Revision<< std::endl
        << "\tAuthor:   " << m_CommitLog.m_Author.c_str() << std::endl
        << "\tDate:     " << m_CommitLog.m_Date.c_str() << std::endl
        << "\tMessage:  " << m_CommitLog.m_Message.c_str() << std::endl);
      m_Commits.push_back(m_CommitLog);
      }
    else if ( strcmp(name, "author") == 0 )
      {
      m_CommitLog.m_Author.assign(&(*(m_CharacterData.begin())), m_CharacterData.size());
      }
    else if ( strcmp(name, "date") == 0 )
      {
      m_CommitLog.m_Date.assign(&(*(m_CharacterData.begin())), m_CharacterData.size());
      }
    else if ( strcmp(name, "msg") == 0 )
      {
      m_CommitLog.m_Message.assign(&(*(m_CharacterData.begin())), m_CharacterData.size());
      }
    m_CharacterData.erase(m_CharacterData.begin(), m_CharacterData.end());
    }
  void CharacterDataHandler(const char* data, int length)
    {
    m_CharacterData.insert(m_CharacterData.end(), data, data+length);
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
  std::vector<char> m_CharacterData;
  cmCTestUpdateHandler* m_UpdateHandler;
  t_CommitLog m_CommitLog;

  t_VectorOfCommits m_Commits;
  int m_MinRevision;
  int m_MaxRevision;
};
//**********************************************************************
//----------------------------------------------------------------------

//----------------------------------------------------------------------
cmCTestUpdateHandler::cmCTestUpdateHandler()
{
  m_CTest = 0;
}

//----------------------------------------------------------------------
int cmCTestUpdateHandler::DetermineType(const char* cmd, const char* type)
{
  if ( type && *type )
    {
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
  return cmCTestUpdateHandler::e_CVS;
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


  // Get source dir
  const char* sourceDirectory = this->GetOption("SourceDirectory");
  if ( !sourceDirectory )
    {
    cmCTestLog(m_CTest, ERROR, "Cannot find SourceDirectory  key in the DartConfiguration.tcl" << std::endl);
    return -1;
    }

  cmCTestLog(m_CTest, HANDLER_OUTPUT, "Updating the repository: " << sourceDirectory << std::endl);

  // Get update command
  std::string updateCommand = m_CTest->GetCTestConfiguration("UpdateCommand");
  if ( updateCommand.empty() )
    {
    updateCommand = m_CTest->GetCTestConfiguration("CVSCommand");
    if ( updateCommand.empty() )
      {
      updateCommand = m_CTest->GetCTestConfiguration("SVNCommand");
      if ( updateCommand.empty() )
        {
        cmCTestLog(m_CTest, ERROR, "Cannot find CVSCommand, SVNCommand, or UpdateCommand key in the DartConfiguration.tcl" << std::endl);
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
    updateType = this->DetermineType(updateCommand.c_str(), m_CTest->GetCTestConfiguration("UpdateType").c_str());
    }

  // And update options
  std::string updateOptions = m_CTest->GetCTestConfiguration("UpdateOptions");
  if ( updateOptions.empty() )
    {
    switch (updateType)
      {
    case cmCTestUpdateHandler::e_CVS:
      updateOptions = m_CTest->GetCTestConfiguration("CVSUpdateOptions");
      break;
    case cmCTestUpdateHandler::e_SVN:
      updateOptions = m_CTest->GetCTestConfiguration("SVNUpdateOptions");
      break;
      }
    }

  // Get update time
  std::string extra_update_opts;
  if ( m_CTest->GetTestModel() == cmCTest::NIGHTLY )
    {
    struct tm* t = m_CTest->GetNightlyTime(m_CTest->GetCTestConfiguration("NightlyStartTime"),
      m_CTest->GetTomorrowTag());
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

  cmGeneratedFileStream ofs;
  if ( !m_CTest->GetShowOnly() )
    {
    m_CTest->OpenOutputFile("Temporary", "LastUpdate.log", ofs);
    }

  // CVS variables
  // SVN variables
  int svn_current_revision = 0;
  int svn_latest_revision = 0;
  int svn_use_status = 0;

  std::string goutput;
  int retVal = 0;
  bool res = true;


  //
  // Get initial repository information if that is possible. With subversion, this will check the current revision.
  //
  if ( !command.empty() )
    {
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "* Get repository information: " << command.c_str() << std::endl);
    if ( !m_CTest->GetShowOnly() )
      {
      res = cmSystemTools::RunSingleCommand(command.c_str(), &goutput, 
        &retVal, sourceDirectory,
        m_HandlerVerbose, 0 /*m_TimeOut*/);
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
          cmsys::RegularExpression current_revision_regex("Revision: ([0-9]+)");
          if ( current_revision_regex.find(goutput.c_str()) )
            {
            std::string currentRevisionString = current_revision_regex.match(1);
            svn_current_revision = atoi(currentRevisionString.c_str());
            cmCTestLog(m_CTest, HANDLER_OUTPUT, "   Old revision of repository is: " << svn_current_revision << std::endl);
            }
          }
        break;
        }
      }
    else
      {
      cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "Update with command: " << command << std::endl);
      }
    }


  //
  // Now update repository and remember what files were updated
  // 
  cmGeneratedFileStream os; 
  if ( !m_CTest->OpenOutputFile(m_CTest->GetCurrentTag(), "Update.xml", os, true) )
    {
    cmCTestLog(m_CTest, ERROR, "Cannot open log file" << std::endl);
    }
  std::string start_time = m_CTest->CurrentTime();
  double elapsed_time_start = cmSystemTools::GetTime();

  cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "* Update repository: " << command.c_str() << std::endl);
  if ( !m_CTest->GetShowOnly() )
    {
    command = "";
    switch( updateType )
      {
    case cmCTestUpdateHandler::e_CVS:
      command = updateCommand + " -z3 update " + updateOptions +
        " " + extra_update_opts;
      res = cmSystemTools::RunSingleCommand(command.c_str(), &goutput, 
        &retVal, sourceDirectory,
        m_HandlerVerbose, 0 /*m_TimeOut*/);
      break;
    case cmCTestUpdateHandler::e_SVN:
        {
        std::string partialOutput;
        command = updateCommand + " update " + updateOptions +
          " " + extra_update_opts;
        bool res1 = cmSystemTools::RunSingleCommand(command.c_str(), &partialOutput, 
          &retVal, sourceDirectory,
          m_HandlerVerbose, 0 /*m_TimeOut*/);
        command = updateCommand + " status";
        res = cmSystemTools::RunSingleCommand(command.c_str(), &partialOutput, 
          &retVal, sourceDirectory,
          m_HandlerVerbose, 0 /*m_TimeOut*/);
        goutput += partialOutput;
        res = res && res1;
        }
      }
    if ( ofs )
      {
      ofs << "--- Update repository ---" << std::endl;
      ofs << goutput << std::endl;; 
      }
    }
  if ( !res || retVal )
    {
    updateProducedError = true;
    }

  os << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
    << "<Update mode=\"Client\" Generator=\"ctest-"
    << cmVersion::GetCMakeVersion() << "\">\n"
    << "\t<Site>" << m_CTest->GetCTestConfiguration("Site") << "</Site>\n"
    << "\t<BuildName>" << m_CTest->GetCTestConfiguration("BuildName")
    << "</BuildName>\n"
    << "\t<BuildStamp>" << m_CTest->GetCurrentTag() << "-"
    << m_CTest->GetTestModelString() << "</BuildStamp>" << std::endl;
  os << "\t<StartDateTime>" << start_time << "</StartDateTime>\n"
    << "\t<UpdateCommand>" << m_CTest->MakeXMLSafe(command)
    << "</UpdateCommand>\n";

  // Even though it failed, we may have some useful information. Try to continue...
  std::vector<cmStdString> lines;
  cmSystemTools::Split(goutput.c_str(), lines);

  // CVS style regular expressions
  cmsys::RegularExpression cvs_date_author_regex("^date: +([^;]+); +author: +([^;]+); +state: +[^;]+;");
  cmsys::RegularExpression cvs_revision_regex("^revision +([^ ]*) *$");
  cmsys::RegularExpression cvs_end_of_file_regex("^=============================================================================$");
  cmsys::RegularExpression cvs_end_of_comment_regex("^----------------------------$");

  // Subversion style regular expressions
  cmsys::RegularExpression svn_status_line_regex("^ *([0-9]+)  *([0-9]+)  *([^ ]+)  *([^ ][^\t\r\n]*)[ \t\r\n]*$");
  cmsys::RegularExpression svn_latest_revision_regex("(Updated to|At) revision ([0-9]+)\\.");

  cmsys::RegularExpression file_update_line("([A-Z])  *(.*)");
  std::string current_path = "<no-path>";
  bool first_file = true;

  cmCTestUpdateHandler::AuthorsToUpdatesMap authors_files_map;
  int num_updated = 0;
  int num_modified = 0;
  int num_conflicting = 0;
  // In subversion, get the latest revision
  if ( updateType == cmCTestUpdateHandler::e_SVN )
    {
    for ( cc= 0 ; cc < lines.size(); cc ++ )
      {
      const char* line = lines[cc].c_str();
      if ( svn_latest_revision_regex.find(line) )
        {
        svn_latest_revision = atoi(svn_latest_revision_regex.match(2).c_str());
        }
      }
    }
  if ( updateType == cmCTestUpdateHandler::e_SVN )
    {
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "   Current revision of repository is: " << svn_latest_revision << std::endl);
    }

  cmCTestLog(m_CTest, HANDLER_OUTPUT, "   Gathering version information (each . represents one updated file):" << std::endl);
  int file_count = 0;
  for ( cc= 0 ; cc < lines.size(); cc ++ )
    {
    const char* line = lines[cc].c_str();
    if ( file_update_line.find(line) )
      {
      if ( file_count == 0 )
        {
        cmCTestLog(m_CTest, HANDLER_OUTPUT, "    " << std::flush);
        }
      cmCTestLog(m_CTest, HANDLER_OUTPUT, "." << std::flush);
      std::string upChar = file_update_line.match(1);
      std::string upFile = file_update_line.match(2);
      char mod = upChar[0];
      bool modifiedOrConflict = false;
      if ( mod != 'M' && mod != 'C' && mod != 'G' )
        {
        count ++;
        modifiedOrConflict = true;
        }
      const char* file = upFile.c_str();
      cmCTestLog(m_CTest, DEBUG, "Line" << cc << ": " << mod << " - " << file << std::endl);

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
          if ( svn_latest_revision > 0 && svn_latest_revision > svn_current_revision )
            {
            cmOStringStream logCommandStream;
            logCommandStream << updateCommand << " log -r " << svn_current_revision << ":" << svn_latest_revision
              << " --xml \"" << file << "\"";
            logcommand = logCommandStream.str();
            }
          else
            {
            logcommand = updateCommand + " status  --verbose \"" + file + "\"";
            svn_use_status = 1;
            }
          break;
          }
        cmCTestLog(m_CTest, DEBUG, "Do log: " << logcommand << std::endl);
        cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "* Get file update information: " << logcommand.c_str() << std::endl);
        res = cmSystemTools::RunSingleCommand(logcommand.c_str(), &output, 
          &retVal, sourceDirectory,
          m_HandlerVerbose, 0 /*m_TimeOut*/);
        if ( ofs )
          {
          ofs << output << std::endl;
          }
        }
      if ( res && retVal == 0)
        {
        cmCTestLog(m_CTest, DEBUG, output << std::endl);
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
            else if ( !have_second && !sline && cvs_date_author_regex.find(clp) )
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
            else if ( sline && cvs_end_of_comment_regex.find(clp) || cvs_end_of_file_regex.find(clp))
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
              cmCTestLog(m_CTest, ERROR, "Bad output from SVN status command: " << output << std::endl);
              }
            else if ( svn_status_line_regex.match(4) != file )
              {
              cmCTestLog(m_CTest, ERROR, "Bad output from SVN status command. The file name returned: \"" << svn_status_line_regex.match(4) << "\" was different than the file specified: \"" << file << "\"" << std::endl);
              }
            else
              {
              srevision1 = svn_status_line_regex.match(2);
              int latest_revision = atoi(svn_status_line_regex.match(2).c_str());
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
              cmCTestUpdateHandlerSVNXMLParser::t_VectorOfCommits::iterator it;
              for ( it = parser.GetCommits()->begin(); 
                it != parser.GetCommits()->end(); 
                ++ it )
                {
                if ( it->m_Revision == maxrev )
                  {
                  cmOStringStream mRevStream;
                  mRevStream << maxrev;
                  srevision1 = mRevStream.str();
                  sauthor1 = it->m_Author;
                  comment1 = it->m_Message;
                  sdate1 = it->m_Date;
                  }
                else if ( it->m_Revision == minrev )
                  {
                  cmOStringStream mRevStream;
                  mRevStream << minrev;
                  srevision2 = mRevStream.str();
                  sauthor2 = it->m_Author;
                  comment2 = it->m_Message;       
                  sdate2 = it->m_Date;
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
          num_conflicting ++;
          os << "\t<Conflicting>" << std::endl;
          }
        else if ( mod == 'G' )
          {
          num_conflicting ++;
          os << "\t<Conflicting>" << std::endl;
          }
        else if ( mod == 'M' )
          {
          num_modified ++;
          os << "\t<Modified>" << std::endl;
          }
        else
          {
          num_updated ++;
          os << "\t<Updated>" << std::endl;
          }
        if ( srevision2 == "Unknown" )
          {
          srevision2 = srevision1;
          }
        cmCTestLog(m_CTest, HANDLER_VERBOSE_OUTPUT, "File: " << path.c_str() << " / " << fname.c_str() << " was updated by "
          << sauthor1.c_str() << " to revision: " << srevision1.c_str()
          << " from revision: " << srevision2.c_str() << std::endl);
        os << "\t\t<File Directory=\"" << cmCTest::MakeXMLSafe(path) << "\">" << cmCTest::MakeXMLSafe(fname)
          << "</File>\n"
          << "\t\t<Directory>" << cmCTest::MakeXMLSafe(path) << "</Directory>\n"
          << "\t\t<FullName>" << cmCTest::MakeXMLSafe(file) << "</FullName>\n"
          << "\t\t<CheckinDate>" << cmCTest::MakeXMLSafe(sdate1) << "</CheckinDate>\n"
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
            << "\t\t\t<PreviousRevision>" << srevision2 << "</PreviousRevision>\n"
            << "\t\t\t<Author>" << cmCTest::MakeXMLSafe(sauthor1) << "</Author>\n"
            << "\t\t\t<Date>" << cmCTest::MakeXMLSafe(sdate1) << "</Date>\n"
            << "\t\t\t<Comment>" << cmCTest::MakeXMLSafe(comment1) << "</Comment>\n"
            << "\t\t\t<Email>" << cmCTest::MakeXMLSafe(semail1) << "</Email>\n"
            << "\t\t</Revisions>\n"
            << "\t\t<Revisions>\n"
            << "\t\t\t<Revision>" << srevision2 << "</Revision>\n"
            << "\t\t\t<PreviousRevision>" << srevision2 << "</PreviousRevision>\n"
            << "\t\t\t<Author>" << cmCTest::MakeXMLSafe(sauthor2) << "</Author>\n"
            << "\t\t\t<Date>" << cmCTest::MakeXMLSafe(sdate2) << "</Date>\n"
            << "\t\t\t<Comment>" << cmCTest::MakeXMLSafe(comment2) << "</Comment>\n"
            << "\t\t\t<Email>" << cmCTest::MakeXMLSafe(semail2) << "</Email>\n"
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
    cmCTestLog(m_CTest, HANDLER_OUTPUT, std::endl);
    }
  if ( num_updated )
    {
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "   Found " << num_updated << " updated files" << std::endl);
    }
  if ( num_modified )
    {
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "   Found " << num_modified << " locally modified files" 
      << std::endl);
    }
  if ( num_conflicting )
    {
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "   Found " << num_conflicting << " conflicting files" 
      << std::endl);
    }
  if ( num_modified == 0 && num_conflicting == 0 && num_updated == 0 )
    {
    cmCTestLog(m_CTest, HANDLER_OUTPUT, "   Project is up-to-date" << std::endl);
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

  cmCTestLog(m_CTest, DEBUG, "End" << std::endl);
  std::string end_time = m_CTest->CurrentTime();
  os << "\t<EndDateTime>" << end_time << "</EndDateTime>\n"
    << "<ElapsedMinutes>" << 
    static_cast<int>((cmSystemTools::GetTime() - elapsed_time_start)/6)/10.0 
    << "</ElapsedMinutes>\n"
    << "\t<UpdateReturnStatus>";
  if ( num_modified > 0 || num_conflicting > 0 )
    {
    os << "Update error: There are modified or conflicting files in the repository";
    cmCTestLog(m_CTest, ERROR, "   There are modified or conflicting files in the repository" << std::endl);
    }
  if ( updateProducedError )
    {
    os << "Update error: ";
    os << m_CTest->MakeXMLSafe(goutput);
    cmCTestLog(m_CTest, ERROR, "   Update with command: " << command << " failed" << std::endl);
    }
  os << "</UpdateReturnStatus>" << std::endl;
  os << "</Update>" << std::endl;

  if ( ofs )
    {
    ofs.close();
    }

  if (! res || retVal )
    {
    cmCTestLog(m_CTest, ERROR, "Error(s) when updating the project" << std::endl);
    cmCTestLog(m_CTest, ERROR, "Output: " << goutput << std::endl);
    return -1;
    }
  return count;
}
