#ifndef cmAddTargetRule_h
#define cmAddTargetRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmAddTargetRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmAddTargetRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  
  // This is the name used in the input file.
  virtual const char* GetName() { return "ADD_TARGET";}
  virtual const char* TerseDocumentaion() 
    {
      return "Add an extra target to the build system.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentaion()
    {
      return
        "ADD_TARGET(Name \"command to run\");";
    }
};



#endif
