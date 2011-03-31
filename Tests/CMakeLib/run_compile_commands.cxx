#include "cmSystemTools.h"

class CompileCommandParser {
public:
  typedef std::map<std::string, std::string> CommandType;
  typedef std::vector<CommandType> TranslationUnitsType;

  CompileCommandParser(std::ifstream *input)
  {
    this->Input = input;
  }

  void Parse()
  {
    NextNonWhitespace();
    ParseTranslationUnits();
  }

  const TranslationUnitsType& GetTranslationUnits()
  {
    return this->TranslationUnits;
  }

private:
  void ParseTranslationUnits()
  {
    this->TranslationUnits = TranslationUnitsType();
    ExpectOrDie('[', "at start of compile command file");
    do
      {
      ParseTranslationUnit();
      this->TranslationUnits.push_back(this->Command);
      } while(Expect(','));
    ExpectOrDie(']', "at end of array");
  }

  void ParseTranslationUnit()
  {
    this->Command = CommandType();
    if(!Expect('{')) return;
    if(Expect('}')) return;
    do
      {
      ParseString();
      std::string name = this->String;
      ExpectOrDie(':', "between name and value");
      ParseString();
      std::string value = this->String;
      this->Command[name] = value;
      } while(Expect(','));
    ExpectOrDie('}', "at end of object");
  }

  void ParseString()
  {
    this->String.clear();
    if(!Expect('"')) return;
    while (!Expect('"'))
      {
      Expect('\\');
      this->String.push_back(C);
      Next();
      }
  }

  bool Expect(char c)
  {
    if(this->C == c)
      {
      NextNonWhitespace();
      return true;
      }
    return false;
  }

  void ExpectOrDie(char c, const std::string & message)
  {
    if (!Expect(c))
      ErrorExit(std::string("'") + c + "' expected " + message + ".");
  }

  void NextNonWhitespace()
  {
    do { Next(); } while (IsWhitespace());
  }

  void Next()
  {
    this->C = Input->get();
    if (this->Input->bad()) ErrorExit("Unexpected end of file.");
  }

  void ErrorExit(const std::string &message) {
    std::cout << "ERROR: " << message;
    exit(1);
  }

  bool IsWhitespace()
  {
    return (this->C == ' ' || this->C == '\t' ||
            this->C == '\n' || this->C == '\r');
  }

  char C;
  TranslationUnitsType TranslationUnits;
  CommandType Command;
  std::string String;
  std::ifstream *Input;
};

int main ()
{
  std::ifstream file("compile_commands.json");
  CompileCommandParser parser(&file);
  parser.Parse();
  for(CompileCommandParser::TranslationUnitsType::const_iterator
      it = parser.GetTranslationUnits().begin(),
      end = parser.GetTranslationUnits().end(); it != end; ++it)
    {
    std::vector<std::string> std_command;
    cmSystemTools::ParseUnixCommandLine(it->at("command").c_str(), std_command);
    std::vector<cmStdString> command(std_command.begin(), std_command.end());
    if (!cmSystemTools::RunSingleCommand(
            command, 0, 0, it->at("directory").c_str()))
      {
      std::cout << "ERROR: Failed to run command \""
                << command[0] << "\"" << std::endl;
      exit(1);
      }
    }
  return 0;
}
