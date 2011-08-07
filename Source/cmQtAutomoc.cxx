#include "cmGlobalGenerator.h"
#include "cmLocalGenerator.h"
#include "cmMakefile.h"
#include "cmSystemTools.h"

#include "cmQtAutomoc.h"

cmQtAutomoc::cmQtAutomoc()
{
}


bool cmQtAutomoc::Run(const char* targetDirectory)
{
  cmake cm;
  cmGlobalGenerator* gg = this->CreateGlobalGenerator(&cm, targetDirectory);
  cmMakefile* makefile = gg->GetCurrentLocalGenerator()->GetMakefile();

  this->ReadAutomocInfoFile(makefile, targetDirectory);
  this->ReadOldMocDefinitionsFile(makefile, targetDirectory);

  delete gg;
  gg = NULL;
  makefile = NULL;

  if (this->QtMajorVersion == "4")
    {
    this->RunAutomocQt4();
    }

  this->WriteOldMocDefinitionsFile(targetDirectory);
}


cmGlobalGenerator* cmQtAutomoc::CreateGlobalGenerator(cmake* cm,
                                                  const char* targetDirectory)
{
  cmGlobalGenerator* gg = new cmGlobalGenerator();
  gg->SetCMakeInstance(cm);

  cmLocalGenerator* lg = gg->CreateLocalGenerator();
  lg->GetMakefile()->SetHomeOutputDirectory(targetDirectory);
  lg->GetMakefile()->SetStartOutputDirectory(targetDirectory);
  lg->GetMakefile()->SetHomeDirectory(targetDirectory);
  lg->GetMakefile()->SetStartDirectory(targetDirectory);
  gg->SetCurrentLocalGenerator(lg);

  return gg;
}


bool cmQtAutomoc::ReadAutomocInfoFile(cmMakefile* makefile,
                                      const char* targetDirectory)
{
  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocInfo.cmake";

  if (!makefile->ReadListFile(0, filename.c_str()))
    {
    cmSystemTools::Error("Error processing file:", filename.c_str());
    }
  return true;
}


bool cmQtAutomoc::ReadOldMocDefinitionsFile(cmMakefile* makefile,
                                            const char* targetDirectory)
{
  std::string filename(cmSystemTools::CollapseFullPath(targetDirectory));
  cmSystemTools::ConvertToUnixSlashes(filename);
  filename += "/AutomocOldMocDefinitions.cmake";

  if (!makefile->ReadListFile(0, filename.c_str()))
    {
    cmSystemTools::Error("Error processing file:", filename.c_str());
    }
  return true;
}


bool cmQtAutomoc::RunAutomocQt4()
{
  return true;
}


void cmQtAutomoc::WriteOldMocDefinitionsFile(const char* targetDirectory)
{
}
