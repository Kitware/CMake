#include "cmDSWBuilder.h"
#include "cmDSWMakefile.h"

cmDSWBuilder::~cmDSWBuilder()
{
  delete m_Makefile;
}

cmDSWBuilder::cmDSWBuilder()
{
  m_Makefile = new cmDSWMakefile;
}

cmMakefile* cmDSWBuilder::GetMakefile()
{
  return m_Makefile;
}

void cmDSWBuilder::CreateDSWFile()
{
  m_Makefile->OutputDSWFile();
}
