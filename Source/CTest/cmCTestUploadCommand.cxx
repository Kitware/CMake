/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCTestUploadCommand.h"

#include <chrono>
#include <set>
#include <sstream>
#include <string>
#include <utility>

#include <cm/memory>
#include <cm/vector>
#include <cmext/string_view>

#include "cmArgumentParser.h"
#include "cmCTest.h"
#include "cmCTestGenericHandler.h"
#include "cmExecutionStatus.h"
#include "cmGeneratedFileStream.h"
#include "cmMakefile.h"
#include "cmMessageType.h"
#include "cmSystemTools.h"
#include "cmVersion.h"
#include "cmXMLWriter.h"

class cmCTestUploadHandler : public cmCTestGenericHandler
{
public:
  using Superclass = cmCTestGenericHandler;

  cmCTestUploadHandler(cmCTest* ctest);

  /*
   * The main entry point for this class
   */
  int ProcessHandler() override;

  /** Specify a set of files to submit.  */
  void SetFiles(std::set<std::string> const& files);

private:
  std::set<std::string> Files;
};

void cmCTestUploadCommand::CheckArguments(HandlerArguments& arguments,
                                          cmExecutionStatus& status) const
{
  cmMakefile& mf = status.GetMakefile();
  auto& args = static_cast<UploadArguments&>(arguments);
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
}

std::unique_ptr<cmCTestGenericHandler> cmCTestUploadCommand::InitializeHandler(
  HandlerArguments& arguments, cmExecutionStatus&) const
{
  auto const& args = static_cast<UploadArguments&>(arguments);
  auto handler = cm::make_unique<cmCTestUploadHandler>(this->CTest);
  handler->SetFiles(
    std::set<std::string>(args.Files.begin(), args.Files.end()));
  handler->SetQuiet(args.Quiet);
  return std::unique_ptr<cmCTestGenericHandler>(std::move(handler));
}

cmCTestUploadHandler::cmCTestUploadHandler(cmCTest* ctest)
  : Superclass(ctest)
{
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
                  this->CTest->GetTestGroupString());
  xml.Attribute("Name", this->CTest->GetCTestConfiguration("Site"));
  xml.Attribute("Generator",
                std::string("ctest-") + cmVersion::GetCMakeVersion());
  this->CTest->AddSiteProperties(xml, this->CMake);
  xml.StartElement("Upload");
  xml.Element("Time", std::chrono::system_clock::now());

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

bool cmCTestUploadCommand::InitialPass(std::vector<std::string> const& args,
                                       cmExecutionStatus& status) const
{
  static auto const parser =
    cmArgumentParser<UploadArguments>{ MakeHandlerParser<UploadArguments>() }
      .Bind("FILES"_s, &UploadArguments::Files)
      .Bind("QUIET"_s, &UploadArguments::Quiet);

  return this->Invoke(parser, args, status, [&](UploadArguments& a) {
    return this->ExecuteHandlerCommand(a, status);
  });
}
