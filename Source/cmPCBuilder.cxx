#include "cmPCBuilder.h"
#include "cmMakefile.h"


cmPCBuilder::cmPCBuilder()
{
}
 
// Delete the m_Makefile
cmPCBuilder::~cmPCBuilder()
{
}

// Read in the given makefile
void cmPCBuilder::SetInputMakefilePath(const char* mfile)
{
  if(!GetMakefile()->ReadMakefile(mfile))
    {
    std::cerr << "Error can not open " << mfile << " for input " << std::endl;
    abort();
    }
}


void cmPCBuilder::SetHomeDirectory(const char* dir)
{
  GetMakefile()->SetHomeDirectory(dir);
} 

void cmPCBuilder::SetMakefileDirectory(const char* dir)
{
  GetMakefile()->SetCurrentDirectory(dir);
}



void cmPCBuilder::SetOutputDirectory(const char* dir)
{
  GetMakefile()->SetOutputDirectory(dir);
}

void cmPCBuilder::SetOutputHomeDirectory(const char* dir)
{
  GetMakefile()->SetOutputHomeDirectory(dir);
}

