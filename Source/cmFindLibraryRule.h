#ifndef cmFindLibraryRule_h
#define cmFindLibraryRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmFindLibraryRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmFindLibraryRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  virtual bool IsInherited() { return true;  }

  // This is the name used in the input file.
  virtual const char* GetName() { return "FIND_LIBRARY";}
  virtual const char* TerseDocumentaion() 
    {
      return "Set a name for the entire project. One argument.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentaion()
    {
      return
        "FIND_LIBRARY(DEFINE try1 try2);";
    }
};



#endif
