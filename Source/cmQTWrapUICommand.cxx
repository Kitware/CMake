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
#include "cmQTWrapUICommand.h"

// cmQTWrapUICommand
bool cmQTWrapUICommand::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 4 )
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  this->Makefile->ExpandSourceListArguments(argsIn, args, 3);

  // what is the current source dir
  std::string cdir = this->Makefile->GetCurrentDirectory();

  // keep the library name
  this->LibraryName = args[0];
  this->HeaderList = args[1];
  this->SourceList = args[2];
  std::string sourceListValue;
  std::string headerListValue;
  const char *def = this->Makefile->GetDefinition(this->SourceList.c_str());  
  if (def)
    {
    sourceListValue = def;
    }
 
  // get the list of classes for this library
  for(std::vector<std::string>::iterator j = (args.begin() + 3);
      j != args.end(); ++j)
    {   
    cmSourceFile *curr = this->Makefile->GetSource(j->c_str());
    
    // if we should wrap the class
    if (!curr || !curr->GetPropertyAsBool("WRAP_EXCLUDE"))
      {
      cmSourceFile header_file;
      cmSourceFile source_file;
      cmSourceFile moc_file;
      std::string srcName = cmSystemTools::GetFilenameWithoutExtension(*j);
      header_file.SetName(srcName.c_str(), 
                          this->Makefile->GetCurrentOutputDirectory(),
                          "h",false);
      source_file.SetName(srcName.c_str(), 
                          this->Makefile->GetCurrentOutputDirectory(),
                          "cxx",false);
      std::string moc_source_name("moc_");
      moc_source_name = moc_source_name + srcName;
      moc_file.SetName(moc_source_name.c_str(), 
                       this->Makefile->GetCurrentOutputDirectory(),
                       "cxx",false);
      std::string origname;
      if ( (*j)[0] == '/' || (*j)[1] == ':' )
        {
        origname = *j;
        }
      else
        {
        if ( curr && curr->GetPropertyAsBool("GENERATED") )
          {
          origname = std::string(this->Makefile->GetCurrentOutputDirectory()) 
            + "/" + *j;
          }
        else
          {
          origname = cdir + "/" + *j;
          }
        }
      std::string hname = header_file.GetFullPath();
      this->WrapUserInterface.push_back(origname);
      // add starting depends
      moc_file.AddDepend(hname.c_str());
      source_file.AddDepend(hname.c_str());
      source_file.AddDepend(origname.c_str());
      header_file.AddDepend(origname.c_str());
      this->WrapHeadersClasses.push_back(header_file);
      this->WrapSourcesClasses.push_back(source_file);
      this->WrapMocClasses.push_back(moc_file);
      this->Makefile->AddSource(header_file);
      this->Makefile->AddSource(source_file);
      this->Makefile->AddSource(moc_file);
      
      // create the list of headers 
      if (headerListValue.size() > 0)
        {
        headerListValue += ";";
        }
      headerListValue += header_file.GetFullPath();
      
      // create the list of sources
      if (sourceListValue.size() > 0)
        {
        sourceListValue += ";";
        }
      sourceListValue += source_file.GetFullPath();
      sourceListValue += ";";
      sourceListValue += moc_file.GetFullPath();
      }
    }
  
  this->Makefile->AddDefinition(this->SourceList.c_str(),
                                sourceListValue.c_str());  
  this->Makefile->AddDefinition(this->HeaderList.c_str(),
                                headerListValue.c_str());  
  return true;
}

void cmQTWrapUICommand::FinalPass() 
{

  // first we add the rules for all the .ui to .h and .cxx files
  size_t lastHeadersClass = this->WrapHeadersClasses.size();
  std::vector<std::string> depends;
  const char* uic_exe =
    this->Makefile->GetRequiredDefinition("QT_UIC_EXECUTABLE");
  const char* moc_exe =
    this->Makefile->GetRequiredDefinition("QT_MOC_EXECUTABLE");

  // wrap all the .h files
  depends.push_back(uic_exe);

  for(size_t classNum = 0; classNum < lastHeadersClass; classNum++)
    {
    // set up .ui to .h and .cxx command
    std::string hres = this->Makefile->GetCurrentOutputDirectory();
    hres += "/";
    hres += this->WrapHeadersClasses[classNum].GetSourceName() + "." +
        this->WrapHeadersClasses[classNum].GetSourceExtension();

    std::string cxxres = this->Makefile->GetCurrentOutputDirectory();
    cxxres += "/";
    cxxres += this->WrapSourcesClasses[classNum].GetSourceName() + "." +
        this->WrapSourcesClasses[classNum].GetSourceExtension();

    std::string mocres = this->Makefile->GetCurrentOutputDirectory();
    mocres += "/";
    mocres += this->WrapMocClasses[classNum].GetSourceName() + "." +
        this->WrapMocClasses[classNum].GetSourceExtension();

    cmCustomCommandLine hCommand;
    hCommand.push_back(uic_exe);
    hCommand.push_back("-o");
    hCommand.push_back(hres);
    hCommand.push_back(this->WrapUserInterface[classNum]);
    cmCustomCommandLines hCommandLines;
    hCommandLines.push_back(hCommand);

    cmCustomCommandLine cxxCommand;
    cxxCommand.push_back(uic_exe);
    cxxCommand.push_back("-impl");
    cxxCommand.push_back(hres);
    cxxCommand.push_back("-o");
    cxxCommand.push_back(cxxres);
    cxxCommand.push_back(this->WrapUserInterface[classNum]);
    cmCustomCommandLines cxxCommandLines;
    cxxCommandLines.push_back(cxxCommand);

    cmCustomCommandLine mocCommand;
    mocCommand.push_back(moc_exe);
    mocCommand.push_back("-o");
    mocCommand.push_back(mocres);
    mocCommand.push_back(hres);
    cmCustomCommandLines mocCommandLines;
    mocCommandLines.push_back(mocCommand);

    depends.clear();
    depends.push_back(this->WrapUserInterface[classNum]);
    const char* no_main_dependency = 0;
    const char* no_comment = 0;
    const char* no_working_dir = 0;
    this->Makefile->AddCustomCommandToOutput(hres.c_str(),
                                         depends,
                                         no_main_dependency,
                                         hCommandLines,
                                         no_comment,
                                         no_working_dir);

    depends.push_back(hres);

    this->Makefile->AddCustomCommandToOutput(cxxres.c_str(),
                                         depends,
                                         no_main_dependency,
                                         cxxCommandLines,
                                         no_comment,
                                         no_working_dir);

    depends.clear();
    depends.push_back(hres);

    this->Makefile->AddCustomCommandToOutput(mocres.c_str(),
                                         depends,
                                         no_main_dependency,
                                         mocCommandLines,
                                         no_comment,
                                         no_working_dir);
    }
}
