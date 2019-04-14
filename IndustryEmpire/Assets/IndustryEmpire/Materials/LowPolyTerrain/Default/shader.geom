#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec2 TexCoords1[];

out vec2 TexCoords;
out vec3 Normal;
out vec3 WorldPos;
out vec3 ColorModifier;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

uniform float _TerrainMinY;
uniform float _TerrainMaxY;

vec3 calcNormal(vec4 dp1, vec4 dp2, vec4 dp3) {
	vec3 v0 = gl_in[0].gl_Position.xyz + dp1.xyz;
	vec3 v1 = gl_in[1].gl_Position.xyz + dp2.xyz;
	vec3 v2 = gl_in[2].gl_Position.xyz + dp3.xyz;

	// edges of the face/triangle
    vec3 U = v1 - v0;
    vec3 V = v2 - v0;
    
    float nx = (U.y * V.z) - (U.z * V.y);
    float ny = (U.z * V.x) - (U.x * V.z);
    float nz = (U.x * V.y) - (U.y * V.x);

    return normalize(vec3(nx, ny, nz));
}

float map(float value, float min1, float max1, float min2, float max2) {
	// Convert the current value to a percentage
	// 0% - min1, 100% - max1
	float perc = (value - min1) / (max1 - min1);

	// Do the same operation backwards with min2 and max2
	return perc * (max2 - min2) + min2;
}

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
	vec3 normal = calcNormal(gl_in[0].gl_Position, gl_in[1].gl_Position, gl_in[2].gl_Position);
	vec3 center = (gl_in[0].gl_Position.xyz + gl_in[1].gl_Position.xyz + gl_in[2].gl_Position.xyz) / 3.0;

	float heightPercent = map(center.y, _TerrainMinY, _TerrainMaxY, 0.0, 2.0);

	heightPercent = map(noise(vec2(center.x, center.z) * 100), -0.5, 0.5, 0.8, 1.0);

	vec3 color = mix(vec3(0.5), vec3(1.0), heightPercent);

	for (int i = 0; i < gl_in.length(); ++i) {
		vec4 vertexPos = gl_in[i].gl_Position;
		gl_Position = ProjectionMatrix * ViewMatrix * vertexPos;
		TexCoords = TexCoords1[i];
		Normal = normal;
		WorldPos = vertexPos.xyz;
		ColorModifier = color;

		EmitVertex();
	}
	EndPrimitive();
}