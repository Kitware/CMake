#include "cmProjectRule.h"

// cmProjectRule
bool cmProjectRule::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 || args.size() > 1)
    {
    this->SetError("PROJECT called with incorrect number of arguments");
    return false;
    }
  m_Makefile->SetProjectName(args[0].c_str());
  return true;
}

