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
in vec3 ViewNormal;
in vec3 Normal;
in vec4 WorldPos;
in vec3 ColorModifier;
in float DistanceFadeValue;
in vec4 ClipSpace;

uniform vec3 ViewPos;

uniform sampler2D _DepthMap;
uniform sampler2D _SceneColor0;

uniform DirLight DirLights[16];
uniform int DirLightSize;
uniform PointLight PointLights[64];
uniform int PointLightCount;
uniform vec4 AmbientLight = vec4(0.0);

uniform vec3 _Color;
uniform vec3 _GrassTint;

uniform sampler2D _Texture;

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * 43758.5453123);
}

void main() {
	float diff = max(dot(vec3(0, 1, 0), DirLights[0].Direction), 0.2);

	FragColor.rgba = texture(_Texture, TexCoords);
	FragColor.rgb = diff * _GrassTint * ColorModifier;

	if(FragColor.a < 0.5) {
		//discard;
	}

	float near = 0.5;
	float far = 10000.0;

	vec2 ndc = (ClipSpace.xy / ClipSpace.w) / 2.0 + 0.5;
	float nonLinearDepth = texture(_DepthMap, vec2(ndc.s, ndc.t)).r;
	float depth = 2.0 * near * far / (far + near - (2.0 * nonLinearDepth - 1.0) * (far - near));

	if(gl_FragCoord.z > nonLinearDepth) {
		discard;
	}

	float fade = smoothstep(0, 0.2, TexCoords.t);
	float noiseValue = random(TexCoords);
	fade = mix(fade, fade * noiseValue, 1.0 - smoothstep(0.1, 0.3, TexCoords.t));

	FragColor.a = DistanceFadeValue * fade * FragColor.a;
}