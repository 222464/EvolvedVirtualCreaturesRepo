uniform sampler2D scene;

const float bloomBlurInfluence = 0.004;
const float bloomIntensity = 0.03;
const float bloomLuminanceFalloffExponent = 8.0;
const int bloomSampleRadius = 2;
const int bloomSampleRadiusPlusOne = bloomSampleRadius + 1;

vec3 bloom()
{
	vec3 color;

	vec3 sum = vec3(0);
	vec2 texcoord = vec2(gl_TexCoord[0].st);
	int j;
	int i;

	for(i = -bloomSampleRadius; i < bloomSampleRadius; i++)
	{
		for(j = -bloomSampleRadius; j < bloomSampleRadius; j++)
			sum += pow(texture2D(scene, texcoord + vec2(j, i) * bloomBlurInfluence).rgb, vec3(bloomLuminanceFalloffExponent));
	}

	vec3 sampleColor = texture2D(scene, texcoord).rgb;
	//float luminance = sampleColor.r * 0.299 + sampleColor.g * 0.6 + sampleColor.b * 0.1;

    color = sum * bloomIntensity + sampleColor.rgb;

	return color;
}

void main()
{
    gl_FragColor = vec4(bloom(), 1.0);
}