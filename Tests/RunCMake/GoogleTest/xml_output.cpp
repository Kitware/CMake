#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char** argv)
{
  // Note: GoogleTestXML.cmake doesn't actually depend on Google Test as such;
  // it only mimics the output file creation using the path passed to this
  // test without any content
  for (int i = 0; i < argc; i++) {
    std::string param(argv[i]);
    if (param.find("--gtest_list_tests") != std::string::npos) {
      // This actually defines the name of the file passed in the 2nd run
      std::cout << "GoogleTestXML." << std::endl;
      std::cout << "  Foo" << std::endl;
      // When changing these names, make sure to adapt the folder creation
      // in GoogleTestXML.cmake
      std::cout << "GoogleTestXMLSpecial/cases." << std::endl;
      std::cout << "  case/0  # GetParam() = 42" << std::endl;
      std::cout << "  case/1  # GetParam() = \"string\"" << std::endl;
      std::cout << "  case/2  # GetParam() = \"path/like\"" << std::endl;
    } else if (param.find("--gtest_output=xml:") != std::string::npos) {
      std::string::size_type split = param.find(":");
      std::string filepath = param.substr(split + 1);
      // The full file path is passed
      std::ofstream ostrm(filepath.c_str(), std::ios::binary);
      if (!ostrm) {
        std::cerr << "Failed to create file: " << filepath.c_str()
                  << std::endl;
        return 1;
      }
      ostrm << "--gtest_output=xml: mockup file\n";
    }
  }

  return 0;
}
