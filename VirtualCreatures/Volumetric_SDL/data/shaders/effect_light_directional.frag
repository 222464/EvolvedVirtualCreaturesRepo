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

vec2 CalcGTexCoord()
{
    return gl_FragCoord.xy / gTexSize;
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