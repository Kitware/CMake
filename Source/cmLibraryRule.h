#ifndef cmLibraryRule_h
#define cmLibraryRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmLibraryRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmLibraryRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  
  // This is the name used in the input file.
  virtual const char* GetName() { return "LIBRARY";}
  virtual const char* TerseDocumentaion() 
    {
      return "Set a name for the Library.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentaion()
    {
      return
        "LIBRARY(libraryname);";
    }
};



#endif
