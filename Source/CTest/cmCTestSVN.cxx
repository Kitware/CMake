/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc. All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmCTestSVN.h"

#include "cmCTest.h"

#include <cmsys/RegularExpression.hxx>

//----------------------------------------------------------------------------
cmCTestSVN::cmCTestSVN(cmCTest* ct, std::ostream& log): cmCTestVC(ct, log)
{
}

//----------------------------------------------------------------------------
cmCTestSVN::~cmCTestSVN()
{
}

//----------------------------------------------------------------------------
void cmCTestSVN::CleanupImpl()
{
  const char* svn = this->CommandLineTool.c_str();
  const char* svn_cleanup[] = {svn, "cleanup", 0};
  OutputLogger out(this->Log, "cleanup-out> ");
  OutputLogger err(this->Log, "cleanup-err> ");
  this->RunChild(svn_cleanup, &out, &err);
}

//----------------------------------------------------------------------------
class cmCTestSVN::InfoParser: public cmCTestVC::LineParser
{
public:
  InfoParser(cmCTestSVN* svn, const char* prefix, std::string& rev):
    SVN(svn), Rev(rev)
    {
    this->SetLog(&svn->Log, prefix);
    this->RegexRev.compile("^Revision: ([0-9]+)");
    this->RegexURL.compile("^URL: +([^ ]+) *$");
    this->RegexRoot.compile("^Repository Root: +([^ ]+) *$");
    }
private:
  cmCTestSVN* SVN;
  std::string& Rev;
  cmsys::RegularExpression RegexRev;
  cmsys::RegularExpression RegexURL;
  cmsys::RegularExpression RegexRoot;
  virtual bool ProcessLine()
    {
    if(this->RegexRev.find(this->Line))
      {
      this->Rev = this->RegexRev.match(1);
      }
    else if(this->RegexURL.find(this->Line))
      {
      this->SVN->URL = this->RegexURL.match(1);
      }
    else if(this->RegexRoot.find(this->Line))
      {
      this->SVN->Root = this->RegexRoot.match(1);
      }
    return true;
    }
};

//----------------------------------------------------------------------------
static bool cmCTestSVNPathStarts(std::string const& p1, std::string const& p2)
{
  // Does path p1 start with path p2?
  if(p1.size() == p2.size())
    {
    return p1 == p2;
    }
  else if(p1.size() > p2.size() && p1[p2.size()] == '/')
    {
    return strncmp(p1.c_str(), p2.c_str(), p2.size()) == 0;
    }
  else
    {
    return false;
    }
}

//----------------------------------------------------------------------------
std::string cmCTestSVN::LoadInfo()
{
  // Run "svn info" to get the repository info from the work tree.
  const char* svn = this->CommandLineTool.c_str();
  const char* svn_info[] = {svn, "info", 0};
  std::string rev;
  InfoParser out(this, "info-out> ", rev);
  OutputLogger err(this->Log, "info-err> ");
  this->RunChild(svn_info, &out, &err);
  return rev;
}

//----------------------------------------------------------------------------
void cmCTestSVN::NoteOldRevision()
{
  this->OldRevision = this->LoadInfo();
  this->Log << "Revision before update: " << this->OldRevision << "\n";
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Old revision of repository is: "
             << this->OldRevision << "\n");
}

//----------------------------------------------------------------------------
void cmCTestSVN::NoteNewRevision()
{
  this->NewRevision = this->LoadInfo();
  this->Log << "Revision after update: " << this->NewRevision << "\n";
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "   New revision of repository is: "
             << this->NewRevision << "\n");

  this->Log << "URL = " << this->URL << "\n";
  this->Log << "Root = " << this->Root << "\n";

  // Compute the base path the working tree has checked out under
  // the repository root.
  if(!this->Root.empty() && cmCTestSVNPathStarts(this->URL, this->Root))
    {
    this->Base = cmCTest::DecodeURL(this->URL.substr(this->Root.size()));
    this->Base += "/";
    }
  this->Log << "Base = " << this->Base << "\n";
}
