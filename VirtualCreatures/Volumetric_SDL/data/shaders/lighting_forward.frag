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
uniform sampler2D diffuseMap;
uniform float specularColor;
uniform sampler2D specularMap;

varying vec3 viewPos;
varying vec3 viewNormal;

void main()
{
	vec4 diffuseFinal = texture2D(diffuseMap, gl_TexCoord[0].st).rgba * diffuseColor;
	float specularFinal = (1.0 + texture2D(specularMap, gl_TexCoord[0].st).r) * specularColor;

	vec3 finalColor = vec3(0.0);

	for(int i = 0; i < numLights; i++)
	{
		// Common
		vec3 lightDir = lightPosition[i] - viewPos;
		float dist = length(lightDir);
	
		lightDir /= dist;

		float lambert = dot(lightDir, viewNormal);

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
		vec3 lightRay = reflect(normalize(-lightDir), viewNormal);
		float specularIntensity = strength * pow(max(0.0, dot(lightRay, normalize(-viewPos))), shininess);
		specularIntensity = max(0.0, specularIntensity);

		finalColor += diffuseFinal.rgb * strength * lambert * lightColor[i] + specularFinal * specularIntensity * lightColor[i];

		// Less transparent when shiny
		diffuseFinal.a += specularIntensity;
	}

	finalColor.rgb += ambient * diffuseFinal.rgb;

	gl_FragColor = vec4(finalColor, diffuseFinal.a);
}