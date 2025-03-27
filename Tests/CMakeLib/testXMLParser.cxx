#include <iostream>

#include "cmXMLParser.h"

#include "testConfig.h"

int testXMLParser(int /*unused*/, char* /*unused*/[])
{
  // TODO: Derive from parser and check attributes.
  cmXMLParser parser;
  if (!parser.ParseFile(SOURCE_DIR "/testXMLParser.xml")) {
    std::cerr << "cmXMLParser failed!" << std::endl;
    return 1;
  }
  return 0;
}
