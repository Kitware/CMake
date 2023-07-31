/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestP4.h"

#include <algorithm>
#include <ctime>
#include <ostream>
#include <utility>

#include <cmext/algorithm>

#include "cmsys/RegularExpression.hxx"

#include "cmCTest.h"
#include "cmCTestVC.h"
#include "cmList.h"
#include "cmRange.h"
#include "cmSystemTools.h"

cmCTestP4::cmCTestP4(cmCTest* ct, std::ostream& log)
  : cmCTestGlobalVC(ct, log)
{
  this->PriorRev = this->Unknown;
}

cmCTestP4::~cmCTestP4() = default;

class cmCTestP4::IdentifyParser : public cmCTestVC::LineParser
{
public:
  IdentifyParser(cmCTestP4* p4, const char* prefix, std::string& rev)
    : Rev(rev)
  {
    this->SetLog(&p4->Log, prefix);
    this->RegexIdentify.compile("^Change ([0-9]+) on");
  }

private:
  std::string& Rev;
  cmsys::RegularExpression RegexIdentify;

  bool ProcessLine() override
  {
    if (this->RegexIdentify.find(this->Line)) {
      this->Rev = this->RegexIdentify.match(1);
      return false;
    }
    return true;
  }
};

class cmCTestP4::ChangesParser : public cmCTestVC::LineParser
{
public:
  ChangesParser(cmCTestP4* p4, const char* prefix)
    : P4(p4)
  {
    this->SetLog(&this->P4->Log, prefix);
    this->RegexIdentify.compile("^Change ([0-9]+) on");
  }

private:
  cmsys::RegularExpression RegexIdentify;
  cmCTestP4* P4;

  bool ProcessLine() override
  {
    if (this->RegexIdentify.find(this->Line)) {
      this->P4->ChangeLists.push_back(this->RegexIdentify.match(1));
    }
    return true;
  }
};

class cmCTestP4::UserParser : public cmCTestVC::LineParser
{
public:
  UserParser(cmCTestP4* p4, const char* prefix)
    : P4(p4)
  {
    this->SetLog(&this->P4->Log, prefix);
    this->RegexUser.compile("^(.+) <(.*)> \\((.*)\\) accessed (.*)$");
  }

private:
  cmsys::RegularExpression RegexUser;
  cmCTestP4* P4;

  bool ProcessLine() override
  {
    if (this->RegexUser.find(this->Line)) {
      User NewUser;

      NewUser.UserName = this->RegexUser.match(1);
      NewUser.EMail = this->RegexUser.match(2);
      NewUser.Name = this->RegexUser.match(3);
      NewUser.AccessTime = this->RegexUser.match(4);
      this->P4->Users[this->RegexUser.match(1)] = NewUser;

      return false;
    }
    return true;
  }
};

/* Diff format:
==== //depot/file#rev - /absolute/path/to/file ====
(diff data)
==== //depot/file2#rev - /absolute/path/to/file2 ====
(diff data)
==== //depot/file3#rev - /absolute/path/to/file3 ====
==== //depot/file4#rev - /absolute/path/to/file4 ====
(diff data)
*/
class cmCTestP4::DiffParser : public cmCTestVC::LineParser
{
public:
  DiffParser(cmCTestP4* p4, const char* prefix)
    : P4(p4)
  {
    this->SetLog(&this->P4->Log, prefix);
    this->RegexDiff.compile("^==== (.*)#[0-9]+ - (.*)");
  }

private:
  cmCTestP4* P4;
  bool AlreadyNotified = false;
  std::string CurrentPath;
  cmsys::RegularExpression RegexDiff;

  bool ProcessLine() override
  {
    if (!this->Line.empty() && this->Line[0] == '=' &&
        this->RegexDiff.find(this->Line)) {
      this->CurrentPath = this->RegexDiff.match(1);
      this->AlreadyNotified = false;
    } else {
      if (!this->AlreadyNotified) {
        this->P4->DoModification(PathModified, this->CurrentPath);
        this->AlreadyNotified = true;
      }
    }
    return true;
  }
};

cmCTestP4::User cmCTestP4::GetUserData(const std::string& username)
{
  auto it = this->Users.find(username);

  if (it == this->Users.end()) {
    std::vector<char const*> p4_users;
    this->SetP4Options(p4_users);
    p4_users.push_back("users");
    p4_users.push_back("-m");
    p4_users.push_back("1");
    p4_users.push_back(username.c_str());
    p4_users.push_back(nullptr);

    UserParser out(this, "users-out> ");
    OutputLogger err(this->Log, "users-err> ");
    this->RunChild(p4_users.data(), &out, &err);

    // The user should now be added to the map. Search again.
    it = this->Users.find(username);
    if (it == this->Users.end()) {
      return cmCTestP4::User();
    }
  }

  return it->second;
}

/* Commit format:

Change 1111111 by user@client on 2013/09/26 11:50:36

        text
        text

Affected files ...

... //path/to/file#rev edit
... //path/to/file#rev add
... //path/to/file#rev delete
... //path/to/file#rev integrate
*/
class cmCTestP4::DescribeParser : public cmCTestVC::LineParser
{
public:
  DescribeParser(cmCTestP4* p4, const char* prefix)
    : LineParser('\n', false)
    , P4(p4)
  {
    this->SetLog(&this->P4->Log, prefix);
    this->RegexHeader.compile("^Change ([0-9]+) by (.+)@(.+) on (.*)$");
    this->RegexDiff.compile(R"(^\.\.\. (.*)#[0-9]+ ([^ ]+)$)");
  }

private:
  cmsys::RegularExpression RegexHeader;
  cmsys::RegularExpression RegexDiff;
  cmCTestP4* P4;

  using Revision = cmCTestP4::Revision;
  using Change = cmCTestP4::Change;
  std::vector<Change> Changes;
  enum SectionType
  {
    SectionHeader,
    SectionBody,
    SectionDiffHeader,
    SectionDiff,
    SectionCount
  };
  SectionType Section = SectionHeader;
  Revision Rev;

  bool ProcessLine() override
  {
    if (this->Line.empty()) {
      this->NextSection();
    } else {
      switch (this->Section) {
        case SectionHeader:
          this->DoHeaderLine();
          break;
        case SectionBody:
          this->DoBodyLine();
          break;
        case SectionDiffHeader:
          break; // nothing to do
        case SectionDiff:
          this->DoDiffLine();
          break;
        case SectionCount:
          break; // never happens
      }
    }
    return true;
  }

  void NextSection()
  {
    if (this->Section == SectionDiff) {
      this->P4->DoRevision(this->Rev, this->Changes);
      this->Rev = Revision();
    }

    this->Section =
      static_cast<SectionType>((this->Section + 1) % SectionCount);
  }

  void DoHeaderLine()
  {
    if (this->RegexHeader.find(this->Line)) {
      this->Rev.Rev = this->RegexHeader.match(1);
      this->Rev.Date = this->RegexHeader.match(4);

      cmCTestP4::User user = this->P4->GetUserData(this->RegexHeader.match(2));
      this->Rev.Author = user.Name;
      this->Rev.EMail = user.EMail;

      this->Rev.Committer = this->Rev.Author;
      this->Rev.CommitterEMail = this->Rev.EMail;
      this->Rev.CommitDate = this->Rev.Date;
    }
  }

  void DoBodyLine()
  {
    if (this->Line[0] == '\t') {
      this->Rev.Log += this->Line.substr(1);
    }
    this->Rev.Log += "\n";
  }

  void DoDiffLine()
  {
    if (this->RegexDiff.find(this->Line)) {
      Change change;
      std::string Path = this->RegexDiff.match(1);
      if (Path.length() > 2 && Path[0] == '/' && Path[1] == '/') {
        size_t found = Path.find('/', 2);
        if (found != std::string::npos) {
          Path = Path.substr(found + 1);
        }
      }

      change.Path = Path;
      std::string action = this->RegexDiff.match(2);

      if (action == "add") {
        change.Action = 'A';
      } else if (action == "delete") {
        change.Action = 'D';
      } else if (action == "edit" || action == "integrate") {
        change.Action = 'M';
      }

      this->Changes.push_back(change);
    }
  }
};

void cmCTestP4::SetP4Options(std::vector<char const*>& CommandOptions)
{
  if (this->P4Options.empty()) {
    const char* p4 = this->CommandLineTool.c_str();
    this->P4Options.emplace_back(p4);

    // The CTEST_P4_CLIENT variable sets the P4 client used when issuing
    // Perforce commands, if it's different from the default one.
    std::string client = this->CTest->GetCTestConfiguration("P4Client");
    if (!client.empty()) {
      this->P4Options.emplace_back("-c");
      this->P4Options.push_back(client);
    }

    // Set the message language to be English, in case the P4 admin
    // has localized them
    this->P4Options.emplace_back("-L");
    this->P4Options.emplace_back("en");

    // The CTEST_P4_OPTIONS variable adds additional Perforce command line
    // options before the main command
    std::string opts = this->CTest->GetCTestConfiguration("P4Options");
    cm::append(this->P4Options, cmSystemTools::ParseArguments(opts));
  }

  CommandOptions.clear();
  for (std::string const& o : this->P4Options) {
    CommandOptions.push_back(o.c_str());
  }
}

std::string cmCTestP4::GetWorkingRevision()
{
  std::vector<char const*> p4_identify;
  this->SetP4Options(p4_identify);

  p4_identify.push_back("changes");
  p4_identify.push_back("-m");
  p4_identify.push_back("1");
  p4_identify.push_back("-t");

  std::string source = this->SourceDirectory + "/...#have";
  p4_identify.push_back(source.c_str());
  p4_identify.push_back(nullptr);

  std::string rev;
  IdentifyParser out(this, "p4_changes-out> ", rev);
  OutputLogger err(this->Log, "p4_changes-err> ");

  bool result = this->RunChild(p4_identify.data(), &out, &err);

  // If there was a problem contacting the server return "<unknown>"
  if (!result) {
    return "<unknown>";
  }

  if (rev.empty()) {
    return "0";
  }
  return rev;
}

bool cmCTestP4::NoteOldRevision()
{
  this->OldRevision = this->GetWorkingRevision();

  cmCTestLog(this->CTest, HANDLER_OUTPUT,
             "   Old revision of repository is: " << this->OldRevision
                                                  << "\n");
  this->PriorRev.Rev = this->OldRevision;
  return true;
}

bool cmCTestP4::NoteNewRevision()
{
  this->NewRevision = this->GetWorkingRevision();

  cmCTestLog(this->CTest, HANDLER_OUTPUT,
             "   New revision of repository is: " << this->NewRevision
                                                  << "\n");
  return true;
}

bool cmCTestP4::LoadRevisions()
{
  std::vector<char const*> p4_changes;
  this->SetP4Options(p4_changes);

  // Use 'p4 changes ...@old,new' to get a list of changelists
  std::string range = this->SourceDirectory + "/...";

  // If any revision is unknown it means we couldn't contact the server.
  // Do not process updates
  if (this->OldRevision == "<unknown>" || this->NewRevision == "<unknown>") {
    cmCTestLog(this->CTest, HANDLER_OUTPUT,
               "   At least one of the revisions "
                 << "is unknown. No repository changes will be reported.\n");
    return false;
  }

  range.append("@")
    .append(this->OldRevision)
    .append(",")
    .append(this->NewRevision);

  p4_changes.push_back("changes");
  p4_changes.push_back(range.c_str());
  p4_changes.push_back(nullptr);

  ChangesParser out(this, "p4_changes-out> ");
  OutputLogger err(this->Log, "p4_changes-err> ");

  this->ChangeLists.clear();
  this->RunChild(p4_changes.data(), &out, &err);

  if (this->ChangeLists.empty()) {
    return true;
  }

  // p4 describe -s ...@1111111,2222222
  std::vector<char const*> p4_describe;
  for (std::string const& i : cmReverseRange(this->ChangeLists)) {
    this->SetP4Options(p4_describe);
    p4_describe.push_back("describe");
    p4_describe.push_back("-s");
    p4_describe.push_back(i.c_str());
    p4_describe.push_back(nullptr);

    DescribeParser outDescribe(this, "p4_describe-out> ");
    OutputLogger errDescribe(this->Log, "p4_describe-err> ");
    this->RunChild(p4_describe.data(), &outDescribe, &errDescribe);
  }
  return true;
}

bool cmCTestP4::LoadModifications()
{
  std::vector<char const*> p4_diff;
  this->SetP4Options(p4_diff);

  p4_diff.push_back("diff");

  // Ideally we would use -Od but not all clients support it
  p4_diff.push_back("-dn");
  std::string source = this->SourceDirectory + "/...";
  p4_diff.push_back(source.c_str());
  p4_diff.push_back(nullptr);

  DiffParser out(this, "p4_diff-out> ");
  OutputLogger err(this->Log, "p4_diff-err> ");
  this->RunChild(p4_diff.data(), &out, &err);
  return true;
}

bool cmCTestP4::UpdateCustom(const std::string& custom)
{
  cmList p4_custom_command{ custom, cmList::EmptyElements::Yes };

  std::vector<char const*> p4_custom;
  p4_custom.reserve(p4_custom_command.size() + 1);
  for (std::string const& i : p4_custom_command) {
    p4_custom.push_back(i.c_str());
  }
  p4_custom.push_back(nullptr);

  OutputLogger custom_out(this->Log, "p4_customsync-out> ");
  OutputLogger custom_err(this->Log, "p4_customsync-err> ");

  return this->RunUpdateCommand(p4_custom.data(), &custom_out, &custom_err);
}

bool cmCTestP4::UpdateImpl()
{
  std::string custom = this->CTest->GetCTestConfiguration("P4UpdateCustom");
  if (!custom.empty()) {
    return this->UpdateCustom(custom);
  }

  // If we couldn't get a revision number before updating, abort.
  if (this->OldRevision == "<unknown>") {
    this->UpdateCommandLine = "Unknown current revision";
    cmCTestLog(this->CTest, ERROR_MESSAGE, "   Unknown current revision\n");
    return false;
  }

  std::vector<char const*> p4_sync;
  this->SetP4Options(p4_sync);

  p4_sync.push_back("sync");

  // Get user-specified update options.
  std::string opts = this->CTest->GetCTestConfiguration("UpdateOptions");
  if (opts.empty()) {
    opts = this->CTest->GetCTestConfiguration("P4UpdateOptions");
  }
  std::vector<std::string> args = cmSystemTools::ParseArguments(opts);
  for (std::string const& arg : args) {
    p4_sync.push_back(arg.c_str());
  }

  std::string source = this->SourceDirectory + "/...";

  // Specify the start time for nightly testing.
  if (this->CTest->GetTestModel() == cmCTest::NIGHTLY) {
    std::string date = this->GetNightlyTime();
    // CTest reports the date as YYYY-MM-DD, Perforce needs it as YYYY/MM/DD
    std::replace(date.begin(), date.end(), '-', '/');

    // Revision specification: /...@"YYYY/MM/DD HH:MM:SS"
    source.append("@\"").append(date).append("\"");
  }

  p4_sync.push_back(source.c_str());
  p4_sync.push_back(nullptr);

  OutputLogger out(this->Log, "p4_sync-out> ");
  OutputLogger err(this->Log, "p4_sync-err> ");

  return this->RunUpdateCommand(p4_sync.data(), &out, &err);
}
