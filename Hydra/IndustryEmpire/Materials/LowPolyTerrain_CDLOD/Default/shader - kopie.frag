#version 330 core

struct DirLight {
    vec3 Direction;
    vec3 Position;
    vec3 Color;
};

out vec4 FragColor;
in vec2 TexCoords;
in vec3 Normal;
in vec3 WorldPos;
in float DistanceToVertex;

uniform DirLight DirLights[16];
uniform int DirLightSize;
uniform vec3 ViewPos;

uniform sampler2D Texture;

uniform sampler2D HeightMap;
uniform sampler2D HeightMapNormal;

void main() {
	vec3 mapNormal = 2.0 * (texture(HeightMapNormal, TexCoords.st).rgb) - 1.0;

	vec3 nearColor = texture(Texture, TexCoords * 8000).rgb;
	vec3 farColor = texture(Texture, TexCoords * 100).rgb;

	float changeDst = 100.0;
	float gradientSize = 25.0;

	float blend = smoothstep(changeDst - gradientSize, changeDst + gradientSize, DistanceToVertex);
    //float blend = 0.0;

    FragColor = vec4(mix(nearColor, farColor, blend), 1.0);
    
    float brightness = dot(mapNormal, DirLights[0].Direction);
    vec3 diffuse = brightness * DirLights[0].Color;

    FragColor.rgb *= diffuse;
}