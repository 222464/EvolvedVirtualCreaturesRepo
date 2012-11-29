/*
Render the scene depth texture to framebuffer so we can 'see' the depth'
*/

uniform vec2 depthRange;  //x=near, y=far
uniform samplerCube depthSampler;

float LinearizeDepth(vec3 xyz)
{
	float n = depthRange.x; // camera z near
	float f = depthRange.y; // camera z far
	float z = texture(depthSampler, xyz).r;
	return 2.0 * n / (f + n - z * (f - n));
}
void main()
{
	float d = LinearizeDepth(gl_TexCoord[0].xyz);
	float one_minus_d = 1.0 - d;
	gl_FragColor = vec4(one_minus_d, one_minus_d, one_minus_d, 1.0);
}