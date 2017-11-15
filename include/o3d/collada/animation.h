/**
 * @file animation.h
 * @brief O3DCollada animation import/export.
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2008-05-31
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_COLLADA_ANIMATION_H
#define _O3D_COLLADA_ANIMATION_H

#include "global.h"
#include <o3d/core/templatearray.h>
#include <o3d/engine/animation/animationnode.h>

namespace o3d {
namespace collada {

class CNode;

//---------------------------------------------------------------------------------------
//! @class CAnimation
//-------------------------------------------------------------------------------------
//! COLLADA animation object. It mean the <animation>
//---------------------------------------------------------------------------------------
class CAnimation : public CBaseObject
{
public:

	//! Default ctor.
	CAnimation(
		o3d::Scene *pScene,
		domCOLLADA *pDom,
		ColladaInfo &infos,
		const domAnimationRef anim);

	//! Destructor
	virtual ~CAnimation();

	//! Import method
	virtual Bool import();

	//! Export method
	virtual Bool doExport();

	//! Set post-import values to the scene
	virtual Bool toScene();

	//! Set pre-export values from the scene
	virtual Bool fromScene();

	//! Get the node target id
	inline const String& getNodeTargetId() const { return m_targetObjectID; }

	//! Get the node target sid
    inline const String& getNodeTargetSId() const { return m_targetObjectSID; }

	//! Define the animation node
	void setAnimationNode(o3d::AnimationNode *pAnimNode);

protected:

    /*const*/ domAnimationRef m_animation;

	Bool m_hasTranslate;
	Bool m_hasRotate;
	Bool m_hasScale;
	Bool m_hasMatrix;
	Bool m_hasSource;

    Bool m_combinedRotTracks;

	UInt32 m_numChannels;
	AnimationNode *m_animNode;

	enum TargetType
	{
		T_ROTATE,
		T_ROTATE_X,
		T_ROTATE_Y,
		T_ROTATE_Z,
		T_TRANSLATE,
		T_SCALE,
		T_MATRIX,
		T_SOURCE,
		T_ANIM_TARGET,
		T_ANIM_TARGET_X,
		T_ANIM_TARGET_Y,
		T_ANIM_TARGET_Z
	};

	enum SamplerType
	{
		LINEAR,
		BEZIER,    // TODO
		TCB        // TODO
	};

	struct Source
	{
		String id;
		ArrayFloat data;
	};

	typedef std::map<String,Source> T_Sources;
	typedef T_Sources::iterator IT_Sources;

	T_Sources m_sources;
	void readSource(domSourceRef source);

	typedef std::map<String,Source*> T_SourcesPtr;
	typedef T_SourcesPtr::iterator IT_SourcesPtr;

	struct Sampler
	{
		T_SourcesPtr sources;
		SamplerType type;

		Sampler() :	type(LINEAR) {}
	};

	typedef std::map<String,Sampler> T_Samplers;
	typedef T_Samplers::iterator IT_Samplers;

	T_Samplers m_samplers;
	void readSampler(domSamplerRef sampler);

	struct Channel
	{
		Source *inputSrc;
		Source *outputSrc;
		Source *leftTangent;
		Source *rightTangent;
		TargetType target;
		String targetMember;
		UInt32 numEltTargets;
		Sampler *sampler;
	};

	String m_targetObjectID;
	String m_targetObjectSID;

	typedef std::vector<Channel> T_Channels;
	typedef T_Channels::iterator IT_Channels;

	T_Channels m_channels;
	void readChannel(domChannelRef channel);

	//! Generate key frame.
	void generateKeys();

	//! Find a rotation key frame at a specified time.
	KeyFrameSmooth<Quaternion>* findRotationKeyFrame(
			AnimationTrack_SmoothQuaternion *track,
			Float time);
};

} // namespace collada
} // namespace o3d

#endif // _O3D_COLLADA_ANIMATION_H

