uniform sampler2D gEffect;
uniform sampler2D gNormal;

uniform vec2 invertedScreenDims;
uniform float weight = 0.6;

// Normal directions
vec2 delta[8] = vec2[8](
vec2(-1,1),
vec2(1,-1),
vec2(-1,1),
vec2(1,1),
vec2(-1,0),
vec2(1,0),
vec2(0,-1),
vec2(0,1));

void main()
{	
	vec3 normal = texture2D(gNormal, gl_TexCoord[0].st).xyz;

	float factor = 0.0f;

	for(int i = 0; i < 4; ++i)
	{
		vec3 t = texture2D(gNormal, gl_TexCoord[0].st + delta[i] * invertedScreenDims).xyz;
		t -= normal;
		factor += dot(t, t);
	}

	factor = min(1.0 , factor) * weight;

	vec4 color = vec4(0.0, 0.0, 0.0, 0.0);

	for(int i = 0; i < 8; ++i)
		color += texture2D(gEffect, gl_TexCoord[0].st + delta[i] * invertedScreenDims * factor);

	color += 2.0 * texture2D(gEffect, gl_TexCoord[0].st);

	gl_FragColor = color * (1.0 / 10.0);
}