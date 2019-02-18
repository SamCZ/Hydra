#version 330 core

in vec3 a_Position;
in vec3 a_Normal;
in vec2 a_TexCoord;

#ifdef USE_INSTANCING
in mat4 a_ModelMatrix;
#endif

out vec2 TexCoords;
out vec3 ViewNormal;
out vec3 Normal;
out vec4 WorldPos;
out float DistanceFadeValue;
out vec3 ColorModifier;
out vec4 ClipSpace;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

uniform mat3 NormalMatrix;
uniform mat3 WorldNormalMatrix;

uniform vec3 ViewPos;
uniform vec3 _ViewCenter;

uniform float _Time;

uniform sampler2D _HeightMap;

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))
                 * 43758.5453123);
}

float noise (in vec2 st) {
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

void main() {
#ifdef USE_INSTANCING
	mat4 modelMatrix = a_ModelMatrix * ModelMatrix;
#else
	mat4 modelMatrix = ModelMatrix;
#endif
	
	vec3 pos = vec3(modelMatrix[3][0], modelMatrix[3][1], modelMatrix[3][2]);
	pos += vec3(3000, 0, 3000);
	pos /= 6000.0;

	vec2 heightMapCoords = pos.xz;

	float terrainY = textureLod(_HeightMap, vec2(heightMapCoords.s, heightMapCoords.t), 0).r * 100.0;

	mat4 posMod = mat4(1.0, 0.0, 0.0, 0.0,
                       0.0, 1.0, 0.0, 0.0,
                       0.0, 0.0, 1.0, 0.0,
                       0, terrainY, 0, 1.0);
	
	float windHeightStrength = pow(a_Position.y + 0.5, 1.2) / 10;
	vec3 windMorph = vec3(cos(_Time), 0, sin(_Time));
	windMorph *= windHeightStrength;

	WorldPos = posMod * modelMatrix * vec4(a_Position, 1.0);
	gl_Position = ProjectionMatrix * ViewMatrix * WorldPos;
	TexCoords = a_TexCoord;

	ClipSpace = gl_Position;
	
	mat3 normalMatrix = mat3(transpose(inverse(ViewMatrix * modelMatrix)));
	ViewNormal = normalize(normalMatrix * a_Normal);
	Normal = normalize(WorldNormalMatrix * a_Normal);

	float distanceToCamera = abs(length(WorldPos.xyz - vec3(_ViewCenter.x, ViewPos.y, _ViewCenter.z)));

	DistanceFadeValue = 1.0 - smoothstep(20, 30, distanceToCamera);
	DistanceFadeValue = 1.0;


	// Create world color gradient
	ColorModifier = mix(vec3(0.8), vec3(0.2, 0.7, 0.1), noise(WorldPos.xz));
}