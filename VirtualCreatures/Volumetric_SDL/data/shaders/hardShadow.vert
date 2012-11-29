uniform vec3 light_position;

// Used for shadow lookup
varying vec4 shadowDir;

void main()
{
	shadowDir = gl_Vertex * gl_ModelViewMatrix - vec4(light_position, 1.0);

	gl_Position = ftransform();

	gl_FrontColor = gl_Color;
}
