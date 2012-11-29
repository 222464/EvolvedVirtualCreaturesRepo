uniform vec3 gDiffuseColor;
uniform float gSpecularColor;

uniform sampler2D gBumpMap0;
uniform sampler2D gBumpMap1;

uniform float bumpMapOffset;

varying vec3 position;
varying vec3 normal;

mat3 CalculateBasis()
{
	vec3 q0 = dFdx(position.xyz);
	vec3 q1 = dFdy(position.xyz);
	vec2 st0 = dFdx(gl_TexCoord[0].st);
	vec2 st1 = dFdy(gl_TexCoord[0].st);
	vec3 S = normalize(q0 * st1.t - q1 * st0.t);
	vec3 T = normalize(-q0 * st1.s + q1 * st0.s);

	return mat3(normal, T, S);
}

void main()
{
	// MRT - normals
	gl_FragData[1].rgb = normalize(CalculateBasis() * (texture2D(gBumpMap0, gl_TexCoord[0].st + vec2(bumpMapOffset)).rgb + texture2D(gBumpMap1, gl_TexCoord[0].st + vec2(-bumpMapOffset)).rgb - 1.0));

	// MRT - positions
	gl_FragData[0].rgb = position;

	// MRT - color (diffuse - rgb, specular - a)
	gl_FragData[2] = vec4(gl_Color.rgb * gDiffuseColor, gSpecularColor);

	// MRT - effect map (write emissivity to it)
	gl_FragData[3] = vec4(0.1, 0.1, 0.1, 1.0);
}