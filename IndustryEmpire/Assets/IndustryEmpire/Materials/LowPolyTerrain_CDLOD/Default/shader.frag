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
in vec2 MapTexCoords;
in vec3 Normal;
in vec3 WorldPos;
in vec3 ColorModifier;

uniform DirLight DirLights[16];
uniform int DirLightSize;
uniform PointLight PointLights[64];
uniform int PointLightCount;
uniform vec4 AmbientLight = vec4(0.0);


uniform float ScaleY = 0.0;
uniform float HeightMapScale;
uniform sampler2D HeightMap;
uniform sampler2D HeightMapNormal;
uniform vec3 ViewPos;

uniform sampler2D _NormalMap;
uniform sampler2D _tile;
uniform vec3 _Tint = vec3(1.0);

uniform vec3 _ViewCenter;


uniform bool _RoadPrewEnabled;
uniform vec3 _RoadPrewStart;
uniform vec3 _RoadPrewEnd;

uniform vec3 _StartRoadPositions[256];
uniform vec3 _EndRoadPositions[256];
uniform int _RoadCount;
uniform float _RoadThickness = 1.0;
uniform float _RoadOuterThickness = 0.1;

uniform vec3 _CursorPos;
uniform bool _ShowCursor;

vec3 GetClosestPointOnLineSegment(vec3 A, vec3 B, vec3 P) {
	vec3 AP = P - A;
    vec3 AB = B - A;

	float magnitudeAB = sqrt(dot(AB, AB));
	float ABAPproduct = dot(AP, AB);

	float distance = ABAPproduct / magnitudeAB;

	if (distance < 0) {
		return A;
	} else if (distance > 1) {
		return B;
	} else {
		return A + AB * distance;
    }
}

float FindDistanceToSegment(vec2 pt, vec2 p1, vec2 p2, out vec2 closest) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    if ((dx == 0) && (dy == 0))
    {
        // It's a point not a line segment.
        closest = p1;
        dx = pt.x - p1.x;
        dy = pt.y - p1.y;
        return sqrt(dx * dx + dy * dy);
    }

    // Calculate the t that minimizes the distance.
    float t = ((pt.x - p1.x) * dx + (pt.y - p1.y) * dy) /
        (dx * dx + dy * dy);

    // See if this represents one of the segment's
    // end points or a point in the middle.
    if (t < 0)
    {
        closest = vec2(p1.x, p1.y);
        dx = pt.x - p1.x;
        dy = pt.y - p1.y;
    }
    else if (t > 1)
    {
        closest = vec2(p2.x, p2.y);
        dx = pt.x - p2.x;
        dy = pt.y - p2.y;
    }
    else
    {
        closest = vec2(p1.x + t * dx, p1.y + t * dy);
        dx = pt.x - closest.x;
        dy = pt.y - closest.y;
    }

    return sqrt(dx * dx + dy * dy);
}

vec3 sampleOffsetDirections[20] = vec3[] (
   vec3( 1,  1,  1), vec3( 1, -1,  1), vec3(-1, -1,  1), vec3(-1,  1,  1), 
   vec3( 1,  1, -1), vec3( 1, -1, -1), vec3(-1, -1, -1), vec3(-1,  1, -1),
   vec3( 1,  1,  0), vec3( 1, -1,  0), vec3(-1, -1,  0), vec3(-1,  1,  0),
   vec3( 1,  0,  1), vec3(-1,  0,  1), vec3( 1,  0, -1), vec3(-1,  0, -1),
   vec3( 0,  1,  1), vec3( 0, -1,  1), vec3( 0, -1, -1), vec3( 0,  1, -1)
);

float ShadowCalculation(vec3 fragPos, vec3 lightPos, samplerCube depthMap) {
    vec3 fragToLight = fragPos - lightPos;
    float currentDepth = length(fragToLight);
    /*float closestDepth = texture(depthMap, fragToLight).r;
    closestDepth *= 500.0;
    float currentDepth = length(fragToLight);
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;*/

    float shadow = 0.0;
    float bias   = 0.005;
    int samples  = 10;
    float far_plane = 500.0;
    float viewDistance = length(ViewPos - fragPos);
    //float diskRadius = (1.0 + (viewDistance / far_plane)) / 25.0;
    float diskRadius = 0.005;
    for(int i = 0; i < samples; ++i)
    {
        float closestDepth = texture(depthMap, fragToLight + sampleOffsetDirections[i] * diskRadius).r;
        closestDepth *= far_plane;
        if(currentDepth - bias > closestDepth)
            shadow += 1.0;
    }
    shadow /= float(samples);
    return shadow;
}

float ComputeDirLightShadow() {
	if(!DirLights[0].HasShadowMap) return 1.0;

	float shadowValue = 0.0;

	vec4 mappedCoords = DirLights[0].ShadowProjection * vec4(WorldPos, 1.0);
	vec3 shadowCoords = (mappedCoords.xyz / mappedCoords.w) * 0.5 + 0.5;

	float closestDepth = texture(DirLights[0].ShadowMap, shadowCoords.xy).r;

	float bias = 0.000001;

	/*if(shadowCoords.z - bias > closestDepth) {
		shadowValue += 1.0;
	}*/

	float currentDepth = shadowCoords.z;

	shadowValue = (currentDepth) > closestDepth  ? 0.0 : 1.0;

	if(shadowCoords.x < 0 || shadowCoords.x > 1 || shadowCoords.y < 0 || shadowCoords.y > 1) {
		return 1.0;
	} else {
		return shadowValue;
	}
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

vec3 getNormalFromMap(sampler2D normMap, vec2 texCoord) {
    vec3 nn = 2.0 * (texture(HeightMapNormal, MapTexCoords.st).rgb) - 1.0;
    vec3 tangentNormal = vec3(0, 1, 0);

    vec3 Q1  = dFdx(WorldPos);
    vec3 Q2  = dFdy(WorldPos);
    vec2 st1 = dFdx(TexCoords);
    vec2 st2 = dFdy(TexCoords);

    vec3 N   = normalize(nn);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
}

void main() {
    vec3 nn = 2.0 * (texture(HeightMapNormal, MapTexCoords.st).rgb) - 1.0;

    vec2 texCoord0 = MapTexCoords * 200;
    vec2 texCoord1 = MapTexCoords * 3000;

	//vec3 nn2 = 2.0 * (texture(_NormalMap, texCoord1.st * 10).rgb) - 1.0;
	//vec3 nn = getNormalFromMap(_NormalMap, MapTexCoords);

    float xx = mod(WorldPos.x, 1.0);
    float zz = mod(WorldPos.z, 1.0);

    vec3 ColorModifier = vec3(xx, 0, zz);

	float distanceToViewCenter = abs(length(vec3(_ViewCenter.x, ViewPos.y, _ViewCenter.z) - WorldPos));
	float sampleMixValue = smoothstep(50, 60, distanceToViewCenter);

    vec3 sample0 = texture(_tile, texCoord0).rgb;
	vec3 sample1 = texture(_tile, texCoord1).rgb;
    

	vec3 objectColor = mix(sample1, sample0, sampleMixValue) * _Tint;

	//objectColor *= mix(vec3(0.8), vec3(0.2, 0.7, 0.1), noise(WorldPos.xz / 20));

	//objectColor *= vec3(0.2, 0.7, 0.1) * 0.5;

    vec3 norm = normalize(nn);

    if(dot(norm, vec3(0, 1, 0)) < 0.5) {
        //objectColor = vec3(70.0 / 255.0, 40 / 255.0, 25 / 255.0);
    }

    if(WorldPos.y < 1.1) {
        //objectColor = vec3(0.2, 0.5, 0.8);
    }


	float roadStripes = 0;
	float roadCorner = 0;
	float roadVal = 0;

	for(int i = 0; i < _RoadCount; i++) {
		vec3 rStart = _StartRoadPositions[i];
		vec3 rEnd = _EndRoadPositions[i];

		if(roadCorner > 0 && roadVal > 0) break;

		vec2 ClosestPoint = vec2(0.0);
		float dist = FindDistanceToSegment(WorldPos.xz, rStart.xz, rEnd.xz, ClosestPoint);

		float distanceToEnd = abs(length(ClosestPoint - rStart.xz));
		float distanceToStart = abs(length(ClosestPoint - rEnd.xz));

		if(dist < 0.05 && (fract(distanceToEnd * 0.5) > 0.5 || distanceToEnd < 0.8 || distanceToStart < 0.8)) {
			objectColor = vec3(0.8, 0.8, 0.8);
			roadStripes += 1;
		} else if(dist < _RoadThickness - _RoadOuterThickness && roadStripes <= 0) {
			objectColor = vec3(0.05, 0.05, 0.05);
			roadVal += 1;
		} else if(dist < _RoadThickness && roadVal <= 0 && roadStripes <= 0) {
			objectColor = vec3(0.2, 0.2, 0.2);
			roadCorner += 1;
		}
	}

	
	if(_RoadPrewEnabled) {
		vec2 ClosestPoint = vec2(0.0);
		float dist = FindDistanceToSegment(WorldPos.xz, _RoadPrewStart.xz, _RoadPrewEnd.xz, ClosestPoint);

		float distanceToEnd = abs(length(ClosestPoint - _RoadPrewStart.xz));

		if(!(roadCorner > 0 && roadVal > 0.5)) {
			if(dist < 0.05 && fract(distanceToEnd * 0.5) > 0.5) {
				objectColor = vec3(0.8, 0.8, 0.8);
				roadStripes += 1;
			} else if(dist < _RoadThickness - _RoadOuterThickness && roadStripes <= 0) {
				objectColor = vec3(0.05, 0.05, 0.05);
				roadVal += 1;
			} else if(dist < _RoadThickness && roadVal <= 0 && roadStripes <= 0) {
				objectColor = vec3(0.2, 0.2, 0.2);
				roadCorner += 1;
			}
		}
	}

	
	if(_ShowCursor) {
		float dist = abs(length(_CursorPos - WorldPos));

		float cursorAlpha = 0.7;

		if(dist < 0.9) {
			objectColor = mix(objectColor, vec3(0.2, 0.5, 0.8), cursorAlpha);
		} else if(dist < 1.0) {
			objectColor = mix(objectColor, vec3(0.3, 0.6, 1.0), cursorAlpha);
		}
	}

	float shadowValue = 1.0;

	/*for(int pl = 0; pl < PointLightCount; pl++) {
		shadowValue += 1.0 - ShadowCalculation(WorldPos, PointLights[pl].Position, PointLights[pl].ShadowMap);
	}*/

	

    float diff = max(dot(norm, DirLights[0].Direction), 0.0);

    vec3 diffuse = diff * DirLights[0].Color * ComputeDirLightShadow();

    vec3 result = (AmbientLight.rgb + diffuse) * objectColor;
    FragColor = vec4(result, 1.0);
}