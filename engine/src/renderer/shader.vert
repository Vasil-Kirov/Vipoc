#version 330 core
layout (location = 0) in vec4 Pos;
layout (location = 1) in vec4 Tex;
layout (location = 2) in vec4 Color;
layout (location = 3) in vec4 Normal;
layout (location = 4) in vec3 WorldPosition;

out vec2 TexCoord;
out vec3 NormalOut;
out vec4 ColorOut;
out vec3 FragPos;

uniform mat4 MVP;


void main()
{
	// @Note: OpenGL wants matrices to be column major
	mat4 Translation = mat4(1.0, 				0.0, 				0.0, 				0,
							0.0, 				1.0, 				0.0, 				0,
							0.0, 				0.0, 				1.0, 				0,
							WorldPosition.x, 	WorldPosition.y, 	WorldPosition.z, 	1.0);
	vec4 position = Translation * Pos;

	gl_Position = MVP * position;

	TexCoord = vec2(Tex.x, Tex.y);
	NormalOut = vec3(Normal.x, Normal.y, Normal.z);
	ColorOut = Color;
	FragPos = vec3(position.x, position.y, position.z);
}