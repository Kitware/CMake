#include "cmAddTargetRule.h"

// cmAddTargetRule
bool cmAddTargetRule::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  return false;
  
//    for(std::vector<std::string>::iterator i = args.begin();
//        i != args.end(); ++i)
//      {
//      m_Makefile->Add((*i).c_str());
//      }
}

