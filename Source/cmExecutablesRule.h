#ifndef cmExecutablesRule_h
#define cmExecutablesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmExecutablesRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmExecutablesRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }

  // This is the name used in the input file.
  virtual const char* GetName() { return "EXECUTABLES";}
  virtual const char* TerseDocumentation() 
    {
      return "Add a list of executables files.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentation()
    {
      return
        "EXECUTABLES(file1 file2 ...)";
    }
};



#endif
