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

bool  GetSkyMask(in vec2 coord)
{
  float matID = GetMaterialIDs(coord);
  matID = floor(matID * 255.0f);
                                                                                                                                                                                                                                  #define OIU3 int
  if (matID < 1.0f || matID > 254.0f)
  {
    return true;
  } else {
    return false;
  }
}

void  CalculateClouds2 (inout vec4 color, vec4 screenSpacePosition, vec4 worldSpacePosition, vec3 worldLightVector)
{
  if (texcoord.s < 0.25f && texcoord.t < 0.25f)
  {
    // surface.cloudAlpha = 0.0f;
    vec2 coord = texcoord.st * 4.0f;


    vec4 screenPosition = GetScreenSpacePosition(coord);

    bool isSky = GetSkyMask(coord);

    float sunglow = CalculateSunglow(screenPosition, lightVector);

    vec4 worldPosition = gbufferModelViewInverse * GetScreenSpacePosition(coord);
       worldPosition.xyz += cameraPosition.xyz;

    float cloudHeight = 220.0f;
    float cloudDepth  = 140.0f;
    float cloudDensity = 1.0f;

    float startingRayDepth = far - 5.0f;

    float rayDepth = startingRayDepth;
        //rayDepth += CalculateDitherPattern1() * 0.85f;
        //rayDepth += texture2D(noisetex, texcoord.st * (viewWidth / noiseTextureResolution, viewHeight / noiseTextureResolution)).x * 0.1f;
        //rayDepth += CalculateDitherPattern2() * 0.1f;
    float rayIncrement = far / 10.0f;

        //rayDepth += CalculateDitherPattern1() * rayIncrement;

    // float dither = CalculateDitherPattern1();

    int i = 0;

    vec3 cloudColor = colorSunlight;
    vec4 cloudSum = vec4(0.0f);
       cloudSum.rgb = colorSkylight * 0.2f;
       cloudSum.rgb = color.rgb;


    float cloudDistanceMult = 800.0f / far;


    float surfaceDistance = length(worldPosition.xyz - cameraPosition.xyz);

    vec4 toEye = gbufferModelView * vec4(0.0f, 0.0f, -1.0f, 0.0f);

    vec4 startPosition = GetCloudSpacePosition(coord, rayDepth, cloudDistanceMult);

    const int numSteps = 800;
    const float numStepsF = 800.0f;

    // while (rayDepth > 0.0f)
    for (int i = 0; i < numSteps; i++)
    {
      //determine worldspace ray position
      // vec4 rayPosition = GetCloudSpacePosition(texcoord.st, rayDepth, cloudDistanceMult);
      float inormalized = i / numStepsF;
          // inormalized += dither / numStepsF;
          // inormalized = pow(inormalized, 0.5);
      vec4 rayPosition = vec4(0.0);
           rayPosition.xyz = mix(startPosition.xyz, cameraPosition.xyz, inormalized);

      float rayDistance = length((rayPosition.xyz - cameraPosition.xyz) / cloudDistanceMult);

      // if (surfaceDistance < rayDistance * cloudDistanceMult && isSky)
      // {
      //  continue; TODO re-enable
      // }

      vec4 proximity =  CloudColor(rayPosition, sunglow, worldLightVector);
         proximity.a *= cloudDensity;

         //proximity.a *=  clamp(surfaceDistance - rayDistance, 0.0f, 1.0f);
         // if (surfaceDistance < rayDistance * cloudDistanceMult && surface.mask.sky < 0.5f)
         //   proximity.a = 0.0f;

         if (!isSky)
         proximity.a *= clamp((surfaceDistance - (rayDistance * cloudDistanceMult)) / rayIncrement, 0.0f, 1.0f);

      //cloudSum.rgb = mix( cloudSum.rgb, proximity.rgb, vec3(min(1.0f, proximity.a * cloudDensity)) );
      //cloudSum.a += proximity.a * cloudDensity;
      color.rgb = mix(color.rgb, proximity.rgb, vec3(min(1.0f, proximity.a * cloudDensity)));

      color.a += proximity.a;

      //Increment ray
      rayDepth -= rayIncrement;

      // if (surface.cloudAlpha >= 1.0)
      // {
      //  break;
      // }

       // if (rayDepth * cloudDistanceMult  < ((cloudHeight - (cloudDepth * 0.5)) - cameraPosition.y))
       // {
       //   break;
       // }
    }

    //color.rgb = mix(color.rgb, cloudSum.rgb, vec3(min(1.0f, cloudSum.a * 20.0f)));
    //color.rgb = cloudSum.rgb;
  }
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


}