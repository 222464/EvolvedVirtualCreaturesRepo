uniform samplerCube shadowMap;

varying vec4 shadowDir;

void main()
{
	float frag_depth = length(shadowDir.xyz);

	vec3 norm_shadowDir = normalize(shadowDir).xyz;

	float shadow_map_depth = textureCube(shadowMap, norm_shadowDir).r;

	float light = 0.0;

	if(shadow_map_depth > frag_depth + 0.0005)
		light = 1.0;
		
	gl_FragColor = gl_Color;
	gl_FragColor.rgb *= light;
}