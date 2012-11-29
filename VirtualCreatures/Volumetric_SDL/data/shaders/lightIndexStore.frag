uniform sampler2D gDiffuseMap;

varying vec3 position;
varying vec3 normal;

void main()
{
	// Alpha testing
	vec4 texColor = texture2D(gDiffuseMap, gl_TexCoord[0].st).rgba;

	if(texColor.a < 0.5)
		discard;

	// MRT - positions
	gl_FragData[0].rgb = position;

	// MRT - normals
	gl_FragData[1].rgb = normal;

	// MRT - color (diffuse - rgb, specular - a)
	gl_FragData[2] = vec4(gl_Color.rgb * texColor.rgb * gDiffuseColor, gSpecularColor * texture2D(gSpecularMap, gl_TexCoord[0].st).r);

	// MRT - effect map (write emissivity to it)
	gl_FragData[3] = vec4(texture2D(gEmissiveMap, gl_TexCoord[0].st).rgb * gEmissiveColor, 1.0);
}