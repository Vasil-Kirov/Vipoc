#version 330 core
layout (location = 0) in vec3 	Pos;
layout (location = 1) in vec2 	Tex;
layout (location = 2) in vec3 	Normal;

out vec2 	TexCoord;
out vec3 	NormalOut;
out vec3 	FragPos;

uniform mat4 MVP;
uniform bool Is3D;

vec4 points[] = {vec4(-0.5, 0.0, 1, 1), vec4(0.5, 0.0, 1, 1), vec4(0.0, 0.5, 1, 1)};

void main()
{
	// @Note: OpenGL wants matrices to be column major
	
	vec4 position = vec4(Pos, 1.0);
	
	if(Is3D)
	{
		gl_Position = MVP * position;
	}
	else
	{
		gl_Position = position;
	}
	
	gl_Position = points[gl_VertexID];
	TexCoord = vec2(Tex.x, Tex.y);
	NormalOut = Normal;
	FragPos = vec3(position.x, position.y, position.z);
}