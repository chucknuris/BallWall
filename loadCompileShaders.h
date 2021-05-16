#ifdef __APPLE__
/* Defined before OpenGL and GLUT includes to avoid deprecation messages */
#define GL_SILENCE_DEPRECATION
#include <OpenGL/gl3.h>
#else
#include <GL/glew.h>
#endif

GLuint loadCompileShader(const char *vShaderName, const char *fShaderName);
GLuint CompileShaders(const char *vertexprogram, const char *fragmentprogram);
