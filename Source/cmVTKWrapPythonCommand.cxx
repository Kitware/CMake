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
#include "cmVTKWrapPythonCommand.h"

// cmVTKWrapPythonCommand
bool cmVTKWrapPythonCommand::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }

  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  if(!m_Makefile->IsOn("VTK_WRAP_PYTHON"))
    {
    return true;
    }

  
  // what is the current source dir
  std::string cdir = m_Makefile->GetCurrentDirectory();

  // keep the library name
  m_LibraryName = args[0];
  m_SourceList = args[1];
  
  // get the list of classes for this library
  cmMakefile::SourceMap &Classes = m_Makefile->GetSources();
  for(std::vector<std::string>::const_iterator j = (args.begin() + 2);
      j != args.end(); ++j)
    {   
    cmMakefile::SourceMap::iterator l = Classes.find(*j);
    if (l == Classes.end())
      {
      this->SetError("bad source list passed to VTKWrapPythonCommand");
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
        std::string newName = curr.GetSourceName() + "Python";
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
  
  return true;
}

void cmVTKWrapPythonCommand::FinalPass() 
{
  // first we add the rules for all the .h to Python.cxx files
  int lastClass = m_WrapClasses.size();
  std::vector<std::string> depends;
  std::string wpython = "${VTK_WRAP_PYTHON_EXE}";
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
  depends.push_back(wpython);
  depends.push_back(hints);
  for(int classNum = 0; classNum < lastClass; classNum++)
    {
    m_Makefile->AddSource(m_WrapClasses[classNum],m_SourceList.c_str());
    std::string res = m_WrapClasses[classNum].GetSourceName() + ".cxx";
    std::vector<std::string> args;
    args.push_back(m_WrapHeaders[classNum]);
    args.push_back(hints);
    args.push_back((m_WrapClasses[classNum].IsAnAbstractClass() ? "0" : "1"));
    args.push_back(res);
    m_Makefile->AddCustomCommand(m_WrapHeaders[classNum].c_str(),
                                 wpython.c_str(), args, depends, 
                                 res.c_str(), m_LibraryName.c_str());
    }
  
}

bool cmVTKWrapPythonCommand::CreateInitFile(std::string& res) 
{
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
  
  return this->WriteInit(m_LibraryName.c_str(), outFileName, classes);
}


/* warning this code is also in getclasses.cxx under pcmaker */
bool cmVTKWrapPythonCommand::WriteInit(const char *kitName, 
                                       std::string& outFileName,
                                       std::vector<std::string>& classes)
{
  unsigned int i;
  
  std::string tempOutputFile = outFileName + ".tmp";
  FILE *fout = fopen(tempOutputFile.c_str(),"w");
  if (!fout)
    {
    return false;
    }
  
  fprintf(fout,"#include <string.h>\n");
  fprintf(fout,"#include \"Python.h\"\n\n");

  for (i = 0; i < classes.size(); i++)
    {
#ifdef _WIN32
    fprintf(fout,"extern  \"C\" {__declspec( dllexport) PyObject *PyVTKClass_%sNew(char *); }\n",classes[i].c_str());
#else
    fprintf(fout,"extern  \"C\" {PyObject *PyVTKClass_%sNew(char *); }\n",classes[i].c_str());
#endif
    }

  fprintf(fout,"\nstatic PyMethodDef Py%s_ClassMethods[] = {\n",
	  kitName);
  fprintf(fout,"{NULL, NULL}};\n\n");
  
#ifdef _WIN32
  fprintf(fout,"extern  \"C\" {__declspec( dllexport) void init%s();}\n\n",kitName);
  fprintf(fout,"void init%s()\n{\n",kitName);
#else
  fprintf(fout,"extern  \"C\" {void initlib%s();}\n\n",kitName);
  fprintf(fout,"void initlib%s()\n{\n",kitName);
#endif
  

  /* module init function */
  fprintf(fout,"  PyObject *m, *d, *c;\n\n");
#ifdef _WIN32
  fprintf(fout,"  static char modulename[] = \"%s\";\n",kitName);
#else
  fprintf(fout,"  static char modulename[] = \"lib%s\";\n",kitName);
#endif
  fprintf(fout,"  m = Py_InitModule(modulename, Py%s_ClassMethods);\n",
	  kitName);
  
  fprintf(fout,"  d = PyModule_GetDict(m);\n");
  fprintf(fout,"  if (!d) Py_FatalError(\"can't get dictionary for module %s!\");\n\n",
	  kitName);

  for (i = 0; i < classes.size(); i++)
    {
    fprintf(fout,"  if ((c = PyVTKClass_%sNew(modulename)))\n",
            classes[i].c_str());
    fprintf(fout,"    if (-1 == PyDict_SetItemString(d, \"%s\", c))\n",
	    classes[i].c_str());
    fprintf(fout,"      Py_FatalError(\"can't add class %s to dictionary!\");\n\n",
	    classes[i].c_str());
    }
  fprintf(fout,"}\n\n");
  fclose(fout);
  
  
  // copy the file if different
  cmSystemTools::CopyFileIfDifferent(tempOutputFile.c_str(),
                                     outFileName.c_str());
  cmSystemTools::RemoveFile(tempOutputFile.c_str());
  return true;
}


