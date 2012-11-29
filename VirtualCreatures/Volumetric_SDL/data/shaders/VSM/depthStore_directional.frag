uniform sampler2D diffuseMap;

varying vec4 vertexPos;

uniform vec2 zClipPlanes;

void main()
{
	if(texture2D(diffuseMap, gl_TexCoord[0].st).a < 0.5)
		discard;

	float depth = vertexPos.z + 0.1; // Offset to remove acne
	//depth = depth * 0.5 + 0.5; // Unit cube [-1,1] to [0,1] coordinate system
	float oDepth = depth;//(depth - zClipPlanes.x) / (zClipPlanes.y - zClipPlanes.x);

	float moment1 = oDepth;
	float moment2 = oDepth * oDepth;

	// Adjusting moments using derivative
	float dx = dFdx(depth);
	float dy = dFdy(depth);

	moment2 += 0.25 * (dx * dx + dy * dy);

	gl_FragColor = vec4(moment1, moment2, moment1, 0.0); // Store depth twice
}