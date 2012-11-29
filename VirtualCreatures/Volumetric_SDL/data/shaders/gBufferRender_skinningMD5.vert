varying vec3 position;
varying vec3 normal;

const int MAX_BONES = 20;

uniform mat4 boneMatrices[MAX_BONES];

/*
	gl_MultiTexCoord0: Vertex UV
	gl_MultiTexCoord1: Vertex weight
	gl_MultiTexCoord2: bone matrix index of vertex
*/

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_TexCoord[1] = gl_MultiTexCoord1;
	gl_TexCoord[2] = gl_MultiTexCoord2;

	mat4 skinTransform = boneMatrices[int(gl_TexCoord[2].x)] * gl_TexCoord[1].x;
	skinTransform += boneMatrices[int(gl_TexCoord[2].y)] * gl_TexCoord[1].y;
	skinTransform += boneMatrices[int(gl_TexCoord[2].z)] * gl_TexCoord[1].z;
	float finalWeight = 1.0 - (gl_TexCoord[1].x + gl_TexCoord[1].y + gl_TexCoord[1].z);
	skinTransform += boneMatrices[int(gl_TexCoord[2].w)] * finalWeight;

	position = (gl_ModelViewMatrix * skinTransform * gl_Vertex).xyz;
	normal = normalize(gl_NormalMatrix * mat3(skinTransform) * gl_Normal);

	gl_Position = ftransform();

	gl_FrontColor = gl_Color;
}
