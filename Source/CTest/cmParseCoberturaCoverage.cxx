#include "cmParseCoberturaCoverage.h"

#include <cstdlib>
#include <cstring>

#include "cmsys/FStream.hxx"

#include "cmCTest.h"
#include "cmCTestCoverageHandler.h"
#include "cmStringAlgorithms.h"
#include "cmSystemTools.h"
#include "cmXMLParser.h"

class cmParseCoberturaCoverage::XMLParser : public cmXMLParser
{
public:
  XMLParser(cmCTest* ctest, cmCTestCoverageHandlerContainer& cont)
    : FilePaths{ cont.SourceDir, cont.BinaryDir }
    , CTest(ctest)
    , Coverage(cont)
  {
  }

protected:
  void EndElement(const std::string& name) override
  {
    if (name == "source") {
      this->InSource = false;
    } else if (name == "sources") {
      this->InSources = false;
    } else if (name == "class") {
      this->SkipThisClass = false;
    }
  }

  void CharacterDataHandler(const char* data, int length) override
  {
    std::string tmp;
    tmp.insert(0, data, length);
    if (this->InSources && this->InSource) {
      this->FilePaths.push_back(tmp);
      cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                         "Adding Source: " << tmp << std::endl,
                         this->Coverage.Quiet);
    }
  }

  void StartElement(const std::string& name, const char** atts) override
  {
    std::string FoundSource;
    std::string finalpath;
    if (name == "source") {
      this->InSource = true;
    } else if (name == "sources") {
      this->InSources = true;
    } else if (name == "class") {
      int tagCount = 0;
      while (true) {
        if (strcmp(atts[tagCount], "filename") == 0) {
          cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                             "Reading file: " << atts[tagCount + 1]
                                              << std::endl,
                             this->Coverage.Quiet);
          std::string filename = atts[tagCount + 1];
          this->CurFileName.clear();

          // Check if this is an absolute path that falls within our
          // source or binary directories.
          for (std::string const& filePath : FilePaths) {
            if (filename.find(filePath) == 0) {
              this->CurFileName = filename;
              break;
            }
          }

          if (this->CurFileName.empty()) {
            // Check if this is a path that is relative to our source or
            // binary directories.
            for (std::string const& filePath : FilePaths) {
              finalpath = cmStrCat(filePath, "/", filename);
              if (cmSystemTools::FileExists(finalpath)) {
                this->CurFileName = finalpath;
                break;
              }
            }
          }

          cmsys::ifstream fin(this->CurFileName.c_str());
          if (this->CurFileName.empty() || !fin) {
            this->CurFileName =
              cmStrCat(this->Coverage.BinaryDir, "/", atts[tagCount + 1]);
            fin.open(this->CurFileName.c_str());
            if (!fin) {
              cmCTestOptionalLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                                 "Skipping system file " << filename
                                                         << std::endl,
                                 this->Coverage.Quiet);

              this->SkipThisClass = true;
              break;
            }
          }
          std::string line;
          FileLinesType& curFileLines =
            this->Coverage.TotalCoverage[this->CurFileName];
          while (cmSystemTools::GetLineFromStream(fin, line)) {
            curFileLines.push_back(-1);
          }

          break;
        }
        ++tagCount;
      }
    } else if (name == "line") {
      int tagCount = 0;
      int curNumber = -1;
      int curHits = -1;
      while (true) {
        if (this->SkipThisClass) {
          break;
        }
        if (strcmp(atts[tagCount], "hits") == 0) {
          curHits = atoi(atts[tagCount + 1]);
        } else if (strcmp(atts[tagCount], "number") == 0) {
          curNumber = atoi(atts[tagCount + 1]);
        }

        if (curHits > -1 && curNumber > 0) {
          FileLinesType& curFileLines =
            this->Coverage.TotalCoverage[this->CurFileName];
          {
            curFileLines[curNumber - 1] = curHits;
          }
          break;
        }
        ++tagCount;
      }
    }
  }

private:
  bool InSources = false;
  bool InSource = false;
  bool SkipThisClass = false;
  std::vector<std::string> FilePaths;
  using FileLinesType =
    cmCTestCoverageHandlerContainer::SingleFileCoverageVector;
  cmCTest* CTest;
  cmCTestCoverageHandlerContainer& Coverage;
  std::string CurFileName;
};

cmParseCoberturaCoverage::cmParseCoberturaCoverage(
  cmCTestCoverageHandlerContainer& cont, cmCTest* ctest)
  : Coverage(cont)
  , CTest(ctest)
{
}

bool cmParseCoberturaCoverage::ReadCoverageXML(const char* xmlFile)
{
  cmParseCoberturaCoverage::XMLParser parser(this->CTest, this->Coverage);
  parser.ParseFile(xmlFile);
  return true;
}
