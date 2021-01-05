/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cmXMLParser.h"

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
  char test_file[] = "libfuzzer.xml";

  FILE* fp = fopen(test_file, "wb");
  if (!fp)
    return 0;
  fwrite(data, size, 1, fp);
  fclose(fp);

  cmXMLParser parser;
  if (!parser.ParseFile(test_file)) {
    return 1;
  }

  remove(test_file);
  return 0;
}
