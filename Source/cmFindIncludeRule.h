#ifndef cmFindIncludeRule_h
#define cmFindIncludeRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmFindIncludeRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmFindIncludeRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  virtual bool IsInherited() { return true;  }

  // This is the name used in the input file.
  virtual const char* GetName() { return "FIND_INCLUDE";}
  virtual const char* TerseDocumentaion() 
    {
      return "Find an include path.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentaion()
    {
      return
        "FIND_INCLUDE(DEFINE try1 try2 ...);";
    }
};



#endif
