#ifdef _MSC_VER
#pragma warning ( disable : 4786 )
#endif
#include "cmDSPBuilder.h"
#include "cmDSPMakefile.h"

cmDSPBuilder::~cmDSPBuilder()
{
  delete m_Makefile;
}

cmDSPBuilder::cmDSPBuilder()
{
  m_Makefile = new cmDSPMakefile;
}

cmMakefile* cmDSPBuilder::GetMakefile()
{
  return m_Makefile;
}


void cmDSPBuilder::CreateDSPFile()
{
  m_Makefile->OutputDSPFile();
}

std::vector<std::string> cmDSPBuilder::GetCreatedProjectNames()
{
  return m_Makefile->GetCreatedProjectNames();
}
