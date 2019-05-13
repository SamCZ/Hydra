#version 330 core

#define RADIUS_PAINT 2

struct RadiusPaint {
	float radius;
	float cutoff;
	float strength;
};

out vec4 FragColor;
in vec2 TexCoords;

uniform vec2 Cursor;
uniform sampler2D ColorMap;
uniform int PaintMode;
uniform RadiusPaint RadPaint;


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
    float color = texture(ColorMap, TexCoords.st).r;
    
    if(PaintMode == RADIUS_PAINT) {
    	float dst = length(Cursor - gl_FragCoord.xy);
    	float attenuation = 1.0 / dst;
    	float v = (1.0 - smoothstep(0, 100, dst)) * RadPaint.strength;
    	color += v;
    }

	color = (noise(TexCoords * 10) + 1.0) / 2.0;

    FragColor.r = color;
	FragColor.g = 0;
	FragColor.b = 0;
	FragColor.a = 1;
}