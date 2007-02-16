#include "cmLocalXCodeGenerator.h"
#include "cmGlobalXCodeGenerator.h"
#include "cmSourceFile.h"

cmLocalXCodeGenerator::cmLocalXCodeGenerator()
{
  // the global generator does this, so do not
  // put these flags into the language flags
  this->EmitUniversalBinaryFlags = false;
}

cmLocalXCodeGenerator::~cmLocalXCodeGenerator()
{
}

void cmLocalXCodeGenerator::
GetTargetObjectFileDirectories(cmTarget* target,
                               std::vector<std::string>& 
                               dirs)
{
  cmGlobalXCodeGenerator* g = (cmGlobalXCodeGenerator*)this->GetGlobalGenerator();
  g->SetCurrentLocalGenerator(this);
  g->GetTargetObjectFileDirectories(target,
                                    dirs);
}
  
  // return the source name for the object file
std::string cmLocalXCodeGenerator::GetSourceObjectName(cmSourceFile& sf )
{
  std::string ret = sf.GetSourceName();
  std::string::size_type pos = ret.find("/");
  if(pos == ret.npos)
    {
    return ret;
    }
  return ret.substr(pos+1);
}
  
