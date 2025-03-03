/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file LICENSE.rst or https://cmake.org/licensing for details.  */
#include "cmCTestUploadCommand.h"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <string>

#include <cm/vector>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmCTest.h"
#include "cmExecutionStatus.h"
#include "cmGeneratedFileStream.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSystemTools.h"
#include "cmVersion.h"
#include "cmXMLWriter.h"

bool cmCTestUploadCommand::ExecuteUpload(UploadArguments& args,
                                         cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();

  std::sort(args.Files.begin(), args.Files.end());
  args.Files.erase(std::unique(args.Files.begin(), args.Files.end()),
                   args.Files.end());

  cm::erase_if(args.Files, [&mf](std::string const& arg) -> bool {
    if (!cmSystemTools::FileExists(arg)) {
      std::ostringstream e;
      e << "File \"" << arg << "\" does not exist. Cannot submit "
        << "a non-existent file.";
      mf.IssueMessage(MessageType::FATAL_ERROR, e.str());
      return true;
    }
    return false;
  });

  cmGeneratedFileStream ofs;
  if (!this->CTest->OpenOutputFile(this->CTest->GetCurrentTag(), "Upload.xml",
                                   ofs)) {
    cmCTestLog(this->CTest, ERROR_MESSAGE,
               "Cannot open Upload.xml file" << std::endl);
    return false;
  }
  std::string buildname =
    cmCTest::SafeBuildIdField(mf.GetSafeDefinition("CTEST_BUILD_NAME"));

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
                  this->CTest->GetTestGroupString());
  xml.Attribute("Name", mf.GetSafeDefinition("CTEST_SITE"));
  xml.Attribute("Generator",
                std::string("ctest-") + cmVersion::GetCMakeVersion());
  this->CTest->AddSiteProperties(xml, mf.GetCMakeInstance());
  xml.StartElement("Upload");
  xml.Element("Time", std::chrono::system_clock::now());

  for (std::string const& file : args.Files) {
    cmCTestOptionalLog(this->CTest, OUTPUT,
                       "\tUpload file: " << file << std::endl, args.Quiet);
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
  return true;
}

bool cmCTestUploadCommand::InitialPass(std::vector<std::string> const& args,
                                       cmExecutionStatus& status) const
{
  static auto const parser =
    cmArgumentParser<UploadArguments>{ MakeBasicParser<UploadArguments>() }
      .Bind("FILES"_s, &UploadArguments::Files)
      .Bind("QUIET"_s, &UploadArguments::Quiet);

  return this->Invoke(parser, args, status, [&](UploadArguments& a) {
    return this->ExecuteUpload(a, status);
  });
}
