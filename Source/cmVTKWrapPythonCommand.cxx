/*=========================================================================

  Program:   CMake - Cross-Platform Makefile Generator
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Kitware, Inc., Insight Consortium.  All rights reserved.
  See Copyright.txt or http://www.cmake.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmVTKWrapPythonCommand.h"

// cmVTKWrapPythonCommand
bool cmVTKWrapPythonCommand::InitialPass(std::vector<std::string> const&
                                         argsIn)
{
  if(argsIn.size() < 3 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  this->Makefile->ExpandSourceListArguments(argsIn, args, 2);

  // Now check and see if the value has been stored in the cache
  // already, if so use that value and don't look for the program
  if(!this->Makefile->IsOn("VTK_WRAP_PYTHON"))
    {
    return true;
    }

  
  // what is the current source dir
  std::string cdir = this->Makefile->GetCurrentDirectory();

  // keep the library name
  this->LibraryName = args[0];
  this->SourceList = args[1];
  std::string sourceListValue;
  // was the list already populated
  const char *def = this->Makefile->GetDefinition(this->SourceList.c_str());  
  if (def)
    {
    sourceListValue = def;
    sourceListValue += ";";
    }

  // Create the init file 
  std::string res = this->LibraryName;
  res += "Init.cxx";

  // add the init file
  std::string initName = this->LibraryName;
  initName += "Init";
  sourceListValue += initName + ".cxx";

  // get the list of classes for this library
  for(std::vector<std::string>::iterator j = (args.begin() + 2);
      j != args.end(); ++j)
    {   
    cmSourceFile *curr = this->Makefile->GetSource(j->c_str());
    
    // if we should wrap the class
    if (!curr || !curr->GetPropertyAsBool("WRAP_EXCLUDE"))
      {
      cmSourceFile file;
      if (curr)
        {
        file.SetProperty("ABSTRACT",curr->GetProperty("ABSTRACT"));
        }
      std::string srcName = cmSystemTools::GetFilenameWithoutExtension(*j);
      std::string newName = srcName + "Python";
      file.SetName(newName.c_str(), 
                   this->Makefile->GetCurrentOutputDirectory(),
                   "cxx",false);
      std::string hname = cdir + "/" + srcName + ".h";
      this->WrapHeaders.push_back(hname);
      // add starting depends
      file.GetDepends().push_back(hname);
      this->WrapClasses.push_back(file);
      sourceListValue += ";";
      sourceListValue += newName + ".cxx";
      }
    }
  
  cmSourceFile cfile;
  cfile.SetProperty("ABSTRACT","0");
  this->CreateInitFile(res);
  cfile.SetName(initName.c_str(), this->Makefile->GetCurrentOutputDirectory(),
                "cxx",false);
  this->Makefile->AddSource(cfile);
  this->Makefile->AddDefinition(this->SourceList.c_str(),
                                sourceListValue.c_str());
  return true;
}

void cmVTKWrapPythonCommand::FinalPass() 
{
  // first we add the rules for all the .h to Python.cxx files
  size_t lastClass = this->WrapClasses.size();
  std::vector<std::string> depends;
  const char* wpython = 
    this->Makefile->GetRequiredDefinition("VTK_WRAP_PYTHON_EXE");
  const char* hints = this->Makefile->GetDefinition("VTK_WRAP_HINTS");

  // wrap all the .h files
  depends.push_back(wpython);
  if(hints)
    {
    depends.push_back(hints);
    }
  for(size_t classNum = 0; classNum < lastClass; classNum++)
    {
    this->Makefile->AddSource(this->WrapClasses[classNum]);
    std::string res = this->Makefile->GetCurrentOutputDirectory();
    res += "/";
    res += this->WrapClasses[classNum].GetSourceName() + ".cxx";
    cmCustomCommandLine commandLine;
    commandLine.push_back(wpython);
    commandLine.push_back(this->WrapHeaders[classNum]);
    if(hints)
      {
      commandLine.push_back(hints);
      }
    commandLine.push_back((this->WrapClasses[classNum].
                           GetPropertyAsBool("ABSTRACT") ? "0" : "1"));
    commandLine.push_back(res);

    cmCustomCommandLines commandLines;
    commandLines.push_back(commandLine);
    std::vector<std::string> outputs;
    outputs.push_back(res);
    const char* no_comment = 0;
    this->Makefile->AddCustomCommandOldStyle(this->LibraryName.c_str(),
                                         outputs,
                                         depends,
                                         this->WrapHeaders[classNum].c_str(),
                                         commandLines,
                                         no_comment);
    }
}

bool cmVTKWrapPythonCommand::CreateInitFile(std::string& res) 
{
  std::vector<std::string> classes;
  size_t lastClass = this->WrapHeaders.size();
  size_t classNum;
  for(classNum = 0; classNum < lastClass; classNum++)
    {
    std::string cls = this->WrapHeaders[classNum];
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
    this->Makefile->GetCurrentOutputDirectory();
  outFileName += "/" + res;
  
  return this->WriteInit(this->LibraryName.c_str(), outFileName, classes);
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
    cmSystemTools::ReportLastSystemError("cmVTKWrapPythonCommand error:");
    return false;
    }
  
  fprintf(fout,"// Generated by cmVTKWrapPythonCommand in CMake\n\n");
  fprintf(fout,"#include <string.h>\n");
  fprintf(fout,"#include \"Python.h\"\n\n");
  fprintf(fout,"// Handle compiler warning messages, etc.\n"
          "#if defined( _MSC_VER ) && !defined(VTK_DISPLAY_WIN32_WARNINGS)\n"
          "#pragma warning ( disable : 4706 )\n"
          "#endif // Windows Warnings \n\n");

  for (i = 0; i < classes.size(); i++)
    {
#ifdef _WIN32
    fprintf(fout, "extern  \"C\" {__declspec( dllexport) "
            "PyObject *PyVTKClass_%sNew(char *); }\n", classes[i].c_str());
#else
    fprintf(fout,"extern  \"C\" {PyObject *PyVTKClass_%sNew(char *); }\n",
            classes[i].c_str());
#endif
    }

  fprintf(fout,"\nstatic PyMethodDef Py%s_ClassMethods[] = {\n",
          kitName);
  fprintf(fout,"{NULL, NULL, 0, NULL}};\n\n");
  
#ifdef _WIN32
  fprintf(fout,"extern  \"C\" {__declspec( dllexport) void init%s();}\n\n",
          kitName);
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
  fprintf(fout,"  if (!d) Py_FatalError"
          "(\"can't get dictionary for module %s!\");\n\n", kitName);

  for (i = 0; i < classes.size(); i++)
    {
    fprintf(fout,"  if ((c = PyVTKClass_%sNew(modulename)))\n",
            classes[i].c_str());
    fprintf(fout,"    if (-1 == PyDict_SetItemString(d, \"%s\", c))\n",
            classes[i].c_str());
    fprintf(fout,
            "      Py_FatalError(\"can't add class %s to dictionary!\");\n\n",
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


