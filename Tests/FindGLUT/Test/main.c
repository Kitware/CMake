#include <GL/glut.h>
#include <stdio.h>

int main()
{
  /* The following should call exit(1) and print
      freeglut  ERROR:  Function <glutCreateWindow> called
      without first calling 'glutInit'.
    to stderr */
  glutCreateWindow("gluttest");
}
