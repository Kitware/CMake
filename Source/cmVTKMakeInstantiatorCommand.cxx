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
#include "cmVTKMakeInstantiatorCommand.h"
#include "cmCacheManager.h"
#include "cmGeneratedFileStream.h"

bool
cmVTKMakeInstantiatorCommand
::InitialPass(std::vector<std::string> const& args)
{
  if(args.size() < 3)
    {
    this->SetError("called with incorrect number of arguments");
    return false;
    }
  
  m_ClassName = args[0];
  m_Makefile->ExpandVariablesInString(m_ClassName);
  
  std::string outSourceList = args[1];
  m_Makefile->ExpandVariablesInString(outSourceList);
  
  std::vector<cmStdString> inSourceLists;
  m_ExportMacro = "-";
  unsigned int groupSize = 10;
  
  // Find the path of the files to be generated.
  std::string filePath = m_Makefile->GetCurrentOutputDirectory();
  std::string headerPath = filePath;
  
  for(unsigned int i=2;i < args.size();++i)
    {
    if(args[i] == "GROUP_SIZE")
      {
      if(++i < args.size())
        {
        std::string gSize = args[i].c_str();
        m_Makefile->ExpandVariablesInString(gSize);
        groupSize = atoi(gSize.c_str());
        }
      else
        {
        this->SetError("GROUP_SIZE option used without value.");
        return false;
        }
      }
    else if(args[i] == "HEADER_LOCATION")
      {
      if(++i < args.size())
        {
        headerPath = args[i];
        m_Makefile->ExpandVariablesInString(headerPath);
        }
      else
        {
        this->SetError("HEADER_LOCATION option used without value.");
        return false;
        }
      }
    else if(args[i] == "EXPORT_MACRO")
      {
      if(++i < args.size())
        {
        m_ExportMacro = args[i];
        m_Makefile->ExpandVariablesInString(m_ExportMacro);
        }
      else
        {
        this->SetError("EXPORT_MACRO option used without value.");
        return false;
        }
      }
    // If not an option, it must be another input source list name.
    else
      {
      std::string s = args[i];
      m_Makefile->ExpandVariablesInString(s);
      inSourceLists.push_back(s);
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
    // Find the source list specified.
    cmMakefile::SourceMap::iterator srcListIter =
      m_Makefile->GetSources().find(*s);
    
    if(srcListIter == m_Makefile->GetSources().end())
      {
      std::string errStr = "No source list named " + *s;
      this->SetError(errStr.c_str());
      return false;
      }
    
    std::vector<cmSourceFile>& srcList = srcListIter->second;
    
    // Collect the names of the classes.
    for(std::vector<cmSourceFile>::iterator src = srcList.begin();
        src != srcList.end();++src)
      {
      // Wrap-excluded and abstract classes do not have a New() method.
      // vtkIndent and vtkTimeStamp are special cases and are not
      // vtkObject subclasses.
      if(!src->GetWrapExclude() && !src->GetIsAnAbstractClass()
         && (src->GetSourceName() != "vtkIndent")
         && (src->GetSourceName() != "vtkTimeStamp"))
        {
        m_Classes.push_back(src->GetSourceName());
        }
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
  m_Makefile->AddSource(file, outSourceList.c_str());
  }

  unsigned int numClasses = m_Classes.size();
  unsigned int numFullBlocks = numClasses / groupSize;
  unsigned int lastBlockSize = numClasses % groupSize;
  unsigned int numBlocks = numFullBlocks + ((lastBlockSize>0)? 1:0);

  // Generate the files with the ::New() calls to each class.  These
  // are done in groups to keep the translation unit size smaller.
  for(unsigned int block=0; block < numBlocks;++block)
    {
    std::string fileName = this->GenerateCreationFileName(block);    
    std::string fullName = filePath+"/"+fileName;
    
    // Generate the output file with copy-if-different.
    {
    cmGeneratedFileStream fout(fullName.c_str());
    
    unsigned int thisBlockSize =
      (block < numFullBlocks)? groupSize:lastBlockSize;
    
    // Actually generate the code in the file.
    this->GenerateCreationFile(fout.GetStream(),
                               block*groupSize, thisBlockSize);
    }
    
    // Add the generated source file into the source list.
    cmSourceFile file;
    file.SetWrapExclude(true);
    file.SetIsAnAbstractClass(false);
    file.SetName(fileName.c_str(), filePath.c_str(),
                 m_Makefile->GetSourceExtensions(),
                 m_Makefile->GetHeaderExtensions());
    m_Makefile->AddSource(file, outSourceList.c_str());
    }

  return true;
}

std::string
cmVTKMakeInstantiatorCommand::GenerateCreationFileName(unsigned int block)
{
  std::strstream nameStr;
  nameStr << m_ClassName.c_str() << block << ".cxx" << std::ends;
  std::string result = nameStr.str();
  nameStr.rdbuf()->freeze(0);
  return result;
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
    "#include \"vtkInstantiator.h\"\n"
    "\n"
    "class " << m_ClassName.c_str() << "Initialize;\n"
    "\n"
    "class " << m_ExportMacro.c_str() << " " << m_ClassName.c_str() << "\n"
    "{\n"
    "  friend class " << m_ClassName.c_str() << "Initialize;\n"
    "\n"
    "  static void ClassInitialize();\n"
    "  static void ClassFinalize();\n"
    "\n";
  
  for(unsigned int i=0;i < m_Classes.size();++i)
    {
    os << "  static vtkObject* Create_" << m_Classes[i].c_str() << "();\n";
    }
  
  // Write the initializer class to make sure the creation functions
  // get registered when this generated header is included.
  os <<
    "};\n"
    "\n"
    "class " << m_ExportMacro.c_str() << " " << m_ClassName.c_str() << "Initialize\n"
    "{\n"
    "public:\n"
    "  " << m_ClassName.c_str() << "Initialize();\n"
    "  ~" << m_ClassName.c_str() << "Initialize();\n"
    "private:\n"
    "  static unsigned int Count;\n"
    "};\n"
    "\n"
    "static " << m_ClassName.c_str() << "Initialize " << m_ClassName.c_str() << "Initializer;\n"
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
  // Write the ClassInitialize method to register all the creation functions.
  os <<
    "#include \"" << m_ClassName.c_str() << ".h\"\n"
    "\n"
    "void " << m_ClassName.c_str() << "::ClassInitialize()\n"
    "{\n";
    
  for(unsigned int i=0;i < m_Classes.size();++i)
    {
    os << "  vtkInstantiator::RegisterInstantiator(\""
       << m_Classes[i].c_str() << "\", " << m_ClassName.c_str() << "::Create_"
       << m_Classes[i].c_str() << ");\n";
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
       << m_Classes[i].c_str() << "\", " << m_ClassName.c_str() << "::Create_"
       << m_Classes[i].c_str() << ");\n";
    }
  
  // Write the constructor and destructor of the initializer class to
  // call the ClassInitialize and ClassFinalize methods at the right
  // time.
  os <<
    "}\n"
    "\n" <<
    m_ClassName.c_str() << "Initialize::" << m_ClassName.c_str() << "Initialize()\n"
    "{\n"
    "  if(++" << m_ClassName.c_str() << "Initialize::Count == 1)\n"
    "    { " << m_ClassName.c_str() << "::ClassInitialize(); }\n"
    "}\n"
    "\n" <<
    m_ClassName.c_str() << "Initialize::~" << m_ClassName.c_str() << "Initialize()\n"
    "{\n"
    "  if(--" << m_ClassName.c_str() << "Initialize::Count == 0)\n"
    "    { " << m_ClassName.c_str() << "::ClassFinalize(); }\n"
    "}\n"
    "\n"
    "// Number of translation units that include this class's header.\n"
    "// Purposely not initialized.  Default is static initialization to 0.\n"
    "unsigned int " << m_ClassName.c_str() << "Initialize::Count;\n";
}

// Generates a file that includes the headers of the classes it knows
// how to create and provides functions which create the classes with
// the New() method.
void
cmVTKMakeInstantiatorCommand
::GenerateCreationFile(std::ostream& os, unsigned int groupStart,
                       unsigned int groupSize)
{
  // Need to include header of generated class.
  os <<
    "#include \"" << m_ClassName.c_str() << ".h\"\n"
    "\n";
  
  // Include class files.
  for(unsigned int i=0;i < groupSize;++i)
    {
    os << "#include \"" << m_Classes[groupStart+i].c_str() << ".h\"\n";
    }

  os <<
    "\n";

  // Write the create function implementations.
  for(unsigned int i=0;i < groupSize;++i)
    {
    os << "vtkObject* " << m_ClassName.c_str() << "::Create_"
       << m_Classes[groupStart+i].c_str() << "() { return "
       << m_Classes[groupStart+i].c_str() << "::New(); }\n";
    }
}
