uniform vec3 gDiffuseColor;
uniform sampler2D gDiffuseMap;
uniform float gSpecularColor;
uniform sampler2D gSpecularMap;
uniform vec3 gEmissiveColor;
uniform sampler2D gEmissiveMap;

uniform sampler2D gNormalMap;

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
	// Alpha testing
	vec4 texColor = texture2D(gDiffuseMap, gl_TexCoord[0].st).rgba;

	if(texColor.a < 0.5)
		discard;

	// MRT - positions
	gl_FragData[0].rgb = position;

	// MRT - normals
	gl_FragData[1].rgb = normalize(CalculateBasis() * (texture2D(gNormalMap, gl_TexCoord[0].st).rgb * 2.0 - 1.0));

	// MRT - color (diffuse - rgb, specular - a)
	gl_FragData[2] = vec4(gl_Color.rgb * texColor.rgb * gDiffuseColor, gSpecularColor * texture2D(gSpecularMap, gl_TexCoord[0].st).r);

	// MRT - effect map (write emissivity to it)
	gl_FragData[3] = vec4(texture2D(gEmissiveMap, gl_TexCoord[0].st).rgb * gEmissiveColor, 1.0);
}