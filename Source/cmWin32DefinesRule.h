#ifndef cmWin32DefinesRule_h
#define cmWin32DefinesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmWin32DefinesRule : public cmRuleMaker
{
public:
  cmWin32DefinesRule();
  virtual cmRuleMaker* Clone() 
    {
      return new cmWin32DefinesRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  virtual bool IsInherited() { return true;  }
  // This is the name used in the input file.
  virtual const char* GetName() { return "WIN32_DEFINES";}
  virtual const char* TerseDocumentation() 
    {
      return "Add -D define flags to command line for win32 environments.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentation()
    {
      return
        "Add -D define flags to command line for win32 environments.\n"
        "WIN32_DEFINES(-DFOO -DBAR ...);";
    }
};



#endif
