#version 330 core
layout (location = 0) in vec3 	Pos;
layout (location = 1) in vec2 	Tex;
layout (location = 2) in vec4 	Normal;
layout (location = 3) in vec3 	WorldPosition;

out vec2 	TexCoord;
out vec3 	NormalOut;
out vec3 	FragPos;

uniform mat4 MVP;
uniform bool Is3D;

void main()
{
	// @Note: OpenGL wants matrices to be column major
	
	vec4 position;
	position.x = Pos.x + WorldPosition.x;
	position.y = Pos.y + WorldPosition.y;
	position.z = Pos.z + WorldPosition.z;
	position.w = 1.0;
	
	if(Is3D)
	{
		gl_Position = MVP * position;
	}
	else
	{
		gl_Position = position;
	}
	
	TexCoord = vec2(Tex.x, Tex.y);
	NormalOut = vec3(Normal.x, Normal.y, Normal.z);
	FragPos = vec3(position.x, position.y, position.z);
}