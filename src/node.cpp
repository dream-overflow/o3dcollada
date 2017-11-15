/**
 * @file node.cpp
 * @brief Implementation of Node.h
 * @author Frederic SCHERMA (frederic.scherma@dreamoverflow.org)
 * @date 2008-04-29
 * @copyright Copyright (c) 2001-2017 Dream Overflow. All rights reserved.
 * @details 
 */

#include "o3d/collada/precompiled.h"
#include "o3d/collada/node.h"

#include <o3d/engine/scene/scene.h>
#include <o3d/engine/hierarchy/hierarchytree.h>
#include <o3d/engine/object/bones.h>
#include <o3d/engine/object/pointgizmo.h>
#include <o3d/engine/object/mtransform.h>
#include <o3d/engine/scene/sceneobjectmanager.h>
#include <o3d/engine/animation/animationnode.h>

#include "o3d/collada/material.h"
#include "o3d/collada/geometry.h"
#include "o3d/collada/camera.h"
#include "o3d/collada/light.h"
#include "o3d/collada/controller.h"
#include "o3d/collada/animation.h"

using namespace o3d;
using namespace o3d::collada;

// Default ctor.
CNode::CNode(
	o3d::Scene *pScene,
	domCOLLADA *pDom,
	ColladaInfo &infos,
	const domNodeRef node) :
		CBaseObject(pScene,pDom,infos),
        m_parentNode(nullptr),
        m_node(nullptr),
        m_join(nullptr),
		m_domNode(node),
        m_father(nullptr),
        m_animNode(nullptr)
{
}

// Destructor
CNode::~CNode()
{
	for (IT_ChildNodeList it = m_childNodes.begin(); it != m_childNodes.end(); ++it)
		deletePtr(*it);
}

// Import method
Bool CNode::import()
{
	m_StrId = m_domNode->getSid() ? m_domNode->getSid() : "";
	m_name = m_domNode->getName() ? m_domNode->getName() : "";
	m_id = m_domNode->getId() ? m_domNode->getId() : "";

	// for each content
	daeElementRefArray &contentArray = m_domNode->getContents();
	for (size_t i = 0; i < contentArray.getCount(); ++i)
	{
		// get the component type string
		String typeName = contentArray[i]->getTypeName() ? contentArray[i]->getTypeName() : "";

		// rotate
		if (typeName == "rotate")
		{
			domRotateRef rot = (domRotate*)(domElement*)contentArray[i];

			Matrix4 m;

			m.setRotation(
				Vector3(
					(Float)rot->getValue().get(0),
					(Float)rot->getValue().get(1),
					(Float)rot->getValue().get(2)),
				o3d::toRadian((Float)rot->getValue().get(3)));

			m_matrix *= m;
		}
		// translate
		else if (typeName == "translate")
		{
			domTranslateRef tr = (domTranslate*)(domElement*)contentArray[i];

			Matrix4 m;
			m.translate(
				Vector3(
					(Float)tr->getValue().get(0),
					(Float)tr->getValue().get(1),
					(Float)tr->getValue().get(2)));

			m_matrix *= m;
		}
		// scale
		else if (typeName == "scale")
		{
			domScaleRef scale = (domScale*)(domElement*)contentArray[i];

			Matrix4 m;
			m.scale(
				Vector3(
					(Float)scale->getValue().get(0),
					(Float)scale->getValue().get(1),
					(Float)scale->getValue().get(2)));

			m_matrix *= m;
		}
		// matrix
		else if (typeName == "matrix")
		{
			domMatrixRef mat = (domMatrix*)(domElement*)contentArray[i];

			Matrix4 m(
				(Float)mat->getValue().get(0),
				(Float)mat->getValue().get(1),
				(Float)mat->getValue().get(2),
				(Float)mat->getValue().get(3),

				(Float)mat->getValue().get(4),
				(Float)mat->getValue().get(5),
				(Float)mat->getValue().get(6),
				(Float)mat->getValue().get(7),

				(Float)mat->getValue().get(8),
				(Float)mat->getValue().get(9),
				(Float)mat->getValue().get(10),
				(Float)mat->getValue().get(11),

				(Float)mat->getValue().get(12),
				(Float)mat->getValue().get(13),
				(Float)mat->getValue().get(14),
				(Float)mat->getValue().get(15));

			m_matrix = m * m_matrix;//m_Matrix *= m;
		}
		// lookat
		else if (typeName == "lookat")
		{
			domLookatRef lookat = (domLookat*)(domElement*)contentArray[i];

			Matrix4 m;
			m.setLookAt(
				Vector3(
					(Float)lookat->getValue().get(0),
					(Float)lookat->getValue().get(1),
					(Float)lookat->getValue().get(2)),
				Vector3(
					(Float)lookat->getValue().get(3),
					(Float)lookat->getValue().get(4),
					(Float)lookat->getValue().get(5)),
				Vector3(
					(Float)lookat->getValue().get(6),
					(Float)lookat->getValue().get(7),
					(Float)lookat->getValue().get(8)));

			m_matrix *= m;
		}
		// skew
		else if (typeName == "skew")
		{
			domSkewRef skew = (domSkew*)(domElement*)contentArray[i];

			Matrix4 m;
			m.setSkew(
				o3d::toRadian((Float)skew->getValue().get(0)),
				Vector3(
					(Float)skew->getValue().get(1),
					(Float)skew->getValue().get(2),
					(Float)skew->getValue().get(3)),
				Vector3(
					(Float)skew->getValue().get(4),
					(Float)skew->getValue().get(5),
					(Float)skew->getValue().get(6)));

			m_matrix *= m;
		}
	}

	// import cameras
	/*const domInstance_camera_Array &cameraArray = m_Node->getInstance_camera_array();
	for (size_t i = 0; i < cameraArray.getCount(); ++i)
	{
        CCamera *pCamera = new CCamera(m_scene, &m_dom, cameraArray[i]);
		if (pCamera->import())
			m_CameraList.push_back(pCamera);
		else
		{
			deletePtr(pCamera);
			return False;
		}
	}*/

	// import lights
	/*const domInstance_light_Array &lightArray = m_Node->getInstance_light_array();
	for (size_t i = 0; i < lightArray.getCount(); ++i)
	{
        CLight *pLight = new CLight(m_scene,&m_Dom, lightArray[i]);
		if (pLight->import())
			m_LightList.push_back(pLight);
		else
		{
			deletePtr(pLight);
			return False;
		}
	}*/

	// import geometry instance
	const domInstance_geometry_Array &geoArray = m_domNode->getInstance_geometry_array();
	for (size_t i = 0; i < geoArray.getCount(); ++i)
	{
		const domBind_materialRef material = geoArray[i]->getBind_material();
		daeElement *geoElt = geoArray[i]->getUrl().getElement();

		CGeometry *geometry = new CGeometry(
			m_scene,
			&m_dom,
			m_infos,
			domGeometryRef((domGeometry*)geoElt),
			material);

		if (geometry->import())
			m_geometryList.push_back(geometry);
		else
		{
			deletePtr(geometry);
			return False;
		}
	}

	// import controller instance
	const domInstance_controller_Array &ctrlArray = m_domNode->getInstance_controller_array();
	for (size_t i = 0; i < ctrlArray.getCount(); ++i)
	{
		const domBind_materialRef material = ctrlArray[i]->getBind_material();
		daeElement *ctrlElt = ctrlArray[i]->getUrl().getElement();

		CController *controller = new CController(
			m_scene,
			&m_dom,
			m_infos,
			domControllerRef((domController*)ctrlElt),
			material);

        const domInstance_controller::domSkeleton_Array &skeleton_array = ctrlArray[i]->getSkeleton_array();
        //if (skeleton_array.getCount() > 1)
        //	O3D_ERROR(E_InvalidFormat("There are more than one skeleton, RT is not supported yet\n"));

        for (size_t j = 0; j < skeleton_array.getCount(); ++j)
		{
            domNode *node = (domNode*)(domElement*)skeleton_array[j]->getValue().getElement();

            // skeleton node, is root node of the skeleton
            if (node)
                controller->addSkeletonNode(node->getID() ? node->getID() : "");
		}

		if (controller->import())
			m_controllerList.push_back(controller);
		else
		{
			deletePtr(controller);
			return False;
		}
	}

	// import recursively each son
	domNode_Array &nodeArray = m_domNode->getNode_array();
	for (size_t i = 0; i < nodeArray.getCount(); ++i)
	{
		CNode *node = new CNode(m_scene, &m_dom, m_infos, nodeArray.get(i));
		node->m_father = this;

		if (node->import())
		{
			m_childNodes.push_back(node);
		}
		else
		{
			deletePtr(node);
			return False;
		}
	}

	m_infos.addNode(this);

	return True;
}

// Export method
Bool CNode::doExport()
{
	O3D_ASSERT(0);

	return True;
}

// Set post-import values to the scene
Bool CNode::toScene()
{
	if (m_domNode->getType() == NODETYPE_JOINT)
        if (m_parentNode)
            m_node = new Bones(m_parentNode);
        else
            m_node = new Bones(m_scene);
	else
        m_node = new Node(m_parentNode);

	if (m_domNode->getContents().getCount())
	{
		MTransform *transform = new MTransform;
		m_node->addTransform(transform);
		transform->setMatrix(m_matrix);
	}

	m_node->setName(m_name);

    if (m_parentNode)
        m_parentNode->addSonLast(m_node);

	if ((m_geometryList.size() +
		(m_join ? 1 : 0) +
        /*m_cameraList.size() +
        m_lightList.size() +*/
		m_controllerList.size()) > 1)
	{
		O3D_ERROR(E_InvalidFormat("Only one object by node is allowed"));
	}

    for (IT_CameraList it = m_cameraList.begin(); it != m_cameraList.end(); ++it)
	{
        //(*it)->setNode(m_node);
		if (!(*it)->toScene())
			return False;
	}

	for (IT_LightList it = m_lightList.begin(); it != m_lightList.end(); ++it)
	{
        //(*it)->setNode(m_node);
		if (!(*it)->toScene())
			return False;
    }

	for (IT_GeometryList it = m_geometryList.begin(); it != m_geometryList.end(); ++it)
	{
		(*it)->setNode(m_node);

		if (!(*it)->toScene())
			return False;
	}

	for (IT_ControllerList it = m_controllerList.begin(); it != m_controllerList.end(); ++it)
	{
        CController *controller = *it;
        controller->setNode(m_node);

        if (!controller->toScene())
			return False;
	}

	// update the subtree (useless... let the line if transform problem appear a day...)
	m_node->update();

	// animations
	if (m_animations.size())
	{
        o3d::AnimationNode *animFatherNode = nullptr;

		if (m_father)
		{
			animFatherNode = m_father->m_animNode;
			
			if (!animFatherNode)
			{
				CNode *cnode = m_father;
				std::vector<CNode*> missingAnimNode;

				// climb to the grand father
				while (cnode && !cnode->m_animNode)
				{
					missingAnimNode.push_back(cnode);
					cnode = cnode->m_father;
				}

				o3d::AnimationNode *parentAnimNode;

				// create the missing animation node hierarchy
				for (size_t i = 0; i < missingAnimNode.size(); ++i)
				{
                    parentAnimNode = missingAnimNode[i]->m_father ? missingAnimNode[i]->m_father->m_animNode : nullptr;
					missingAnimNode[i]->m_animNode = new o3d::AnimationNode(parentAnimNode);

					if (parentAnimNode)
						parentAnimNode->addSon(*missingAnimNode[i]->m_animNode);
				}

				animFatherNode = m_father->m_animNode;
			}
		}

		m_animNode = new o3d::AnimationNode(animFatherNode);

		if (animFatherNode)
			animFatherNode->addSon(*m_animNode);

		for (IT_AnimationList it = m_animations.begin(); it != m_animations.end(); ++it)
		{
			((CAnimation*)(*it))->setAnimationNode(m_animNode);

			if (!((CAnimation*)(*it))->toScene())
				return False;
		}
	}

	for (IT_ChildNodeList it = m_childNodes.begin(); it != m_childNodes.end(); ++it)
	{
		(*it)->setParentNode(m_node);

		if (!(*it)->toScene())
			return False;
	}

	return True;
}

// Apply skeleton to skinning
Bool CNode::postImportPass()
{
	for (IT_ControllerList it = m_controllerList.begin(); it != m_controllerList.end(); ++it)
	{
		if (!(*it)->postImportPass())
			return False;
	}

	for (IT_ChildNodeList it = m_childNodes.begin(); it != m_childNodes.end(); ++it)
	{
		if (!(*it)->postImportPass())
			return False;
	}

	return True;
}

// Set pre-export values from the scene
Bool CNode::fromScene()
{
	O3D_ASSERT(0);

	return True;
}

// Set the animation object if it exists
void CNode::addAnimation(CAnimation *pAnim)
{
	m_animations.push_back(pAnim);
}

Bool CNode::isAnimationRoot() const
{
    return ((m_animNode != nullptr) && m_animNode->getFather() == nullptr);
}

Bool CNode::isJoin() const
{
    return m_domNode->getType() == NODETYPE_JOINT;
}

