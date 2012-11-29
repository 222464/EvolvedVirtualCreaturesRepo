uniform sampler2D scene;

uniform float blurSize = 1.0 / 256.0;

const int numSamples = 9;
const int halfNumSamples = 4;

const float samples[9] = float[9](
0.05,
0.09,
0.12,
0.15,
0.16,
0.15,
0.12,
0.09,
0.05
);

void main()
{
	// First sample manually
	vec3 moments = texture2D(scene, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y - halfNumSamples * blurSize)).rgb;

	vec3 sum = moments.rgb * samples[0];

	float maxDepth = max(moments.z, moments.x);

	for(int i = 1; i < numSamples; i++)
	{
		moments = texture2D(scene, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y + (i - halfNumSamples) * blurSize)).rgb;
		sum += moments.rgb * samples[i];
		maxDepth = max(maxDepth, moments.z);
		maxDepth = max(maxDepth, moments.x);
	}

	maxDepth = max(maxDepth, sum.z);
	maxDepth = max(maxDepth, sum.x);
 
	gl_FragColor = vec4(sum.rg, maxDepth, 0.0);
}