// G buffer
uniform sampler2D gColor;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform vec2 gTexSize;

// Light source
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform float lightRange;

// Attenuation factors
uniform vec3 attenuation;

uniform float lightIntensity;

// Specularity info
uniform float shininess;

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

	float fallOff = max(0.0, (lightRange - dist) / lightRange);

	float strength = min(fallOff * lightIntensity * (1.0 / (attenuation.x + attenuation.y * dist + attenuation.z * dist * dist)), 1.0);

	vec4 color = texture2D(gColor, gTexCoord);

	if(color.a == 0.0) // No specular
		gl_FragColor = vec4(color.rgb * strength * lambert * lightColor, 1.0);
	else // Specular
	{
		vec3 lightRay = reflect(normalize(-lightDir), viewNormal);
		float specularIntensity = strength * pow(max(0.0, dot(lightRay, normalize(-viewPos))), shininess);

		gl_FragColor = vec4(color.rgb * strength * lambert * lightColor + color.a * specularIntensity * lightColor, 1.0);
	}
}