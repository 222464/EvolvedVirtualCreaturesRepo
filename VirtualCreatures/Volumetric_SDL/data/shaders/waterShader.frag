// Scene texture copy, for obtaining refractions
uniform sampler2D gPosition;
uniform sampler2D effectCopy;
uniform vec2 effectSize;

uniform float fDepthScalar = 20;
uniform float fMaxOffset = 30;

// Get "random" waves by overlapping 2 bump maps at offsets
uniform sampler2D bumpMap0;
uniform sampler2D bumpMap1;
uniform vec2 bumpMapOffset0;
uniform vec2 bumpMapOffset1;

uniform float refractionRatio = 0.333;

// Light sources
uniform int numLights;

#define LIGHT_POINT 0
#define LIGHT_SPOT 1

uniform int lightType[8];
uniform vec3 lightPosition[8];
uniform vec3 lightColor[8];
uniform vec3 lightSpotDirection[8];
uniform float lightSpotExponent[8];
uniform float lightSpreadAngleCos[8];
uniform float lightRange[8];
uniform float lightIntensity[8];

uniform vec3 ambient;

// Attenuation factors
uniform vec3 attenuation;

// Specularity info
uniform float shininess;

// Textures
uniform vec4 diffuseColor;
uniform float specularColor;

varying vec3 viewPos;
varying vec3 viewNormal;

mat3 CalculateBasis()
{
	vec3 q0 = dFdx(viewPos.xyz);
	vec3 q1 = dFdy(viewPos.xyz);
	vec2 st0 = dFdx(gl_TexCoord[0].st);
	vec2 st1 = dFdy(gl_TexCoord[0].st);
	vec3 S = normalize(q0 * st1.t - q1 * st0.t);
	vec3 T = normalize(-q0 * st1.s + q1 * st0.s);

	return mat3(viewNormal, T, S);
}

void main()
{
	if(gl_FragCoord.y == 0)
		discard;

	vec3 perturbedNormal = normalize(viewNormal + (CalculateBasis() * ((texture2D(bumpMap0, gl_TexCoord[0].st * 80 + bumpMapOffset0).rgb + texture2D(bumpMap1, gl_TexCoord[0].st * 80 + bumpMapOffset1).rgb) - 1.0)));

	// ----------------------------------------- Refraction -----------------------------------------

	// Get refracted ray
	vec3 refracted = normalize(refract(normalize(viewPos), perturbedNormal, refractionRatio));

	// Project ray to get approximation of where to sample the scene texture
	vec4 projRefracted = (gl_ProjectionMatrix * vec4(refracted, 1.0));

	float fWaterDepth = length(viewPos - texture2D(gPosition, (gl_FragCoord.xy + projRefracted.xy) / vec2(800, 600)).xyz);

	float fScale = 1.0 - (1.0 / (1.0 + fDepthScalar * fWaterDepth));
	vec2 offset = projRefracted.xy * fScale * fMaxOffset;

	vec4 diffuseFinal = vec4(texture2D(effectCopy, (gl_FragCoord.xy + offset) / vec2(800, 600)).rgb, 1.0) * diffuseColor;

	// -------------------------------------- Forward Lighting --------------------------------------

	vec3 finalColor = diffuseFinal.rgb;

	for(int i = 0; i < numLights; i++)
	{
		// Common
		vec3 lightDir = lightPosition[i] - viewPos;
		float dist = length(lightDir);
	
		lightDir /= dist;

		float lambert = dot(lightDir, perturbedNormal);

		if(lambert <= 0.0)
			continue;

		// Only strength changes between light types
		float strength;

		switch(lightType[i])
		{
		case LIGHT_POINT: // Render point light
			{
				float fallOff = max(0.0, (lightRange[i] - dist) / lightRange[i]);

				strength = clamp(fallOff * lightIntensity[i] * (1.0 / (attenuation.x + attenuation.y * dist + attenuation.z * dist * dist)), 0.0, 1.0);
			}
			break;
		case LIGHT_SPOT: // Render spot light
			{
				float lightCos = dot(lightSpotDirection[i], -lightDir);

				float spotFallOff = (lightCos - lightSpreadAngleCos[i]) / (1.0 - lightSpreadAngleCos[i]) * pow(lightCos, lightSpotExponent[i]);

				if(spotFallOff <= 0.0)
					continue;

				float fallOff = (lightRange[i] - dist) / lightRange[i];
				fallOff = max(0.0, fallOff);

				strength = fallOff * spotFallOff * lightIntensity[i] * (1.0 / (attenuation.x + attenuation.y * dist + attenuation.z * dist * dist));
				strength = clamp(strength, 0.0, 1.0);
			}

			break;
		}

		// Specular
		vec3 lightRay = reflect(normalize(-lightDir), perturbedNormal);
		float specularIntensity = strength * pow(max(0.0, dot(lightRay, normalize(-viewPos))), shininess);
		specularIntensity = max(0.0, specularIntensity);

		finalColor += diffuseFinal.rgb * strength * lambert * lightColor[i] + specularColor * specularIntensity * lightColor[i];
	}

	finalColor.rgb += ambient * diffuseFinal.rgb;

	gl_FragColor = vec4(finalColor + diffuseFinal.rgb, 1.0);
}