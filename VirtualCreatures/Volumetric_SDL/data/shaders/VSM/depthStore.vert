varying vec4 vertexPos;

void main()
{
	vertexPos = gl_Position = ftransform();

	gl_TexCoord[0] = gl_MultiTexCoord0;

	// Needed for alpha testing
	gl_FrontColor = gl_Color;
}