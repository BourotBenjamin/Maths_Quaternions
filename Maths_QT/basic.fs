#version 330 core

in vec2 TexCoords;
in vec3 Normal;  
in vec3 FragPos;  
in vec4 FragPosLightSpace;  


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

#define NR_POINT_LIGHTS 
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
uniform sampler2D texture_shininess1;
uniform sampler2D shadowMap;


float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // Check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;
    return shadow;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewPos)
{
    vec3 lightDir = normalize(-light.direction);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec =  pow(max(dot(viewPos, reflectDir), 0.0), texture(texture_shininess1, TexCoords).x);
    vec3 ambient  = light.ambient  * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    float shadow = ShadowCalculation(FragPosLightSpace);  
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));  
    return lighting;
}  

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewPos)
{
    vec3 lightDir = normalize(light.position - fragPos);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec =  pow(max(dot(viewPos, reflectDir), 0.0), texture(texture_shininess1, TexCoords).x);
    float distance    = length(light.position - fragPos);
    float attenuation = 1.0f / (light.constant + light.linear * distance + light.quadratic * (distance * distance));    
    vec3 ambient  = light.ambient  * vec3(texture(texture_diffuse1, TexCoords));
    vec3 diffuse  = light.diffuse  * diff * vec3(texture(texture_diffuse1, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(texture_specular1, TexCoords));
    ambient  *= attenuation;
    diffuse  *= attenuation;
    specular *= attenuation;
    float shadow = ShadowCalculation(FragPosLightSpace);       
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular));  
    return lighting;
} 

void main()
{    
    vec3 resultLights = CalcDirLight(dirLight, Normal, viewPos) * dirLight.coeff;
    //for(int i = 0 ; i < NR_POINT_LIGHTS ; i++)
        resultLights += CalcPointLight(pointLights[0], Normal, FragPos, viewPos) * pointLights[0].coeff;  
		
    vec4 result = vec4((resultLights), 1.0f) * vec4( 1.0f) * texture(texture_diffuse1, TexCoords);
    float gamma = 2.2;
    result = pow(result, vec4(1.0/gamma));
    color = vec4(result);
}