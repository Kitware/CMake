/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestUploadHandler.h"

#include <ostream>
#include <string>

#include "cmCTest.h"
#include "cmGeneratedFileStream.h"
#include "cmVersion.h"
#include "cmXMLWriter.h"

cmCTestUploadHandler::cmCTestUploadHandler()
{
  this->Initialize();
}

void cmCTestUploadHandler::Initialize()
{
  this->Superclass::Initialize();
  this->Files.clear();
}

void cmCTestUploadHandler::SetFiles(std::set<std::string> const& files)
{
  this->Files = files;
}

int cmCTestUploadHandler::ProcessHandler()
{
  cmGeneratedFileStream ofs;
  if (!this->CTest->OpenOutputFile(this->CTest->GetCurrentTag(), "Upload.xml",
                                   ofs)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot open Upload.xml file" << std::endl);
    return -1;
  }
  std::string buildname =
    cmCTest::SafeBuildIdField(this->CTest->GetCTestConfiguration("BuildName"));

  cmXMLWriter xml(ofs);
  xml.StartDocument();
  xml.ProcessingInstruction("xml-stylesheet",
                            "type=\"text/xsl\" "
                            "href=\"Dart/Source/Server/XSL/Build.xsl "
                            "<file:///Dart/Source/Server/XSL/Build.xsl> \"");
  xml.StartElement("Site");
  xml.Attribute("BuildName", buildname);
  xml.Attribute("BuildStamp",
                this->CTest->GetCurrentTag() + "-" +
                  this->CTest->GetTestModelString());
  xml.Attribute("Name", this->CTest->GetCTestConfiguration("Site"));
  xml.Attribute("Generator",
                std::string("ctest-") + cmVersion::GetCMakeVersion());
  this->CTest->AddSiteProperties(xml);
  xml.StartElement("Upload");

  for (std::string const& file : this->Files) {
    cmCTestOptionalLog(this->CTest, OUTPUT,
                       "\tUpload file: " << file << std::endl, this->Quiet);
    xml.StartElement("File");
    xml.Attribute("filename", file);
    xml.StartElement("Content");
    xml.Attribute("encoding", "base64");
    xml.Content(this->CTest->Base64EncodeFile(file));
    xml.EndElement(); // Content
    xml.EndElement(); // File
  }
  xml.EndElement(); // Upload
  xml.EndElement(); // Site
  xml.EndDocument();
  return 0;
}
