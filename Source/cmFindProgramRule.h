#ifndef cmFindProgramRule_h
#define cmFindProgramRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmFindProgramRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmFindProgramRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  virtual bool IsInherited() { return true;  }

  // This is the name used in the input file.
  virtual const char* GetName() { return "FIND_PROGRARM";}
  virtual const char* TerseDocumentaion() 
    {
      return "not implemented.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentaion()
    {
      return
        "not implemented.\n"
        "FIND_PROGRARM(NAME try1 try2 ...);";
    }
};



#endif
