#include "cmLinkDirectoriesRule.h"

// cmLinkDirectoriesRule
bool cmLinkDirectoriesRule::Invoke(std::vector<std::string>& args)
{
 if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  for(std::vector<std::string>::iterator i = args.begin();
      i != args.end(); ++i)
    {
    m_Makefile->AddLinkDirectory((*i).c_str());
    }
  return true;
}

