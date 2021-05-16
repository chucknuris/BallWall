#include <vector>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

#include "loadCompileShaders.h"

// from https://www.khronos.org/opengl/wiki/Example/GLSL_Full_Compile_Linking
GLuint CompileShaders(const char *vertexprogram, const char *fragmentprogram)
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER); // Create an empty vertex shader handle
	
	glShaderSource(vertexShader, 1, &vertexprogram, NULL );
	glCompileShader(vertexShader); // Compile the vertex shader

	GLint isCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if(isCompiled == GL_FALSE) {
		cout << "\nVertex Shader compilation failed..." << '\n';
		GLint maxLength = 0;
		glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &maxLength);
		vector<char> infoLog(maxLength);
		glGetShaderInfoLog(vertexShader, maxLength, &maxLength, &infoLog[0]);
		cout << &infoLog[0];
		cout << vertexprogram;
		// We don't need the shader anymore.
		glDeleteShader(vertexShader);
		return 0;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER); // Create an empty fragment shader handle
	glShaderSource(fragmentShader, 1, &fragmentprogram, NULL);
	glCompileShader(fragmentShader); // Compile the fragment shader

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE) {
		cout << "\nFragment Shader compilation failed...\n";
		GLint maxLength = 0;
		glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &maxLength);
		vector<char> infoLog(maxLength);
		glGetShaderInfoLog(fragmentShader, maxLength, &maxLength, &infoLog[0]);
		cout << &infoLog[0];
		cout << fragmentprogram;
		// We don't need the shader anymore.
		glDeleteShader(fragmentShader);
		// Either of them. Don't leak shaders.
		glDeleteShader(vertexShader);
		return 0;
	}

	// Vertex and fragment shaders are successfully compiled.
	// Now time to link them together into a program.
	// Get a program object.
	GLuint program = glCreateProgram();

	// Attach our shaders to our program
	glAttachShader(program, vertexShader);
	glAttachShader(program, fragmentShader);

	// link the objects for an executable program
	glLinkProgram(program);

	// Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(program, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE) {
		cout << "Link failed...\n";
		// We don't need the program anymore.
		glDeleteProgram(program);
		// Don't leak shaders either.
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		return 0;
	}

	// Always detach shaders after a successful link.
	glDetachShader(program, vertexShader);
	glDetachShader(program, fragmentShader);

	// return the program
	return program;
}


int read_file_into_string(const char *filename, string &s)
{
	ifstream in(filename, ios::binary);
  if (in)
  {
		in.seekg(0, ios::end); // go to end of file
		size_t size = in.tellg(); // size of file
		s.resize(size, 0);
		in.seekg(0); // go to beginning
		in.read(&s[0], size);
    return 0;
  }
  return -1;
}


GLuint loadCompileShader(const char *vShaderName, const char *fShaderName) {
	string vscode, fscode;
	if (read_file_into_string(vShaderName, vscode)) { // read the vertex shader code file
		cout << "Error, file " << vShaderName << " not found.\n";
		return 0;
	}
	if (read_file_into_string(fShaderName, fscode)) { // read the fragment shader code file
		cout << "Error, file " << fShaderName << " not found.\n";
		return 0;
	}
	return CompileShaders(vscode.c_str(), fscode.c_str());
}

