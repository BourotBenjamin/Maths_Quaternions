#version 330 core

in vec2 TexCoords;
in vec3 Normal;  
in vec3 FragPos;  


out vec4 color;
uniform vec3 viewPos;

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	float coeff;
};  
uniform DirLight dirLight;

#define NR_POINT_LIGHTS 1
struct PointLight {
    vec3 position;  
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
	
    float constant;
    float linear;
    float quadratic;
	float coeff;
}; 
uniform PointLight pointLights[NR_POINT_LIGHTS];


uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewPos)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = max(dot(viewPos, reflectDir), 0.0);
    vec3 ambient  = light.ambient  * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    return (ambient + diffuse + specular);
}  

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewPos)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = max(dot(viewPos, reflectDir), 0.0);
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    vec3 ambient  = light.ambient  * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
} 

void main()
{    
    vec3 resultLights = CalcDirLight(dirLight, Normal, viewPos) * dirLight.coeff;
    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        resultLights += CalcPointLight(pointLights[i], Normal, FragPos, viewPos) * pointLights[i].coeff;  
    vec4 result = vec4((resultLights), 1.0f) * vec4(texture(texture_diffuse1, TexCoords));
    color = vec4(result);
}