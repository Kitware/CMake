#ifndef cmLinkDirectoriesRule_h
#define cmLinkDirectoriesRule_h

#include "cmStandardIncludes.h"
#include "cmRuleMaker.h"


class cmLinkDirectoriesRule : public cmRuleMaker
{
public:
  virtual cmRuleMaker* Clone() 
    {
      return new cmLinkDirectoriesRule;
    }
  // This is called when the rule is firt encountered in
  // the input file
  virtual bool Invoke(std::vector<std::string>& args);
  virtual void FinalPass() { }
  virtual bool IsInherited() { return true;  }

  
  // This is the name used in the input file.
  virtual const char* GetName() { return "LINK_DIRECTORIES";}
  virtual const char* TerseDocumentaion() 
    {
      return "Specify link directories.";
    }
  
  // Return full documentation for the rule
  virtual const char* FullDocumentaion()
    {
      return
        "Specify the paths to the libraries that will be linked in.\n"
        "LINK_DIRECTORIES(directory1 directory2 ...);\n"
        "The directories can use built in definitions like \n"
        "CMAKE_BINARY_DIR and CMAKE_SOURCE_DIR.";
    }
};



#endif
