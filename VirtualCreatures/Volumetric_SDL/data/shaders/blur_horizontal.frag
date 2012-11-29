uniform sampler2D scene;
 
uniform float blurSize = 1.0 / 256.0;
 
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
 
	gl_FragColor = vec4(sum, 1.0);
}