#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;

out vec3 fragNor;
out vec2 vTexCoord;
out vec3 view;
out vec3 fragPos;

void main()
{
	fragPos = vec3(M * vertPos);
	fragNor = (M * vec4(vertNor, 0.0)).xyz;
    //lightDir = lightPos - (M*vertPos).xyz;
    view = -1 * (M * vertPos).xyz;
    vTexCoord = vertTex;
    gl_Position = P * V * M * vertPos;
}
