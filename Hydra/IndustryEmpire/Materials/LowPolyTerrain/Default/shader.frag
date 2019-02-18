#version 330 core

struct DirLight {
    vec3 Direction;
    vec3 Position;
    vec3 Color;
    float Intensity;
	bool HasShadowMap;
    sampler2D ShadowMap;
	mat4 ShadowProjection;
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
in vec3 ColorModifier;

uniform DirLight DirLights[16];
uniform int DirLightSize;
uniform PointLight PointLights[64];
uniform int PointLightCount;
uniform vec4 AmbientLight = vec4(0.0);

uniform float _terrainSize;
uniform sampler2D _tile;
uniform sampler2D _NormalMap;

uniform float _TerrainMinY;

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * 43758.5453123);
}

float noise(in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    // Smooth Interpolation

    // Cubic Hermine Curve.  Same as SmoothStep()
    vec2 u = f*f*(3.0-2.0*f);
    // u = smoothstep(0.,1.,f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

float ComputeDirLightShadow() {
	if(!DirLights[0].HasShadowMap) return 1.0;

	float shadowValue = 0.0;

	vec4 mappedCoords = DirLights[0].ShadowProjection * vec4(WorldPos.xyz, 1.0);
	vec3 shadowCoords = (mappedCoords.xyz / mappedCoords.w) * 0.5 + 0.5;

	float closestDepth = texture(DirLights[0].ShadowMap, shadowCoords.xy).r;

	float bias = 0.0001;

	//float bias = max(0.05 * (1.0 - dot(Normal, DirLights[0].Direction)), 0.005);  

	/*if(shadowCoords.z - bias > closestDepth) {
		shadowValue += 1.0;
	}*/

	float currentDepth = shadowCoords.z;

	shadowValue = (currentDepth - bias) > closestDepth  ? 0.0 : 1.0;

	if(shadowCoords.x < 0 || shadowCoords.x > 1 || shadowCoords.y < 0 || shadowCoords.y > 1) {
		return 1.0;
	} else {
		return shadowValue;
	}
}

void main() {
	vec2 texCoord = TexCoords * 400;

	vec3 objectColor = texture(_tile, texCoord).xyz;

	objectColor = mix(objectColor, vec3(103.0 / 255, 78.0 / 255, 45.0 / 255), smoothstep(-1.5, 1, WorldPos.x));

	//objectColor *= noise(WorldPos.xz * 0.5);

	vec3 nn = 2.0 * (texture(_NormalMap, TexCoords.st * 1.0).rgb) - 1.0;
	vec3 norm = normalize(nn);

	if(dot(norm, vec3(0, 1, 0)) < 0.8) {
		//objectColor = vec3(70.0 / 255.0, 40 / 255.0, 25 / 255.0);
	}

	if(WorldPos.y < 1.1) {
		//objectColor = vec3(0.2, 0.5, 0.8);
	}

	float diff = max(dot(norm, DirLights[0].Direction), 0.0);
	vec3 diffuse = diff * DirLights[0].Color * ComputeDirLightShadow();

	vec3 result = (AmbientLight.rgb + diffuse) * objectColor;
	FragColor = vec4(result, 1.0);


	//FragColor.rgb = vec3(0);
}