#include <aspell.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
  AspellConfig* config = new_aspell_config();
  assert(config && "Failed creating AspellConfig");

  AspellCanHaveError* result = new_aspell_speller(config);
  delete_aspell_config(config);
  AspellSpeller* speller = to_aspell_speller(result);
  assert(aspell_error_number(result) == 0 && "Failed creating AspellSpeller");

  char const* word = "conjunction";

  if (aspell_speller_check(speller, word, (int)strlen(word))) {
    printf("Word \"%s\" is spelled correctly\n", word);
  } else {
    printf("Word \"%s\" is misspelled\n", word);
  }

  delete_aspell_speller(speller);

  return 0;
}
