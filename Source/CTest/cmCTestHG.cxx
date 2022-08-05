/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestHG.h"

#include <ostream>
#include <vector>

#include <cmext/algorithm>

#include "cmsys/RegularExpression.hxx"

#include "cmCTest.h"
#include "cmCTestVC.h"
#include "cmProcessTools.h"
#include "cmSystemTools.h"
#include "cmXMLParser.h"

cmCTestHG::cmCTestHG(cmCTest* ct, std::ostream& log)
  : cmCTestGlobalVC(ct, log)
{
  this->PriorRev = this->Unknown;
}

cmCTestHG::~cmCTestHG() = default;

class cmCTestHG::IdentifyParser : public cmCTestVC::LineParser
{
public:
  IdentifyParser(cmCTestHG* hg, const char* prefix, std::string& rev)
    : Rev(rev)
  {
    this->SetLog(&hg->Log, prefix);
    this->RegexIdentify.compile("^([0-9a-f]+)");
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

class cmCTestHG::StatusParser : public cmCTestVC::LineParser
{
public:
  StatusParser(cmCTestHG* hg, const char* prefix)
    : HG(hg)
  {
    this->SetLog(&hg->Log, prefix);
    this->RegexStatus.compile("([MARC!?I]) (.*)");
  }

private:
  cmCTestHG* HG;
  cmsys::RegularExpression RegexStatus;

  bool ProcessLine() override
  {
    if (this->RegexStatus.find(this->Line)) {
      this->DoPath(this->RegexStatus.match(1)[0], this->RegexStatus.match(2));
    }
    return true;
  }

  void DoPath(char status, std::string const& path)
  {
    if (path.empty()) {
      return;
    }

    // See "hg help status".  Note that there is no 'conflict' status.
    switch (status) {
      case 'M':
      case 'A':
      case '!':
      case 'R':
        this->HG->DoModification(PathModified, path);
        break;
      case 'I':
      case '?':
      case 'C':
      case ' ':
      default:
        break;
    }
  }
};

std::string cmCTestHG::GetWorkingRevision()
{
  // Run plumbing "hg identify" to get work tree revision.
  const char* hg = this->CommandLineTool.c_str();
  const char* hg_identify[] = { hg, "identify", "-i", nullptr };
  std::string rev;
  IdentifyParser out(this, "rev-out> ", rev);
  OutputLogger err(this->Log, "rev-err> ");
  this->RunChild(hg_identify, &out, &err);
  return rev;
}

bool cmCTestHG::NoteOldRevision()
{
  this->OldRevision = this->GetWorkingRevision();
  cmCTestLog(this->CTest, HANDLER_OUTPUT,
             "   Old revision of repository is: " << this->OldRevision
                                                  << "\n");
  this->PriorRev.Rev = this->OldRevision;
  return true;
}

bool cmCTestHG::NoteNewRevision()
{
  this->NewRevision = this->GetWorkingRevision();
  cmCTestLog(this->CTest, HANDLER_OUTPUT,
             "   New revision of repository is: " << this->NewRevision
                                                  << "\n");
  return true;
}

bool cmCTestHG::UpdateImpl()
{
  // Use "hg pull" followed by "hg update" to update the working tree.
  {
    const char* hg = this->CommandLineTool.c_str();
    const char* hg_pull[] = { hg, "pull", "-v", nullptr };
    OutputLogger out(this->Log, "pull-out> ");
    OutputLogger err(this->Log, "pull-err> ");
    this->RunChild(&hg_pull[0], &out, &err);
  }

  // TODO: if(this->CTest->GetTestModel() == cmCTest::NIGHTLY)

  std::vector<char const*> hg_update;
  hg_update.push_back(this->CommandLineTool.c_str());
  hg_update.push_back("update");
  hg_update.push_back("-v");

  // Add user-specified update options.
  std::string opts = this->CTest->GetCTestConfiguration("UpdateOptions");
  if (opts.empty()) {
    opts = this->CTest->GetCTestConfiguration("HGUpdateOptions");
  }
  std::vector<std::string> args = cmSystemTools::ParseArguments(opts);
  for (std::string const& arg : args) {
    hg_update.push_back(arg.c_str());
  }

  // Sentinel argument.
  hg_update.push_back(nullptr);

  OutputLogger out(this->Log, "update-out> ");
  OutputLogger err(this->Log, "update-err> ");
  return this->RunUpdateCommand(hg_update.data(), &out, &err);
}

class cmCTestHG::LogParser
  : public cmCTestVC::OutputLogger
  , private cmXMLParser
{
public:
  LogParser(cmCTestHG* hg, const char* prefix)
    : OutputLogger(hg->Log, prefix)
    , HG(hg)
  {
    this->InitializeParser();
  }
  ~LogParser() override { this->CleanupParser(); }

private:
  cmCTestHG* HG;

  using Revision = cmCTestHG::Revision;
  using Change = cmCTestHG::Change;
  Revision Rev;
  std::vector<Change> Changes;
  Change CurChange;
  std::vector<char> CData;

  bool ProcessChunk(const char* data, int length) override
  {
    this->OutputLogger::ProcessChunk(data, length);
    this->ParseChunk(data, length);
    return true;
  }

  void StartElement(const std::string& name, const char** atts) override
  {
    this->CData.clear();
    if (name == "logentry") {
      this->Rev = Revision();
      if (const char* rev =
            cmCTestHG::LogParser::FindAttribute(atts, "revision")) {
        this->Rev.Rev = rev;
      }
      this->Changes.clear();
    }
  }

  void CharacterDataHandler(const char* data, int length) override
  {
    cm::append(this->CData, data, data + length);
  }

  void EndElement(const std::string& name) override
  {
    if (name == "logentry") {
      this->HG->DoRevision(this->Rev, this->Changes);
    } else if (!this->CData.empty() && name == "author") {
      this->Rev.Author.assign(this->CData.data(), this->CData.size());
    } else if (!this->CData.empty() && name == "email") {
      this->Rev.EMail.assign(this->CData.data(), this->CData.size());
    } else if (!this->CData.empty() && name == "date") {
      this->Rev.Date.assign(this->CData.data(), this->CData.size());
    } else if (!this->CData.empty() && name == "msg") {
      this->Rev.Log.assign(this->CData.data(), this->CData.size());
    } else if (!this->CData.empty() && name == "files") {
      std::vector<std::string> paths = this->SplitCData();
      for (std::string const& path : paths) {
        // Updated by default, will be modified using file_adds and
        // file_dels.
        this->CurChange = Change('U');
        this->CurChange.Path = path;
        this->Changes.push_back(this->CurChange);
      }
    } else if (!this->CData.empty() && name == "file_adds") {
      std::string added_paths(this->CData.begin(), this->CData.end());
      for (Change& change : this->Changes) {
        if (added_paths.find(change.Path) != std::string::npos) {
          change.Action = 'A';
        }
      }
    } else if (!this->CData.empty() && name == "file_dels") {
      std::string added_paths(this->CData.begin(), this->CData.end());
      for (Change& change : this->Changes) {
        if (added_paths.find(change.Path) != std::string::npos) {
          change.Action = 'D';
        }
      }
    }
    this->CData.clear();
  }

  std::vector<std::string> SplitCData()
  {
    std::vector<std::string> output;
    std::string currPath;
    for (char i : this->CData) {
      if (i != ' ') {
        currPath += i;
      } else {
        output.push_back(currPath);
        currPath.clear();
      }
    }
    output.push_back(currPath);
    return output;
  }

  void ReportError(int /*line*/, int /*column*/, const char* msg) override
  {
    this->HG->Log << "Error parsing hg log xml: " << msg << "\n";
  }
};

bool cmCTestHG::LoadRevisions()
{
  // Use 'hg log' to get revisions in a xml format.
  //
  // TODO: This should use plumbing or python code to be more precise.
  // The "list of strings" templates like {files} will not work when
  // the project has spaces in the path.  Also, they may not have
  // proper XML escapes.
  std::string range = this->OldRevision + ":" + this->NewRevision;
  const char* hg = this->CommandLineTool.c_str();
  const char* hgXMLTemplate = "<logentry\n"
                              "   revision=\"{node|short}\">\n"
                              "  <author>{author|person}</author>\n"
                              "  <email>{author|email}</email>\n"
                              "  <date>{date|isodate}</date>\n"
                              "  <msg>{desc}</msg>\n"
                              "  <files>{files}</files>\n"
                              "  <file_adds>{file_adds}</file_adds>\n"
                              "  <file_dels>{file_dels}</file_dels>\n"
                              "</logentry>\n";
  const char* hg_log[] = {
    hg,           "log",         "--removed", "-r", range.c_str(),
    "--template", hgXMLTemplate, nullptr
  };

  LogParser out(this, "log-out> ");
  out.Process("<?xml version=\"1.0\"?>\n"
              "<log>\n");
  OutputLogger err(this->Log, "log-err> ");
  this->RunChild(hg_log, &out, &err);
  out.Process("</log>\n");
  return true;
}

bool cmCTestHG::LoadModifications()
{
  // Use 'hg status' to get modified files.
  const char* hg = this->CommandLineTool.c_str();
  const char* hg_status[] = { hg, "status", nullptr };
  StatusParser out(this, "status-out> ");
  OutputLogger err(this->Log, "status-err> ");
  this->RunChild(hg_status, &out, &err);
  return true;
}
