uniform sampler2D random;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
//uniform sampler2D gDepth;

uniform mat4 projection;

uniform float radius;

const float noiseScale = 534.536323;

const int sampleKernalSize = 8; // 8 sphere samples

// Random sampling sphere
const vec3 sampleKernal[8] = vec3[8](
vec3(0.025118, 0.0233795, 0.0772801),
vec3(0.0150215, -0.0693198, 0.0794698),
vec3(0.0355327, 0.0179009, 0.0711296),
vec3(-0.0538582, 0.138739, 0.0972095),
vec3(-0.223511, -0.186185, 0.0873117),
vec3(-0.172339, 0.122002, 0.212534),
vec3(0.173605, -0.521377, 0.0488551),
vec3(-0.159742, -0.0908105, 0.478315));

void main()
{
	// Screen space fragment position
	vec3 viewPos = texture2D(gPosition, gl_TexCoord[0].st).xyz;

	vec3 viewNormal = texture2D(gNormal, gl_TexCoord[0].st).xyz;

	// ----------------------- Construct Change of Basis Matrix -----------------------

	// Used to orient kernal along normal (by getting x, y, z axis basis vectors relative to normal)

	vec3 randomVec = texture2D(random, gl_TexCoord[0].st * noiseScale).xyz * 2.0 - 1.0;

	vec3 tangent = normalize(randomVec - viewNormal * dot(randomVec, viewNormal));
	vec3 biTangent = cross(viewNormal, tangent);

	mat3 changeOfBasis = mat3(tangent, biTangent, viewNormal);

	// ----------------------- Sample Depths -----------------------

	float occlusion = 0.0;

	for(int i = 0; i < sampleKernalSize; i++)
	{
		// Sample position
		vec3 samplePos = changeOfBasis * sampleKernal[i];
		samplePos = samplePos * radius + viewPos;

		// Project sample position
		vec4 offset = vec4(samplePos, 1.0);
		offset = projection * offset;
		offset.xy /= offset.w;
		offset.xy = offset.xy * 0.5 + 0.5;

		// Sample depth
		//float depthAtSamplePos = texture2D(gDepth, offset.xy).x;
		float depthAtSamplePos = (projection * vec4(texture2D(gPosition, offset.xy).xyz, 1.0)).z;
	
		float rangeCheck = (offset.z - depthAtSamplePos) < radius ? 1.0 : 0.0;

		occlusion += (depthAtSamplePos < offset.z ? 1.0 : 0.0) * rangeCheck;
	}

	// Normalize occlusion factor, invert, and curve
	occlusion = pow(1.0 - occlusion / sampleKernalSize, 3);

	gl_FragColor = vec4(occlusion, occlusion, occlusion, 1.0);
}