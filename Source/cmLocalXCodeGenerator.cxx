#include "cmLocalXCodeGenerator.h"

cmLocalXCodeGenerator::cmLocalXCodeGenerator()
{
  // the global generator does this, so do not
  // put these flags into the language flags
  this->EmitUniversalBinaryFlags = false;
}

cmLocalXCodeGenerator::~cmLocalXCodeGenerator()
{
}

