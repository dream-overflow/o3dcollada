/**
 * @file node.h
 * @brief Collada node importer/expoter.
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2008-04-29
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#ifndef _O3D_COLLADA_NODE_H
#define _O3D_COLLADA_NODE_H

#include "global.h"

#include <o3d/core/vector3.h>
#include <o3d/core/quaternion.h>

namespace o3d {

class Bones;
class AnimationNode;
class Skeleton;

namespace collada {

class CGeometry;
class CCamera;
class CLight;
class CController;
class CAnimation;

//---------------------------------------------------------------------------------------
//! @class CNode
//-------------------------------------------------------------------------------------
//! COLLADA visual scene node object. It mean the <node>
//---------------------------------------------------------------------------------------
class CNode : public CBaseObject
{
public:

	//! Default ctor.
	CNode(
		o3d::Scene *scene,
		domCOLLADA *dom,
		ColladaInfo &infos,
		const domNodeRef node);

	//! Destructor
	virtual ~CNode();

	//! Import method
	virtual Bool import();

	//! Export method
	virtual Bool doExport();

	//! Set post-import values to the scene
	virtual Bool toScene();

	//! Set pre-export values from the scene
	virtual Bool fromScene();

	//! Apply skeleton to skinning
	Bool postImportPass();

	//! Set the scene node
	inline void setParentNode(Node *node) { m_parentNode = node; }

    //! Set the skeleton object
    inline void setSkeleton(Skeleton *skeleton) { m_skeleton = skeleton; }

	//! Return the scene node
	inline o3d::Node* getSceneNode() const { return m_node; }

	//! add an animation object
	void addAnimation(CAnimation *anim);

	//! Is an animation root is contained into this node
	Bool isAnimationRoot() const;

    //! is the node a join (bone)
    Bool isJoin() const;

	//! Get the animation node or null
	o3d::AnimationNode* getAnimationNode() const { return m_animNode; }

protected:

	o3d::Node *m_parentNode;
	o3d::Node *m_node;

    o3d::Bones *m_join;
    o3d::Skeleton *m_skeleton;

	const domNodeRef m_domNode;

	Matrix4 m_matrix;

	typedef std::list<CNode*> T_ChildNodeList;
	typedef T_ChildNodeList::iterator IT_ChildNodeList;
	T_ChildNodeList m_childNodes;

    typedef std::list<CCamera*> T_CameraList;
	typedef T_CameraList::iterator IT_CameraList;
    T_CameraList m_cameraList;

	typedef std::list<CLight*> T_LightList;
	typedef T_LightList::iterator IT_LightList;
    T_LightList m_lightList;

	typedef std::list<CGeometry*> T_GeometryList;
	typedef T_GeometryList::iterator IT_GeometryList;
	T_GeometryList m_geometryList;

	typedef std::list<CController*> T_ControllerList;
	typedef T_ControllerList::iterator IT_ControllerList;
	T_ControllerList m_controllerList;

	CNode *m_father;

	typedef std::vector<CBaseObject*>::iterator IT_AnimationList;
	std::vector<CBaseObject*> m_animations;

	o3d::AnimationNode *m_animNode;
};

} // namespace collada
} // namespace o3d

#endif // _O3D_COLLADA_NODE_H

