#include <windows.h>

int main(int argc, char** argv) {
   HRSRC hello = ::FindResource(0, "hello", "TEXT");
   if(hello) {
      return 0;
   } else {
      return 1;
   }
}
