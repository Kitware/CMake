/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) 2000 National Library of Medicine
  All rights reserved.

  See COPYRIGHT.txt for copyright details.

=========================================================================*/
#include "cmWrapTclCommand.h"

// cmWrapTclCommand
bool cmWrapTclCommand::Invoke(std::vector<std::string>& args)
{
  if(args.size() < 0 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // get the list of classes for this library
  std::vector<cmClassFile> &classes = m_Makefile->GetClasses();
  
  // add in new classes for the wrappers
  int lastClass = classes.size();
  // what is the current source dir
  std::string cdir = m_Makefile->GetCurrentDirectory();
  
  for(int classNum = 0; classNum < lastClass; classNum++)
    {
    cmClassFile &curr = classes[classNum];
    cmClassFile file;
    file.m_AbstractClass = curr.m_AbstractClass;
    std::string newName = curr.m_ClassName + "Tcl";
    file.SetName(newName.c_str(), m_Makefile->GetCurrentOutputDirectory(),
                 "cxx",false);
    m_WrapClasses.push_back(file);
    std::string hname = cdir + "/" + curr.m_ClassName + ".h";
    m_WrapHeaders.push_back(hname);
    }
  
  return true;
}

void cmWrapTclCommand::FinalPass() 
{
  // first we add the rules for all the .h to Tcl.cxx files
  int lastClass = m_WrapClasses.size();
  std::vector<std::string> depends;
  std::string wtcl = "${WRAP_TCL_EXE}";
  std::string hints = "${WRAP_HINTS}";
  std::string wti = "${WRAP_TCL_INIT_EXE}";
  
  m_Makefile->ExpandVariablesInString(wtcl);
  m_Makefile->ExpandVariablesInString(hints);
  m_Makefile->ExpandVariablesInString(wti);
  
  depends.push_back(wti);

  // Create the init file 
  std::string res = m_Makefile->GetLibraryName();
  res += "Init.cxx";
  this->CreateInitFile(res);
  
  // add the init file
  cmClassFile cfile;
  cfile.m_AbstractClass = false;
  std::string newName = m_Makefile->GetLibraryName();
  newName += "Init";
  cfile.SetName(newName.c_str(), m_Makefile->GetCurrentOutputDirectory(),
                "cxx",false);
  m_Makefile->AddClass(cfile);
  
  // wrap all the .h files
  depends.push_back(wtcl);
  for(int classNum = 0; classNum < lastClass; classNum++)
    {
    m_Makefile->AddClass(m_WrapClasses[classNum]);
    std::string res = m_WrapClasses[classNum].m_ClassName + ".cxx";
    std::string cmd = wtcl + " " + m_WrapHeaders[classNum] + " "
		+ hints + (m_WrapClasses[classNum].m_AbstractClass ? " 0 " : " 1 ") + " > " + m_WrapClasses[classNum].m_ClassName + ".cxx";
    m_Makefile->AddCustomCommand(m_WrapHeaders[classNum].c_str(), 
                                 res.c_str(), 
                                 cmd.c_str(), depends);
    }
  
}

char *names[1000];
int anindex = 0;

bool cmWrapTclCommand::CreateInitFile(std::string& res) 
{
  int i;
  
  /* we have to make sure that the name is the correct case */
  std::string kitName = m_Makefile->GetLibraryName();
  if (kitName[0] > 90) kitName[0] -= 32;
  for (i = 1; i < kitName.size(); i++)
    {
    if ((kitName[i] > 64)&&(kitName[i] < 91))
      {
      kitName[i] += 32;
      }
    }
  
  std::vector<std::string> classes;
  int lastClass = m_WrapHeaders.size();
  int classNum;
  for(classNum = 0; classNum < lastClass; classNum++)
    {
    std::string cls = m_WrapHeaders[classNum];
    cls = cls.substr(0,cls.size()-2);
    std::string::size_type pos = cls.rfind('/');    
    if(pos != std::string::npos)
      {
      cls = cls.substr(pos+1);
      }
    classes.push_back(cls);
    }
  
  // open the init file
  std::string outFileName = 
    m_Makefile->GetCurrentOutputDirectory();
  outFileName += "/" + res;
  
  return this->WriteInit(kitName.c_str(), outFileName, classes);
}


/* warning this code is also in getclasses.cxx under pcmaker */
bool cmWrapTclCommand::WriteInit(const char *kitName, std::string& outFileName,
                                 std::vector<std::string>& classes)
{
  int i;
  FILE *fout = fopen(outFileName.c_str(),"w");
  if (!fout)
    {
    return false;
    }
  
  fprintf(fout,"#include \"vtkTclUtil.h\"\n");
  
  for (i = 0; i < classes.size(); i++)
    {
    fprintf(fout,"int %sCommand(ClientData cd, Tcl_Interp *interp,\n             int argc, char *argv[]);\n",classes[i].c_str());
    fprintf(fout,"ClientData %sNewCommand();\n",classes[i].c_str());
    }
  
  if (!strcmp(kitName,"Vtkcommon"))
    {
    fprintf(fout,"int vtkCommand(ClientData cd, Tcl_Interp *interp,\n             int argc, char *argv[]);\n");
    fprintf(fout,"\nTcl_HashTable vtkInstanceLookup;\n");
    fprintf(fout,"Tcl_HashTable vtkPointerLookup;\n");
    fprintf(fout,"Tcl_HashTable vtkCommandLookup;\n");
    }
  else
    {
    fprintf(fout,"\nextern Tcl_HashTable vtkInstanceLookup;\n");
    fprintf(fout,"extern Tcl_HashTable vtkPointerLookup;\n");
    fprintf(fout,"extern Tcl_HashTable vtkCommandLookup;\n");
    }
  fprintf(fout,"extern void vtkTclDeleteObjectFromHash(void *);\n");  
  fprintf(fout,"extern void vtkTclListInstances(Tcl_Interp *interp, ClientData arg);\n");
  
  fprintf(fout,"\n\nextern \"C\" {int VTK_EXPORT %s_SafeInit(Tcl_Interp *interp);}\n\n",
	  kitName);
  fprintf(fout,"\n\nextern \"C\" {int VTK_EXPORT %s_Init(Tcl_Interp *interp);}\n\n",
	  kitName);
  
  /* create an extern ref to the generic delete function */
  fprintf(fout,"\n\nextern void vtkTclGenericDeleteObject(ClientData cd);\n\n");

  /* the main declaration */
  fprintf(fout,"\n\nint VTK_EXPORT %s_SafeInit(Tcl_Interp *interp)\n{\n",kitName);
  fprintf(fout,"  return %s_Init(interp);\n}\n",kitName);
  
  fprintf(fout,"\n\nint VTK_EXPORT %s_Init(Tcl_Interp *interp)\n{\n",
          kitName);
  if (!strcmp(kitName,"Vtkcommontcl"))
    {
    fprintf(fout,
	    "  vtkTclInterpStruct *info = new vtkTclInterpStruct;\n");
    fprintf(fout,
            "  info->Number = 0; info->InDelete = 0; info->DebugOn = 0;\n");
    fprintf(fout,"\n");
    fprintf(fout,"\n");
    fprintf(fout,
	    "  Tcl_InitHashTable(&info->InstanceLookup, TCL_STRING_KEYS);\n");
    fprintf(fout,
	    "  Tcl_InitHashTable(&info->PointerLookup, TCL_STRING_KEYS);\n");
    fprintf(fout,
	    "  Tcl_InitHashTable(&info->CommandLookup, TCL_STRING_KEYS);\n");
    fprintf(fout,
            "  Tcl_SetAssocData(interp,(char *) \"vtk\",NULL,(ClientData *)info);\n");

    /* create special vtkCommand command */
    fprintf(fout,"  Tcl_CreateCommand(interp,(char *) \"vtkCommand\",vtkCommand,\n		    (ClientData *)NULL, NULL);\n\n");
    }
  
  for (i = 0; i < classes.size(); i++)
    {
    fprintf(fout,"  vtkTclCreateNew(interp,(char *) \"%s\", %sNewCommand,\n",
	    classes[i].c_str(), classes[i].c_str());
    fprintf(fout,"                  %sCommand);\n",classes[i].c_str());
    }
  
  fprintf(fout,"  return TCL_OK;\n}\n");
  fclose(fout);

  return true;
}


