#version 330 core

in vec3 a_Position;
in vec2 a_TexCoord;

out vec2 TexCoords1;

uniform mat4 ModelMatrix;

void main() {
	vec4 worldPos = ModelMatrix * vec4(a_Position, 1.0);

	worldPos.y = 0.2;

	if(worldPos.x > 1) {
		worldPos.y = -1;
	}

	gl_Position = worldPos;
	TexCoords1 = a_TexCoord;
}