#include "cmCPluginAPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct 
{
  char *LibraryName;
} cmVTKWrapTclData;


/* do almost everything in the initial pass */
static int InitialPass(void *inf, void *mf, int argc, char *argv[])
{
  cmLoadedCommandInfo *info = (cmLoadedCommandInfo *)inf;

  cmVTKWrapTclData *cdata = 
    (cmVTKWrapTclData *)malloc(sizeof(cmVTKWrapTclData));
  cdata->LibraryName = "BOO";
  info->CAPI->SetClientData(info,cdata);
  
  /* Now check and see if the value has been stored in the cache */
  /* already, if so use that value and don't look for the program */
  if(!info->CAPI->IsOn(mf,"TEST_COMMAND_TEST1"))
    {
    info->CAPI->AddDefinition(mf, "TEST_DEF", "HOO");  
    return 1;
    }
  
  info->CAPI->AddDefinition(mf, "TEST_DEF", "HOO");  
  cdata->LibraryName = "HOO";
  return 1;
}

static void FinalPass(void *inf, void *mf) 
{
  cmLoadedCommandInfo *info = (cmLoadedCommandInfo *)inf;
  /* get our client data from initial pass */
  cmVTKWrapTclData *cdata = 
    (cmVTKWrapTclData *)info->CAPI->GetClientData(info);
  if (strcmp(info->CAPI->GetDefinition(mf, "TEST_DEF"),"HOO") ||
      strcmp(cdata->LibraryName,"HOO"))
    {
    fprintf(stderr,"*** Failed LOADED COMMAND Final Pass\n");
    }
}

static void Destructor(void *inf) 
{
  cmLoadedCommandInfo *info = (cmLoadedCommandInfo *)inf;
  /* get our client data from initial pass */
  cmVTKWrapTclData *cdata = 
    (cmVTKWrapTclData *)info->CAPI->GetClientData(info);
  free(cdata);
}

#ifdef MUCHO_MUDSLIDE
void CM_PLUGIN_EXPORT CMAKE_TEST_COMMANDInit(cmLoadedCommandInfo *info)
{
  info->InitialPass = InitialPass;
  info->FinalPass = FinalPass;
  info->Destructor = Destructor;
  info->m_Inherited = 0;
  info->Name = "CMAKE_TEST_COMMAND";
}
#endif




