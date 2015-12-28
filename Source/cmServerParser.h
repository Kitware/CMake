
#ifndef cmServerParser_h
#define cmServerParser_h

#include "cmListFileLexer.h"
#include "cmStandardIncludes.h"
#include "cmState.h"

#if defined(CMAKE_BUILD_WITH_CMAKE)
#include "cm_jsoncpp_value.h"
#endif

class DifferentialFileContent;

struct Token
{
  std::string Type;
  int Line;
  int Column;
  int Length;
};

class cmServerParser
{
public:
  cmServerParser(cmState* state, std::string const& fileName,
                 std::string const& rootDir);
  ~cmServerParser();

  void Add(cmListFileLexer_Token* token, std::vector<Token>& value,
           DifferentialFileContent diff, const char* name = 0,
           cmCommand* cmd = 0, std::vector<std::string> const& args = {});

  Json::Value Parse(DifferentialFileContent diff);

  bool ParseFunction(cmListFileLexer_Token* incomingToken,
                     std::vector<Token>& ret, DifferentialFileContent diff);

  bool AddArgument();

private:
  cmListFileLexer_Token* Scan();
  void Scan(DifferentialFileContent diff, std::vector<Token>& ret);

  int TransformLine(long line, DifferentialFileContent diff);

  cmListFileLexer* Lexer;
  std::string FileName;
  std::string mRootDir;
  bool IsString;
  cmState* State;
  enum
  {
    SeparationOkay,
    SeparationWarning,
    SeparationError
  } Separation;
};

#endif
