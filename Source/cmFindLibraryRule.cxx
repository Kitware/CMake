#include "cmFindLibraryRule.h"

// cmFindLibraryRule
bool cmFindLibraryRule::Invoke(std::vector<std::string>& args)
{
  return false;
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  for(std::vector<std::string>::iterator i = args.begin();
      i != args.end(); ++i)
    {
    m_Makefile->AddDefineFlag((*i).c_str());
    }
}

