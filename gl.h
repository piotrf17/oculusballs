#include <string>

#include <GL/glew.h>
#include <GL/glx.h>

// A structure capturing all the state for an OpenGL window through GLX.
struct GLWindow {
  Display                *dpy;
  int                     screen;
  Window                  win;
  GLXContext              ctx;
  XSetWindowAttributes    attr;
  unsigned int            bpp;
  int                     x,y;
  unsigned int            width, height;
};

GLWindow* CreateGLWindow(const int width, const int height,
                         const std::string& title);

void DestroyGLWindow(GLWindow* gl_win);
