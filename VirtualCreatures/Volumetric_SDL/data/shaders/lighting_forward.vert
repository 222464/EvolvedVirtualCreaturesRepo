varying vec3 viewPos;
varying vec3 viewNormal;

void main()
{
	viewPos = (gl_ModelViewMatrix * gl_Vertex).xyz;
	viewNormal = normalize(gl_NormalMatrix * gl_Normal);
	
	gl_TexCoord[0] = gl_MultiTexCoord0;

	gl_Position = ftransform();

	gl_FrontColor = gl_Color;
}
