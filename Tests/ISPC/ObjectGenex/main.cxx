#include <stdio.h>

/*
  Define GENERATED_HEADER macro to allow c++ files to include headers
  generated based on different configuration types.
*/

/* clang-format off */
#define GENERATED_HEADER(x) GENERATED_HEADER0(CONFIG_TYPE/x)
/* clang-format on */
#define GENERATED_HEADER0(x) GENERATED_HEADER1(x)
#define GENERATED_HEADER1(x) <x>

#include GENERATED_HEADER(path_to_objs.h)

#include <vector>
std::vector<std::string> expandList(std::string const& arg)
{
  std::vector<std::string> output;
  // If argument is empty or no `;` just copy the current string
  if (arg.empty() || arg.find(';') == std::string::npos) {
    output.emplace_back(arg);
    return output;
  }

  std::string newArg;
  // Break the string at non-escaped semicolons not nested in [].
  int squareNesting = 0;
  auto last = arg.begin();
  auto const cend = arg.end();
  for (auto c = last; c != cend; ++c) {
    switch (*c) {
      case '\\': {
        // We only want to allow escaping of semicolons.  Other
        // escapes should not be processed here.
        auto cnext = c + 1;
        if ((cnext != cend) && *cnext == ';') {
          newArg.append(last, c);
          // Skip over the escape character
          last = cnext;
          c = cnext;
        }
      } break;
      case '[': {
        ++squareNesting;
      } break;
      case ']': {
        --squareNesting;
      } break;
      case ';': {
        // Break the string here if we are not nested inside square
        // brackets.
        if (squareNesting == 0) {
          newArg.append(last, c);
          // Skip over the semicolon
          last = c + 1;
          if (!newArg.empty()) {
            // Add the last argument if the string is not empty.
            output.push_back(newArg);
            newArg.clear();
          }
        }
      } break;
      default: {
        // Just append this character.
      } break;
    }
  }
  newArg.append(last, cend);
  if (!newArg.empty()) {
    // Add the last argument if the string is not empty.
    output.push_back(std::move(newArg));
  }

  return output;
}

int main()
{
  // determine that the number of object files specified in obj_paths
  // is equal to the number of arch's

  std::vector<std::string> paths = expandList(obj_paths);
  const bool correctSize = (paths.size() == ExpectedISPCObjects);

  return (correctSize) ? 0 : 1;
}
