#version 330 core

struct DirLight {
    vec3 Direction;
    vec3 Position;
    vec3 Color;
    float Intensity;
};

struct PointLight {
    vec3 Position;
    vec3 Color;
    float Radius;
    bool HasShadowMap;
    samplerCube ShadowMap;
};

out vec4 FragColor;

in vec2 TexCoords;
in vec3 Normal;
in vec3 WorldPos;
in vec4 ClipSpace;
in vec3 ColorModifier;

uniform DirLight DirLights[16];
uniform int DirLightSize;
uniform PointLight PointLights[64];
uniform int PointLightCount;
uniform vec4 AmbientLight = vec4(0.0);

uniform float _terrainSize;
uniform sampler2D _tile;
uniform vec3 ViewPos;

uniform sampler2D ReflectionMap;
uniform sampler2D RefractionMap;
uniform sampler2D RefractionDepthMap;
uniform sampler2D DUDVMap;
uniform sampler2D NormalMap;

uniform float waveStrength = 0.02;
uniform float ShineDamper = 20.0;
uniform float Reflectivity = 0.6;

float MoveFactor = 1.0;

void main() {
	vec2 ndc = (ClipSpace.xy / ClipSpace.w) / 2.0 + 0.5;

	vec2 reflectTexCoords = vec2(ndc.x, -ndc.y);
	vec2 refractionTexCoords = vec2(ndc.x, ndc.y);

	float near = 0.5;
	float far = 1000.0;
	float depth = texture(RefractionDepthMap, refractionTexCoords).r;
	float floorDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

	depth = gl_FragCoord.z;
	float waterDistance = 2.0 * near * far / (far + near - (2.0 * depth - 1.0) * (far - near));

	float waterDepth = floorDistance - waterDistance;


	vec2 distortedTexCoords = texture(DUDVMap, vec2(TexCoords.x + MoveFactor, TexCoords.y)).rg*0.1;
	distortedTexCoords = TexCoords + vec2(distortedTexCoords.x, distortedTexCoords.y+MoveFactor);
	vec2 totalDistortion = (texture(DUDVMap, distortedTexCoords).rg) * waveStrength;

	reflectTexCoords += totalDistortion * 0.5;
	reflectTexCoords.x = clamp(reflectTexCoords.x, 0.001, 0.999);
	reflectTexCoords.y = clamp(reflectTexCoords.y, -0.999, -0.001);
		
	refractionTexCoords += totalDistortion;
	refractionTexCoords = clamp(refractionTexCoords, 0.001, 0.999);

	vec4 reflectionColor = texture2D(ReflectionMap, reflectTexCoords);
	vec4 refrationColor = texture2D(RefractionMap, refractionTexCoords);

	vec3 normalMapColor = texture(NormalMap, totalDistortion).rgb;
	vec3 normal = vec3(normalMapColor.r * 2.0 - 1.0, normalMapColor.g, normalMapColor.b * 2.0 - 1.0);
	normal = normalize(normalMapColor);

	vec3 objectColor = vec3(76.0 / 255.0, 139.0 / 255.0, 144.0 / 255.0);

	vec3 nn = 2.0 * (texture(NormalMap, TexCoords.st * 2).rgb) - 1.0;

	vec3 norm = normalize(nn);
	float diff = max(dot(norm, DirLights[0].Direction), 0.0);
	vec3 diffuse = diff * DirLights[0].Color;
	//Specular
	float specularStrength = 0.5;

	vec3 viewDir = normalize(ViewPos - WorldPos);
	vec3 reflectDir = reflect(-DirLights[0].Direction, norm);  

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 126);
	vec3 specular = specularStrength * spec * DirLights[0].Color;  

	vec3 viewVector = normalize(ViewPos - WorldPos);
	float refractiveFactor = dot(viewVector, vec3(0, 1.0, 0));

	vec3 color = mix(reflectionColor.rgb, refrationColor.rgb, refractiveFactor) * (diffuse + specular) * ColorModifier * vec3(0.2, 0.6, 0.8);

	vec3 leftColor = vec3(22.0 / 255.0,28 / 255.0,36 / 255.0);
	vec3 rightColor = vec3(28.0 / 255.0,41.0 / 255.0,52.0 / 255.0);

	//color *= mix(leftColor, rightColor, smoothstep(0, 10, WorldPos.x));

	float mm = dot(Normal, vec3(0, 1, 0));

	//color = mix(color, vec3(1.0), mm);

	//color *= mm;

	FragColor = vec4(color, clamp(waterDepth / 0.5, 0.0, 1.0));
	/*vec2 texCoord = TexCoords * _terrainSize;

	vec3 objectColor = vec3(76.0 / 255.0, 139.0 / 255.0, 144.0 / 255.0);

	vec3 norm = normalize(Normal);

	float diff = max(dot(norm, DirLights[0].Direction), 0.0);
	vec3 diffuse = diff * DirLights[0].Color;

	//Specular
	float specularStrength = 0.5;

	vec3 viewDir = normalize(ViewPos - WorldPos);
	vec3 reflectDir = reflect(-DirLights[0].Direction, norm);  

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	vec3 specular = specularStrength * spec * DirLights[0].Color;  

	vec3 result = (AmbientLight.rgb + diffuse + specular) * objectColor;
	FragColor = vec4(result, 1.0);*/
}