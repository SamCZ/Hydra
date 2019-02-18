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

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 ModelMatrix;

uniform vec4 ClipPlane;

uniform mat3 NormalMatrix;
uniform mat3 WorldNormalMatrix;

uniform float _Time;

void main() {
#ifdef USE_INSTANCING
	mat4 modelMatrix = a_ModelMatrix * ModelMatrix;
#else
	mat4 modelMatrix = ModelMatrix;
#endif
	WorldPos = modelMatrix * vec4(a_Position, 1.0);
	gl_Position = ProjectionMatrix * ViewMatrix * WorldPos;
	TexCoords = a_TexCoord;
	
	mat3 normalMatrix = mat3(transpose(inverse(ViewMatrix * modelMatrix)));
	ViewNormal = normalize(normalMatrix * a_Normal);
	Normal = normalize(WorldNormalMatrix * a_Normal);

	gl_ClipDistance[0] = dot(WorldPos, ClipPlane);
}