#version 330 core
out vec4 FragColor;

in vec3     TexCoord;
in vec3     NormalOut;
in vec3	 FragPos;


struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
}; 


uniform sampler2D texture2d;
uniform sampler3D texture3d;

uniform vec3 view_pos;
uniform Light light;
uniform Material material;

uniform bool Is3D;
uniform vec4 Color;


void main()
{
	vec3 light_result = vec3(1.0);
	
	vec4 texture_color; 
	
	if(!Is3D)
	{
		light_result = vec3(1.0, 1.0, 1.0);
		texture_color = texture(texture2d, vec2(TexCoord));
	}
	else
	{
		// ambient
		vec3 ambient = light.ambient * material.ambient;
		
		// diffuse
		vec3 norm = normalize(NormalOut);
		vec3 light_dir = normalize(light.position - FragPos);
		float diff = max(dot(norm, light_dir), 0.0);
		vec3 diffuse = light.diffuse * (diff * material.diffuse);
		
		// specular
		vec3 view_dir = normalize(view_pos - FragPos);
		vec3 reflect_dir = reflect(-light_dir, norm);

		float spec = pow(max(dot(view_dir, reflect_dir), 0.0), material.shininess);
		vec3 specular = light.specular * (spec * material.specular);  
			
		light_result = ambient + diffuse + specular;
		texture_color = texture(texture3d, TexCoord);
	}
	
	if(TexCoord.x < 0)
	{
		FragColor = Color * vec4(light_result, 1.0f);
	}
	else
	{
		FragColor = (texture_color * Color) * vec4(light_result, 1.0f);
	}
	
	 //FragColor = vec4(light_result, 1);
	 //FragColor = vec4(abs(NormalOut), 1.0f);
	
}
