#version 330 compatibility
// adapted from the shader code in main.cpp file, http://www.songho.ca/opengl/files/sphereShader.zip

uniform vec4 lightPosition;             // should be in the eye space
uniform vec4 lightAmbient;              // light ambient color
uniform vec4 lightDiffuse;              // light diffuse color
uniform vec4 lightSpecular;             // light specular color
uniform vec4 materialColor;             // material ambient and diffuse color
uniform vec4 materialSpecular;          // material specular color
uniform float materialShininess;        // material specular shininess

in vec3 esVertex, esNormal;
in float z;

out vec4 vFragColor;

void main()
{
    vec3 normal = normalize(esNormal);
    vec3 light;
    if(lightPosition.w == 0.0)
    {
        light = normalize(lightPosition.xyz);
    }
    else
    {
        light = normalize(lightPosition.xyz - esVertex);
    }
    vec3 view = normalize(-esVertex);
    vec3 halfv = normalize(light + view);

    vec3 color = lightAmbient.rgb * materialColor.rgb;        // begin with ambient
    float dotNL = max(dot(normal, light), 0.0);

    color += lightDiffuse.rgb * materialColor.rgb * dotNL;    // add diffuse
    float dotNH = max(dot(normal, halfv), 0.0);
    color += pow(dotNH, materialShininess) * lightSpecular.rgb * materialSpecular.rgb; // add specular
		
		vec3 fogColor = vec3(0.0, 0.0, 0.0);
		color = mix(color, fogColor, smoothstep(-8.0, 27.0, z));

    // set frag color
    vFragColor = vec4(color, materialColor.a);
}
