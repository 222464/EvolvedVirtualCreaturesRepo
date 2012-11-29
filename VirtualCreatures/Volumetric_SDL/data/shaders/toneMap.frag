uniform sampler2D scene;

//uniform float exposure;
//uniform float maxBrightness;
uniform float colorScalar;

const float gammaCorrectionExponent = 1.3;

vec3 gamma(vec3 color)
{
    return pow(color, vec3(gammaCorrectionExponent));
}

vec3 tonemap(vec3 texColor)
{
	/*float L = 0.299 * texColor.r + 0.587 * texColor.g + 0.114 * texColor.b;
    float Lp = L / averageLuminance;
    float nL = (Lp * (1.0 + Lp / whiteSquared)) / (1.0 + Lp);
    texColor *= nL / L;*/

	//vec3 ldrColor = 1.0 - exp2(-texColor * key / averageLuminance);

	//return ldrColor;

	return texColor * colorScalar;
}

void main(void)
{
    gl_FragColor = vec4(gamma(tonemap(texture2D(scene, gl_TexCoord[0].st).rgb)), 1.0);
}
