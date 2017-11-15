/**
 * @file animation.cpp
 * @brief Implementation of Animation.h
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2008-05-31
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/collada/precompiled.h"
#include "o3d/collada/animation.h"
#include "o3d/collada/node.h"

#include <o3d/engine/scene/scene.h>
#include <o3d/engine/animation/animationnode.h>

using namespace o3d;
using namespace o3d::collada;

// Default ctor.
CAnimation::CAnimation(
	o3d::Scene *scene,
	domCOLLADA *dom,
	ColladaInfo &infos,
	const domAnimationRef anim) :
		CBaseObject(scene, dom, infos),
		m_animation(anim),
		m_hasTranslate(False),
		m_hasRotate(False),
		m_hasScale(False),
		m_hasMatrix(False),
		m_hasSource(False),
        m_combinedRotTracks(True),
		m_numChannels(0),
        m_animNode(nullptr)
{
}

// Destructor
CAnimation::~CAnimation()
{
}

// Import method
Bool CAnimation::import()
{
	m_id = m_animation->getId() ? m_animation->getId() : "";

    // sub-animation node, we use it
    if (m_animation->getAnimation_array().getCount() > 0)
        m_animation = m_animation->getAnimation_array().get(0);

	// get the number of sources
	domSource_Array &source_array = m_animation->getSource_array();
	for (size_t i = 0; i < source_array.getCount(); ++i)
	{
		readSource(source_array[i]);
	}

	domSampler_Array &sampler_array = m_animation->getSampler_array();
	for (size_t s = 0; s < sampler_array.getCount(); ++s)
	{
		readSampler(sampler_array[s]);
	}

	domChannel_Array &channel_array = m_animation->getChannel_array();
	for (size_t i = 0; i< channel_array.getCount(); ++i)
	{
		readChannel(channel_array[i]);
	}

    O3D_ASSERT(m_targetObjectID.isValid());

	CNode *node = (CNode*)m_infos.findNodeUsingId(m_targetObjectID);
	if (node)
		node->addAnimation(this);

	domAnimation_Array &animation_array = m_animation->getAnimation_array();
	for (size_t i = 0; i < animation_array.getCount(); ++i)
	{
		CAnimation *animation = new CAnimation(m_scene, &m_dom, m_infos, animation_array[i]);
		if (!animation->import())
		{
			deletePtr(animation);
			break;
		}
	}

	return True;
}

// Export method
Bool CAnimation::doExport()
{
	O3D_ASSERT(0);

	return True;
}

// Set post-import values to the scene
Bool CAnimation::toScene()
{
	if (m_channels.size() > 0)
		generateKeys();

	return True;
}

// Set pre-export values from the scene
Bool CAnimation::fromScene()
{
	O3D_ASSERT(0);

	return True;
}

// Define the animation node
void CAnimation::setAnimationNode(o3d::AnimationNode *animNode)
{
	O3D_ASSERT(animNode);
	m_animNode = animNode;
}

void CAnimation::readSource(domSourceRef source)
{
	if (!source->getId())
	{
		O3D_WARNING("Undefined collada animation source id");
		return;
	}

	m_sources[source->getId()] = Source();
	Source &lsrc = m_sources[source->getId()];

	lsrc.id = source->getId();

	// Copy over the float array data if any
	if (source->getFloat_array())
	{
		daeDoubleArray &floatArray = source->getFloat_array()->getValue();
		lsrc.data.setSize(floatArray.getCount());

		// copy the array data
		for (size_t a = 0; a < floatArray.getCount(); ++a)
		{
			lsrc.data[a] = (Float)floatArray[a];
		}
	}
	else if (source->getName_array())
	{
		// hope the entire array is of the same interpolation type
	/*	daeStringArray &nameArray = source->getName_array()->getValue();

		if (nameArray.getCount() != 0)
		{
			if (strcmp(nameArray[0],"LINEAR") == 0)
				lsampler.type = Linear;

			if (strcmp(nameArray[0],"BEZIER") == 0)
				lsampler.type = Bezier;

			if (strcmp(nameArray[0],"TCB") == 0)
				lsampler.type = Tcb;
		}*/
	}
	else
	{
		O3D_ASSERT(0);
	}
}

void CAnimation::readSampler(domSamplerRef sampler)
{
	m_samplers[sampler->getId()] = Sampler();
	Sampler &lsampler = m_samplers[sampler->getId()];

	domInputLocal_Array &input_array = sampler->getInput_array();
	for (size_t i = 0; i < input_array.getCount(); ++i)
	{
		domSource *source = (domSource*)(domElement*)input_array[i]->getSource().getElement();
		lsampler.sources[input_array[i]->getSemantic()] = &m_sources[source->getId()];

		// interpolation source
		if (source->getName_array())
		{
			// hope the entire array is of the same interpolation type
			// otherwise it is not supported
			daeStringArray &nameArray = source->getName_array()->getValue();

			if (nameArray.getCount() != 0)
			{
				if (strcmp(nameArray[0],"LINEAR") == 0)
					lsampler.type = LINEAR;

				else if (strcmp(nameArray[0],"BEZIER") == 0)
					lsampler.type = BEZIER;

				else if (strcmp(nameArray[0],"TCB") == 0)
					lsampler.type = TCB;

				SamplerType stype;

				for (size_t z = 1; z < nameArray.getCount(); ++z)
				{
					if (strcmp(nameArray[z],"LINEAR") == 0)
						stype = LINEAR;

					else if (strcmp(nameArray[z],"BEZIER") == 0)
						stype = BEZIER;

					else if (strcmp(nameArray[z],"TCB") == 0)
						stype = TCB;

					// mixed mode
					if (stype != lsampler.type)
						O3D_ERROR(E_InvalidFormat("Mixed animations key mode are not supported"));
				}
			}
		}
	}
}

void CAnimation::readChannel(domChannelRef channel)
{
	m_channels.push_back(Channel());
	Channel &lchannel = m_channels.back();

	domSampler *sampler = (domSampler*)(domElement*)channel->getSource().getElement();
	String target = channel->getTarget() ? channel->getTarget() : "";

	lchannel.sampler = &m_samplers[sampler->getId()];

	lchannel.inputSrc = &m_sources[lchannel.sampler->sources["INPUT"]->id];
	lchannel.outputSrc = &m_sources[lchannel.sampler->sources["OUTPUT"]->id];

	// in/out tangents for Bezier interpolation
	if (lchannel.sampler->sources["IN_TANGENT"])
		lchannel.leftTangent = &m_sources[lchannel.sampler->sources["IN_TANGENT"]->id];
	else
        lchannel.leftTangent = nullptr;

	if (lchannel.sampler->sources["OUT_TANGENT"])
		lchannel.rightTangent = &m_sources[lchannel.sampler->sources["OUT_TANGENT"]->id];
	else
        lchannel.rightTangent = nullptr;

	for (Int32 i = 0; i < lchannel.inputSrc->data.getSize(); ++i)
	{
        m_infos.maxAnimationDuration(lchannel.inputSrc->data[i]);
	}

	// parse target
	// get the target element
	Int32 idSep = target.find('/');
	if (idSep == -1)
		idSep = target.find('(');
	if (idSep == -1)
		idSep = target.find('.');
	if (idSep != -1)
	{
		m_targetObjectID = target.sub(0,idSep);

		Int32 sidSep = target.find('.');
		if (sidSep == -1)
			sidSep = target.find('(');
		
		if (idSep != -1 && sidSep != -1 && sidSep > idSep)
            m_targetObjectSID = target.sub(idSep+1, sidSep);

        if (sidSep != -1)
            lchannel.targetMember = target.sub(sidSep+1, -1);
	}
	else
	{
		m_targetObjectID = target;
	}

	// resolve target
    domElement *element = nullptr;
	domElement *rootnode = channel->getDocument()->getDomRoot();
	daeSIDResolver sidresolver(rootnode, target.toUtf8().getData());
	element = sidresolver.getElement();
    if (element == nullptr)
	{
		m_channels.pop_back();
		O3D_WARNING("Animation target can not be solved");
		return;
	}

	// set channel info
    COLLADA_TYPE::TypeEnum type = element->typeID();
    if (type == domTranslate::ID())
    {
        m_hasTranslate = True;
        lchannel.target = T_TRANSLATE;
        lchannel.numEltTargets = 3;
    }
    else if (type == domRotate::ID())
    {
        m_hasRotate = True;
        lchannel.target = T_ROTATE;
        lchannel.numEltTargets = 4;
    }
    else if (type == domScale::ID())
    {
        m_hasScale = True;
        lchannel.target = T_SCALE;
        lchannel.numEltTargets = 3;
    }
    else if (type == domSource::ID())
    {
        m_hasSource = True;
        lchannel.target = T_SOURCE;
        lchannel.numEltTargets = 1;
    }
    else if (type == domMatrix::ID())
    {
        m_hasMatrix = True;
        lchannel.target = T_MATRIX;
        lchannel.numEltTargets = 16;
    }
    else
    {
        m_channels.pop_back();
        O3D_WARNING("Unsupported animation target");
        return;
    }

	// parse member
	if (lchannel.targetMember.isValid())
	{
		if ((lchannel.targetMember.compare("AXIS",String::CASE_INSENSITIVE) == 0) ||
			(lchannel.targetMember.compare("ANGLE",String::CASE_INSENSITIVE) == 0))
		{
			domRotate *rotate = (domRotate*)element;
			if (rotate->getValue()[0] == 1)
			{
				m_hasRotate = True;
				lchannel.target = T_ROTATE_X;
				lchannel.numEltTargets = 1;
			}
			else if (rotate->getValue()[1] == 1)
			{
				m_hasRotate = True;
				lchannel.target = T_ROTATE_Y;
				lchannel.numEltTargets = 1;
			}
			else if (rotate->getValue()[2] == 1)
			{
				m_hasRotate = True;
				lchannel.target = T_ROTATE_Z;
				lchannel.numEltTargets = 1;
			}
		}
		else if (lchannel.targetMember.compare("X",String::CASE_INSENSITIVE) == 0)
		{
			lchannel.target = T_ANIM_TARGET_X;
			lchannel.numEltTargets = 1;
		}
		else if (lchannel.targetMember.compare("Y",String::CASE_INSENSITIVE) == 0)
		{
			lchannel.target = T_ANIM_TARGET_Y;
			lchannel.numEltTargets = 1;
		}
		else if (lchannel.targetMember.compare("Z",String::CASE_INSENSITIVE) == 0)
		{
			lchannel.target = T_ANIM_TARGET_Z;
			lchannel.numEltTargets = 1;
		}
		else if (lchannel.targetMember[0] >= '0' && lchannel.targetMember[0] <= '9')
		{
			lchannel.target = T_SOURCE;
			lchannel.numEltTargets = 1;
		}
		else
		{
			lchannel.target = T_ANIM_TARGET;
			lchannel.numEltTargets = 3;
		}
	}

	m_numChannels = o3d::max<UInt32>(m_numChannels, lchannel.numEltTargets);
}

void CAnimation::generateKeys()
{
	Float invDuration = 1.f / m_infos.getAnimationDuration();
	
	CNode *node = (CNode*)m_infos.findNodeUsingId(m_targetObjectID);
	Matrix4 nodeMat;
	if (node && node->getSceneNode() && node->getSceneNode()->getTransform())
		nodeMat = node->getSceneNode()->getTransform()->getMatrix().invertStd();

	for (size_t i = 0; i < m_channels.size(); ++i)
	{
		Channel &channel = m_channels[i];
		UInt32 numKeys = channel.inputSrc->data.getSize();
		UInt32 numCh = channel.numEltTargets;

		if (channel.sampler->type == LINEAR)
		{
            AnimationTrack_SmoothQuaternion *quatRotTrack = nullptr;
            AnimationTrack_LinearVector *linearPosTrack = nullptr;
            AnimationTrack_LinearVector *linearScaleTrack = nullptr;

			if ((channel.target >= T_ROTATE) && (channel.target <= T_ROTATE_Z))
			{
				if ((numCh != 1) && (numCh != 4))
					O3D_ERROR(E_InvalidFormat("Rotation track should have 1 or 4 components"));

                // combined rotations tracks
                if (m_combinedRotTracks)
                {
                    // if a rotation track exists reuse it
                    T_AnimationTrackList &trackList = m_animNode->getTrackList();
                    for (IT_AnimationTrackList it = trackList.begin(); it != trackList.end(); ++it)
                    {
                        if ((*it)->isRotationTarget())
                            quatRotTrack = (AnimationTrack_SmoothQuaternion*)*it;
                    }

                    // need a new one
                    if (quatRotTrack == nullptr)
                    {
                        quatRotTrack = new AnimationTrack_SmoothQuaternion(
                                    AnimationTrack::TARGET_OBJECT_ROT,
                                    0,
                                    AnimationTrack::TRACK_MODE_LOOP,
                                    AnimationTrack::TRACK_MODE_LOOP);

                        m_animNode->addTrack(*quatRotTrack);

                        /*// time 0
                        KeyFrameSmooth<Quaternion> *key = new KeyFrameSmooth<Quaternion>(
                                    0,
                                    Quaternion());

                        quatRotTrack->addKeyFrame(*key);*/
                    }

                    if (numCh == 1)
                    {
                        Vector3 axis;

                        if (channel.target == T_ROTATE_X)
                            axis.set(1.0f, 0.0f, 0.0f);
                        else if (channel.target == T_ROTATE_Y)
                            axis.set(0.0f, 1.0f, 0.0f);
                        else if (channel.target == T_ROTATE_Z)
                            axis.set(0.0f, 0.0f, 1.0f);

                        // set the actual key info
                        for (UInt32 i = 0 ; i < numKeys; ++i)
                        {
                            Quaternion quat;
                            quat.fromAxisAngle3(axis, o3d::toRadian(channel.outputSrc->data[i]));

                            // first search for a key at time
                            KeyFrameSmooth<Quaternion> *key = nullptr;
                            key = findRotationKeyFrame(quatRotTrack, channel.inputSrc->data[i]*invDuration);

                            // need a new key
                            if (key == nullptr)
                            {
                                key = new KeyFrameSmooth<Quaternion>(
                                            channel.inputSrc->data[i]*invDuration,
                                            quat);

                                quatRotTrack->addKeyFrame(*key);
                            }
                            else
                            {
                                // transform the found key
                                key->Data *= quat;
                                key->Data.normalize();
                            }
                        }
                    }
                    else if (numCh == 4)
                    {
                        O3D_ASSERT(0);
                        /*
                    // set the actual key info
                    for (UInt32 i = 0 ; i < numKeys; ++i)
                    {
                        // fill in all the keys for each anim key set
                        UInt32 offset = i*numCh;

                        Quaternion quat;
                        quat.fromAxisAngle3(
                            Vector3(
                                channel.outputSrc->data[offset+X],
                                channel.outputSrc->data[offset+Y],
                                channel.outputSrc->data[offset+Z]),
                            channel.outputSrc->data[offset+W]);

                        KeyFrameSmooth<Quaternion> *key = NULL;
                        key = FindRotationKeyFrame(quatRotTrack, channel.inputSrc->data[i]*invDuration);

                        // need a new key
                        if (key == NULL)
                        {
                            key = new KeyFrameSmooth<Quaternion>(
                                    channel.inputSrc->data[i]*invDuration,
                                    quat);

                            quatRotTrack->addKeyFrame(*key);
                        }
                        else
                        {
                            // transform the found key
                            key->Data *= quat;
                            key->Data.normalize();
                        }
                    }*/
                    }
                }
                else
                {
                    AnimationTrack::Target target = AnimationTrack::TARGET_OBJECT_ROT;
                    switch (channel.target)
                    {
                    case T_ROTATE_X:
                        target = AnimationTrack::TARGET_OBJECT_ROT_X;
                        break;
                    case T_ROTATE_Y:
                        target = AnimationTrack::TARGET_OBJECT_ROT_Y;
                        break;
                    case T_ROTATE_Z:
                        target = AnimationTrack::TARGET_OBJECT_ROT_Z;
                        break;
                    default:
                        break;
                    }

                    AnimationTrack_LinearFloat *rotTrack = new AnimationTrack_LinearFloat(
                                target,
                                0,
                                AnimationTrack::TRACK_MODE_LOOP,
                                AnimationTrack::TRACK_MODE_LOOP);

                    m_animNode->addTrack(*rotTrack);

                    /*// time 0
                    KeyFrameSmooth<Quaternion> *key = new KeyFrameSmooth<Quaternion>(
                                0,
                                Quaternion());

                    rotTrack->addKeyFrame(*key);*/

                    if (numCh == 1)
                    {
                        // set the actual key info
                        for (UInt32 i = 0 ; i < numKeys; ++i)
                        {
                            KeyFrameLinear<Float> *key = new KeyFrameLinear<Float>(
                                        channel.inputSrc->data[i]*invDuration,
                                        o3d::toRadian(channel.outputSrc->data[i]));

                            rotTrack->addKeyFrame(*key);
                        }
                    }
                    else if (numCh == 4)
                    {
                        O3D_ASSERT(0);
                    }
                }
			}
			else if (channel.target == T_TRANSLATE)
			{
				if (numCh != 3)
					O3D_ERROR(E_InvalidFormat("Translation track should have 3 components"));

				linearPosTrack = new AnimationTrack_LinearVector(
						AnimationTrack::TARGET_OBJECT_POS,
						0,
						AnimationTrack::TRACK_MODE_LOOP,
						AnimationTrack::TRACK_MODE_LOOP);

				m_animNode->addTrack(*linearPosTrack);

                KeyFrameLinear<Vector3> *key = new KeyFrameLinear<Vector3>(
                    0,
                    Vector3());

                linearPosTrack->addKeyFrame(*key);

				// set the actual key info
				for (UInt32 i = 0 ; i < numKeys; ++i)
				{
					// fill in all the keys for each anim key set
					UInt32 offset = i*numCh;

					Vector3 vec(&channel.outputSrc->data[offset]);

					KeyFrameLinear<Vector3> *key = new KeyFrameLinear<Vector3>(
						channel.inputSrc->data[i]*invDuration,
						vec);

					linearPosTrack->addKeyFrame(*key);
				}
			}
			else if (channel.target == T_SCALE)
			{
				if (numCh != 3)
				{
					O3D_ERROR(E_InvalidFormat("Scale track should have 3 components"));
					return;
				}

				linearScaleTrack = new AnimationTrack_LinearVector(
						AnimationTrack::TARGET_OBJECT_SCALE,
						0,
						AnimationTrack::TRACK_MODE_LOOP,
						AnimationTrack::TRACK_MODE_LOOP);
				m_animNode->addTrack(*linearScaleTrack);

                KeyFrameLinear<Vector3> *key = new KeyFrameLinear<Vector3>(
                    0,
                    Vector3());

                linearScaleTrack->addKeyFrame(*key);

				// set the actual key info
				for (UInt32 i = 0 ; i < numKeys; ++i)
				{
					// fill in all the keys for each anim key set
					UInt32 offset = i*numCh;

					Vector3 vec(&channel.outputSrc->data[offset]);

					KeyFrameLinear<Vector3> *key = new KeyFrameLinear<Vector3>(
						channel.inputSrc->data[i]*invDuration,
						vec);

					linearScaleTrack->addKeyFrame(*key);
				}
			}
			else if (channel.target == T_MATRIX)
			{
                if (numCh != 16)
					O3D_ERROR(E_InvalidFormat("Animation matrix track should have 16 components"));

				// set the actual key info
                for (UInt32 i = 0 ; i < numKeys; ++i)
				{
					// fill in all the keys for each anim key set
					UInt32 offset = i*16;//numCh;

					const Float *src = &channel.outputSrc->data[offset];

					Matrix4 mat(
						src[0],
						src[1],
						src[2],
						src[3],

						src[4],
						src[5],
						src[6],
						src[7],

						src[8],
						src[9],
						src[10],
						src[11],

						src[12],
						src[13],
						src[14],
						src[15]);
					
					mat = nodeMat * mat;

					// rotation
					Quaternion quat;
					quat.fromMatrix4(mat);

					if (!quat.isIdentity())
					{
						if (!quatRotTrack)
						{
							quatRotTrack = new AnimationTrack_SmoothQuaternion(
									AnimationTrack::TARGET_OBJECT_ROT,
									0,
									AnimationTrack::TRACK_MODE_LOOP,
									AnimationTrack::TRACK_MODE_LOOP);
							m_animNode->addTrack(*quatRotTrack);

                            /*// time 0
                            KeyFrameSmooth<Quaternion> *keyRot = new KeyFrameSmooth<Quaternion>(
                                0,
                                Quaternion());

                            quatRotTrack->addKeyFrame(*keyRot);*/
						}

						KeyFrameSmooth<Quaternion> *keyRot = new KeyFrameSmooth<Quaternion>(
							channel.inputSrc->data[i]*invDuration,
							quat);

						quatRotTrack->addKeyFrame(*keyRot);
					}
					
                    // translation
                    if (mat.getTranslation().squareLength())
                    {
                        // add the translation track
                        if (!linearPosTrack)
                        {
                            linearPosTrack = new AnimationTrack_LinearVector(
                                    AnimationTrack::TARGET_OBJECT_POS,
                                    0,
                                    AnimationTrack::TRACK_MODE_LOOP,
                                    AnimationTrack::TRACK_MODE_LOOP);
                            m_animNode->addTrack(*linearPosTrack);

                            /*// time 0
                            KeyFrameLinear<Vector3> *keyPos = new KeyFrameLinear<Vector3>(
                                0,
                                Vector3());

                            linearPosTrack->addKeyFrame(*keyPos);*/
                        }

                        KeyFrameLinear<Vector3> *keyPos = new KeyFrameLinear<Vector3>(
                            channel.inputSrc->data[i]*invDuration,
                            mat.getTranslation());

                        linearPosTrack->addKeyFrame(*keyPos);
                    }
					
					// scale track
					// TODO
				}

			}
		}
		else if (channel.sampler->type == BEZIER)
		{
			if ((channel.target >= T_ROTATE) && (channel.target <= T_ROTATE_Z))
            {
                // Bezier is one channel per axis
                if ((numCh != 1) && (numCh != 3))
                    O3D_ERROR(E_InvalidFormat("Rotation track should have 1 or 3 components"));

                AnimationTrack::Target target = AnimationTrack::TARGET_OBJECT_ROT;
                switch (channel.target)
                {
                case T_ROTATE_X:
                    target = AnimationTrack::TARGET_OBJECT_ROT_X;
                    break;
                case T_ROTATE_Y:
                    target = AnimationTrack::TARGET_OBJECT_ROT_Y;
                    break;
                case T_ROTATE_Z:
                    target = AnimationTrack::TARGET_OBJECT_ROT_Z;
                    break;
                default:
                    break;
                }

                AnimationTrack_BezierFloat *rotTrack = new AnimationTrack_BezierFloat(
                            target,
                            0,
                            AnimationTrack::TRACK_MODE_LOOP,
                            AnimationTrack::TRACK_MODE_LOOP);

                m_animNode->addTrack(*rotTrack);

                if (numCh == 1)
                {
                    // set the actual key info
                    for (UInt32 i = 0 ; i < numKeys; ++i)
                    {
                        Vector2f *left = new Vector2f(o3d::toRadian(channel.leftTangent->data[i*2]), o3d::toRadian(channel.leftTangent->data[i*2+1]));
                        Vector2f *right = new Vector2f(o3d::toRadian(channel.rightTangent->data[i*2]), o3d::toRadian(channel.leftTangent->data[i*2+1]));
                        Evaluator1D_Bezier *evaluator = new Evaluator1D_Bezier();

                        KeyFrameBezier<Float> *key = new KeyFrameBezier<Float>(
                                    channel.inputSrc->data[i]*invDuration,
                                    o3d::toRadian(channel.outputSrc->data[i]));

                        key->TangentLeft = left;
                        key->TangentRight = right;
                        key->Evaluator = evaluator;

                        rotTrack->addKeyFrame(*key);
                    }

                    rotTrack->initAllEvaluators();
                }
                else if (numCh == 3)
                {
                    O3D_ASSERT(0);
                }
            }
            else if (channel.target == T_TRANSLATE)
            {
                if (numCh != 3)
                    O3D_ERROR(E_InvalidFormat("Translation track should have 3 components"));

                AnimationTrack_BezierVector *posTrack = new AnimationTrack_BezierVector(
                            AnimationTrack::TARGET_OBJECT_POS,
                            0,
                            AnimationTrack::TRACK_MODE_LOOP,
                            AnimationTrack::TRACK_MODE_LOOP);

                m_animNode->addTrack(*posTrack);

				// set the actual key info
                for (UInt32 i = 0 ; i < numKeys; ++i)
				{
					// fill in all the keys for each anim key set
					UInt32 offset = i*numCh;

                    Vector3 vec(&channel.outputSrc->data[offset]);

                    KeyFrameLinear<Vector3> *key = new KeyFrameLinear<Vector3>(
						channel.inputSrc->data[i]*invDuration,
						vec);

                    posTrack->addKeyFrame(*key);
                }
			}
			else if (channel.target == T_SCALE)
			{
                if (numCh != 3)
                    O3D_ERROR(E_InvalidFormat("Scale track should have 3 components"));

                AnimationTrack_BezierVector *scaleTrack = new AnimationTrack_BezierVector(
                            AnimationTrack::TARGET_OBJECT_SCALE,
                            0,
                            AnimationTrack::TRACK_MODE_LOOP,
                            AnimationTrack::TRACK_MODE_LOOP);

                m_animNode->addTrack(*scaleTrack);

				// set the actual key info
                for (UInt32 i = 0 ; i < numKeys; ++i)
				{
					// fill in all the keys for each anim key set
					UInt32 offset = i*numCh;

                    Vector3 vec(&channel.outputSrc->data[offset]);

                    KeyFrameLinear<Vector3> *key = new KeyFrameLinear<Vector3>(
						channel.inputSrc->data[i]*invDuration,
						vec);

                    scaleTrack->addKeyFrame(*key);
                }
			}
		}
		else if (channel.sampler->type == TCB)
		{
			O3D_ASSERT(0);
		}
	}
}

// Find a rotation key frame at a specified time.
KeyFrameSmooth<Quaternion>* CAnimation::findRotationKeyFrame(
		AnimationTrack_SmoothQuaternion *track,
		Float time)
{
    O3D_ASSERT(track != nullptr);

	T_KeyFrameList &keyFrameList = track->getKeyFrameList();
	for (IT_KeyFrameList it = keyFrameList.begin(); it != keyFrameList.end(); ++it)
	{
		if ((*it)->getTime() == time)
			return (KeyFrameSmooth<Quaternion>*)(*it);
	}

    return nullptr;
}

