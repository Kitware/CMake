#include "cmMSProjectGenerator.h"
#include "cmDSWMakefile.h"
#include "cmDSPMakefile.h"

cmMSProjectGenerator::cmMSProjectGenerator()
{
  m_DSWMakefile = 0;
  m_DSPMakefile = 0;
  SetBuildDSW();
}

void cmMSProjectGenerator::GenerateMakefile()
{
  if(m_BuildDSW)
    {
    m_DSWMakefile = new cmDSWMakefile(m_Makefile);
    m_DSWMakefile->OutputDSWFile();
    }
  else
    {
    m_DSPMakefile = new cmDSPMakefile(m_Makefile);
    m_DSPMakefile->OutputDSPFile();
    }
}

cmMSProjectGenerator::~cmMSProjectGenerator()
{
  delete m_DSPMakefile;
  delete m_DSWMakefile;
}

