#version 330 core
layout (location = 0) in vec4 Pos;
layout (location = 1) in vec4 Tex;
layout (location = 2) in vec4 Color;

out vec2 TexCoord;
out vec4 ColorOut;

uniform mat4 MVP;


void main()
{
	gl_Position = MVP * Pos;
	TexCoord = vec2(Tex.x, Tex.y);
	ColorOut = Color;
}