#version 330 core
layout (location = 0) in vec3 Pos;
layout (location = 1) in vec3 Tex;

out vec2 TexCoord;

void main()
{
	gl_Position = vec4(Pos, 1.0);
	TexCoord = vec2(Tex.x, Tex.y);
}