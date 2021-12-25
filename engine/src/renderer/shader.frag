#version 330 core
out vec4 FragColor;

in vec2     TexCoord;
in vec3     NormalOut;
in vec4     ColorOut;
in vec3     FragPos;
in float    IsAffectedByLight;


struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

struct Material {
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
}; 


uniform sampler2D texture1;
uniform vec3 view_pos;
uniform Light light;
uniform Material material;



void main()
{
	vec3 light_result = vec3(1.0);
	
	if(IsAffectedByLight == 0.0)
	{
		light_result = vec3(1.0, 1.0, 1.0);
	}
	else
	{
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
	}
	
	if(TexCoord.x < 0)
	{
		FragColor = ColorOut * vec4(light_result, 1.0f);
	}
	else
	{
		FragColor = (texture(texture1, TexCoord) * ColorOut) * vec4(light_result, 1.0f);
	}
}
