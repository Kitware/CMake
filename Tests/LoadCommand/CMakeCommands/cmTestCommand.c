#include "cmCPluginAPI.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct 
{
  char *LibraryName;
} cmVTKWrapTclData;


/* do almost everything in the initial pass */
int InitialPass(void *inf, void *mf, int argc, char *argv[])
{
  cmLoadedCommandInfo *info = (cmLoadedCommandInfo *)inf;

  cmVTKWrapTclData *cdata = 
    (cmVTKWrapTclData *)malloc(sizeof(cmVTKWrapTclData));
  cdata->LibraryName = "BOO";
  info->CAPI->SetClientData(info,cdata);
  
  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  if(!info->CAPI->IsOn(mf,"TEST_COMMAND_TEST1"))
    {
    info->CAPI->AddDefinition(mf, "TEST_DEF", "HOO");  
    return 1;
    }
  
  info->CAPI->AddDefinition(mf, "TEST_DEF", "HOO");  
  cdata->LibraryName = "HOO";
  return 1;
}

void FinalPass(void *inf, void *mf) 
{
  cmLoadedCommandInfo *info = (cmLoadedCommandInfo *)inf;
  // get our client data from initial pass
  cmVTKWrapTclData *cdata = 
    (cmVTKWrapTclData *)info->CAPI->GetClientData(info);
  if (strcmp(info->CAPI->GetDefinition(mf, "TEST_DEF"),"HOO") ||
      strcmp(cdata->LibraryName,"HOO"))
    {
    fprintf(stderr,"*** Failed LOADED COMMAND Final Pass\n");
    }
}

CM_PLUGIN_EXPORT const char *cmGetName()
{
  return "CMAKE_TEST_COMMAND";
}

void CM_PLUGIN_EXPORT cmInitializeCommand(cmLoadedCommandInfo *info)
{
  info->InitialPass = InitialPass;
  info->FinalPass = FinalPass;
  info->m_Inherited = 0;
}




