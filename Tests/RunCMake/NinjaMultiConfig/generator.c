#include <genex_config.h>
#include <stdio.h>
/* FIXME Get this working */
/*#include <intdir_config.h>*/

const char* generatorlib_genex_config_definition(void);
const char* generatorlib_genex_config_include_dir(void);
const char* generatorobj_genex_config_definition(void);
const char* generatorobj_genex_config_include_dir(void);

static const char contents[] =
  /* clang-format off */
"#include <stdio.h>\n"
"\n"
"#include <genex_config.h>\n"
/* FIXME Get this working */
/*"#include <intdir_config.h>\n"*/
"\n"
"const char* generatorlib_genex_config_definition(void);\n"
"const char* generatorlib_genex_config_include_dir(void);\n"
"const char* generatorobj_genex_config_definition(void);\n"
"const char* generatorobj_genex_config_include_dir(void);\n"
"\n"
"int main(void)\n"
"{\n"
"  printf(\n"
"    \"Generator genex config definition: "
  GENEX_CONFIG_DEFINITION "\\n\"\n"
/* FIXME Get this working */
/*"    \"Generator INTDIR config definition: "
  INTDIR_CONFIG_DEFINITION "\\n\"\n"*/
"    \"Generator genex config include dir: "
  GENEX_CONFIG_INCLUDE_DIR "\\n\"\n"
/* FIXME Get this working */
/*"    \"Generator INTDIR config include dir: "
  INTDIR_CONFIG_INCLUDE_DIR "\\n\"\n"*/
"    \"Generator library genex config definition: %s\\n\"\n"
/* FIXME Get this working */
/*"    \"Generator library INTDIR config definition: %s\\n\"\n"*/
"    \"Generator library genex config include dir: %s\\n\"\n"
/* FIXME Get this working */
/*"    \"Generator library INTDIR config include dir: %s\\n\"\n"*/
"    \"Generator object genex config definition: %s\\n\"\n"
/* FIXME Get this working */
/*"    \"Generator object INTDIR config definition: %s\\n\"\n"*/
"    \"Generator object genex config include dir: %s\\n\"\n"
/* FIXME Get this working */
/*"    \"Generator object INTDIR config include dir: %s\\n\"\n"*/
"    \"Generated genex config definition: \""
  " GENEX_CONFIG_DEFINITION \"\\n\"\n"
/* FIXME Get this working */
/*"    \"Generated INTDIR config definition: \""
  " INTDIR_CONFIG_DEFINITION \"\\n\"\n"*/
"    \"Generated genex config include dir: \""
  " GENEX_CONFIG_INCLUDE_DIR \"\\n\"\n"
/* FIXME Get this working */
/*"    \"Generated INTDIR config include dir: \""
  " INTDIR_CONFIG_INCLUDE_DIR \"\\n\"\n"*/
"    \"Generated library genex config definition: %%s\\n\"\n"
/* FIXME Get this working */
/*"    \"Generated library INTDIR config definition: %%s\\n\"\n"*/
"    \"Generated library genex config include dir: %%s\\n\"\n"
/* FIXME Get this working */
/*"    \"Generated library INTDIR config include dir: %%s\\n\"\n"*/
"    \"Generated object genex config definition: %%s\\n\"\n"
/* FIXME Get this working */
/*"    \"Generated object INTDIR config definition: %%s\\n\"\n"*/
"    \"Generated object genex config include dir: %%s\\n\"\n"
/* FIXME Get this working */
/*"    \"Generated object INTDIR config include dir: %%s\\n\"\n"*/
"    , generatorlib_genex_config_definition()\n"
"    , generatorlib_genex_config_include_dir()\n"
"    , generatorobj_genex_config_definition()\n"
"    , generatorobj_genex_config_include_dir());\n"
"  return 0;\n"
"}\n";
/* clang-format on */

int main(int argc, char** argv)
{
  const char* filename;
  FILE* fout;

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  filename = argv[1];
  if (!(fout = fopen(filename, "w"))) {
    fprintf(stderr, "Could not open %s for writing\n", filename);
    return 1;
  }
  fprintf(fout, contents, generatorlib_genex_config_definition(),
          generatorlib_genex_config_include_dir(),
          generatorobj_genex_config_definition(),
          generatorobj_genex_config_include_dir());
  fclose(fout);

  return 0;
}
