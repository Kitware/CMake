#include "cmWin32LibrariesRule.h"
cmWin32LibrariesRule::cmWin32LibrariesRule()
{
#ifndef _WIN32
  this->EnabledOff();
#endif
}


// cmWin32LibrariesRule
bool cmWin32LibrariesRule::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 1 )
    {
    this->SetError("Called with incorrect number of arguments");
    return false;
    }
  for(std::vector<std::string>::iterator i = args.begin();
      i != args.end(); ++i)
    {
    m_Makefile->AddLinkLibrary((*i).c_str());
    }
  return true;
}

