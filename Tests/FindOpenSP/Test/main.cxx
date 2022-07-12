#include <cassert>
#include <string>

#include "ParserEventGeneratorKit.h"

std::string CharStringtostring(const SGMLApplication::CharString source)
{
  // The CharString type might have multi-byte characters if SP_MULTI_BYTE was
  // defined
  std::string result;
  result.resize(source.len);
  for (size_t i = 0; i < source.len; i++) {
    result[i] = static_cast<char>(source.ptr[i]);
  }
  return result;
}

class OutlineApplication : public SGMLApplication
{
public:
  OutlineApplication()
    : depth_(0)
  {
  }
  void startElement(const StartElementEvent& event)
  {
    for (unsigned i = 0; i < depth_; i++)
      parsedOutput += "\t";
    parsedOutput += CharStringtostring(event.gi);
    depth_++;
  }
  void endElement(const EndElementEvent&) { depth_--; }
  std::string parsedOutput;

private:
  unsigned depth_;
};

int main()
{
  std::string expectedOutput = "TESTDOC\tTESTELEMENT";
  char file_name[] = "test.sgml";
  char* files[] = { file_name, 0 };

  ParserEventGeneratorKit parserKit;
  EventGenerator* egp = parserKit.makeEventGenerator(1, files);
  OutlineApplication app;
  unsigned nErrors = egp->run(app);

  assert(nErrors == 0);
  assert(app.parsedOutput.compare(expectedOutput) == 0);

  delete egp;
  return 0;
}
