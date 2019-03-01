#version 330 core

#define UINT_MAX 4294967295U
#define PI 3.141592653589793

out vec4 FragColor;

in vec2 TexCoords;

uniform mat4 inv_proj;
uniform mat4 inv_view;

uniform sampler2D iChannel0;
uniform samplerCube iChannel1;
uniform vec3 ViewPos;
uniform vec2 view_port;
uniform vec3 LightDir;
uniform float iGlobalTime;
uniform bool UseCubeMap = false;

const mat2 m2 = mat2(0.8,-0.6,0.6,0.8);

float fbm( vec2 p ) {
    float f = 0.0;
    f += 0.5000*texture( iChannel0, p/256.0, -100. ).x; p = m2*p*2.02;
    f += 0.2500*texture( iChannel0, p/256.0, -100. ).x; p = m2*p*2.03;
    f += 0.1250*texture( iChannel0, p/256.0, -100. ).x; p = m2*p*2.01;
    f += 0.0625*texture( iChannel0, p/256.0, -100. ).x;
    return f/0.9375;
}

/*vec3 mapColour (in vec3 pos, in vec3 nor) {    
    float darken = (1.0 - 0.5 * length(normalize(pos)));
    vec3 tint = vec3(.7, .7, .6);
    vec3 texture = texture( iChannel1, 0.006125*pos.xz, -100. ).xyz;
    return  texture * tint;
}*/

bool isnan( float val ) {
  return ( val < 0.0 || 0.0 < val || val == 0.0 ) ? false : true;
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

  if(UseCubeMap) {
    FragColor = texture(iChannel1, ray_world);
    return;
  }

	vec4 cloud_color = vec4(0.0);
	vec4 finalColor = cloud_color;

  float sundot = clamp(dot(ray_world,LightDir),0.0,1.0);
  float nightdot = clamp(dot(vec3(0, -1, 0), LightDir), 0.0, 1.0);
  vec3 blueSky = vec3(0.3, .55, 0.8);
  vec3 redSky = vec3(0.8, 0.8, 0.6);
  //redSky = mix(redSky, blueSky, 1.0 - nightdot);
  vec3 sky = mix(blueSky, redSky, 1.5*pow(sundot, 8.));

  sky = mix(sky, vec3(0.0), nightdot);

  finalColor.rgb =  sky*(1.0-0.8*ray_world.y);

  vec3 ro = vec3(ViewPos.x + iGlobalTime * 100, ViewPos.y - 100000, ViewPos.z + iGlobalTime * 100) / 100.0;

  float s = texture( iChannel0, ray_world.xz * 1.25, -100.0 ).x;
  s += texture( iChannel0, ray_world.xz * 4., -100.0 ).x;
  s = pow(s, 17.0) * 0.00005 * max(ray_world.y, -0.2) * pow((1. - max(sundot, 0.)), 2.);
  if(!isnan(s)) {
    if (s > .0) {
      vec3 backStars = vec3(s);
      backStars = mix(backStars, vec3(0.0), 1.0 - nightdot);
      finalColor.rgb += backStars;
    }
  }

  vec3 sun = vec3(0.0);

  sun += 0.1*vec3(0.9, 0.3, 0.9)*pow(sundot, 0.5);
  sun += 0.2*vec3(1., 0.7, 0.7)*pow(sundot, 1.);
  sun += 0.95*vec3(1.)*pow(sundot, 256.);

  finalColor.rgb += mix(sun, vec3(0.0), nightdot);

  float cloudSpeed = 0.01;
  float cloudFlux = 0.5;

  vec3 cloudColour = mix(vec3(1.0,0.95,1.0), 0.35*redSky,pow(sundot, 2.));
  cloudColour = mix(cloudColour, cloudColour * 0.1, nightdot);

  if(ray_world.y > 0) {
    /*for(int i = 0; i < 10; i++) {
      vec2 sc = cloudSpeed * 50.*iGlobalTime * ro.xz + ray_world.xz*((1000.0 + (i * 10.0))-ro.y)/ray_world.y;
      finalColor.rgb = mix( finalColor.rgb, cloudColour, 0.5*smoothstep(0.5,0.8,fbm(0.0005*sc+fbm(0.0005*sc+iGlobalTime*cloudFlux))));
    }*/
    //sc = cloudSpeed * 30.*iGlobalTime * ro.xz + ray_world.xz*(500.0-ro.y)/ray_world.y;
    //finalColor.rgb = mix( finalColor.rgb, cloudColour, 0.5*smoothstep(0.5,0.8,fbm(0.0002*sc+iGlobalTime*0.1+fbm(0.0005*sc+iGlobalTime*cloudFlux))));
  }

  float pp = pow( 1.-max(ray_world.y+0.1,0.0), 8.0);
  if(!isnan(pp)) {
    vec3 bottomColor = mix( finalColor.rgb, 0.9 * vec3(0.9,0.75,0.8), pp);
    finalColor.rgb = mix(finalColor.rgb, bottomColor, 1.0 - nightdot);
  }

  // contrast
  finalColor.rgb = clamp(finalColor.rgb, 0., 1.);
  finalColor.rgb = finalColor.rgb*finalColor.rgb*(3.0-2.0*finalColor.rgb);
    
    
  // saturation (amplify colour, subtract grayscale)
  float sat = 0.2;
  finalColor.rgb = finalColor.rgb * (1. + sat) - sat*dot(finalColor.rgb, vec3(0.33));
    
  // vignette
  //finalColor.rgb = finalColor.rgb * (1.0 - dot(ray_nds, ray_nds) * 0.1);

	finalColor.a = 1.0;
  FragColor = finalColor;
}