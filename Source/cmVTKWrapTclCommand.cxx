/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) 2001 Insight Consortium
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

 * Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

 * Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

 * The name of the Insight Consortium, nor the names of any consortium members,
   nor of any contributors, may be used to endorse or promote products derived
   from this software without specific prior written permission.

  * Modified source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER AND CONTRIBUTORS ``AS IS''
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

=========================================================================*/
#include "cmVTKWrapTclCommand.h"

// cmVTKWrapTclCommand
bool cmVTKWrapTclCommand::InitialPass(std::vector<std::string>& args)
{
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // keep the library name
  m_LibraryName = args[0];

  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  if(!m_Makefile->IsOn("VTK_WRAP_TCL"))
    {
    return true;
    }

  // extract the sources and commands parameters
  std::vector<std::string> sources;
  bool doing_sources = true;

  for(std::vector<std::string>::iterator j = (args.begin() + 1);
      j != args.end(); ++j)
    {   
    if(*j == "SOURCES")
      {
      doing_sources = true;
      }
    else if (*j == "COMMANDS")
      {
      doing_sources = false;
      }
    else
      { 
      if(doing_sources)
        {
        sources.push_back(*j);
        }
      else
        {
        m_Commands.push_back(*j);
        }
      }
    }

  // get the list of classes for this library
  if (sources.size())
    {
    // what is the current source dir
    std::string cdir = m_Makefile->GetCurrentDirectory();

    // get the resulting source list name
    m_SourceList = sources[0];

    cmMakefile::SourceMap &Classes = m_Makefile->GetSources();
    for(std::vector<std::string>::iterator j = (sources.begin() + 1);
        j != sources.end(); ++j)
      {   
      cmMakefile::SourceMap::iterator l = Classes.find(*j);
      if (l == Classes.end())
	{
	this->SetError("bad source list passed to VTKWrapTclCommand");
	return false;
	}
      for(std::vector<cmSourceFile>::iterator i = l->second.begin(); 
          i != l->second.end(); i++)
        {
        cmSourceFile &curr = *i;
        // if we should wrap the class
        if (!curr.GetWrapExclude())
          {
          cmSourceFile file;
          file.SetIsAnAbstractClass(curr.IsAnAbstractClass());
          std::string newName = curr.GetSourceName() + "Tcl";
          file.SetName(newName.c_str(), m_Makefile->GetCurrentOutputDirectory(),
                       "cxx",false);
          std::string hname = cdir + "/" + curr.GetSourceName() + ".h";
          m_WrapHeaders.push_back(hname);
          // add starting depends
          file.GetDepends().push_back(hname);
          m_WrapClasses.push_back(file);
          }
        }
      }
    }
  
  return true;
}

void cmVTKWrapTclCommand::FinalPass() 
{
  // first we add the rules for all the .h to Tcl.cxx files
  int lastClass = m_WrapClasses.size();
  std::vector<std::string> depends;
  std::string wtcl = "${VTK_WRAP_TCL_EXE}";
  std::string hints = "${VTK_WRAP_HINTS}";
  
  m_Makefile->ExpandVariablesInString(hints);

  // Create the init file 
  std::string res = m_LibraryName;
  res += "Init.cxx";
  this->CreateInitFile(res);
  
  // add the init file
  cmSourceFile cfile;
  cfile.SetIsAnAbstractClass(false);
  std::string newName = m_LibraryName;
  newName += "Init";
  cfile.SetName(newName.c_str(), m_Makefile->GetCurrentOutputDirectory(),
                "cxx",false);
  m_Makefile->AddSource(cfile,m_SourceList.c_str());
  
  // wrap all the .h files
  depends.push_back(wtcl);
  depends.push_back(hints);
  for(int classNum = 0; classNum < lastClass; classNum++)
    {
    m_Makefile->AddSource(m_WrapClasses[classNum],m_SourceList.c_str());
    std::string res = m_WrapClasses[classNum].GetSourceName() + ".cxx";
    std::vector<std::string> args;
    args.push_back(m_WrapHeaders[classNum]);
    args.push_back(hints);
    args.push_back((m_WrapClasses[classNum].IsAnAbstractClass() ? "0" : "1"));
    args.push_back(">");
    std::string tmp = m_Makefile->GetCurrentOutputDirectory();
    tmp += "/";
    tmp += m_WrapClasses[classNum].GetSourceName() + ".cxx";
    args.push_back(tmp);
    
    m_Makefile->AddCustomCommand(m_WrapHeaders[classNum].c_str(),
                                 wtcl.c_str(), args, depends, 
                                 res.c_str(), m_LibraryName.c_str());
    }
  
}

bool cmVTKWrapTclCommand::CreateInitFile(std::string& res) 
{
  /* we have to make sure that the name is the correct case */
  std::string kitName = cmSystemTools::Capitalized(m_LibraryName);
  
  std::vector<std::string> classes;
  int lastClass = m_WrapHeaders.size();
  int classNum;
  for(classNum = 0; classNum < lastClass; classNum++)
    {
    if (!m_WrapClasses[classNum].IsAnAbstractClass())
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
    }
  
  // open the init file
  std::string outFileName = 
    m_Makefile->GetCurrentOutputDirectory();
  outFileName += "/" + res;
  
  return this->WriteInit(kitName.c_str(), outFileName, classes);
}


/* warning this code is also in getclasses.cxx under pcmaker */
bool cmVTKWrapTclCommand::WriteInit(const char *kitName, 
                                    std::string& outFileName,
                                    std::vector<std::string>& classes)
{
  unsigned int i;
  std::string tempOutputFile = outFileName + ".tmp";
  FILE *fout = fopen(tempOutputFile.c_str(),"w");
  if (!fout)
    {
    cmSystemTools::Error("Failed to open TclInit file for ", tempOutputFile.c_str());
    return false;
    }

  // capitalized commands just once
  std::vector<std::string> capcommands;
  for (i = 0; i < m_Commands.size(); i++)
    {
    capcommands.push_back(cmSystemTools::Capitalized(m_Commands[i]));
    }
  
  fprintf(fout,"#include \"vtkTclUtil.h\"\n");
  
  for (i = 0; i < classes.size(); i++)
    {
    fprintf(fout,"int %sCommand(ClientData cd, Tcl_Interp *interp,\n             int argc, char *argv[]);\n",classes[i].c_str());
    fprintf(fout,"ClientData %sNewCommand();\n",classes[i].c_str());
    }
  
  if (!strcmp(kitName,"Vtkcommontcl"))
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

  for (i = 0; i < m_Commands.size(); i++)
    {
    fprintf(fout,"\nextern \"C\" {int VTK_EXPORT %s_Init(Tcl_Interp *interp);}\n",
            capcommands[i].c_str());
    }
  
  fprintf(fout,"\n\nextern \"C\" {int VTK_EXPORT %s_SafeInit(Tcl_Interp *interp);}\n",
	  kitName);
  fprintf(fout,"\nextern \"C\" {int VTK_EXPORT %s_Init(Tcl_Interp *interp);}\n",
	  kitName);
  
  /* create an extern ref to the generic delete function */
  fprintf(fout,"\nextern void vtkTclGenericDeleteObject(ClientData cd);\n");

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
  
  for (i = 0; i < m_Commands.size(); i++)
    {
    fprintf(fout,"  %s_Init(interp);\n", capcommands[i].c_str());
    }
  fprintf(fout,"\n");

  for (i = 0; i < classes.size(); i++)
    {
    fprintf(fout,"  vtkTclCreateNew(interp,(char *) \"%s\", %sNewCommand,\n",
	    classes[i].c_str(), classes[i].c_str());
    fprintf(fout,"                  %sCommand);\n",classes[i].c_str());
    }
  
  fprintf(fout,"  return TCL_OK;\n}\n");
  fclose(fout);

  // copy the file if different
  cmSystemTools::CopyFileIfDifferent(tempOutputFile.c_str(),
                                     outFileName.c_str());
  cmSystemTools::RemoveFile(tempOutputFile.c_str());

  return true;
}


