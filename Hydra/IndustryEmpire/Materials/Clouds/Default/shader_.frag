#version 330 core

#define UINT_MAX 4294967295U
#define PI 3.141592653589793

out vec4 FragColor;

in vec2 TexCoords;

uniform mat4 inv_proj;
uniform mat4 inv_view;

uniform sampler2D iChannel0;
uniform vec3 ViewPos;
uniform vec2 view_port;
uniform vec3 LightDir;

uniform float iGlobalTime;


const int _VolumeSteps = 64;
const float _StepSize = 0.05; 
const float _Density = 0.1;
const float _OpacityThreshold = 0.95;

const float _SphereRadius = 100.2;
const float _NoiseFreq = 0.5;
const float _NoiseAmp = 2.0;

const vec4 innerColor = vec4(0.7, 0.7, 0.7, _Density);
const vec4 outerColor = vec4(1.0, 1.0, 1.0, 0.0);


float noise( in vec3 x ) {
	return texture(iChannel0, x.xz / x.y).r;
}

const vec3 sunDir = vec3(-0.666, 0.333, 0.666);

bool isnan( float val ) {
  return ( val < 0.0 || 0.0 < val || val == 0.0 ) ? false : true;
}

vec3 sky(vec3 v) {
	// gradient
	vec3 c = mix(vec3(0.0, 0.5, 1.0), vec3(0, 0.25, 0.5), abs(v.y));
	//vec3 c = mix(vec3(1.0, 0.5, 0.0), vec3(0, 0.5, 1.0), abs(sqrt(v.y)));
	float sun = pow(dot(v, LightDir), 1000.0);
	if(!isnan(sun))
		c += sun*vec3(3.0, 2.0, 1.0);
	return c;
}

// Description : Array and textureless GLSL 2D/3D/4D simplex 
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
// 

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

  // First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

  // Other corners
  vec3 g = step(x0.yzx, x0.xyz);	  
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

  // Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

  // Gradients: 7x7 points over a square, mapped onto an octahedron.
  // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

  //Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

  // Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}


float fbm(vec3 p)
{
    float f;
    f = 0.5000*snoise( p ); p = p*2.02;
    f += 0.2500*snoise( p ); p = p*2.03;
    f += 0.1250*snoise( p ); p = p*2.01;
    f += 0.0625*snoise( p );
    return f;
    //return 0.2000*mod289( p );
}

float distanceFunc(vec3 p) {
	p.x -= iGlobalTime;		// translate with time
	//p += snoise(p*0.5)*1.0;	// domain warp!
	
	vec3 q = p;	
	// repeat on grid
	q.xz = mod(q.xz - vec2(20.5), 20.0) - vec2(20.5);
    q.y *= 0.01;	// squash in y
	float d = length(q) - 20;	// distance to sphere

	// offset distance with noise
	//p = normalize(p) * _SphereRadius;	// project noise point to sphere surface
	p.y -= iGlobalTime*0.3;	// animate noise with time
	//d += fbm(p*_NoiseFreq) * _NoiseAmp;
	//d += fbm(p * 0.01) * 10;
	return fbm(p * 0.01) * 10;
}

vec4 shade(float d) {	
	return mix(innerColor, outerColor, smoothstep(0.1, 1.0, d));
}

vec4 volumeFunc(vec3 p) {
	float d = distanceFunc(p);
	vec4 c = shade(d);
	c.rgb *= smoothstep(300, 310, p.y)*0.5+0.5;	// fake shadows
	//float r = length(p)*0.04;
	//c.a *= exp(-r*r);	// fog
	return c;
}

vec4 render(in vec3 pos, in vec3 dir, in ivec2 px ) {
	vec4 sum = vec4(0.0);

	//float yDist = abs(distance(pos.y, 100));

	int len = 150;
	float delta = 10;

	float startY = 300;
	float endY = startY + 50;

	if(dir.y < 0) return sum;

	vec3 hitPos = pos;
	/*for(int i = 0; i < 500; i++) {
		float dist = (i * delta);
		vec3 hitPos = pos + (dir * dist);
		if(hitPos.y > startY) {
			doVolume = true;
		}
	}´*/

	len = 300;
	delta = 2.0;

	for(int i = 0; i < len; i++) {
		float dist = (i * delta);
		vec3 newPos = hitPos + (dir * dist);

		if(sum.a > 0.95) {
			break;
		}

		if(newPos.y > startY && newPos.y < endY) {
			vec4 col = volumeFunc(newPos);
			col.rgb *= col.a;
			sum = sum + col*(1.0 - sum.a);
		}
	}

    return sum;
}

void main() {
	float x = 2.0 * gl_FragCoord.x / view_port.x - 1.0;
	float y = 2.0 * gl_FragCoord.y / view_port.y - 1.0;
	vec2 ray_nds = vec2(x, y);
	vec4 ray_clip = vec4(ray_nds, -1.0, 1.0);
	vec4 ray_view = inv_proj * ray_clip;
	ray_view = vec4(ray_view.xy, -1.0, 0.0);
	vec3 ray_world = (inv_view * ray_view).xyz;
	ray_world = normalize(ray_world);

	vec4 cloud_color = render(ViewPos, ray_world, ivec2(gl_FragCoord.xy-0.5));

	vec4 finalColor = cloud_color;

	//finalColor.rgb += sky(ray_world);
	finalColor += vec4(sky(ray_world), 0)*(1.0 - finalColor.w);
	finalColor.a = 1.0;

    FragColor = finalColor;
}