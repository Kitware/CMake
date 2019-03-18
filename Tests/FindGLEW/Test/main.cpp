#include <GL/glew.h>

int main()
{
  GLenum init_return = glewInit();

  return (init_return == GLEW_OK);
}
