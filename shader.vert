#version 330 compatibility
// adapted from the shader code in main.cpp file, http://www.songho.ca/opengl/files/sphereShader.zip

layout (location = 0) in vec3 vertexPosition;
layout (location = 1) in vec3 vertexNormal;

uniform mat4 matrixModelViewProjection;
uniform mat4 matrixModelView;
uniform mat3 matrixNormal;
uniform vec3 displacement;

out vec3 esVertex, esNormal;
out float z;

void main()
{
    vec4 newposition = vec4(vertexPosition + displacement, 1.0);
    esVertex = vec3(matrixModelView * newposition);
    esNormal = matrixNormal * vertexNormal;
    z = -newposition.z;
    gl_Position = matrixModelViewProjection * newposition;
}
