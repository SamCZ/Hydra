#version 330 core

in vec3 a_Position;
out vec2 TexCoord2;

uniform mat4 ProjectionMatrix;
uniform mat4 ModelMatrix;
uniform vec3 ViewPos;

uniform int Lod;
uniform vec2 Index;
uniform float ScaleY = 0.0;
uniform float Gap;
uniform vec2 Location;
uniform int LodMorphArea[8];

uniform sampler2D HeightMap;

float morphLatitude(vec2 position){
	vec2 frac = position - Location;
	
	if (Index == vec2(0,0)){
		float morph = frac.x - frac.y;
		if (morph > 0)
			return morph;
	}
	if (Index == vec2(1,0)){
		float morph = Gap - frac.x - frac.y;
		if (morph > 0)
			return morph;
	}
	if (Index == vec2(0,1)){
		float morph = frac.x + frac.y - Gap;
		if (morph > 0)
			return -morph;
	}
	if (Index == vec2(1,1)){
		float morph = frac.y - frac.x;
		if (morph > 0)
			return -morph;
	}
	return 0;
}

float morphLongitude(vec2 position) {
	vec2 frac = position - Location;
	
	if (Index == vec2(0,0)){
		float morph = frac.y - frac.x;
		if (morph > 0)
			return -morph;
	}
	else if (Index == vec2(1,0)){
		float morph = frac.y - (Gap - frac.x);
		if (morph > 0)
			return morph;
	}
	else if (Index == vec2(0,1)){
		float morph = Gap - frac.y - frac.x;
		if (morph > 0)
			return -morph;
	}
	else if (Index == vec2(1,1)){
		float morph = frac.x - frac.y;
		if (morph > 0)
			return morph;
	}
	return 0;
}

vec2 morph(int morph_area){

	vec2 morphing = vec2(0,0);
	
	vec2 fixPointLatitude;
	vec2 fixPointLongitude;
	float distLatitude;
	float distLongitude;
	
	if(Index == vec2(0,0)){
		fixPointLatitude = Location + vec2(Gap,0);
		fixPointLongitude = Location + vec2(0,Gap);
	}
	else if(Index == vec2(1,0)){
		fixPointLatitude = Location;
		fixPointLongitude = Location + vec2(Gap,Gap);
	}
	else if(Index == vec2(0,1)){
		fixPointLatitude = Location + vec2(Gap,Gap);
		fixPointLongitude = Location;
	}
	else if(Index == vec2(1,1)){
		fixPointLatitude = Location + vec2(0,Gap);
		fixPointLongitude = Location + vec2(Gap,0);
	}
	
	float planarFactor = ViewPos.y;
	/*if (ViewPos.y > abs(ScaleY)) {
		planarFactor = 1;
	} else {
		planarFactor = ViewPos.y / abs(ScaleY);
	}*/

	distLatitude = length(ViewPos - (ModelMatrix * vec4(fixPointLatitude.x,planarFactor,fixPointLatitude.y,1)).xyz);
	distLongitude = length(ViewPos - (ModelMatrix * vec4(fixPointLongitude.x,planarFactor,fixPointLongitude.y,1)).xyz);
	
	if (distLatitude > morph_area) {
		morphing.x = morphLatitude(a_Position.xz);
	}

	if (distLongitude > morph_area) {
		morphing.y = morphLongitude(a_Position.xz);
	}

	/*if (distLatitude > morph_area)
		morphing.x = morphLatitude(a_Position.xz);
	if (distLongitude > morph_area)
		morphing.y = morphLongitude(a_Position.xz);*/
		
	return morphing;
}

void main() {
	vec2 vertex = a_Position.xz;

	if (Lod > 0) {
		vertex += morph(LodMorphArea[Lod-1]);
	}

	float height = texture(HeightMap, vertex).r * ScaleY;

	TexCoord2 = vertex;
	gl_Position = ModelMatrix * vec4(vertex.x, height, vertex.y, 1.0);
}