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
in vec2 MapTexCoords;
in vec3 Normal;
in vec3 WorldPos;
in vec3 ColorModifier;

uniform DirLight DirLights[16];
uniform int DirLightSize;
uniform PointLight PointLights[64];
uniform int PointLightCount;
uniform vec4 AmbientLight = vec4(0.0);


uniform vec3 ViewPos;

uniform sampler2D _tile;
uniform vec3 _Tint = vec3(1.0);

uniform vec3 _ViewCenter;

void main() {
    float xx = mod(WorldPos.x, 1.0);
    float zz = mod(WorldPos.z, 1.0);

    vec3 ColorModifier = vec3(xx, 0, zz);

	float distanceToViewCenter = abs(length(vec3(_ViewCenter.x, ViewPos.y, _ViewCenter.z) - WorldPos));
	float sampleMixValue = smoothstep(50, 60, distanceToViewCenter);

	vec3 objectColor = vec3(0.1, 0.3, 0.6);

    vec3 norm = normalize(Normal);

    float diff = max(dot(norm, DirLights[0].Direction), 0.0);

    vec3 diffuse = diff * DirLights[0].Color;

    vec3 result = (AmbientLight.rgb + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}