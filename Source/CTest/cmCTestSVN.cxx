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
    }
private:
  cmCTestSVN* SVN;
  std::string& Rev;
  cmsys::RegularExpression RegexRev;
  virtual bool ProcessLine()
    {
    if(this->RegexRev.find(this->Line))
      {
      this->Rev = this->RegexRev.match(1);
      }
    return true;
    }
};

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
}
