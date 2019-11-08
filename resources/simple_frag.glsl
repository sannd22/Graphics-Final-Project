#version 330 core 

struct Light 
{
    vec3 position;
    vec3  direction;
  
    float constant;
    float linear;
    float quadratic;

    float cutOff;
    float outerCutOff;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular; 
};


uniform sampler2D Texture0;
in vec3 fragNor;
in vec3 view;
in vec3 fragPos;

in vec2 vTexCoord;

out vec4 color;


uniform vec3 difCol;
uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float shine;
uniform vec3 lightCol;
uniform vec3 viewPos;


uniform vec3 positionFlash; 
uniform vec3  directionFlash;

uniform float constantFlash;
uniform float linearFlash;
uniform float quadraticFlash;

uniform float cutOff;
uniform float outerCutOff;

uniform vec3 ambFlash;
uniform vec3 difFlash;
uniform vec3 specFlash;

Light setFlash() {
	Light flash;
	flash.position = positionFlash;
	flash.direction = directionFlash;

	flash.constant =  constantFlash;
	flash.linear =  linearFlash;
	flash.quadratic = quadraticFlash;

	flash.ambient = ambFlash;
	flash.diffuse = difFlash;
	flash.specular = specFlash;
	
	flash.cutOff = cutOff;
	flash.outerCutOff = outerCutOff;

	return flash;
}

vec3 CalcFlashLight(Light light, vec3 normal, vec3 fragPos, vec3 viewDir) {
    
    vec3 lightDir = normalize(light.position - fragPos);
    // diffuse shading
    float diff = max(dot(normal, lightDir), 0.0);
    // specular shading
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shine);
    // attenuation
    float distance  = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + 
  			     light.quadratic * (distance * distance));    

    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

	//intensity = 1.0;

	  // combine results
    vec3 ambient1  = light.ambient;// * vec3(1, 0.54, 0);
    vec3 diffuse1 = light.diffuse;//  * diff * vec3(1, 0.54, 0);;
    vec3 specular1 = light.specular * spec;// * vec3(1, 0.54, 0);;
    vec3 tex = vec3(texture(Texture0, vTexCoord));
    if (tex.x != 0 || tex.y != 0|| tex.z != 0) {
    	ambient1 *= attenuation * intensity * vec3(1, 1, 1) * tex;
    	diffuse1 *= attenuation * intensity * vec3(1, 1, 1) * vec3(texture(Texture0, vTexCoord));
    	specular1 *= attenuation * intensity * vec3(1, 1, 1) * vec3(texture(Texture0, vTexCoord));
    } else {
    	ambient1 *= attenuation * intensity * vec3(1, 1, 1);
    	diffuse1 *= attenuation * intensity * vec3(1, 1, 1);
    	specular1 *= attenuation * intensity * vec3(1, 1, 1);
    }

    
    return (ambient1 + diffuse1 + specular1);
} 

void main()
{
	vec3 viewDir1 = normalize(viewPos - fragPos);
	vec4 texColor0 = texture(Texture0, vTexCoord);
	vec3 normal = normalize(fragNor);
	Light flash = setFlash();

	vec4 FLASH1 = vec4(CalcFlashLight(flash, normal, fragPos, viewDir1), 1.0);

	vec3 lightDir = vec3(0.0);
	vec3 light = normalize(lightDir);
	vec3 halfwayDir = normalize(light + view);
	

	if (texColor0.r == 0 && texColor0.g == 0 && texColor0.b == 0) {
		color = vec4(dot(MatAmb, lightCol) + (MatDif * max(0, dot(normal, light))*lightCol) + (MatSpec * pow(max(0, dot(normal, halfwayDir)), shine) * lightCol), 1.0)/3.5;
		color += FLASH1*1.4;
	} else {
		color = texColor0 * vec4(dot(MatAmb, lightCol) + (MatDif * max(0, dot(normal, light))*lightCol) + (MatSpec * pow(max(0, dot(normal, halfwayDir)), shine) * lightCol), 1.0)/3.5;
		color += FLASH1*1.4;
	}

	
}

