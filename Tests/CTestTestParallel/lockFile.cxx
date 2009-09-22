#include <iostream>
#include <fstream>

//if run serially, works fine
//if run in parallel, someone will attempt to delete
//a locked file, which will fail
int main()
{
  std::string fname = "lockedFile.txt";
  std::fstream fout;
  fout.open(fname.c_str(), std::ios::out);

  for(int i = 0; i < 10000; i++)
  {
    fout << "x";
    fout.flush();
  }
  fout.close();
  return std::remove("lockedFile.txt");
}
