#include <iostream>
#include <oct.h>
#include <octave.h>
#include <parse.h>
#include <toplev.h>

int main(void)
{
  string_vector argv(2);
  argv(0) = "embedded";
  argv(1) = "-q";

  try {
    octave_main(2, argv.c_str_vec(), 1);
    octave_value_list in;
    in(0) = 72.0;
    const octave_value_list result = feval("sqrt", in);
    std::cout << "result is " << result(0).scalar_value() << std::endl;
    clean_up_and_exit(0);
  } catch (const octave::exit_exception& ex) {
    std::cerr << "Octave interpreter exited with status = " << ex.exit_status()
              << std::endl;
  } catch (const octave::execution_exception&) {
    std::cerr << "error encountered in Octave evaluator!" << std::endl;
  }
}
