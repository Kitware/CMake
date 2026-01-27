#include <format>
#include <string>
#include <string_view>

int main()
{
  std::string s{ std::format("inspect") };
  std::string_view v{ s };
}
