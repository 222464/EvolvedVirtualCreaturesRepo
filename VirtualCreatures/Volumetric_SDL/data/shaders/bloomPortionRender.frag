uniform sampler2D scene;

uniform float bloomLuminanceFalloffExponent = 22.0;
uniform float bloomIntensity = 9.0;
uniform float blurSize = 0.006345434f;

void main()
{
	vec3 sum = vec3(0.0);

	sum += texture2D(scene, vec2(gl_TexCoord[0].x - 4.0 * blurSize, gl_TexCoord[0].y)).rgb * 0.05;
	sum += texture2D(scene, vec2(gl_TexCoord[0].x - 3.0 * blurSize, gl_TexCoord[0].y)).rgb * 0.09;
	sum += texture2D(scene, vec2(gl_TexCoord[0].x - 2.0 * blurSize, gl_TexCoord[0].y)).rgb * 0.12;
	sum += texture2D(scene, vec2(gl_TexCoord[0].x - blurSize, gl_TexCoord[0].y)).rgb * 0.15;
	sum += texture2D(scene, vec2(gl_TexCoord[0].x, gl_TexCoord[0].y)).rgb * 0.16;
	sum += texture2D(scene, vec2(gl_TexCoord[0].x + blurSize, gl_TexCoord[0].y)).rgb * 0.15;
	sum += texture2D(scene, vec2(gl_TexCoord[0].x + 2.0 * blurSize, gl_TexCoord[0].y)).rgb * 0.12;
	sum += texture2D(scene, vec2(gl_TexCoord[0].x + 3.0 * blurSize, gl_TexCoord[0].y)).rgb * 0.09;
	sum += texture2D(scene, vec2(gl_TexCoord[0].x + 4.0 * blurSize, gl_TexCoord[0].y)).rgb * 0.05;

	float lum = 0.2126 * sum.r + 0.7152 * sum.g + 0.0722 * sum.b;
 
	gl_FragColor = vec4(pow(sum * lum, vec3(bloomLuminanceFalloffExponent)) * bloomIntensity, 1.0);

}
