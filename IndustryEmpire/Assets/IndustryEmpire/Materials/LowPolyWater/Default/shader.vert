#version 330 core

in vec3 a_Position;
in vec2 a_TexCoord;

out vec2 TexCoords1;

uniform mat4 ModelMatrix;
uniform float _Time;

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
	vec4 worldPos = ModelMatrix * vec4(a_Position, 1.0);

	worldPos.y *= 0.0;

	worldPos.y = 0.0 + (cos(a_Position.x + _Time) * sin(a_Position.z + _Time)) * 0.1;

	//worldPos.y = 10 + noise((worldPos.xz / 2.5) + _Time) * 1.8;

	//worldPos.x += noise((worldPos.xz / 2.5) + _Time) * 0.25;
	//worldPos.z += noise((worldPos.zy / 1.5) + _Time) * 0.40;

	gl_Position = worldPos;
	TexCoords1 = a_TexCoord;
}