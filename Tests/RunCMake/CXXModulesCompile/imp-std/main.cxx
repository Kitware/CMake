import std;

int main(int argc, char* argv[])
{
  // std::chrono::current_zone() references _Global_tzdb_list, which is
  // provided by std.ixx.obj on MSVC. Ensure we get a failed link if this
  // definition isn't available.
  static_cast<void>(std::chrono::current_zone());

  if (argc > 0 && argv[0]) {
    std::string argv0 = argv[0];
    std::cout << "program: " << argv0 << std::endl;
  }
  return 0;
}
