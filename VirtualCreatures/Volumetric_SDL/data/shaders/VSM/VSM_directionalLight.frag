// G buffer
uniform sampler2D gColor;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform vec2 gTexSize;

// Light source
uniform vec3 lightColor;
uniform vec3 lightDirection;

uniform float lightIntensity;

// Specularity info
uniform float shininess;

// Shadowing
const int numCascades = 4;

uniform float splitDistances[numCascades];
uniform sampler2D cascades[numCascades];
uniform mat4 lightBiasViewProjections[numCascades];

vec4 shadowCoordPostW;
	
float ChebyshevUpperBound(float distance, int cascade)
{
	// Obtain the two moments previously stored (depth and depth * depth)
	vec3 moments = texture2D(cascades[cascade], shadowCoordPostW.xy).rgb;
		
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

	vec3 viewNormal = texture2D(gNormal, gTexCoord).xyz;

	float lambert = (dot(-lightDirection, viewNormal) + 1.0) / 2.0;

	if(lambert <= 0.0)
	{
		gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		return;
	}

	// Determine cascade
	for(int i = 0; i < numCascades; i++)
	{
		if(viewPos.z < splitDistances[i]) // Cascade found
		{
			// Perform shadowed lighting
			vec4 shadowCoord = lightBiasViewProjections[i] * vec4(viewPos, 1.0);
			shadowCoordPostW.x = shadowCoord.x / shadowCoord.w;
			shadowCoordPostW.y = shadowCoord.y / shadowCoord.w;
			shadowCoordPostW.z = shadowCoord.z;

			float shadow = ChebyshevUpperBound(shadowCoordPostW.z, i);

			vec4 color = texture2D(gColor, gTexCoord);

			// Specular
			if(color.a == 0.0) // No specular
				gl_FragColor = shadow * vec4(color.rgb * lightIntensity * lambert * lightColor, 1.0);
			else // Specular
			{
				vec3 lightRay = reflect(normalize(lightDirection), viewNormal);
				float specularIntensity = lightIntensity * pow(max(0.0, dot(lightRay, normalize(-viewPos))), shininess);

				gl_FragColor = shadow * vec4(color.rgb * lightIntensity * lambert * lightColor + color.a * specularIntensity * lightColor, 1.0);
			}

			return;
		}
	}

	// Unshadowed lighting
	vec4 color = texture2D(gColor, gTexCoord);

	// Specular
	if(color.a == 0.0) // No specular
		gl_FragColor = vec4(color.rgb * lightIntensity * lambert * lightColor, 1.0);
	else // Specular
	{
		vec3 lightRay = reflect(normalize(lightDirection), viewNormal);
		float specularIntensity = lightIntensity * pow(max(0.0, dot(lightRay, normalize(-viewPos))), shininess);

		gl_FragColor = vec4(color.rgb * lightIntensity * lambert * lightColor + color.a * specularIntensity * lightColor, 1.0);
	}
}