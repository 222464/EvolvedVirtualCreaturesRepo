// G buffer
uniform sampler2D gColor;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform vec2 gTexSize;

// Light source
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 lightSpotDirection;
uniform float lightSpotExponent;
uniform float lightRange;
uniform float lightSpreadAngleCos;
uniform float lightIntensity;

// Attenuation factors
uniform vec3 attenuation;

// Specularity info
uniform float shininess;

// Shadowing
uniform sampler2D shadowMap;
uniform mat4 lightBiasViewProjection;

vec4 shadowCoordPostW;
	
float ChebyshevUpperBound(float distance)
{
	// Obtain the two moments previously stored (depth and depth * depth)
	vec3 moments = texture2D(shadowMap, shadowCoordPostW.xy).rgb;
		
	// If the current fragment is before the light occluder, it is fully lit
	if(distance <= moments.x)
		return 1.0;

	// Envelope test
	//if(distance > moments.z) // Add a little offset to avoid self shadow
	//	return 0.0;
	
	// The fragment is either in shadow or penumbra. We now use chebyshev's upperBound to check
	// How likely this pixel is to be lit (p_max)
	float variance = moments.y - (moments.x * moments.x);
	variance = max(variance, 0.00002);
	
	float d = distance - moments.x;

	return variance / (variance + d * d);
}

void main()
{	
	vec2 gTexCoord = gl_FragCoord.xy / gTexSize;

	vec3 viewPos = texture2D(gPosition, gTexCoord).xyz;

	vec3 lightDir = lightPosition - viewPos;
	float dist = length(lightDir);

	lightDir /= dist;

	vec3 viewNormal = texture2D(gNormal, gTexCoord).xyz;

	float lambert = dot(lightDir, viewNormal);

	if(lambert <= 0.0)
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	float lightCos = dot(lightSpotDirection, -lightDir);

	float spotFallOff = (lightCos - lightSpreadAngleCos) / (1 - lightSpreadAngleCos) * pow(lightCos, lightSpotExponent);
	
	float fallOff =  max(0.0, spotFallOff * (lightRange - dist) / lightRange);

	if(fallOff <= 0.0)
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	float strength = min(fallOff * lightIntensity * (1.0 / (attenuation.x + attenuation.y * dist + attenuation.z * dist * dist)), 1.0);

	// Shadowing
	vec4 shadowCoord = lightBiasViewProjection * vec4(viewPos, 1.0);
	shadowCoordPostW.x = shadowCoord.x / shadowCoord.w;
	shadowCoordPostW.y = shadowCoord.y / shadowCoord.w;
	shadowCoordPostW.z = shadowCoord.z;

	float shadow = ChebyshevUpperBound(shadowCoordPostW.z);

	vec4 color = texture2D(gColor, gTexCoord);

	if(color.a == 0.0) // No specular
		gl_FragColor = shadow * vec4(color.rgb * strength * lambert * lightColor, 1.0);
	else // Specular
	{
		// Specular
		vec3 lightRay = reflect(normalize(-lightDir), viewNormal);
		float specularIntensity = strength * pow(max(0.0, dot(lightRay, normalize(-viewPos))), shininess);

		gl_FragColor = shadow * vec4(color.rgb * strength * lambert * lightColor + color.a * specularIntensity * lightColor, 1.0);
	}	
}