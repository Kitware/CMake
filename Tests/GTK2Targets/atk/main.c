#include <atk/atk.h>

int main(int argc, char *argv[])
{
    guint major = atk_get_major_version();
    guint minor = atk_get_minor_version();
    guint micro = atk_get_micro_version();
    return 0;
}
