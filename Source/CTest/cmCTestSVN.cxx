/*============================================================================
  CMake - Cross Platform Makefile Generator
  Copyright 2000-2009 Kitware, Inc.

  Distributed under the OSI-approved BSD License (the "License");
  see accompanying file Copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even the
  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
  See the License for more information.
============================================================================*/
#include "cmCTestSVN.h"

#include "cmCTest.h"
#include "cmSystemTools.h"
#include "cmXMLParser.h"
#include "cmXMLSafe.h"

#include <cmsys/RegularExpression.hxx>

//----------------------------------------------------------------------------
cmCTestSVN::cmCTestSVN(cmCTest* ct, std::ostream& log):
  cmCTestGlobalVC(ct, log)
{
  this->PriorRev = this->Unknown;
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
  this->PriorRev.Rev = this->OldRevision;
}

//----------------------------------------------------------------------------
void cmCTestSVN::NoteNewRevision()
{
  this->NewRevision = this->LoadInfo();
  this->Log << "Revision after update: " << this->NewRevision << "\n";
  cmCTestLog(this->CTest, HANDLER_OUTPUT, "   New revision of repository is: "
             << this->NewRevision << "\n");

  // this->Root = ""; // uncomment to test GuessBase
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

//----------------------------------------------------------------------------
void cmCTestSVN::GuessBase(std::vector<Change> const& changes)
{
  // Subversion did not give us a good repository root so we need to
  // guess the base path from the URL and the paths in a revision with
  // changes under it.

  // Consider each possible URL suffix from longest to shortest.
  for(std::string::size_type slash = this->URL.find('/');
      this->Base.empty() && slash != std::string::npos;
      slash = this->URL.find('/', slash+1))
    {
    // If the URL suffix is a prefix of at least one path then it is the base.
    std::string base = cmCTest::DecodeURL(this->URL.substr(slash));
    for(std::vector<Change>::const_iterator ci = changes.begin();
        this->Base.empty() && ci != changes.end(); ++ci)
      {
      if(cmCTestSVNPathStarts(ci->Path, base))
        {
        this->Base = base;
        }
      }
    }

  // We always append a slash so that we know paths beginning in the
  // base lie under its path.  If no base was found then the working
  // tree must be a checkout of the entire repo and this will match
  // the leading slash in all paths.
  this->Base += "/";

  this->Log << "Guessed Base = " << this->Base << "\n";
}

//----------------------------------------------------------------------------
const char* cmCTestSVN::LocalPath(std::string const& path)
{
  if(path.size() > this->Base.size() &&
     strncmp(path.c_str(), this->Base.c_str(), this->Base.size()) == 0)
    {
    // This path lies under the base, so return a relative path.
    return path.c_str() + this->Base.size();
    }
  else
    {
    // This path does not lie under the base, so ignore it.
    return 0;
    }
}

//----------------------------------------------------------------------------
class cmCTestSVN::UpdateParser: public cmCTestVC::LineParser
{
public:
  UpdateParser(cmCTestSVN* svn, const char* prefix): SVN(svn)
    {
    this->SetLog(&svn->Log, prefix);
    this->RegexUpdate.compile("^([ADUCGE ])([ADUCGE ])[B ] +(.+)$");
    }
private:
  cmCTestSVN* SVN;
  cmsys::RegularExpression RegexUpdate;

  bool ProcessLine()
    {
    if(this->RegexUpdate.find(this->Line))
      {
      this->DoPath(this->RegexUpdate.match(1)[0],
                   this->RegexUpdate.match(2)[0],
                   this->RegexUpdate.match(3));
      }
    return true;
    }

  void DoPath(char path_status, char prop_status, std::string const& path)
    {
    char status = (path_status != ' ')? path_status : prop_status;
    std::string dir = cmSystemTools::GetFilenamePath(path);
    std::string name = cmSystemTools::GetFilenameName(path);
    // See "svn help update".
    switch(status)
      {
      case 'G':
        this->SVN->Dirs[dir][name].Status = PathModified;
        break;
      case 'C':
        this->SVN->Dirs[dir][name].Status = PathConflicting;
        break;
      case 'A': case 'D': case 'U':
        this->SVN->Dirs[dir][name].Status = PathUpdated;
        break;
      case 'E': // TODO?
      case '?': case ' ': default:
        break;
      }
    }
};

//----------------------------------------------------------------------------
bool cmCTestSVN::UpdateImpl()
{
  // Get user-specified update options.
  std::string opts = this->CTest->GetCTestConfiguration("UpdateOptions");
  if(opts.empty())
    {
    opts = this->CTest->GetCTestConfiguration("SVNUpdateOptions");
    }
  std::vector<cmStdString> args = cmSystemTools::ParseArguments(opts.c_str());

  // Specify the start time for nightly testing.
  if(this->CTest->GetTestModel() == cmCTest::NIGHTLY)
    {
    args.push_back("-r{" + this->GetNightlyTime() + " +0000}");
    }

  std::vector<char const*> svn_update;
  svn_update.push_back(this->CommandLineTool.c_str());
  svn_update.push_back("update");
  svn_update.push_back("--non-interactive");
  for(std::vector<cmStdString>::const_iterator ai = args.begin();
      ai != args.end(); ++ai)
    {
    svn_update.push_back(ai->c_str());
    }
  svn_update.push_back(0);

  UpdateParser out(this, "up-out> ");
  OutputLogger err(this->Log, "up-err> ");
  return this->RunUpdateCommand(&svn_update[0], &out, &err);
}

//----------------------------------------------------------------------------
class cmCTestSVN::LogParser: public cmCTestVC::OutputLogger,
                             private cmXMLParser
{
public:
  LogParser(cmCTestSVN* svn, const char* prefix):
    OutputLogger(svn->Log, prefix), SVN(svn) { this->InitializeParser(); }
  ~LogParser() { this->CleanupParser(); }
private:
  cmCTestSVN* SVN;

  typedef cmCTestSVN::Revision Revision;
  typedef cmCTestSVN::Change Change;
  Revision Rev;
  std::vector<Change> Changes;
  Change CurChange;
  std::vector<char> CData;

  virtual bool ProcessChunk(const char* data, int length)
    {
    this->OutputLogger::ProcessChunk(data, length);
    this->ParseChunk(data, length);
    return true;
    }

  virtual void StartElement(const char* name, const char** atts)
    {
    this->CData.clear();
    if(strcmp(name, "logentry") == 0)
      {
      this->Rev = Revision();
      if(const char* rev = this->FindAttribute(atts, "revision"))
        {
        this->Rev.Rev = rev;
        }
      this->Changes.clear();
      }
    else if(strcmp(name, "path") == 0)
      {
      this->CurChange = Change();
      if(const char* action = this->FindAttribute(atts, "action"))
        {
        this->CurChange.Action = action[0];
        }
      }
    }

  virtual void CharacterDataHandler(const char* data, int length)
    {
    this->CData.insert(this->CData.end(), data, data+length);
    }

  virtual void EndElement(const char* name)
    {
    if(strcmp(name, "logentry") == 0)
      {
      this->SVN->DoRevision(this->Rev, this->Changes);
      }
    else if(strcmp(name, "path") == 0 && !this->CData.empty())
      {
      this->CurChange.Path.assign(&this->CData[0], this->CData.size());
      this->Changes.push_back(this->CurChange);
      }
    else if(strcmp(name, "author") == 0 && !this->CData.empty())
      {
      this->Rev.Author.assign(&this->CData[0], this->CData.size());
      }
    else if(strcmp(name, "date") == 0 && !this->CData.empty())
      {
      this->Rev.Date.assign(&this->CData[0], this->CData.size());
      }
    else if(strcmp(name, "msg") == 0 && !this->CData.empty())
      {
      this->Rev.Log.assign(&this->CData[0], this->CData.size());
      }
    this->CData.clear();
    }

  virtual void ReportError(int, int, const char* msg)
    {
    this->SVN->Log << "Error parsing svn log xml: " << msg << "\n";
    }
};

//----------------------------------------------------------------------------
void cmCTestSVN::LoadRevisions()
{
  // We are interested in every revision included in the update.
  std::string revs;
  if(atoi(this->OldRevision.c_str()) < atoi(this->NewRevision.c_str()))
    {
    revs = "-r" + this->OldRevision + ":" + this->NewRevision;
    }
  else
    {
    revs = "-r" + this->NewRevision;
    }

  // Run "svn log" to get all global revisions of interest.
  const char* svn = this->CommandLineTool.c_str();
  const char* svn_log[] = {svn, "log", "--xml", "-v", revs.c_str(), 0};
  {
  LogParser out(this, "log-out> ");
  OutputLogger err(this->Log, "log-err> ");
  this->RunChild(svn_log, &out, &err);
  }
}

//----------------------------------------------------------------------------
void cmCTestSVN::DoRevision(Revision const& revision,
                            std::vector<Change> const& changes)
{
  // Guess the base checkout path from the changes if necessary.
  if(this->Base.empty() && !changes.empty())
    {
    this->GuessBase(changes);
    }
  this->cmCTestGlobalVC::DoRevision(revision, changes);
}

//----------------------------------------------------------------------------
class cmCTestSVN::StatusParser: public cmCTestVC::LineParser
{
public:
  StatusParser(cmCTestSVN* svn, const char* prefix): SVN(svn)
    {
    this->SetLog(&svn->Log, prefix);
    this->RegexStatus.compile("^([ACDIMRX?!~ ])([CM ])[ L]... +(.+)$");
    }
private:
  cmCTestSVN* SVN;
  cmsys::RegularExpression RegexStatus;
  bool ProcessLine()
    {
    if(this->RegexStatus.find(this->Line))
      {
      this->DoPath(this->RegexStatus.match(1)[0],
                   this->RegexStatus.match(2)[0],
                   this->RegexStatus.match(3));
      }
    return true;
    }

  void DoPath(char path_status, char prop_status, std::string const& path)
    {
    char status = (path_status != ' ')? path_status : prop_status;
    // See "svn help status".
    switch(status)
      {
      case 'M': case '!': case 'A': case 'D': case 'R':
        this->SVN->DoModification(PathModified, path);
        break;
      case 'C': case '~':
        this->SVN->DoModification(PathConflicting, path);
        break;
      case 'X': case 'I': case '?': case ' ': default:
        break;
      }
    }
};

//----------------------------------------------------------------------------
void cmCTestSVN::LoadModifications()
{
  // Run "svn status" which reports local modifications.
  const char* svn = this->CommandLineTool.c_str();
  const char* svn_status[] = {svn, "status", "--non-interactive", 0};
  StatusParser out(this, "status-out> ");
  OutputLogger err(this->Log, "status-err> ");
  this->RunChild(svn_status, &out, &err);
}

//----------------------------------------------------------------------------
void cmCTestSVN::WriteXMLGlobal(std::ostream& xml)
{
  this->cmCTestGlobalVC::WriteXMLGlobal(xml);

  xml << "\t<SVNPath>" << this->Base << "</SVNPath>\n";
}
