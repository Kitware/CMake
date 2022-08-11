#include "cmConfigure.h" // IWYU pragma: keep

#include <cstdlib>
#include <iostream>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "cmsys/FStream.hxx"

#include "cmSystemTools.h"

class CompileCommandParser
{
public:
  class CommandType : public std::map<std::string, std::string>
  {
  public:
    std::string const& at(std::string const& k) const
    {
      auto i = this->find(k);
      if (i != this->end()) {
        return i->second;
      }
      static std::string emptyString;
      return emptyString;
    }
  };
  using TranslationUnitsType = std::vector<CommandType>;

  CompileCommandParser(std::istream& input)
    : Input(input)
  {
  }

  void Parse()
  {
    this->NextNonWhitespace();
    this->ParseTranslationUnits();
  }

  const TranslationUnitsType& GetTranslationUnits()
  {
    return this->TranslationUnits;
  }

private:
  void ParseTranslationUnits()
  {
    this->TranslationUnits = TranslationUnitsType();
    this->ExpectOrDie('[', "at start of compile command file\n");
    do {
      this->ParseTranslationUnit();
      this->TranslationUnits.push_back(this->Command);
    } while (this->Expect(','));
    this->ExpectOrDie(']', "at end of array");
  }

  void ParseTranslationUnit()
  {
    this->Command = CommandType();
    if (!this->Expect('{')) {
      return;
    }
    if (this->Expect('}')) {
      return;
    }
    do {
      this->ParseString();
      std::string name = this->String;
      this->ExpectOrDie(':', "between name and value");
      this->ParseString();
      std::string value = this->String;
      this->Command[name] = value;
    } while (this->Expect(','));
    this->ExpectOrDie('}', "at end of object");
  }

  void ParseString()
  {
    this->String = "";
    if (!this->Expect('"')) {
      return;
    }
    while (!this->Expect('"')) {
      this->Expect('\\');
      this->String.append(1, this->C);
      this->Next();
    }
  }

  bool Expect(char c)
  {
    if (this->C == c) {
      this->NextNonWhitespace();
      return true;
    }
    return false;
  }

  void ExpectOrDie(char c, const std::string& message)
  {
    if (!this->Expect(c)) {
      this->ErrorExit(std::string("'") + c + "' expected " + message + ".");
    }
  }

  void NextNonWhitespace()
  {
    do {
      this->Next();
    } while (this->IsWhitespace());
  }

  void Next()
  {
    this->C = static_cast<char>(this->Input.get());
    if (this->Input.bad()) {
      this->ErrorExit("Unexpected end of file.");
    }
  }

  void ErrorExit(const std::string& message)
  {
    std::cout << "ERROR: " << message;
    exit(1);
  }

  bool IsWhitespace() const
  {
    return (this->C == ' ' || this->C == '\t' || this->C == '\n' ||
            this->C == '\r');
  }

  char C;
  TranslationUnitsType TranslationUnits;
  CommandType Command;
  std::string String;
  std::istream& Input;
};

int main()
{
  cmsys::ifstream file("compile_commands.json");
  CompileCommandParser parser(file);
  parser.Parse();
  for (auto const& tu : parser.GetTranslationUnits()) {
    std::vector<std::string> command;
    cmSystemTools::ParseUnixCommandLine(tu.at("command").c_str(), command);
    if (!cmSystemTools::RunSingleCommand(command, nullptr, nullptr, nullptr,
                                         tu.at("directory").c_str())) {
      std::cout << "ERROR: Failed to run command \"" << command[0] << "\""
                << std::endl;
      exit(1);
    }
  }
  return 0;
}
