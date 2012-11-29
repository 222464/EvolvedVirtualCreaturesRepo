#pragma once

#include <AssetManager/Asset.h>

#include <Renderer/Animation_MD5/BaseFrame.h>
#include <Renderer/Animation_MD5/FrameData.h>
#include <Renderer/Animation_MD5/JointDesc.h>
#include <Renderer/Animation_MD5/SkeletalJoint.h>

#include <Constructs/AABB.h>

typedef std::vector<SkeletalJoint> FrameSkeleton;

class Animation_MD5 :
	public Asset
{
private:
	int m_version;
	int m_numFrames;
	int m_numJoints;
	int m_frameRate;
	int m_numAnimatedComponents;

	float m_animationDuration;
	float m_frameDuration;

	std::vector<JointDesc> m_jointDescs;
	std::vector<AABB> m_aabbs;
	std::vector<BaseFrame> m_baseFrames;
	std::vector<FrameData> m_frames;
	std::vector<FrameSkeleton> m_skeletons;

	FrameSkeleton m_interpolatedSkeleton;
	std::vector<Matrix4x4f> m_interpolatedMatrices; // For GPU Skinning

	AABB m_interpolatedAABB;

	// Creates a skeleton for a frame
	void ConstructFrameSkeleton(std::vector<FrameSkeleton> &skeletons,
		const std::vector<JointDesc> &jointDescs,
		const std::vector<BaseFrame> &baseFrames, 
		const FrameData &frameData);

	// Interpolate two frame skeletons
	void InterpolateFrameSkeletons(FrameSkeleton &interpolatedSkeleton,
		const FrameSkeleton &firstSkeleton,
		const FrameSkeleton &secondSkeleton,
		float interpolationCoefficient);

	void InterpolateFrameAABBs(AABB &interpolatedAABB, const AABB &firstAABB, const AABB &secondAABB, float interpolationCoefficient);

	bool m_loaded;

public:
	Animation_MD5();

	// Inherited from Asset
	bool LoadAsset(const std::string &name);

	void Update(float time);

	const FrameSkeleton &GetInterpolatedSkeleton();

	int GetNumJoints();

	const JointDesc &GetJointDesc(unsigned int index);

	float GetAnimationDuration();

	const AABB &GetAABB();

	const std::vector<Matrix4x4f> &GetSkeletonMatrices();

	// Get a pointer to the permanent joint (in animation)
	//JointDesc* GetJoint_Permanent(const std::string &name, float frame);

	// Asset factory
	static Asset* Asset_Factory();
};

