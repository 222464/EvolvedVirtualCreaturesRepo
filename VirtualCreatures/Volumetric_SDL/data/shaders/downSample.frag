uniform sampler2D scene;

const int downSampleScale = 4;
const int downSampleSquared = 16;

uniform float offsets_x[downSampleScale];
uniform float offsets_y[downSampleScale];

void main(void)
{
	vec3 color = vec3(0.0, 0.0, 0.0);

	for(int x = 0; x < downSampleScale; x++)
		for(int y = 0; y < downSampleScale; y++)
			color += texture2D(scene, gl_TexCoord[0].st + vec2(offsets_x[x], offsets_y[y])).rgb;

    gl_FragColor = vec4(color / downSampleSquared, 1.0);
}
