uniform sampler2D gDiffuseMap;
uniform sampler2D gDepth;

varying vec3 position;
varying vec3 normal;

const float width = 800;
const float height = 600;

const float depthThreshold = 0.01;

void main()
{
	// Alpha testing
	vec4 texColor = texture2D(gDiffuseMap, gl_TexCoord[0].st).rgba;

	if(texColor.a < 0.5)
		discard;

	float depth = texture(gDepth, gl_FragCoord.xy / vec2(width, height)).r;

	if(abs(gl_FragCoord.z - depth) > depthThreshold)
		discard;

	// MRT - positions
	gl_FragData[0].rgb = position;

	// MRT - normals
	gl_FragData[1].rgb = normal;

	// MRT - color (diffuse - rgb, specular - a)
	gl_FragData[2] = vec4(gl_Color.rgb * texColor.rgb, 0.0);

	gl_FragData[3] = vec4(0.0, 0.0, 0.0, 1.0);
}