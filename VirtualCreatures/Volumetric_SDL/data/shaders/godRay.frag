uniform float exposure;
uniform float decay;
uniform float density;
uniform float weight;
uniform vec2 lightPositionOnScreen;
uniform sampler2D firstPass;
const int NUM_SAMPLES = 50;

void main()
{	
	vec2 deltaTextCoord = vec2(gl_TexCoord[0].st - lightPositionOnScreen.xy);

	vec2 textCoo = gl_TexCoord[0].st;

	deltaTextCoord *= 1.0 /  float(NUM_SAMPLES) * density;

	float illuminationDecay = 1.0;

	float accumAlpha = 0.0;

	for(int i = 0; i < NUM_SAMPLES ; i++)
	{
		textCoo -= deltaTextCoord;

		float sampleAlpha = texture2D(firstPass, textCoo).a;
    	
		sampleAlpha *= illuminationDecay * weight;
    			
		accumAlpha += sampleAlpha;
    			
		illuminationDecay *= decay;
	}

	accumAlpha *= exposure;

	gl_FragColor.r = 0.0;
	gl_FragColor.g = 0.0;
	gl_FragColor.b = 0.0;
	gl_FragColor.a = accumAlpha;
}

	