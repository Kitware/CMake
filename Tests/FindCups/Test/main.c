#include <cups/cups.h>

int main()
{
  int num_options = 0;
  cups_option_t* options = NULL;

  num_options = cupsAddOption(CUPS_COPIES, "1", num_options, &options);
  cupsFreeOptions(num_options, options);

  return 0;
}
