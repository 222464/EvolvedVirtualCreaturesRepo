#include <Renderer/Animation_MD5/Animation_MD5.h>

#include <Utilities/UtilFuncs.h>

#include <sstream>
#include <fstream>
#include <assert.h>

Animation_MD5::Animation_MD5()
	: m_loaded(false)
{
}

bool Animation_MD5::LoadAsset(const std::string &name)
{
	assert(!m_loaded);
	
	std::ifstream fromFile(name);

	if(!fromFile.is_open())
	{
		std::cerr << "Could not find file " << name << std::endl;
		return false;
	}

	std::string paramName;

	// Load header
	fromFile >> paramName;
	assert(paramName == "MD5Version");
	fromFile >> m_version;
	assert(m_version == 10);
	fromFile >> paramName;
	assert(paramName == "commandline");
	getline(fromFile, paramName); // Skip
	fromFile >> paramName;
	assert(paramName == "numFrames");
	fromFile >> m_numFrames;
	fromFile >> paramName;
	assert(paramName == "numJoints");
	fromFile >> m_numJoints;
	fromFile >> paramName;
	assert(paramName == "frameRate");
	fromFile >> m_frameRate;
	fromFile >> paramName;
	assert(paramName == "numAnimatedComponents");
	fromFile >> m_numAnimatedComponents;

	while(!fromFile.eof())
	{
		fromFile >> paramName;

		if(paramName == "hierarchy")
		{
			// Skip to next line
			fromFile.ignore(std::numeric_limits<std::streamsize>::signaling_NaN(), '\n');

			for(int i = 0; i < m_numJoints; i++)
			{
				std::string line;
				getline(fromFile, line);
				std::stringstream ss(line);

				JointDesc j;

				ss >> j.m_name;

				RemoveOuterCharacters(j.m_name);

				ss >> j.m_parentID >> j.m_flags >> j.m_startIndex;

				m_jointDescs.push_back(j);
			}

			// Skip }
			fromFile >> paramName;
			assert(paramName == "}");
		}
		else if(paramName == "bounds")
		{
			// Skip to next line
			fromFile.ignore(std::numeric_limits<std::streamsize>::signaling_NaN(), '\n');

			for(int i = 0; i < m_numFrames; i++)
			{
				std::string line;
				getline(fromFile, line);
				std::stringstream ss(line);

				AABB aabb;

				ss >> paramName; // Skip (
				ss >> aabb.m_lowerBound.x >> aabb.m_lowerBound.y >> aabb.m_lowerBound.z;
				ss >> paramName >> paramName; // Skip ) (
				ss >> aabb.m_upperBound.x >> aabb.m_upperBound.y >> aabb.m_upperBound.z;

				m_aabbs.push_back(aabb);
			}

			// Skip }
			fromFile >> paramName;
			assert(paramName == "}");
		}
		else if(paramName == "baseframe")
		{
			// Skip to next line
			fromFile.ignore(std::numeric_limits<std::streamsize>::signaling_NaN(), '\n');

			for(int i = 0; i < m_numJoints; i++)
			{
				std::string line;
				getline(fromFile, line);
				std::stringstream ss(line);

				BaseFrame baseFrame;

				ss >> paramName; // Skip (
				ss >> baseFrame.m_pos.x >> baseFrame.m_pos.y >> baseFrame.m_pos.z;
				ss >> paramName >> paramName; // Skip ) (
				ss >> baseFrame.m_orient.x >> baseFrame.m_orient.y >> baseFrame.m_orient.z;

				m_baseFrames.push_back(baseFrame);
			}

			// Skip }
			fromFile >> paramName;
			assert(paramName == "}");
		}
		else if(paramName == "frame")
		{
			FrameData frame;

			// Read the frame ID
			fromFile >> frame.m_frameID;

			// Skip to next line
			fromFile.ignore(std::numeric_limits<std::streamsize>::signaling_NaN(), '\n');

			for(int i = 0; i < m_numAnimatedComponents; i++)
			{
				float component;
				fromFile >> component;

				frame.m_frameData.push_back(component);
			}

			m_frames.push_back(frame);

			// Construct a skeleton for this frame
			ConstructFrameSkeleton(m_skeletons, m_jointDescs, m_baseFrames, frame);

			// Skip }
			fromFile >> paramName;
			assert(paramName == "}");
		}
		else
			break;
	}

	fromFile.close();

	m_interpolatedSkeleton.assign(m_numJoints, SkeletalJoint());

	m_interpolatedMatrices.assign(m_numJoints, Matrix4x4f()); // For GPU Skinning

	m_frameDuration = 1.0f / m_frameRate;
	m_animationDuration = m_frameDuration * m_numFrames;

	assert(m_jointDescs.size() == m_numJoints);
	assert(m_aabbs.size() == m_numFrames);
	assert(m_baseFrames.size() == m_numJoints);
	assert(m_frames.size() == m_numFrames);
	assert(m_skeletons.size() == m_numFrames);

	m_loaded = true;

	return true;
}

void Animation_MD5::ConstructFrameSkeleton(std::vector<FrameSkeleton> &skeletons,
		const std::vector<JointDesc> &jointDescs,
		const std::vector<BaseFrame> &baseFrames, 
		const FrameData &frameData)
{
	FrameSkeleton skeleton;

	for(unsigned int i = 0, size = m_jointDescs.size(); i < size; i++)
	{
		unsigned int j = 0;

		const JointDesc &jd = m_jointDescs[i];

		// Start with base frame
		SkeletalJoint animationJoint(m_baseFrames[i]);

		animationJoint.m_parentID = jd.m_parentID;

		// Flags
		if(jd.m_flags & 1) // pos x
		{
			animationJoint.m_pos.x = frameData.m_frameData[jd.m_startIndex + j];
			j++;
		}
		if(jd.m_flags & 2) // pos y
		{
			animationJoint.m_pos.y = frameData.m_frameData[jd.m_startIndex + j];
			j++;
		}
		if(jd.m_flags & 4) // pos z
		{
			animationJoint.m_pos.z = frameData.m_frameData[jd.m_startIndex + j];
			j++;
		}
		if(jd.m_flags & 8) // orient x
		{
			animationJoint.m_orient.x = frameData.m_frameData[jd.m_startIndex + j];
			j++;
		}
		if(jd.m_flags & 16) // orient y
		{
			animationJoint.m_orient.y = frameData.m_frameData[jd.m_startIndex + j];
			j++;
		}
		if(jd.m_flags & 32) // orient z
		{
			animationJoint.m_orient.z = frameData.m_frameData[jd.m_startIndex + j];
			j++;
		}

		animationJoint.m_orient.CalculateWFromXYZ();

		// If interpolated joint has a parent
		if(animationJoint.m_parentID != -1)
		{
			SkeletalJoint &parentJoint = skeleton[animationJoint.m_parentID];

			// Transform this joint by the parent joint (additive transformation of skeleton)
			animationJoint.m_pos = parentJoint.m_pos + parentJoint.m_orient * animationJoint.m_pos;
			animationJoint.m_orient = parentJoint.m_orient * animationJoint.m_orient;

			animationJoint.m_orient.NormalizeThis();
		}

		skeleton.push_back(animationJoint);
	}

	skeletons.push_back(skeleton);
}

void Animation_MD5::Update(float time)
{
	assert(m_loaded);

	if(m_numFrames < 1)
		return;

	// Wrap the time
	time = Wrap(time, m_animationDuration);

	// Determine the frames that need to be interpolated based on the time
	float frameNum = m_frameRate * time;

	int firstFrame = static_cast<int>(floorf(frameNum));
	int secondFrame = static_cast<int>(ceilf(frameNum)); 

	// Wrap
	firstFrame = firstFrame % m_numFrames;
	secondFrame = secondFrame % m_numFrames;

	float interpolationCoefficient = fmodf(time, m_frameDuration) / m_frameDuration;

	// Calculate the skeleton for the current time
	InterpolateFrameSkeletons(m_interpolatedSkeleton, m_skeletons[firstFrame], m_skeletons[secondFrame], interpolationCoefficient);

	// Interpolate the AABB
	InterpolateFrameAABBs(m_interpolatedAABB, m_aabbs[firstFrame],  m_aabbs[secondFrame], interpolationCoefficient);
}

void Animation_MD5::InterpolateFrameSkeletons(FrameSkeleton &interpolatedSkeleton,
		const FrameSkeleton &firstSkeleton,
		const FrameSkeleton &secondSkeleton,
		float interpolationCoefficient)
{
	for(int i = 0; i < m_numJoints; i++)
	{
		SkeletalJoint &interpolateJoint = interpolatedSkeleton[i];

		const SkeletalJoint &firstSkeletalJoint = firstSkeleton[i];
		const SkeletalJoint &secondSkeletalJoint = secondSkeleton[i];

		interpolateJoint.m_parentID = firstSkeletalJoint.m_parentID;

		interpolateJoint.m_pos = Lerp(firstSkeletalJoint.m_pos, secondSkeletalJoint.m_pos, interpolationCoefficient);
		interpolateJoint.m_orient = Quaternion::Slerp(firstSkeletalJoint.m_orient, secondSkeletalJoint.m_orient, interpolationCoefficient);

		// For GPU Skinning
		m_interpolatedMatrices[i] = Matrix4x4f::TranslateMatrix(interpolateJoint.m_pos) * interpolateJoint.m_orient.GetMatrix();
	}
}

void Animation_MD5::InterpolateFrameAABBs(AABB &interpolatedAABB, const AABB &firstAABB, const AABB &secondAABB, float interpolationCoefficient)
{
	interpolatedAABB.m_lowerBound = Lerp(firstAABB.m_lowerBound, secondAABB.m_lowerBound, interpolationCoefficient);
	interpolatedAABB.m_upperBound = Lerp(firstAABB.m_upperBound, secondAABB.m_upperBound, interpolationCoefficient);
}

const FrameSkeleton &Animation_MD5::GetInterpolatedSkeleton()
{
	return m_interpolatedSkeleton;
}

const std::vector<Matrix4x4f> &Animation_MD5::GetSkeletonMatrices()
{
	return m_interpolatedMatrices;
}

int Animation_MD5::GetNumJoints()
{
	return m_numJoints;
}

const JointDesc &Animation_MD5::GetJointDesc(unsigned int index)
{
	assert(index >= 0 && index < m_jointDescs.size());
	return m_jointDescs[index];
}

float Animation_MD5::GetAnimationDuration()
{
	return m_animationDuration;
}

const AABB &Animation_MD5::GetAABB()
{
	return m_interpolatedAABB;
}

Asset* Animation_MD5::Asset_Factory()
{
	return new Animation_MD5();
}