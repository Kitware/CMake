#ifndef cmUnixDefinesRule_h
#define cmUnixDefinesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmUnixDefinesRule : public cmRuleMaker
{
public:
  cmUnixDefinesRule();
  virtual cmRuleMaker* Clone() 
    {
      return new cmUnixDefinesRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  virtual bool IsInherited() { return true;  }

  // This is the name used in the input file.
  virtual const char* GetName() { return "UNIX_DEFINES";}
  virtual const char* TerseDocumentation() 
    {
      return "Add -D flags to the command line for unix only.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentation()
    {
      return
        "Add -D flags to the command line for unix only.\n"
        "UNIX_DEFINES(-DFOO -DBAR);";
    }
};



#endif
