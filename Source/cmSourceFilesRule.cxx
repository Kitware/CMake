#include "cmSourceFilesRule.h"

// cmSourceFilesRule
bool cmSourceFilesRule::Invoke(std::vector<std::string>& args)
{
 if(args.size() < 1 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  for(std::vector<std::string>::iterator i = args.begin();
      i != args.end(); ++i)
    {
    cmClassFile file;
    file.m_AbstractClass = false;
    file.SetName((*i).c_str(), m_Makefile->GetCurrentDirectory());
    m_Makefile->AddClass(file);
    }
  return true;
}

