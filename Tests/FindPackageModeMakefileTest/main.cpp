#include <stdio.h>
#include <png.h>

int main()
{
 printf("PNG copyright: %s\n", png_get_copyright(NULL));
 return 0;
}
