/* Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
   file Copyright.txt or https://cmake.org/licensing for details.  */
#include "cmCursesColor.h"

#include <cctype>
#include <cstdlib>
#include <cstring>
#include <unordered_map>
#include <utility>

#include "cmCursesStandardIncludes.h"

bool cmCursesColor::HasColors()
{
#ifdef HAVE_CURSES_USE_DEFAULT_COLORS
  return has_colors();
#else
  return false;
#endif
}

void cmCursesColor::InitColors()
{
#ifdef HAVE_CURSES_USE_DEFAULT_COLORS
  if (HasColors()) {
    start_color();
    use_default_colors();
    init_pair(BoolOff, GetColor('N', COLOR_RED), -1);
    init_pair(BoolOn, GetColor('Y', COLOR_GREEN), -1);
    init_pair(String, GetColor('S', COLOR_CYAN), -1);
    init_pair(Path, GetColor('P', COLOR_YELLOW), -1);
    init_pair(Choice, GetColor('C', COLOR_MAGENTA), -1);
  }
#endif
}

short cmCursesColor::GetColor(char id, short fallback)
{
  static bool initialized = false;
  static std::unordered_map<char, short> env;

  if (!initialized) {
    if (auto* v = getenv("CCMAKE_COLORS")) {
      while (v[0] && v[1] && v[1] == '=') {
        auto const n = std::toupper(*v);

        char buffer[12];
        memset(buffer, 0, sizeof(buffer));

        if (auto* const e = strchr(v, ':')) {
          if (static_cast<size_t>(e - v) > sizeof(buffer)) {
            break;
          }

          strncpy(buffer, v + 2, static_cast<size_t>(e - v - 2));
          v = e + 1;
        } else {
          auto const l = strlen(v);
          if (l > sizeof(buffer)) {
            break;
          }

          strncpy(buffer, v + 2, l - 2);
          v += l;
        }

        auto const c = atoi(buffer);
        if (c && c < COLORS) {
          env.emplace(n, static_cast<short>(c));
        }
      }
    }
    initialized = true;
  }

  auto const iter = env.find(id);
  return (iter == env.end() ? fallback : iter->second);
}
