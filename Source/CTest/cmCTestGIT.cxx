/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestGIT.h"

#include "cmCTest.h"
#include "cmSystemTools.h"
#include "cmXMLSafe.h"

#include <cmsys/RegularExpression.hxx>
#include <cmsys/ios/sstream>
#include <cmsys/Process.h>

#include <sys/types.h>
#include <time.h>
#include <ctype.h>

//----------------------------------------------------------------------------
cmCTestGIT::cmCTestGIT(cmCTest* ct, std::ostream& log):
  cmCTestGlobalVC(ct, log)
{
  this->PriorRev = this->Unknown;
}

//----------------------------------------------------------------------------
cmCTestGIT::~cmCTestGIT()
{
}

//----------------------------------------------------------------------------
class cmCTestGIT::OneLineParser: public cmCTestVC::LineParser
{
public:
  OneLineParser(cmCTestGIT* git, const char* prefix,
                std::string& l): Line1(l)
    {
    this->SetLog(&git->Log, prefix);
    }
private:
  std::string& Line1;
  virtual bool ProcessLine()
    {
    // Only the first line is of interest.
    this->Line1 = this->Line;
    return false;
    }
};

//----------------------------------------------------------------------------
std::string cmCTestGIT::GetWorkingRevision()
{
  // Run plumbing "git rev-list" to get work tree revision.
  const char* git = this->CommandLineTool.c_str();
  const char* git_rev_list[] = {git, "rev-list", "-n", "1", "HEAD", 0};
  std::string rev;
  OneLineParser out(this, "rl-out> ", rev);
  OutputLogger err(this->Log, "rl-err> ");
  this->RunChild(git_rev_list, &out, &err);
  return rev;
}

//----------------------------------------------------------------------------
void cmCTestGIT::NoteOldRevision()
{
  this->OldRevision = this->GetWorkingRevision();
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "   Old revision of repository is: "
             << this->OldRevision << "\n");
  this->PriorRev.Rev = this->OldRevision;
}

//----------------------------------------------------------------------------
void cmCTestGIT::NoteNewRevision()
{
  this->NewRevision = this->GetWorkingRevision();
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "   New revision of repository is: "
             << this->NewRevision << "\n");
}

//----------------------------------------------------------------------------
bool cmCTestGIT::UpdateImpl()
{
  // Use "git pull" to update the working tree.
  std::vector<char const*> git_pull;
  git_pull.push_back(this->CommandLineTool.c_str());
  git_pull.push_back("pull");

  // TODO: if(this->CTest->GetTestModel() == cmCTest::NIGHTLY)

  // Add user-specified update options.
  std::string opts = this->CTest->GetCTestConfiguration("UpdateOptions");
  if(opts.empty())
    {
    opts = this->CTest->GetCTestConfiguration("GITUpdateOptions");
    }
  std::vector<cmStdString> args = cmSystemTools::ParseArguments(opts.c_str());
  for(std::vector<cmStdString>::const_iterator ai = args.begin();
      ai != args.end(); ++ai)
    {
    git_pull.push_back(ai->c_str());
    }

  // Sentinel argument.
  git_pull.push_back(0);

  OutputLogger out(this->Log, "pull-out> ");
  OutputLogger err(this->Log, "pull-err> ");
  return this->RunUpdateCommand(&git_pull[0], &out, &err);
}

//----------------------------------------------------------------------------
/* Diff format:

   :src-mode dst-mode src-sha1 dst-sha1 status\0
   src-path\0
   [dst-path\0]

   The format is repeated for every file changed.  The [dst-path\0]
   line appears only for lines with status 'C' or 'R'.  See 'git help
   diff-tree' for details.
*/
class cmCTestGIT::DiffParser: public cmCTestVC::LineParser
{
public:
  DiffParser(cmCTestGIT* git, const char* prefix):
    LineParser('\0', false), GIT(git), DiffField(DiffFieldNone)
    {
    this->SetLog(&git->Log, prefix);
    }

  typedef cmCTestGIT::Change Change;
  std::vector<Change> Changes;
protected:
  cmCTestGIT* GIT;
  enum DiffFieldType { DiffFieldNone, DiffFieldChange,
                       DiffFieldSrc, DiffFieldDst };
  DiffFieldType DiffField;
  Change CurChange;

  void DiffReset()
    {
    this->DiffField = DiffFieldNone;
    this->Changes.clear();
    }

  virtual bool ProcessLine()
    {
    if(this->Line[0] == ':')
      {
      this->DiffField = DiffFieldChange;
      this->CurChange = Change();
      }
    if(this->DiffField == DiffFieldChange)
      {
      // :src-mode dst-mode src-sha1 dst-sha1 status
      if(this->Line[0] != ':')
        {
        this->DiffField = DiffFieldNone;
        return true;
        }
      const char* src_mode_first = this->Line.c_str()+1;
      const char* src_mode_last  = this->ConsumeField(src_mode_first);
      const char* dst_mode_first = this->ConsumeSpace(src_mode_last);
      const char* dst_mode_last  = this->ConsumeField(dst_mode_first);
      const char* src_sha1_first = this->ConsumeSpace(dst_mode_last);
      const char* src_sha1_last  = this->ConsumeField(src_sha1_first);
      const char* dst_sha1_first = this->ConsumeSpace(src_sha1_last);
      const char* dst_sha1_last  = this->ConsumeField(dst_sha1_first);
      const char* status_first   = this->ConsumeSpace(dst_sha1_last);
      const char* status_last    = this->ConsumeField(status_first);
      if(status_first != status_last)
        {
        this->CurChange.Action = *status_first;
        this->DiffField = DiffFieldSrc;
        }
      else
        {
        this->DiffField = DiffFieldNone;
        }
      }
    else if(this->DiffField == DiffFieldSrc)
      {
      // src-path
      if(this->CurChange.Action == 'C')
        {
        // Convert copy to addition of destination.
        this->CurChange.Action = 'A';
        this->DiffField = DiffFieldDst;
        }
      else if(this->CurChange.Action == 'R')
        {
        // Convert rename to deletion of source and addition of destination.
        this->CurChange.Action = 'D';
        this->CurChange.Path = this->Line;
        this->Changes.push_back(this->CurChange);

        this->CurChange = Change('A');
        this->DiffField = DiffFieldDst;
        }
      else
        {
        this->CurChange.Path = this->Line;
        this->Changes.push_back(this->CurChange);
        this->DiffField = this->DiffFieldNone;
        }
      }
    else if(this->DiffField == DiffFieldDst)
      {
      // dst-path
      this->CurChange.Path = this->Line;
      this->Changes.push_back(this->CurChange);
      this->DiffField = this->DiffFieldNone;
      }
    return true;
    }

  const char* ConsumeSpace(const char* c)
    {
    while(*c && isspace(*c)) { ++c; }
    return c;
    }
  const char* ConsumeField(const char* c)
    {
    while(*c && !isspace(*c)) { ++c; }
    return c;
    }
};

//----------------------------------------------------------------------------
/* Commit format:

   commit ...\n
   tree ...\n
   parent ...\n
   author ...\n
   committer ...\n
   \n
       Log message indented by (4) spaces\n
       (even blank lines have the spaces)\n
   \n
   [Diff format]

   The header may have more fields.  See 'git help diff-tree'.
*/
class cmCTestGIT::CommitParser: public cmCTestGIT::DiffParser
{
public:
  CommitParser(cmCTestGIT* git, const char* prefix):
    DiffParser(git, prefix), Section(SectionHeader)
    {
    this->Separator = SectionSep[this->Section];
    }

private:
  typedef cmCTestGIT::Revision Revision;
  enum SectionType { SectionHeader, SectionBody, SectionDiff, SectionCount };
  static char const SectionSep[SectionCount];
  SectionType Section;
  Revision Rev;

  struct Person
  {
    std::string Name;
    std::string EMail;
    unsigned long Time;
    long TimeZone;
    Person(): Name(), EMail(), Time(0), TimeZone(0) {}
  };

  void ParsePerson(const char* str, Person& person)
    {
    // Person Name <person@domain.com> 1234567890 +0000
    const char* c = str;
    while(*c && isspace(*c)) { ++c; }

    const char* name_first = c;
    while(*c && *c != '<') { ++c; }
    const char* name_last = c;
    while(name_last != name_first && isspace(*(name_last-1))) { --name_last; }
    person.Name.assign(name_first, name_last-name_first);

    const char* email_first = *c? ++c : c;
    while(*c && *c != '>') { ++c; }
    const char* email_last = *c? c++ : c;
    person.EMail.assign(email_first, email_last-email_first);

    person.Time = strtoul(c, (char**)&c, 10);
    person.TimeZone = strtol(c, (char**)&c, 10);
    }

  virtual bool ProcessLine()
    {
    if(this->Line.empty())
      {
      this->NextSection();
      }
    else
      {
      switch(this->Section)
        {
        case SectionHeader: this->DoHeaderLine(); break;
        case SectionBody:   this->DoBodyLine(); break;
        case SectionDiff:   this->DiffParser::ProcessLine(); break;
        case SectionCount:  break; // never happens
        }
      }
    return true;
    }

  void NextSection()
    {
    this->Section = SectionType((this->Section+1) % SectionCount);
    this->Separator = SectionSep[this->Section];
    if(this->Section == SectionHeader)
      {
      this->GIT->DoRevision(this->Rev, this->Changes);
      this->Rev = Revision();
      this->DiffReset();
      }
    }

  void DoHeaderLine()
    {
    // Look for header fields that we need.
    if(strncmp(this->Line.c_str(), "commit ", 7) == 0)
      {
      this->Rev.Rev = this->Line.c_str()+7;
      }
    else if(strncmp(this->Line.c_str(), "author ", 7) == 0)
      {
      Person author;
      this->ParsePerson(this->Line.c_str()+7, author);
      this->Rev.Author = author.Name;

      // Convert the time to a human-readable format that is also easy
      // to machine-parse: "CCYY-MM-DD hh:mm:ss".
      time_t seconds = static_cast<time_t>(author.Time);
      struct tm* t = gmtime(&seconds);
      char dt[1024];
      sprintf(dt, "%04d-%02d-%02d %02d:%02d:%02d",
              t->tm_year+1900, t->tm_mon+1, t->tm_mday,
              t->tm_hour, t->tm_min, t->tm_sec);
      this->Rev.Date = dt;

      // Add the time-zone field "+zone" or "-zone".
      char tz[32];
      if(author.TimeZone >= 0)
        {
        sprintf(tz, " +%04ld", author.TimeZone);
        }
      else
        {
        sprintf(tz, " -%04ld", -author.TimeZone);
        }
      this->Rev.Date += tz;
      }
    }

  void DoBodyLine()
    {
    // Commit log lines are indented by 4 spaces.
    if(this->Line.size() >= 4)
      {
      this->Rev.Log += this->Line.substr(4);
      }
    this->Rev.Log += "\n";
    }
};

char const cmCTestGIT::CommitParser::SectionSep[SectionCount] =
{'\n', '\n', '\0'};

//----------------------------------------------------------------------------
void cmCTestGIT::LoadRevisions()
{
  // Use 'git rev-list ... | git diff-tree ...' to get revisions.
  std::string range = this->OldRevision + ".." + this->NewRevision;
  const char* git = this->CommandLineTool.c_str();
  const char* git_rev_list[] =
    {git, "rev-list", "--reverse", range.c_str(), "--", 0};
  const char* git_diff_tree[] =
    {git, "diff-tree", "--stdin", "--always", "-z", "-r", "--pretty=raw",
     "--encoding=utf-8", 0};
  this->Log << this->ComputeCommandLine(git_rev_list) << " | "
            << this->ComputeCommandLine(git_diff_tree) << "\n";

  cmsysProcess* cp = cmsysProcess_New();
  cmsysProcess_AddCommand(cp, git_rev_list);
  cmsysProcess_AddCommand(cp, git_diff_tree);
  cmsysProcess_SetWorkingDirectory(cp, this->SourceDirectory.c_str());

  CommitParser out(this, "dt-out> ");
  OutputLogger err(this->Log, "dt-err> ");
  this->RunProcess(cp, &out, &err);

  // Send one extra zero-byte to terminate the last record.
  out.Process("", 1);

  cmsysProcess_Delete(cp);
}

//----------------------------------------------------------------------------
void cmCTestGIT::LoadModifications()
{
  const char* git = this->CommandLineTool.c_str();

  // Use 'git update-index' to refresh the index w.r.t. the work tree.
  const char* git_update_index[] = {git, "update-index", "--refresh", 0};
  OutputLogger ui_out(this->Log, "ui-out> ");
  OutputLogger ui_err(this->Log, "ui-err> ");
  this->RunChild(git_update_index, &ui_out, &ui_err);

  // Use 'git diff-index' to get modified files.
  const char* git_diff_index[] = {git, "diff-index", "-z", "HEAD", 0};
  DiffParser out(this, "di-out> ");
  OutputLogger err(this->Log, "di-err> ");
  this->RunChild(git_diff_index, &out, &err);

  for(std::vector<Change>::const_iterator ci = out.Changes.begin();
      ci != out.Changes.end(); ++ci)
    {
    this->DoModification(PathModified, ci->Path);
    }
}
