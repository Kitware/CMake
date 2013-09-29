#include "cmStandardIncludes.h"
#include "cmSystemTools.h"
#include "cmXMLParser.h"
#include "cmParsePythonCoverage.h"
#include <cmsys/Directory.hxx>


//----------------------------------------------------------------------------
class cmParsePythonCoverage::XMLParser: public cmXMLParser
{
public:
  XMLParser(cmCTest* ctest, cmCTestCoverageHandlerContainer& cont)
    : CTest(ctest), Coverage(cont)
  {
    cmXMLParser::cmXMLParser();
  }

  virtual ~XMLParser()
  {
  }

protected:

  virtual void StartElement(const char* name, const char** atts)
  {
    if(strcmp(name, "class") == 0)
    {
      int tagCount = 0;
      while(true)
      {
        if(strcmp(atts[tagCount], "filename") == 0)
        {
          cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT, "Reading file: "
                     << atts[tagCount+1] << std::endl);
          this->CurFileName = this->Coverage.SourceDir + "/" +
                                 atts[tagCount+1];
          FileLinesType& curFileLines =
            this->Coverage.TotalCoverage[this->CurFileName];
          std::ifstream fin(this->CurFileName.c_str());
          if(!fin)
          {
            cmCTestLog(this->CTest, HANDLER_VERBOSE_OUTPUT,
                       "Python Coverage: Error opening " << this->CurFileName
                       << std::endl);
            break;
          }

          std::string line;
          curFileLines.push_back(-1);
          while(cmSystemTools::GetLineFromStream(fin, line))
          {
            curFileLines.push_back(-1);
          }

          break;
        }
        ++tagCount;
      }
    }
    else if(strcmp(name, "line") == 0)
    {
      int tagCount = 0;
      int curNumber = -1;
      int curHits = -1;
      while(true)
      {
        if(strcmp(atts[tagCount], "hits") == 0)
        {
          curHits = atoi(atts[tagCount+1]);
        }
        else if(strcmp(atts[tagCount], "number") == 0)
        {
          curNumber = atoi(atts[tagCount+1]);
        }

        if(curHits > -1 && curNumber > -1)
        {
          FileLinesType& curFileLines =
            this->Coverage.TotalCoverage[this->CurFileName];
          curFileLines[curNumber] = curHits;
          break;
        }
        ++tagCount;
      }
    }
  }

  virtual void EndElement(const char* name) {}

private:

  typedef cmCTestCoverageHandlerContainer::SingleFileCoverageVector
     FileLinesType;
  cmCTestCoverageHandlerContainer& Coverage;
  cmCTest* CTest;
  std::string CurFileName;

};


cmParsePythonCoverage::cmParsePythonCoverage(
    cmCTestCoverageHandlerContainer& cont,
    cmCTest* ctest)
    :Coverage(cont), CTest(ctest)
{
}

bool cmParsePythonCoverage::ReadCoverageXML(const char* xmlFile)
{
  cmParsePythonCoverage::XMLParser parser(this->CTest, this->Coverage);
  parser.ParseFile(xmlFile);
  return true;
}
