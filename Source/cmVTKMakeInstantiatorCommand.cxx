/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

  Copyright (c) 2002 Insight Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#include "cmVTKMakeInstantiatorCommand.h"
#include "cmCacheManager.h"
#include "cmGeneratedFileStream.h"

bool
cmVTKMakeInstantiatorCommand
::InitialPass(std::vector<std::string> const& argsIn)
{
  if(argsIn.size() < 3)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  std::vector<std::string> args;
  m_Makefile->ExpandSourceListArguments(argsIn, args, 2);
  std::string sourceListValue;
  
  m_ClassName = args[0];
  
  std::vector<cmStdString> inSourceLists;
  m_ExportMacro = "-";
  bool includesMode = false;
  
  // Find the path of the files to be generated.
  std::string filePath = m_Makefile->GetCurrentOutputDirectory();
  std::string headerPath = filePath;
  
  for(unsigned int i=2;i < args.size();++i)
    {
    if(args[i] == "HEADER_LOCATION")
      {
      includesMode = false;
      if(++i < args.size())
        {
        headerPath = args[i];
        }
      else
        {
        this->SetError("HEADER_LOCATION option used without value.");
        return false;
        }
      }
    else if(args[i] == "EXPORT_MACRO")
      {
      includesMode = false;
      if(++i < args.size())
        {
        m_ExportMacro = args[i];
        }
      else
        {
        this->SetError("EXPORT_MACRO option used without value.");
        return false;
        }
      }
    else if(args[i] == "INCLUDES")
      {
      includesMode = true;
      }
    // If not an option, it must be another input source list name or
    // an include file.
    else
      {
      if(!includesMode)
        {
        inSourceLists.push_back(args[i]);
        }
      else
        {
        m_Includes.push_back(args[i]);
        }
      }
    }
  
  if(m_ExportMacro == "-")
    {
    this->SetError("No EXPORT_MACRO option given.");
    return false;
    }
  
  for(std::vector<cmStdString>::const_iterator s = inSourceLists.begin();
      s != inSourceLists.end(); ++s)
    {
    std::string srcName = cmSystemTools::GetFilenameWithoutExtension(*s);
    cmSourceFile *sf = m_Makefile->GetSource(s->c_str());
    
    // Wrap-excluded and abstract classes do not have a New() method.
    // vtkIndent and vtkTimeStamp are special cases and are not
    // vtkObject subclasses.
    if(
      (!sf || (!sf->GetWrapExclude() && !sf->GetIsAnAbstractClass())) &&
      ((srcName != "vtkIndent") && (srcName != "vtkTimeStamp")))
      {
      m_Classes.push_back(srcName);
      }
    }    
  
  // Generate the header with the class declaration.
  {
  std::string fileName = m_ClassName + ".h";
  std::string fullName = headerPath+"/"+fileName;
  
  // Generate the output file with copy-if-different.
  cmGeneratedFileStream fout(fullName.c_str());
  
  // Actually generate the code in the file.
  this->GenerateHeaderFile(fout.GetStream());
  }
  
  // Generate the implementation file.
  {
  std::string fileName = m_ClassName + ".cxx";
  std::string fullName = filePath+"/"+fileName;
  
  // Generate the output file with copy-if-different.
  {
  cmGeneratedFileStream fout(fullName.c_str());
  
  // Actually generate the code in the file.
  this->GenerateImplementationFile(fout.GetStream());
  }
  
  // Add the generated source file into the source list.
  cmSourceFile file;
  file.SetWrapExclude(true);
  file.SetIsAnAbstractClass(false);
  file.SetName(fileName.c_str(), filePath.c_str(),
               m_Makefile->GetSourceExtensions(),
               m_Makefile->GetHeaderExtensions());
  m_Makefile->AddSource(file);
  sourceListValue += file.GetSourceName() + ".cxx";
  }
  
  m_Makefile->AddDefinition(args[1].c_str(), sourceListValue.c_str());  
  return true;
}

// Generates the class header file with the definition of the class
// and its initializer class.
void
cmVTKMakeInstantiatorCommand
::GenerateHeaderFile(std::ostream& os)
{
  os <<
    "#ifndef __" << m_ClassName.c_str() << "_h\n"
    "#define __" << m_ClassName.c_str() << "_h\n"
    "\n"
    "#include \"vtkInstantiator.h\"\n";
  for(unsigned int i=0;i < m_Includes.size();++i)
    {
    os << "#include \"" << m_Includes[i].c_str() << "\"\n";
    }
  
  // Write the instantiator class definition.
  os <<
    "\n"
    "class " << m_ExportMacro.c_str() << " " << m_ClassName.c_str() << "\n"
    "{\n"
    "public:\n"
    "  " << m_ClassName.c_str() << "();\n"
    "  ~" << m_ClassName.c_str() << "();\n"
    "private:\n"
    "  static void ClassInitialize();\n"
    "  static void ClassFinalize();\n"
    "  static unsigned int Count;\n"
    "};\n"
    "\n";
  
  // Write the initialization instance to make sure the creation
  // functions get registered when this generated header is included.
  os <<
    "static "
     << m_ClassName.c_str() << " "
     << m_ClassName.c_str() << "Initializer;\n"
    "\n"
    "#endif\n";  
}

// Generates the file with the implementation of the class.  All
// methods except the actual object creation functions are generated
// here.
void
cmVTKMakeInstantiatorCommand
::GenerateImplementationFile(std::ostream& os)
{
  // Include the instantiator class header.
  os <<
    "#include \"" << m_ClassName.c_str() << ".h\"\n"
    "\n";
  
  // Write the extern declarations for all the creation functions.
  for(unsigned int i=0;i < m_Classes.size();++i)
    {
    os << "extern vtkObject* vtkInstantiator" << m_Classes[i].c_str() << "New();\n";
    }
  
  // Write the ClassInitialize method to register all the creation functions.
  os <<
    "\n"
    "void " << m_ClassName.c_str() << "::ClassInitialize()\n"
    "{\n";
  
  for(unsigned int i=0;i < m_Classes.size();++i)
    {
    os << "  vtkInstantiator::RegisterInstantiator(\""
       << m_Classes[i].c_str() << "\", vtkInstantiator"
       << m_Classes[i].c_str() << "New);\n";
    }
  
  // Write the ClassFinalize method to unregister all the creation functions.
  os <<
    "}\n"
    "\n"
    "void " << m_ClassName.c_str() << "::ClassFinalize()\n"
    "{\n";
  
  for(unsigned int i=0;i < m_Classes.size();++i)
    {
    os << "  vtkInstantiator::UnRegisterInstantiator(\""
       << m_Classes[i].c_str() << "\", vtkInstantiator"
       << m_Classes[i].c_str() << "New);\n";
    }
  
  // Write the constructor and destructor of the initializer class to
  // call the ClassInitialize and ClassFinalize methods at the right
  // time.
  os <<
    "}\n"
    "\n" <<
    m_ClassName.c_str() << "::" << m_ClassName.c_str() << "()\n"
    "{\n"
    "  if(++" << m_ClassName.c_str() << "::Count == 1)\n"
    "    { " << m_ClassName.c_str() << "::ClassInitialize(); }\n"
    "}\n"
    "\n" <<
    m_ClassName.c_str() << "::~" << m_ClassName.c_str() << "()\n"
    "{\n"
    "  if(--" << m_ClassName.c_str() << "::Count == 0)\n"
    "    { " << m_ClassName.c_str() << "::ClassFinalize(); }\n"
    "}\n"
    "\n"
    "// Number of translation units that include this class's header.\n"
    "// Purposely not initialized.  Default is static initialization to 0.\n"
    "unsigned int " << m_ClassName.c_str() << "::Count;\n";
}
